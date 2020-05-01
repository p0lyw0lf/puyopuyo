#include "runner.h"

int runner_mainloop(void* data) {
  bool unlocked_quit = false;
  runnerData* grdata = (runnerData*)data;
  while (!unlocked_quit) {
    if (SDL_LockMutex(grdata->mutex) != 0) {
      fprintf(stderr, "Could not lock mutex in runner_mainloop! SDL_Error %s\n", SDL_GetError());
      break;
    }
    unlocked_quit = grdata->quit;
    SDL_UnlockMutex(grdata->mutex);

    if (SDL_LockMutex(grdata->board->mutex) != 0) {
      fprintf(stderr, "Could not lock board mutex in runner_mainloop! SDL_Error %s\n", SDL_GetError());
      break;
    }
    grdata->board->score += puyo_pop_chain(grdata->board);

    SDL_UnlockMutex(grdata->board->mutex);

    SDL_Event event;
    SDL_zero(event);
    event.type = SDL_USEREVENT;
    event.user.code = REFRESH_REQUEST;
    event.user.data1 = NULL;
    event.user.data2 = NULL;
    SDL_PushEvent(&event);

    SDL_Delay(1000);
  }

  return 0;
}

runnerData* runner_create(puyo_board_t* board) {
  runnerData* data = (runnerData*)malloc(sizeof(runnerData));
  data->mutex = SDL_CreateMutex();
  if (data->mutex == NULL) {
    fprintf(stderr, "Could not create mutex in runner_create! SDL Error: %s\n", SDL_GetError());
    free(data);
    return NULL;
  }
  data->board = board;
  data->quit = false;
  return data;
}

int runner_stop_thread(SDL_Thread* thread, runnerData* grdata) {
  // NOTE: thread passed in should be the same one created using grdata,
  // otherwise this has a good chance of blocking forever!
  if (SDL_LockMutex(grdata->mutex) != 0) {
    fprintf(stderr, "Could not lock mutex in runner_stop_thread! SDL_Error %s\n", SDL_GetError());
    return 1;
  }
  grdata->quit = true;
  SDL_UnlockMutex(grdata->mutex);

  return 0;
}

void runner_destroy(runnerData* data) {
  if (data != NULL) {
    if (data->mutex != NULL) {
      SDL_DestroyMutex(data->mutex);
      data->mutex = NULL;
    }
    // assume reference to board is kept elsewhere (like in the gui)
    free(data);
  }
}
