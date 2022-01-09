#include "main.h"

// putting method definitions up here so they can be used
int refresh_screen(SDL_Event event, void* data);
int main_quit(SDL_Event event, void* data);


bool init(mainVars* globals) {
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		fprintf(stderr, "SDL could not initalize! SDL_Error: %s\n", SDL_GetError());
		return false;
	}

	globals->window = SDL_CreateWindow(
		SCREEN_NAME,
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		SCREEN_WIDTH, SCREEN_HEIGHT,
		SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI
	);

	if (globals->window == NULL) {
		fprintf(stderr, "Window could not be created! SDL_Error: %s\n", SDL_GetError());
		return false;
	}

	globals->renderer = SDL_CreateRenderer(globals->window, -1, SDL_RENDERER_ACCELERATED);
	if (globals->renderer == NULL) {
		fprintf(stderr, "Renderer could not be created! SDL_Error: %s\n", SDL_GetError());
		return false;
	}

	SDL_SetRenderDrawColor(globals->renderer, 0xFF, 0xFF, 0xFF, 0xFF);

	globals->gui = ACGL_gui_init(globals->window);
	if (globals->gui == NULL) {
		fprintf(stderr, "Could not create gui! See previous error\n");
		return false;
	}

	const SDL_Scancode scancodes[NUM_KEY_CODES] = {
			SDL_SCANCODE_UP,
			SDL_SCANCODE_LEFT,
			SDL_SCANCODE_DOWN,
			SDL_SCANCODE_RIGHT,
			SDL_SCANCODE_Z,
			SDL_SCANCODE_X,
			SDL_SCANCODE_RETURN,
			SDL_SCANCODE_ESCAPE
	};

	globals->keybinds = ACGL_ih_init_keybinds(scancodes, NUM_KEY_CODES);
	globals->medata = ACGL_ih_init_eventdata(NUM_KEY_CODES);

	ACGL_ih_register_keyevent(globals->medata, KEY_ESC, &main_quit, globals);
	ACGL_ih_register_windowevent(globals->medata, &refresh_screen, globals);

	globals->board = puyo_create_board();

	globals->boarddata = NULL;
	globals->gamethread = NULL;
	globals->run = true;

	return true;
}

bool loadMedia(mainVars* globals) {
	globals->boarddata = gameboard_load_media(globals->gui, globals->board);
	return globals->boarddata != NULL;
}

int refresh_screen(SDL_Event event, void* data) {
	mainVars* globals = (mainVars*)data;

	if (globals->board != NULL) {
		if (SDL_LockMutex(globals->board->mutex) != 0) {
			fprintf(stderr, "Unable to obtain board mutex in refresh_screen! SDL Error: %s\n", SDL_GetError());
			return 1;
		}

		globals->board->board_has_changed = true;

		SDL_UnlockMutex(globals->board->mutex);
	}

  ACGL_gui_force_update(globals->gui);
	ACGL_gui_render(globals->gui);
	SDL_RenderPresent(globals->renderer);

	return 0;
}

int main_quit(SDL_Event event, void* data) {
	mainVars* globals = (mainVars*)data;

	globals->run = false;

	return 0;
}

void main_close(mainVars* globals) {
	ACGL_thread_destroy(globals->gamethread);
	globals->gamethread = NULL;

	puyo_free_board(globals->board);
	globals->board = NULL;
	// Already going to be freed when gui is destroyed
	// gameboard_destroy(globals->boarddata);
	globals->boarddata = NULL;

	ACGL_ih_deinit_keybinds(globals->keybinds);
	globals->keybinds = NULL;
	ACGL_ih_deinit_eventdata(globals->medata);
	globals->medata = NULL;

	ACGL_gui_destroy(globals->gui);
	globals->gui = NULL;

	SDL_DestroyRenderer(globals->renderer);
	globals->renderer = NULL;
	SDL_DestroyWindow(globals->window);
	globals->window = NULL;

	IMG_Quit();
	SDL_Quit();
}

int mainloop(mainVars* globals) {
	SDL_Event e;

	globals->gamethread = ACGL_thread_create(
		&runner_setup,
		&runner_loop,
		NULL,
		333,
		(void*)runner_create(globals->board),
		&runner_destroy
	);

	if (globals->gamethread == NULL) {
		fprintf(stderr, "Could not create background thread! SDL Error: %s\n", SDL_GetError());
		return 1;
	}
	if (!ACGL_thread_start(globals->gamethread, "runner_mainloop")) {
		fprintf(stderr, "Error starting runner_mainloop. See above\n");
		return 1;
	}

  while (globals->run && SDL_WaitEvent(&e) != 0) {
    switch (e.type) {
      case SDL_QUIT:
        globals->run = false;
        break;
      case SDL_WINDOWEVENT:
        ACGL_ih_handle_windowevent(e, globals->medata);
        break;
      case SDL_KEYUP:
      case SDL_KEYDOWN:
        ACGL_ih_handle_keyevent(e, globals->keybinds, globals->medata);
        break;
      case SDL_USEREVENT:
        switch (e.user.code) {
          case REFRESH_REQUEST:
            ACGL_gui_render(globals->gui);
	          SDL_RenderPresent(globals->renderer);
            break;
          default:
            break;
        }
        break;
      default:
        break;
    }
  }

	if (ACGL_thread_stop(globals->gamethread) != 0) {
		fprintf(stderr, "Error stopping runner_mainloop. See above\n");
		return 1;
	}

	return 0;
}

int main(int argc, char** argv) {
	mainVars g;

	if (!init(&g) || !loadMedia(&g)) {
		return 1;
	}

	ACGL_gui_render(g.gui);

	int returnval = mainloop(&g);

	main_close(&g);

	return returnval;
}
