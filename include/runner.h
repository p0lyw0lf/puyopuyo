#ifndef RUNNER_H
#define RUNNER_H

#include <SDL.h>
#include <stdbool.h>
#include <stdio.h>
#include "puyo.h"
#include "ipc.h"

typedef struct {
  SDL_mutex* mutex;
  puyo_board_t* board;
  bool quit;
} runnerData;

extern int runner_mainloop(void* data); // expects a pointer to runnerData, but SDL needs this format for threading function

extern runnerData* runner_create(puyo_board_t* board);
extern int runner_stop_thread(SDL_Thread* thread, runnerData* data);
extern void runner_destroy(runnerData* data);

#endif //GAMERUNNER_H
