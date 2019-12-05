#ifndef MAIN_H
#define MAIN_H

#include <SDL.h>
#include <stdio.h>
#include <stdbool.h>

#include <acgl/gui.h>
#include <acgl/inputhandler.h>
#include "gameboard.h"
#include "runner.h"
#include "spritesheet.h"

#define SCREEN_WIDTH 400
#define SCREEN_HEIGHT 600
#define SCREEN_NAME "PuyoPuyo"

enum KEY_CODES {
  KEY_UP, // SDL_SCANCODE_UP;
  KEY_MOVE_LEFT, // SDL_SCANCODE_LEFT;
  KEY_DOWN, // SDL_SCANCODE_DOWN;
  KEY_MOVE_RIGHT, // SDL_SCANCODE_RIGHT;
  KEY_ROT_LEFT, // SDL_SCANCODE_Z;
  KEY_ROT_RIGHT, // SDL_SCANCODE_X;
  KEY_ENTER, // SDL_SCANCODE_RETURN;
  KEY_ESC, // SDL_SCANCODE_ESCAPE;
  NUM_KEY_CODES
};

typedef struct {
  bool run;
  SDL_Window* window;
  SDL_Renderer* renderer;
  puyo_board_t* board;
  runnerData* gamedata;
  SDL_Thread* gamethread;
  gb_data_t* boarddata;
  ACGL_gui_t* gui;
  ACGL_ih_keybinds_t* keybinds;
  ACGL_ih_eventdata_t* medata;
} mainVars;

extern bool init(mainVars* globals);
extern bool loadMedia(mainVars* globals);
extern void close(mainVars* globals);
extern int mainloop(mainVars* globals);

#endif //MAIN_H
