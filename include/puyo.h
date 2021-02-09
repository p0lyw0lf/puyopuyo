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
#define PUYO_HEIGHT_ACT ((PUYO_HEIGHT) + (PUYO_HEIGHT_HIDDEN))
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
  ROT_RIGHT,  // o3
  ROT_LOCKING,
  // The following states are for when you are in a column,
  // and tapping rotate twice will flip the puyo around
  ROT_DOWN_LEFT,
  ROT_DOWN_RIGHT,
  ROT_UP_LEFT,
  ROT_UP_RIGHT
};

// All the possible types of puyo
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

typedef enum PUYO_COLOR_IDS puyo_t;
typedef uint32_t score_t;
typedef struct puyo_pair puyo_pair_t;
struct puyo_pair {
  puyo_t top;
  puyo_t bot;
};

typedef struct puyo_board puyo_board_t;
struct puyo_board {
  // PUYO_WIDTH * PUYO_HEIGHT_ACT array representing the current puyos on the board
  // organized by (0,0) is the bottom left corner, iterating through it goes up
  // columns, then right in rows
  puyo_t* area;

  // Current score associated with the board
  score_t score;
  // Number of incoming garbage particles
  score_t incoming_garbage;
  // Current length of the chain. 0 if no chain going off
  Uint8 chain;

  // puyos in the falling pair
  puyo_pair_t current_pair;
  // the rotation state of the falling pair
  enum PAIR_ROT_STATES rot_state;
  // Coordinates of where to render the falling pair on the board.
  // Uses same coordinate scale as `area`
  float puyo1_x, puyo1_y, puyo2_x, puyo2_y;

  // Upcoming pairs
  puyo_pair_t next_pair;
  puyo_pair_t next_next_pair;

  // Flag telling if we are in a state where puyos are popping
  bool is_popping;
  // Mask telling which puyos are currently popping, if they are
  bool* popping_area;
  // Counter for how long we've been popping the current batch of puyos
  Uint16 popping_counter;

  // For graphics use only, so we don't have to
  // redraw everything every time
  bool board_has_changed;
  bool pairs_have_changed;
  // A mutex for this object
  SDL_mutex* mutex;

  // I was trying to avoid putting graphics stuff in here,
  // but I think these are necessary. These are the objects
  // responsible for rendering the board. Useful to be kept
  // with this object for telling whether they need a re-render
  ACGL_gui_object_t* board_object;
  ACGL_gui_object_t* pairs_object;

  // Lookup table to get a sprite based on a puyo_t color
  int color_to_sprite[PUYO_NUM_COLORS];
};

// Lookup tables for scoring
extern const score_t PUYO_CHAIN_MULTIPLIERS[PUYO_MAX_CHAIN + 1];
extern const score_t PUYO_COLOR_MULTIPLIERS[PUYO_NUM_COLORS + 1];
extern const score_t PUYO_GROUP_MULTIPLIERS[PUYO_MAX_GROUP + 1];

// Gets a random puyo color
extern puyo_t puyo_get_random();
// Gets a random pair.
extern puyo_pair_t puyo_get_random_pair();
// Checks if a puyo color is a "playable" puyo
static inline bool is_playable_puyo(puyo_t puyo_color) {
  return (puyo_color == PUYO_COLOR_1 || puyo_color == PUYO_COLOR_2 || puyo_color == PUYO_COLOR_3 || puyo_color == PUYO_COLOR_4);
}

// An (PUYO_WIDTH*PUYO_HEIGHT_ACT) array of puyo_t
// forms the basic board. It is indexed by board[col*PUYO_HEIGHT_ACT+row]
extern puyo_board_t* puyo_create_board();

// Pops all groups on the board, and returns the score generated.
// Also increments chain counter if required. If no pops, returns 0
extern score_t puyo_pop_groups(puyo_board_t* board);

// Removes all puyos that are marked as popping from the board.
// Returns true when puyos were removed
extern bool puyo_apply_pops(puyo_board_t* board);

// Applies gravity to the board, making all the puyos fall to their
// lowest possible position. Should be called immediately after
// puyo_pop_chain returns nonzero or a new piece is locked.
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
