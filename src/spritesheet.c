#include "spritesheet.h"

SDL_Texture* ss_load(SDL_Renderer* renderer) {
	int imgFlags = IMG_INIT_PNG;
	if (!(IMG_Init(imgFlags) & imgFlags)) {
		fprintf(stderr, "SDL_Image could not initalize! SDL_Image Error: %s\n", IMG_GetError());
		return NULL;
	}

	SDL_RWops* buffer_rwop = SDL_RWFromMem((void*)spritesheet_buf, 
	        spritesheet_buf_length);
	SDL_Surface* image = IMG_LoadPNG_RW(buffer_rwop);
	if (image == NULL) {
		fprintf(stderr, "IMG_LoadPNG_RW Failed, Error: %s\n", IMG_GetError());
		return NULL;
	}

	SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, image);
	if (texture == NULL) {
		fprintf(stderr, "Unable to create spritesheet texture from surface! SDL Error: %s\n", SDL_GetError());
		return NULL;
	}

	SDL_FreeSurface(image);
	return texture;
}

SDL_Rect* ss_split() {
	SDL_Rect* clips = (SDL_Rect*)malloc(SHEET_ROWS*SHEET_COLS*sizeof(SDL_Rect));

	int i = 0;
	for (int y=0; y<SHEET_ROWS; y++) {
		for (int x=0; x<SHEET_COLS; x++) {
			clips[i].x = x*SPRITE_WIDTH;
			clips[i].y = y*SPRITE_HEIGHT;
			clips[i].w = SPRITE_WIDTH;
			clips[i].h = SPRITE_HEIGHT;
			i++;
		}
	}

	return clips;
}

void ss_render(SDL_Renderer* renderer, SDL_Texture* spritesheet, SDL_Rect* clips, SDL_Rect* renderQuad, int spriteId) {
	if (spriteId < 0 || spriteId >= SHEET_ROWS*SHEET_COLS) {
		fprintf(stderr, "Internal error: Sprite ID %d is out of range!\n", spriteId);
		return;
	}
	SDL_RenderCopy(
		renderer,
		spritesheet,
		&clips[spriteId],
		renderQuad
	);
}
