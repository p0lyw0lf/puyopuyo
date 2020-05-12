#ifndef RUNNER_H
#define RUNNER_H

#include <SDL.h>
#include <stdbool.h>
#include <stdio.h>
#include "puyo.h"
#include "ipc.h"

typedef enum runnerStates runner_state_t;
enum runnerStates {
  READY,
  POP_FLASHING,
  POP_FALLING,
};

typedef struct runnerData runnerData;
struct runnerData {
  puyo_board_t* board;
  runner_state_t state;
};

extern bool runner_setup(runnerData* data);
extern bool runner_loop(runnerData* data);

extern runnerData* runner_create(puyo_board_t* board);
extern void runner_destroy(runnerData* data);

#endif //GAMERUNNER_H
