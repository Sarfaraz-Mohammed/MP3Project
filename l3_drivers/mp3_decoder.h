#pragma once

#include "FreeRTOS.h"
#include "cli_handlers.h"
#include "clock.h"
#include "common_macros.h"
#include "delay.h"
#include "gpio.h"
#include "gpio_lab.h"
#include "lpc40xx.h"
#include "lpc_peripherals.h"
#include "queue.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

/********************************************

    ADAFRUIT VS1053 CODEC V4 MP3 DECODER

**********************************************/

#define SCI_MODE 0x00        // Mode Control
#define SCI_STATUS 0x01      // Status
#define SCI_BASS 0x02        // Bass Treble Control
#define SCI_CLOCKF 0x03      // Clock Frequency and Multiplier
#define SCI_DECODE_TIME 0x04 // Decoding Time
#define SCI_AUDATA 0x05      // Audio Data
#define SCI_WRAM 0x06        // Write Data RAM
#define SCI_WRAMADDR 0x07    // RAM Address
#define SCI_HDAT0 0x08       // Data Header 0
#define SCI_HDAT1 0x09       // Data Header 1
#define SCI_AIADDR 0x0A      // Application Address Start
#define SCI_VOL 0x0B         // Volume Control
#define SCI_AICTRL0 0x0C     // Control Register 0
#define SCI_AICTRL1 0x0D     // Control Register 1
#define SCI_AICTRL2 0x0E     // Control Register 2
#define SCI_AICTRL3 0x0F     // Control Register 3
#define MAX_VOLUME 0x0000    // Volume Max
#define MIN_VOLUME 0xFAFA    // Volume Min

bool get_DREQ_high();
void low_CS();
void high_CS();
void low_xdcs();
void high_xdcs();
void low_reset();
void high_reset();
void decoder_initialize();
void send_data_decoder(uint8_t data);
uint16_t read_decoder_register(uint16_t reg_addr);
void write_decoder_register(uint16_t reg_addr, uint8_t MSB, uint8_t LSB);
