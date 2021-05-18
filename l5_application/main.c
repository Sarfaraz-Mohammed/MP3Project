
// ------------MP3 Project ------------------------

#include "FreeRTOS.h"
#include "acceleration.h"
#include "adc.h"
#include "board_io.h"
#include "cli_handlers.h"
#include "common_macros.h"
#include "delay.h"
#include "event_groups.h"
#include "ff.h"
#include "gpio.h"
#include "gpio_isr.h"
#include "gpio_lab.h"
#include "i2c.h"
#include "i2c_slave_functions.h"
#include "i2c_slave_init.h"
#include "lpc40xx.h"
#include "lpc_peripherals.h"
#include "mp3_decoder.h"
#include "mp3_lcd.h"
#include "periodic_scheduler.h"
#include "queue.h"
#include "semphr.h"
#include "sj2_cli.h"
#include "song_display.h"
#include "ssp2.h"
#include "ssp2_lab.h"
#include "task.h"
#include "uart.h"
#include "uart_lab.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

QueueHandle_t Q_trackname;
QueueHandle_t Q_songdata;
typedef char trackname_t[64];
typedef char songdata_t[512];

TaskHandle_t player_handle;

SemaphoreHandle_t next_song;
SemaphoreHandle_t previous_song;
SemaphoreHandle_t play_pause;
SemaphoreHandle_t choice;

static void interrupts();
static void next_song_ISR();
static void previous_song_ISR();
static void play_pause_ISR();
static void play_pause_Button();

typedef struct {
  char Tag[3];
  char Title[30];
  char Artist[30];
  char Album[30];
  char Year[4];
  uint8_t genre;
} mp3_meta;

void print_metadata(char *meta) {
  uint8_t title_counter = 0;
  uint8_t artist_counter = 0;
  uint8_t album_counter = 0;
  uint8_t year_counter = 0;
  mp3_meta song_meta = {0};
  for (int i = 0; i < 128; i++) {
    if ((((int)(meta[i]) > 47) && ((int)(meta[i]) < 58)) || (((int)(meta[i]) > 64) && ((int)(meta[i]) < 91)) ||
        (((int)(meta[i]) > 96) && ((int)(meta[i]) < 123)) || ((int)(meta[i])) == 32) {
      char character = (int)(meta[i]);
      if (i < 3) {
        song_meta.Tag[i] = character; // Tag
      } else if (i > 2 && i < 33) {
        song_meta.Title[title_counter++] = character; // Song Name
      } else if (i > 32 && i < 63) {
        song_meta.Artist[artist_counter++] = character; // Artist
      } else if (i > 62 && i < 93) {
        song_meta.Album[album_counter++] = character; // Album
      } else if (i > 92 && i < 97) {
        song_meta.Year[year_counter++] = character; // Year
      }
    }
  }

  lcd_print(song_meta.Title, page_0, init);
  lcd_print(song_meta.Artist, page_3, 0);
  lcd_print(song_meta.Album, page_5, 0);
  // lcd_print(song_meta.Year, page_7, 0);
}

static void mp3_reader_task(void *p) {
  trackname_t song_name;
  songdata_t byte512;
  UINT br;

  while (1) {
    xQueueReceive(Q_trackname, song_name, portMAX_DELAY);

    lcd_print(song_name, page_0, init);
    horizontal_addressing(page_0, page_0);

    const char *file_name = song_name;
    FIL file;
    FRESULT result = f_open(&file, file_name, (FA_READ));

    static char meta_128[128];

    f_lseek(&file, f_size(&file) - (sizeof(char) * 128));
    f_read(&file, meta_128, sizeof(meta_128), &br);
    print_metadata(meta_128);
    f_lseek(&file, 0);

    if (FR_OK == result) {
      f_read(&file, byte512, sizeof(byte512), &br);
      while (br != 0) {
        f_read(&file, byte512, sizeof(byte512), &br);
        xQueueSend(Q_songdata, byte512, portMAX_DELAY);
        if (uxQueueMessagesWaiting(Q_trackname)) {
          printf("New song request\n");
          break;
        }
      }
      f_close(&file);
    } else if (br == 0) {
      xSemaphoreGive(choice);
      xSemaphoreGive(next_song);
      printf("Read byte is 0\n");
    } else {
      printf("Failed to open mp3 object_file \n");
    }
  }
}

