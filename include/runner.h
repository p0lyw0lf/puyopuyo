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

// Actually expect runnerData*, need to be void* to supress gcc warning
extern bool runner_setup(void* data);
extern bool runner_loop(void* data);

extern runnerData* runner_create(puyo_board_t* board);
extern void runner_destroy(void* data);

#endif //GAMERUNNER_H
