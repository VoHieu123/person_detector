/* Copyright 2019 The TensorFlow Authors. All Rights Reserved.

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
#include "image_provider.h"
#include "model_settings.h"
#include <stdio.h>
#include <stdint.h>
#include "sl_iostream_handles.h"

#define COLOR_IMAGE_SIZE 27648u
#define WIDTH            96
#define HEIGHT           96
#define CHANNELS         1u
#define GREY_IMAGE_SIZE  (WIDTH * CHANNELS * HEIGHT)

TfLiteStatus GetImage(int8_t *image_data)
{
  uint8_t      color_image[COLOR_IMAGE_SIZE] = {0};
  int8_t       gray_image[GREY_IMAGE_SIZE]   = {0};
  size_t       byte_read                     = 0;
  size_t       read_index                    = 0;
  sl_status_t  status                        = SL_STATUS_OK;

  /* Get color image from camera */
  do
  {
    status = sl_iostream_read(sl_iostream_vcom_handle,
                              color_image + read_index,
                              COLOR_IMAGE_SIZE,
                              &byte_read);
    if (status == SL_STATUS_INVALID_CONFIGURATION)
    {
       return kTfLiteError;
    }
    read_index += byte_read;
  } while (read_index < COLOR_IMAGE_SIZE);

  /* Convert color image to gray image */
  for (size_t loop_index = 0; loop_index < COLOR_IMAGE_SIZE; loop_index += 3)
  {
      gray_image[loop_index/3] = (color_image[loop_index]/4) +
                                 (color_image[loop_index + 1]/4) +
                                 (color_image[loop_index + 2]/4);
  }

  /* Convert uint8_t data to int8_t data */
  for (size_t loop_index = 0; loop_index < GREY_IMAGE_SIZE; loop_index++)
  {
      image_data[loop_index] = int8_t(gray_image[loop_index] - 0x80u);
  }

  return kTfLiteOk;
}
