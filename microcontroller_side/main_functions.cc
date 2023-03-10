/* Copyright 2022 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#include "main_functions.h"
#include "detection_responder.h"
#include "model_settings.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "person_detect_model_data.h"
#include "tensorflow/lite/micro/system_setup.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "image_provider.h"
#include "sl_simple_led.h"
#include "sl_simple_led_instances.h"
#include "sl_iostream_handles.h"
#include <stdio.h>

// Globals, used for compatibility with Arduino-style sketches.
namespace {
const tflite::Model* model = nullptr;
tflite::ErrorReporter* error_reporter = nullptr;
tflite::MicroInterpreter* interpreter = nullptr;
TfLiteTensor* input = nullptr;

// In order to use optimized tensorflow lite kernels, a signed int8_t quantized
// model is preferred over the legacy unsigned model format. This means that
// throughout this project, input images must be converted from unisgned to
// signed format. The easiest and quickest way to convert from unsigned to
// signed 8-bit integers is to subtract 128 from the unsigned value to get a
// signed value.

// An area of memory to use for input, output, and intermediate arrays.
constexpr int kTensorArenaSize = 0x18000;
static uint8_t tensor_arena[kTensorArenaSize];
}  // namespace

// The name of this function is important for Arduino compatibility.
void setup() {
  tflite::InitializeTarget();
  static tflite::MicroErrorReporter micro_error_reporter;
  error_reporter = &micro_error_reporter;
  // Map the model into a usable data structure. This doesn't involve any
  // copying or parsing, it's a very lightweight operation.
  model = tflite::GetModel(g_person_detect_model_data);
  if (model->version() != TFLITE_SCHEMA_VERSION) {
    printf("Model provided is schema version %ld not equal "
            "to supported version %d.",
            model->version(), TFLITE_SCHEMA_VERSION);
    return;
  }
  // Pull in only the operation implementations we need.
  // This relies on a complete list of all the ops needed by this graph.
  // An easier approach is to just use the AllOpsResolver, but this will
  // incur some penalty in code space for op implementations that are not
  // needed by this graph.
  //
  // tflite::AllOpsResolver resolver;
  // NOLINTNEXTLINE(runtime-global-variables)
  static tflite::MicroMutableOpResolver<5> micro_op_resolver(error_reporter);
  if (micro_op_resolver.AddAveragePool2D() != kTfLiteOk)
  {
    return;
  }
  if (micro_op_resolver.AddConv2D() != kTfLiteOk) {
    return;
  }
  if (micro_op_resolver.AddDepthwiseConv2D() != kTfLiteOk)
  {
    return;
  }
  if (micro_op_resolver.AddReshape() != kTfLiteOk)
  {
    return;
  }
  if (micro_op_resolver.AddSoftmax() != kTfLiteOk)
  {
    return;
  }

  // Build an interpreter to run the model with.
  // NOLINTNEXTLINE(runtime-global-variables)
  static tflite::MicroInterpreter static_interpreter(
      model, micro_op_resolver, tensor_arena, kTensorArenaSize, error_reporter);
  interpreter = &static_interpreter;
  // Allocate memory from the tensor_arena for the model's tensors.
  TfLiteStatus allocate_status = interpreter->AllocateTensors();
  if (allocate_status != kTfLiteOk)
  {
    printf("AllocateTensors() failed");
    return;
  }
  // Get information about the memory area to use for the model's input.
  input = interpreter->input(0);
}

// The name of this function is important for Arduino compatibility.
void loop()
{
  TfLiteStatus tf_status  = kTfLiteOk;

  // Get image from provider.
  tf_status = GetImage(input->data.int8);
  if (kTfLiteOk != tf_status) {
    printf("Failed to read.\r\n");
  }
  // Run the model on this input and make sure it succeeds.
  tf_status = interpreter->Invoke();
  if (kTfLiteOk != tf_status) {
    printf("Invoke failed.\r\n");
  }

  TfLiteTensor* output = interpreter->output(0);

  // Process the inference results.
  int8_t person_score = output->data.int8[kPersonIndex];
  int8_t no_person_score = output->data.int8[kNotAPersonIndex];
  RespondToDetection(person_score, no_person_score);
}
