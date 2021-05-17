#pragma once

#include "ff.h"
#include <stddef.h>
#include <stdio.h>
#include <string.h>

typedef char song_memory_t[128];

#if 0
static song_memory_t list_songs[32];
static size_t total_songs;
#endif

void song_list_display(void);
const char *song_list_name(size_t item);
void song_counter(void);
size_t num_songs(void);
