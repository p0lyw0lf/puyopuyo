#ifndef GAMEBOARD_H
#define GAMEBOARD_H

#include <SDL.h>
#include <acgl/gui.h>
#include "puyo.h"
#include "spritesheet.h"

typedef struct {
  SDL_Texture* spritesheet;
  SDL_Rect* clips;
  puyo_board_t* board;
  int x, y;
} gb_pos_data_t;

typedef struct {
  SDL_Texture* spritesheet;
  SDL_Rect* clips;
  puyo_board_t* board;

  ACGL_gui_t* gui;
} gb_data_t;

static const int MAGIC_X = -1;

enum MAGIC_Y {
  MAGIC_Y_BASE = -1,
  MAGIC_Y_FALLING_TOP =    -0b010,
  MAGIC_Y_FALLING_BOT =    -0b011,
  MAGIC_Y_UPCOMING_1_TOP = -0b100,
  MAGIC_Y_UPCOMING_1_BOT = -0b101,
  MAGIC_Y_UPCOMING_2_TOP = -0b110,
  MAGIC_Y_UPCOMING_2_BOT = -0b111,
};


extern gb_data_t* gameboard_load_media(ACGL_gui_t* gui, puyo_board_t* board);
extern void gameboard_contruct_gui(gb_data_t* gbdata);
extern void gameboard_destroy(gb_data_t* gbdata);
// this is where most of the magic happens in terms of rendering the game
// oh boy is this going to be a long function
extern void gameboard_render(SDL_Renderer* renderer, SDL_Rect* location, puyo_board_t* board, SDL_Texture* spritesheet, SDL_Rect* clips);

extern bool gameboard_puyo_render(SDL_Renderer* renderer, SDL_Rect location, void* data);
extern bool gameboard_background_render(SDL_Renderer* renderer, SDL_Rect location, void* data);

#endif //GAMEBOARD_H
