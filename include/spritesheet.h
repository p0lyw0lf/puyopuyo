#ifndef SPRITESHEET_H
#define SPRITESHEET_H

#include <SDL.h>
#include <SDL_image.h>
#include <stdio.h>

#include "spritesheet_png.h"

#define SPRITE_HEIGHT 32
#define SPRITE_WIDTH 32
#define SHEET_ROWS 20
#define SHEET_COLS 21

// Returns an array the SDL_Rects to be used in rendering
// array is length (SHEET_ROWS*SHEET_COLS), going across
// spritesheet row by row
extern SDL_Rect* ss_split();
// Loads the texture from memory, optimizing it for use
// with the specified renderer
extern SDL_Texture* ss_load(SDL_Renderer* renderer);
// Actually renders the specified sprite. Mostly just includes
// bounds checking on spriteId
extern void ss_render(SDL_Renderer* renderer, SDL_Texture* spritesheet, SDL_Rect* clips, SDL_Rect* renderQuad, int spriteId);

// Sprite ID calculator.
enum PUYO_IDS {
	// Main block of colored puyo values
	PUYO_RED = 0 * SHEET_COLS,
	PUYO_GREEN = 1 * SHEET_COLS,
	PUYO_BLUE = 2 * SHEET_COLS,
	PUYO_YELLOW = 3 * SHEET_COLS,
	PUYO_PURPLE = 4 * SHEET_COLS,
	PUYO_PINK = 5 * SHEET_COLS,
	PUYO_CYAN = 6 * SHEET_COLS,
	// Modifiers, can be added in any combination
	PUYO_DOWN = 0b0001,
	PUYO_UP = 0b0010,
	PUYO_RIGHT = 0b0100,
	PUYO_LEFT = 0b1000,
	PUYO_CRYSTAL = 7 * SHEET_COLS,

	// Also modifiers but are exclusive
	PUYO_SURPRISED = 17,
	PUYO_EYES = 18,
	PUYO_GREY = 20,

	// Now for special ids that can't be modified
	PUYO_GARBAGE_BLOB = 0 * SHEET_COLS + 19,
	PUYO_GARBAGE_SINGLE = 15 * SHEET_COLS + 20,
	PUYO_GARBAGE_LINE = 15 * SHEET_COLS + 19,
	PUYO_GARBAGE_ROCK = 14 * SHEET_COLS + 20,
	PUYO_GARBAGE_STAR = 14 * SHEET_COLS + 19,
	PUYO_GARBAGE_MOON = 14 * SHEET_COLS + 18,
	PUYO_GARBAGE_GROWN = 14 * SHEET_COLS + 17,
};

#endif //SPRITESHEET_H
