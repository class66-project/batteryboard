/* Copyright (C) 2024 Richard Franks - All Rights Reserved
 *
 * You may use, distribute and modify this code under the
 * terms of the Apache 2.0 license.
 *
 * See LICENSE for details
 */


#include <Arduino.h>
#include "statusled.h"
#include "definitions.h"

void flashStatusLED() {
  digitalWrite(STATUS_LED, 1);
  delay(100);
  digitalWrite(STATUS_LED, 0);
  delay(50);
}


/**
 * Function to display an error on the status LED.
 *
 * This function takes in an error code (defined as a uint_8t[10])
 * and then displays it non-stop.
 *
 * @param error a uint_8t[10] pointer of the pattern to display
 */
void error_led(uint8_t *error) {
  int i;
  while (1) {
    for (i=0; i< 10; i++) {
      digitalWrite(STATUS_LED, error[i]);
      delay(100);
    }
  }
}