static void mp3_player_task(void *p) {
  int counter = 1;
  while (1) {
    songdata_t byte512;
    xQueueReceive(Q_songdata, byte512, portMAX_DELAY);
    for (int i = 0; i < 512; i++) {
      while (!get_DREQ_high()) {
        vTaskDelay(1);
      }
      send_data_decoder(byte512[i]);
    }
    // printf("Transmitting: %d (times)\n", counter);
    counter++;
  }
}

static void mp3_song_controller_task(void *p) {
  volatile size_t index = 0;
  song_list_display();
  while (1) {
    if (xSemaphoreTake(choice, portMAX_DELAY)) {
      if (xSemaphoreTake(next_song, 10)) {
        if (index >= num_songs()) {
          index = 0;
        }
        xQueueSend(Q_trackname, song_list_name(index), portMAX_DELAY);
        index++;
      } else if (xSemaphoreTake(previous_song, 10)) {
        if (index == 0) {
          index = num_songs();
        }
        xQueueSend(Q_trackname, song_list_name(index), portMAX_DELAY);
        index--;
      }
    }
    vTaskDelay(100);
  }
}

int main(void) {
  // Queues Create
  Q_trackname = xQueueCreate(1, sizeof(trackname_t));
  Q_songdata = xQueueCreate(1, sizeof(songdata_t));
  next_song = xSemaphoreCreateBinary();
  previous_song = xSemaphoreCreateBinary();
  play_pause = xSemaphoreCreateBinary();
  choice = xSemaphoreCreateBinary();

  // Inits
  interrupts();
  decoder_initialize();
  sj2_cli__init();
  song_list_display();
  song_counter();

  // Task Create
  xTaskCreate(mp3_reader_task, "reader", (2048 * 4) / sizeof(void *), NULL, 1, NULL);
  xTaskCreate(mp3_player_task, "player", (2048 * 4) / sizeof(void *), NULL, 1, &player_handle);
  xTaskCreate(mp3_song_controller_task, "song_controller", (2048 * 4) / sizeof(void *), NULL, 1, NULL);
  xTaskCreate(play_pause_Button, "play_pause", (2048) / sizeof(void *), NULL, 1, NULL);
  vTaskStartScheduler();
  return 0;
}

static void interrupts() {
  LPC_IOCON->P0_25 &= ~(0b111);
  gpio0__set_as_input(25);
  lpc_peripheral__enable_interrupt(LPC_PERIPHERAL__GPIO, gpio0__interrupt_dispatcher, "INTR Port 0");
  gpio0__attach_interrupt(29, GPIO_INTR__FALLING_EDGE, play_pause_ISR);
  gpio0__attach_interrupt(30, GPIO_INTR__FALLING_EDGE, next_song_ISR);
  gpio0__attach_interrupt(25, GPIO_INTR__RISING_EDGE, previous_song_ISR);
}

static void next_song_ISR() {
  xSemaphoreGiveFromISR(choice, NULL);
  xSemaphoreGiveFromISR(next_song, NULL);
}

static void play_pause_ISR() { xSemaphoreGiveFromISR(play_pause, NULL); }

static void previous_song_ISR() {
  xSemaphoreGiveFromISR(choice, NULL);
  xSemaphoreGiveFromISR(previous_song, NULL);
}

volatile bool pause = true;

static void play_pause_Button() {
  while (1) {
    if (xSemaphoreTake(play_pause, portMAX_DELAY)) {
      if (pause) {
        vTaskSuspend(player_handle);
        vTaskDelay(300);
        pause = false;
      } else {
        vTaskResume(player_handle);
        vTaskDelay(300);
        pause = true;
      }
    }
  }
}
