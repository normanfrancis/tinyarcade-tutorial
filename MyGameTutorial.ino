const uint16_t ALPHA = 0x1111;

#include <TinyScreen.h>
#include <Wire.h>
#include <SPI.h>
#include "TinyArcade.h"
#include "GameTutorialSprites.h"

TinyScreen display = TinyScreen(TinyScreenPlus);

#define zmax(a,b) ((a)>(b)?(a):(b))
#define zmin(a,b) ((a)<(b)?(a):(b))

typedef struct {
  int x;
  int y;
  int width;
  int height;
  int collisions;
  const unsigned int *bitmap;
} ts_sprite;

ts_sprite ball = {44, 28, 4, 4, 0, ballBitmap};

int amtSprites = 1;
ts_sprite * spriteList[1] = {&ball};

int backgroundColor = TS_16b_Black;

void setup() {
  // put your setup code here, to run once:
  arcadeInit();
  display.begin();
  display.setBitDepth(TSBitDepth16);
  display.setBrightness(15);
  display.setFlip(false);

  USBDevice.init();
  USBDevice.attach();
  SerialUSB.begin(9600);
}

void loop() {
  drawBuffer();
  readInputs();
}

void drawBuffer() {
  uint8_t lineBuffer[96 * 64 * 2];
  display.startData();
  for (int y = 0; y < 64; y++) {
    for (int b = 0; b < 96; b++) {
      lineBuffer[b * 2] = backgroundColor >> 8;
      lineBuffer[b * 2 + 1] = backgroundColor;
    }
    for (int spriteIndex = 0; spriteIndex < amtSprites; spriteIndex++) {
      ts_sprite *cs = spriteList[spriteIndex];
      if (y >= cs->y && y < cs->y + cs->height) {
        int endX = cs->x + cs->width;
        if (cs->x < 96 && endX > 0) {
          int xBitmapOffset = 0;
          int xStart = 0;
          if (cs->x < 0) xBitmapOffset -= cs->x;
          if (cs->x > 0) xStart = cs->x;
          int yBitmapOffset = (y - cs->y) * cs->width;
          for (int x = xStart; x < endX; x++) {
            unsigned int color = cs->bitmap[xBitmapOffset + yBitmapOffset++];
            if (color != ALPHA) {
              lineBuffer[(x) * 2] = color >> 8;
              lineBuffer[(x) * 2 + 1] = color;
            }
          }
        }
      }
    }
    display.writeBuffer(lineBuffer, 96 * 2);
  }
  display.endTransfer();
}

void readInputs() {
  //allow movement below top
  if (ball.y > 0) {
    if (checkJoystick(TAJoystickUp)) ball.y -= 1;
  }
  //allow movement above bottom
  if ((ball.y + ball.height) < 63 ) {
    if (checkJoystick(TAJoystickDown)) ball.y += 1;
  }
  //allow movement before left edge
  if ((ball.x + ball.width) < 95 ) {
    if (checkJoystick(TAJoystickRight)) ball.x += 1;
  }
  //allow after right edge
  if (ball.x > 0) {
    if (checkJoystick(TAJoystickLeft)) ball.x -= 1;
  }
}

bool testBitmapCollision(ts_sprite *s1, ts_sprite *s2) {
  //s1 left edge less than s2 right edge and
  //s1 right edge greater than s2 left edge
  if ((s1->x < s2->x + s2->width) && (s1->x + s1->width > s2->x))
  //s2 top edge less than s1 bottom edge and
  //s2 bottom edge greater than s1 top edge
    if ((s2->y < s1->y + s1->height) && (s2->y + s2->height > s1->y))
      return true;
  return false;
    
}


