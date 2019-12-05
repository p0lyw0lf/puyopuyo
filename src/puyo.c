#include "puyo.h"

const score_t PUYO_CHAIN_MULTIPLIERS[PUYO_MAX_CHAIN+1] = {
  0,
  1,   // 1
  4,   // 2
  10,  // 3
  20,  // 4
  40,  // 5
  1,   // 6
  1,   // 7
  1,   // 8
  1,   // 9
  1,   // 10
  1,   // 11
  1,   // 12
  1,   // 13
  1,   // 14
  1,   // 15
  1,   // 16
  1,   // 17
  1,   // 18
  1,   // 19
  1,   // 20
  1,   // 21
};


puyo_board_t* puyo_create_board() {
  srand(time(NULL));
  //printf("%ld\n", time(NULL));

  puyo_board_t* board = (puyo_board_t*)malloc(sizeof(puyo_board_t));
  board->mutex = SDL_CreateMutex();
  if (board->mutex == NULL) {
    fprintf(stderr, "Error: could not create mutex. SDL Error: %s\n", SDL_GetError());
    free(board);
    return NULL;
  }
  board->area = (puyo_t*)malloc(sizeof(puyo_t) * (PUYO_WIDTH*PUYO_HEIGHT_ACT));
  for (int x=0; x<PUYO_WIDTH; x++) {
    for (int y=0; y<PUYO_HEIGHT_ACT; y++) {
      board->area[x*PUYO_HEIGHT_ACT+y] = PUYO_COLOR_NONE;
    }
  }
  board->prev_area = NULL;
  board->score = 0;
  board->incoming_garbage = 0;
  board->chain = 0;

  board->current_pair = puyo_get_random_pair();
  board->current_rot_state = ROT_DOWN;
  board->current_x = 3;
  board->current_y = PUYO_HEIGHT;

  board->next_pair = puyo_get_random_pair();
  board->next_next_pair = puyo_get_random_pair();

  board->board_has_changed = true;
  board->pairs_have_changed = true;

  // Later: randomly initialize this
  board->color_to_sprite[PUYO_COLOR_1] = PUYO_RED;
  board->color_to_sprite[PUYO_COLOR_2] = PUYO_GREEN;
  board->color_to_sprite[PUYO_COLOR_3] = PUYO_BLUE;
  board->color_to_sprite[PUYO_COLOR_4] = PUYO_YELLOW;

  return board;
}

void puyo_free_board(puyo_board_t* board) {
  if (board != NULL) {
    if (board->area != NULL) {
      free(board->area);
      board->area = NULL;
    }
    if (board->prev_area != NULL) {
      free(board->prev_area);
      board->prev_area = NULL;
    }
    if (board->mutex != NULL) {
      SDL_DestroyMutex(board->mutex);
      board->mutex = NULL;
    }
    free(board);
    // Don't forget to set board to NULL on the outside too!
  }
}

puyo_t puyo_get_random() {
  return rand() % PUYO_NUM_COLORS;
}

puyo_pair_t puyo_get_random_pair() {
  puyo_pair_t pair;
  pair.top = puyo_get_random();
  pair.bot = puyo_get_random();
  return pair;
}

score_t puyo_pop_chain(puyo_board_t* board) {
  // TODO: change to a function that actually does something useful.
  for (int x=0; x<PUYO_WIDTH; x++) {
    for (int y=0; y<PUYO_HEIGHT_ACT; y++) {
      board->area[x*PUYO_HEIGHT_ACT+y] = puyo_get_random();
    }
  }

  board->next_pair = puyo_get_random_pair();
  board->next_next_pair = puyo_get_random_pair();
}
