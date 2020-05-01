#include "puyo.h"

const score_t PUYO_CHAIN_MULTIPLIERS[PUYO_MAX_CHAIN+1] = {
  0,     // 0
  4,     // 1
  20,    // 2
  24,    // 3
  32,    // 4
  48,    // 5
  96,    // 6
  160,   // 7
  240,   // 8
  320,   // 9
  480,   // 10
  600,   // 11
  700,   // 12
  800,   // 13
  900,   // 14
  999,   // 15
};


puyo_board_t* puyo_create_board() {
  srand(time(NULL));

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

  board->board_object = NULL;
  board->pairs_object = NULL;

  // Later: randomly initialize this
  board->color_to_sprite[PUYO_COLOR_1] = PUYO_RED;
  board->color_to_sprite[PUYO_COLOR_2] = PUYO_GREEN;
  board->color_to_sprite[PUYO_COLOR_3] = PUYO_BLUE;
  board->color_to_sprite[PUYO_COLOR_4] = PUYO_YELLOW;

  return board;
}

bool puyo_mark_board_changed(puyo_board_t* board) {
  // Assuming caller has locked mutex already. Checking for NULL just in case
  if (board == NULL) {
    return false;
  } 

  board->board_has_changed = true;

  if (board->board_object != NULL) {
    if (SDL_LockMutex(board->board_object->mutex) != 0) {
      fprintf(stderr, "Could not lock ACGL mutex in puyo_mark_board_changed! SDL_Error %s\n", SDL_GetError());
      return false;
    }
    board->board_object->needs_update = true;
    SDL_UnlockMutex(board->board_object->mutex);
  }

  return true;
}

bool puyo_mark_pairs_changed(puyo_board_t* board) {
  // Assuming caller has locked mutex already. Checking for NULL just in case
  if (board == NULL) {
    return false;
  } 

  board->pairs_have_changed = true;

  if (board->pairs_object != NULL) {
    if (SDL_LockMutex(board->pairs_object->mutex) != 0) {
      fprintf(stderr, "Could not lock ACGL mutex in puyo_mark_board_changed! SDL_Error %s\n", SDL_GetError());
      return false;
    }
    board->pairs_object->needs_update = true;
    SDL_UnlockMutex(board->pairs_object->mutex);
  }

  return true;
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
    // Only zeroing out b/c these are ACGL objects, handled elsewhere
    board->board_object = NULL;
    board->pairs_object = NULL;
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
  if (board == NULL) {
    return 0;
  } 

  for (int x=0; x<PUYO_WIDTH; x++) {
    for (int y=0; y<PUYO_HEIGHT_ACT; y++) {
      board->area[x*PUYO_HEIGHT_ACT+y] = puyo_get_random();
    }
  }

  board->next_pair = puyo_get_random_pair();
  board->next_next_pair = puyo_get_random_pair();
  puyo_mark_board_changed(board);
  puyo_mark_pairs_changed(board);
}
