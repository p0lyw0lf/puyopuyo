#ifndef PUYO_H
#define PUYO_H

#include <SDL.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include <acgl/gui.h>
#include "spritesheet.h"

#define PUYO_WIDTH 6
#define PUYO_HEIGHT 12
#define PUYO_HEIGHT_HIDDEN 2
#define PUYO_HEIGHT_ACT PUYO_HEIGHT + PUYO_HEIGHT_HIDDEN
#define PUYO_MAX_CHAIN 15
#define PUYO_MAX_GROUP 11
#define PUYO_GROUP_POP 4

enum PAIR_ROT_STATES {
  ROT_DOWN,   //    0
              //    o
  ROT_LEFT,   // 1o
              //    o
  ROT_UP,     //    2
              //
  ROT_RIGHT   // o3
};

enum PUYO_COLOR_IDS {
  PUYO_COLOR_1 = 0,
  PUYO_COLOR_2 = 1,
  PUYO_COLOR_3 = 2,
  PUYO_COLOR_4 = 3,
  PUYO_NUM_COLORS = 4,
  PUYO_COLOR_NONE,
  PUYO_COLOR_GARBAGE,
  PUYO_COLOR_INDESTRUCTABLE,
  PUYO_TOTAL_COLOR_IDS,
};

typedef uint8_t puyo_t;
typedef uint32_t score_t;
typedef struct puyo_pair puyo_pair_t;
struct puyo_pair {
  puyo_t top;
  puyo_t bot;
};

typedef struct puyo_board puyo_board_t;
struct puyo_board {
  puyo_t* area;
  puyo_t* prev_area;

  score_t score;
  score_t incoming_garbage;
  int chain;

  puyo_pair_t current_pair;
  enum PAIR_ROT_STATES current_rot_state, prev_rot_state;
  int current_x, current_y, prev_x, prev_y;
  puyo_pair_t next_pair;
  puyo_pair_t next_next_pair;

  // For graphics use only, so we don't have to
  // redraw everything every time
  bool board_has_changed;
  bool pairs_have_changed;
  // A mutex for the object
  SDL_mutex* mutex;

  // I was trying to avoid putting graphics stuff in here,
  // but I think these are necessary.
  ACGL_gui_object_t* board_object;
  ACGL_gui_object_t* pairs_object;

  int color_to_sprite[PUYO_NUM_COLORS];
};

const score_t PUYO_CHAIN_MULTIPLIERS[PUYO_MAX_CHAIN + 1];
const score_t PUYO_COLOR_MULTIPLIERS[PUYO_NUM_COLORS + 1];
const score_t PUYO_GROUP_MULTIPLIERS[PUYO_MAX_GROUP + 1];

// Gets a random puyo color
extern puyo_t puyo_get_random();
// Gets a random pair.
extern puyo_pair_t puyo_get_random_pair();

// An (PUYO_WIDTH*PUYO_HEIGHT_ACT) array of puyo_t
// forms the basic board. It is indexed by board[col*PUYO_HEIGHT_ACT+row]
extern puyo_board_t* puyo_create_board();

// Pops all chains on the board, and returns the score generated.
// Also increments chain counter if required. If no pops, returns 0
extern score_t puyo_pop_chain(puyo_board_t* board);

// Applies gravity to the board, making all the puyos fall to their
// lowest possible position. Should be called immediately after
// the above method returns nonzero.
// Returns true if any puyos moved.
extern bool puyo_apply_gravity(puyo_board_t* board);

// Applies garbage to the board. Returns true if this would make the
// player top out.
extern bool puyo_apply_garbage(puyo_board_t* board);

// Marks a board has been changed. Returns false if there was an error.
extern bool puyo_mark_board_changed(puyo_board_t* board);

// Marks that the pairs have changed and need to re-render. Returns false on error
extern bool puyo_mark_pairs_changed(puyo_board_t* board);

// Correctly frees from memory a puyo_board_t*
extern void puyo_free_board(puyo_board_t* board);

#endif //PUYO_H
