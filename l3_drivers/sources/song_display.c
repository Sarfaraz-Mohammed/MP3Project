#include "song_display.h"

static song_memory_t list_songs[32];
static size_t total_songs;

// copy song files names
static void song_name_file(const char *file) {
  if (NULL != strstr(file, ".mp3")) {
    strncpy(list_songs[total_songs], file, sizeof(song_memory_t) - 1);
    ++total_songs;
  }
}

const char *song_list_name(size_t item) {
  const char *rtn_ptr = "";

  if (item >= total_songs) {
    rtn_ptr = "";
  } else {
    rtn_ptr = list_songs[item];
  }

  return rtn_ptr;
}

// display songs as a list on LCD screen by reading file
void song_list_display(void) {
  FRESULT result;
  static FILINFO file_info;
  const char *path = "/";

  DIR dir;
  result = f_opendir(&dir, path);

  if (result == FR_OK) {
    for (;;) {
      result = f_readdir(&dir, &file_info);
      if (result != FR_OK || file_info.fname[0] == 0) {
        break;
      }
      if (file_info.fattrib & AM_DIR) {
      } else {
        song_name_file(file_info.fname);
      }
    }
    f_closedir(&dir);
  }
}

// count songs for list
void song_counter(void) {
  for (size_t song_number = 0; song_number < num_songs(); song_number++) {
    printf("Song %2d: %s\n", (1 + song_number), song_list_name(song_number));
  }
}

// return total number of songs
size_t num_songs(void) { return total_songs; }
