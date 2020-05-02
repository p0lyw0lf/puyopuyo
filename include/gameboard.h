#ifndef GAMEBOARD_H
#define GAMEBOARD_H

#include <SDL.h>
#include <acgl/gui.h>
#include "puyo.h"
#include "spritesheet.h"

typedef struct gb_data gb_data_t;
struct gb_data {
  SDL_Texture* spritesheet;
  SDL_Rect* clips;
  puyo_board_t* board;

  ACGL_gui_t* gui;
};


extern gb_data_t* gameboard_load_media(ACGL_gui_t* gui, puyo_board_t* board);
extern void gameboard_contruct_gui(gb_data_t* gbdata);
extern void gameboard_destroy(gb_data_t* gbdata);
// this is where most of the magic happens in terms of rendering the game
// oh boy is this going to be a long function
extern bool gameboard_render(SDL_Renderer* renderer, SDL_Rect location, void* data);

extern bool gameboard_puyo_render(SDL_Renderer* renderer, SDL_Rect location, void* data);
extern bool gameboard_background_render(SDL_Renderer* renderer, SDL_Rect location, void* data);
extern bool gameboard_upcomming_render(SDL_Renderer* renderer, SDL_Rect location, void* data);

// And some internal stuff to check data structure consistency
bool __gameboard_is_gb_data_t(gb_data_t* gbdata);
#endif //GAMEBOARD_H
