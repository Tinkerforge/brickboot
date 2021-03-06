/* master-brick
 * Copyright (C) 2010 Olaf Lüke <olaf@tinkerforge.com>
 *
 * config.h: Master-Brick specific configuration
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef CONFIG_H
#define CONFIG_H

#include "bricklib/drivers/board/sam3s/SAM3S.h"

//#define at91sam3s4c


// ************** BRICK SETTINGS **************

// Frequencies
#define BOARD_MCK      48000000 // Frequency of brick
#define BOARD_MAINOSC  16000000 // Frequency of oscillator
#define BOARD_ADC_FREQ  6000000 // Frequency of ADC
#define BOARD_OSC_EXTERNAL      // Use external oscillator


// LED
#ifdef sam3s2
#define PIN_LED_RED   {1 << 10, PIOA, ID_PIOA, PIO_OUTPUT_0, PIO_DEFAULT}
#define PIN_LED_BLUE  {1 << 9, PIOA, ID_PIOA, PIO_OUTPUT_0, PIO_DEFAULT}
#else
#ifdef sam3s4
#define PIN_LED_RED   {1 << 19, PIOC, ID_PIOC, PIO_OUTPUT_0, PIO_DEFAULT}
#define PIN_LED_BLUE  {1 << 18, PIOC, ID_PIOC, PIO_OUTPUT_0, PIO_DEFAULT}
#endif
#endif

#define PINS_LEDS     PIN_LED_RED, PIN_LED_BLUE

#endif
