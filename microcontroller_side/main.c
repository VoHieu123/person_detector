/***************************************************************************//**
 * @file main.c
 * @brief main() function.
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/
#include "sl_system_init.h"
#include "sl_system_process_action.h"
#include "main_functions.h"

int main(void)
{
  // Initialize Silicon Labs device, system, service(s) and protocol stack(s).
  sl_system_init();

  setup();

  while (1)
  {
    // Silicon Labs components process action routine.
    sl_system_process_action();

    loop();
  }
}
