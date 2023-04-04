// doomgeneric for cross-platform development library 'Simple DirectMedia Layer'

#include "doomkeys.h"
#include "m_argv.h"
#include "doomgeneric.h"

#include <stdio.h>
#include <unistd.h>

#include <stdbool.h>
#include <emscripten.h>

uint32_t *DG_ScreenBuffer = 0;
uint32_t start = 0;

void M_FindResponseFile(void);
void D_DoomMain(void);

int main()
{
  M_FindResponseFile();

  DG_ScreenBuffer = malloc(DOOMGENERIC_RESX * DOOMGENERIC_RESY * 4);

  DG_Init();

  D_DoomMain();

  emscripten_set_main_loop(doomgeneric_Tick, 0, 1);

  return 0;
}

void DG_Init(void)
{
  start = (int)emscripten_get_now();
}

void DG_SleepMs(uint32_t ms)
{
  emscripten_sleep(ms);
}

void DG_DrawFrame(void)
{
  EM_ASM({
    const canvas = Module.canvas;
    const context = Module.context;

    if (!Module.imageData) Module.imageData = new ImageData(
      new Uint8ClampedArray(
        Module.asm.memory.buffer,
        $0,
        canvas.width * canvas.height * 4
      ),
      canvas.width,
      canvas.height
    );

    context.putImageData(Module.imageData, 0, 0);
  }, DG_ScreenBuffer);
}

void DG_SetWindowTitle(const char *title)
{
  emscripten_set_window_title(title);
}

uint32_t DG_GetTicksMs()
{
  return (uint32_t)emscripten_get_now() - start;
}

#define KEYQUEUE_SIZE 16

static unsigned short s_KeyQueue[KEYQUEUE_SIZE];
static unsigned int s_KeyQueueWriteIndex = 0;
static unsigned int s_KeyQueueReadIndex = 0;

static unsigned char convertToDoomKey(unsigned int key)
{
  // JS keycodes to DOOM keys
  switch (key)
    {
    case 13:
      key = KEY_ENTER;
      break;
    case 8:
      key = KEY_ESCAPE;
      break;
    case 37:
      key = KEY_LEFTARROW;
      break;
    case 39:
      key = KEY_RIGHTARROW;
      break;
    case 38:
      key = KEY_UPARROW;
      break;
    case 40:
      key = KEY_DOWNARROW;
      break;
    case 17:
      key = KEY_FIRE;
      break;
    case 32:
      key = KEY_USE;
      break;
    case 16:
      key = KEY_RSHIFT;
      break;
    case 18:
      key = KEY_LALT;
      break;
    case 113:
      key = KEY_F2;
      break;
    case 114:
      key = KEY_F3;
      break;
    case 115:
      key = KEY_F4;
      break;
    case 116:
      key = KEY_F5;
      break;
    case 117:
      key = KEY_F6;
      break;
    case 118:
      key = KEY_F7;
      break;
    case 119:
      key = KEY_F8;
      break;
    case 120:
      key = KEY_F9;
      break;
    case 121:
      key = KEY_F10;
      break;
    case 122:
      key = KEY_F11;
      break;
    case 187:
      key = KEY_EQUALS;
      break;
    case 189:
      key = KEY_MINUS;
      break;
    default:
      break;
    }

  return key;
}

static void addKeyToQueue(int pressed, unsigned int keyCode)
{
  unsigned char key = convertToDoomKey(keyCode);

  unsigned short keyData = (pressed << 8) | key;

  s_KeyQueue[s_KeyQueueWriteIndex] = keyData;
  s_KeyQueueWriteIndex++;
  s_KeyQueueWriteIndex %= KEYQUEUE_SIZE;
}

extern void doomgeneric_AddKey(int pressed, unsigned int keyCode)
{
  addKeyToQueue(pressed, keyCode);
}

int DG_GetKey(int *pressed, unsigned char *doomKey)
{
  if (s_KeyQueueReadIndex == s_KeyQueueWriteIndex)
  {
    // key queue is empty
    return 0;
  }
  else
  {
    unsigned short keyData = s_KeyQueue[s_KeyQueueReadIndex];
    s_KeyQueueReadIndex++;
    s_KeyQueueReadIndex %= KEYQUEUE_SIZE;

    *pressed = keyData >> 8;
    *doomKey = keyData & 0xFF;

    return 1;
  }

  return 0;
}
