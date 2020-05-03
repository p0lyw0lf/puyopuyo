#include "runner.h"

int runner_mainloop(void* data) {
  // Each thread needs to re-seed independently
  // for whatever reason
  // I can't believe they didn't make `rand` return
  // well-seeded numbers by default
  srand((unsigned)time(NULL));

  bool unlocked_quit = false;
  runnerData* grdata = (runnerData*)data;

  if (SDL_LockMutex(grdata->board->mutex) != 0) {
    fprintf(stderr, "Could not lock board mutex in runner_mainloop init! SDL_Error %s\n", SDL_GetError());
    return 1;
  }

  for (char y = 0; y < PUYO_HEIGHT; y++) {
    for (char x = 0; x < PUYO_WIDTH; x++) {
      grdata->board->area[x * PUYO_HEIGHT_ACT + y] = puyo_get_random();
    }
  }

  SDL_UnlockMutex(grdata->board->mutex);

  while (!unlocked_quit) {
    if (SDL_LockMutex(grdata->mutex) != 0) {
      fprintf(stderr, "Could not lock mutex in runner_mainloop! SDL_Error %s\n", SDL_GetError());
      return 1;
    }
    unlocked_quit = grdata->quit;
    SDL_UnlockMutex(grdata->mutex);

    if (SDL_LockMutex(grdata->board->mutex) != 0) {
      fprintf(stderr, "Could not lock board mutex in runner_mainloop! SDL_Error %s\n", SDL_GetError());
      return 1;
    }
    score_t new_score = puyo_pop_chain(grdata->board);
    printf("%d\n", new_score);
    grdata->board->score += new_score;

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
  if (grdata == NULL) {
    fprintf(stderr, "Passed a NULL grdata in runner_stop_thread! SDL_Error: %s\n", SDL_GetError());
    return 1;
  }

  // NOTE: thread passed in should be the same one created using grdata,
  // otherwise this has a good chance of blocking forever!
  if (SDL_LockMutex(grdata->mutex) != 0) {
    fprintf(stderr, "Could not lock mutex in runner_stop_thread! SDL_Error %s\n", SDL_GetError());
    return 1;
  }
  grdata->quit = true;
  SDL_UnlockMutex(grdata->mutex);

  int result;
  // It's actually safe to pass a NULL value here according to SDL docs
  SDL_WaitThread(thread, &result);

  return result;
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
