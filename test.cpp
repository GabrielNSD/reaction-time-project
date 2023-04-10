#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <string>
#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

SDL_Window *window;
SDL_Renderer *renderer;

bool running = false;

SDL_Point center = {.x = 0, .y = 0};
const int radius = 50;
const int width = 600;
const int height = 300;

const int maxMatches = 10;
double results[maxMatches];
uint32_t startTime = 0;

int circleR = 0x00;
int circleG = 0x00;
int circleB = 0x00;

int backgroundColorR = 0x00;
int backgroundColorG = 0x00;
int backgroundColorB = 0x00;

int expectedColor = 0;
int hits, miss = 0;

void randomizeCircle()
{
  int randomColor;
  int lowest = 1, highest = 4;
  randomColor = std::rand() % (highest - lowest + 1) + lowest;
  expectedColor = randomColor;

  int minimumWidth = radius;
  int maximumWidth = width - radius;
  int minimumHeight = radius;
  int maximumHeight = height - radius;

  int newX, newY;

  newX = std::rand() % (maximumWidth - minimumWidth + 1) + minimumWidth;
  newY = std::rand() % (maximumHeight - minimumHeight + 1) + minimumHeight;

  center = {.x = newX, .y = newY};

  switch (randomColor)
  {
  case 1: // #785ef0 up
    circleR = 120;
    circleG = 94;
    circleB = 240;
    break;
  case 2: // #DC267F down
    circleR = 220;
    circleG = 38;
    circleB = 126;
    break;
  case 3: // #FE6100 right
    circleR = 254;
    circleG = 97;
    circleB = 0;
    break;
  case 4: // #FEFE62 left
    circleR = 254;
    circleG = 254;
    circleB = 98;
    break;
  }
}

void redraw()
{
  SDL_SetRenderDrawColor(renderer, backgroundColorR, backgroundColorG, backgroundColorB, 0xFF);
  SDL_RenderClear(renderer);
  filledCircleRGBA(renderer, center.x, center.y, radius, circleR, circleG,
                   circleB, 0xFF);
  SDL_RenderPresent(renderer);
}

void markStartTime()
{
  startTime = SDL_GetTicks();
}

void resetSate()
{
  running = false;
  hits = miss = 0;
}

void finish()
{
  std::string a = "document.getElementById('average').textContent = ";
  // make the average value of results
  uint32_t sum = 0;
  for (int i = 0; i < maxMatches; i++)
  {
    sum += results[i];
  }
  float average = sum / maxMatches;

  char averageScript[100];
  sprintf(averageScript, "document.getElementById('average').textContent = '%2f'", average / 1000);

  char scoreScript[100];
  sprintf(scoreScript, "document.getElementById('score').textContent = '%d'", hits - miss);

  emscripten_run_script(averageScript);
  emscripten_run_script(scoreScript);
  resetSate();
  backgroundColorB = 0x80;
  circleR = circleG = 0x00;
  circleB = 0x80;
  redraw();
}

void match(uint32_t timeStamp)
{
  uint32_t endTime = timeStamp;
  randomizeCircle();
  results[hits] = endTime - startTime;
  hits++;
  if (hits == maxMatches)
  {
    finish();
  }
  else
  {
    markStartTime();
  }
  redraw();
}

void start()
{
  running = true;
  hits = miss = 0;
  backgroundColorR = backgroundColorG = backgroundColorB = 0x00;
  markStartTime();
  randomizeCircle();
  redraw();
}

void checkEvent(SDL_Event e, int expectedColor)
{
  uint32_t timeStamp = e.key.timestamp;

  int button = 0;

  switch (e.key.keysym.sym)
  {
  case SDLK_UP:
    button = 1;
    break;
  case SDLK_DOWN:
    button = 2;
    break;
  case SDLK_RIGHT:
    button = 3;
    break;
  case SDLK_LEFT:
    button = 4;
    break;

  default:
    button = 0;
    break;
  }

  if (button == expectedColor)
  {
    match(timeStamp);
  }
  else
  {
    miss++;
  }
}

uint32_t ticksForNextKeyDown = 0;

bool handle_events()
{
  SDL_Event event;
  SDL_PollEvent(&event);
  if (event.type == SDL_QUIT)
  {
    return false;
  }
  if (event.type == SDL_KEYDOWN)
  {
    if (event.key.keysym.sym == SDLK_RETURN && !running)
    {
      start();
    }
    else
    {
      uint32_t ticksNow = SDL_GetTicks();
      checkEvent(event, expectedColor);
    }
  }
  return true;
}

void run_main_loop()
{
#ifdef __EMSCRIPTEN__
  emscripten_set_main_loop([]()
                           { handle_events(); },
                           0, true);
#else
  while (handle_events())
    ;
#endif
}

int main()
{
  SDL_Init(SDL_INIT_VIDEO);

  backgroundColorG = 0x80;
  circleR = circleB = 0x00;
  circleG = 0x80;

  SDL_CreateWindowAndRenderer(width, height, 0, &window, &renderer);

  redraw();
  run_main_loop();

  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);

  SDL_Quit();
}

#ifdef __cplusplus
#define EXTERN extern "C"
#else
#define EXTERN
#endif

EXTERN EMSCRIPTEN_KEEPALIVE void myFunction(int argc, char **argv)
{
  // printf("MyFunction Called\n");
  // match();
  // randomizeCircle();
  redraw();
}

EXTERN EMSCRIPTEN_KEEPALIVE void showResults(int argc, char **argv)
{
  printf("showResults called\n");
  printf("Hits: %d\n", hits);
  printf("Miss: %d\n", miss);
  for (int i = 0; i < maxMatches; i++)
  {
    printf("Result %d: %f\n", i, results[i]);
  }
}

EXTERN EMSCRIPTEN_KEEPALIVE void reset(int argc, char **argv)
{
  printf("reset called\n");
  resetSate();
  backgroundColorG = 0x80;
  circleR = circleB = 0x00;
  circleG = 0x80;
  redraw();
}