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
ts_sprite redBrick = {43, 10, 10, 4, 0, redBrickBitmap};

int amtSprites = 2;
ts_sprite * spriteList[2] = {&ball, &redBrick};

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

  if (testPixelCollision(&ball, &redBrick)) {
    //set random x within screen width minus sprite width so it doesn't overflow on right
    redBrick.x = random(0, (95-redBrick.width));
    //set random y within screen height minus sprite height so it doesn't overflow on bottom
    redBrick.y = random(0, (63-redBrick.height));
  }

  redBrick.x++;
  if (redBrick.x == 95) {
    //set x so that right is just outside left edge of screen (sprite is 10 across)
    redBrick.x = -1*(redBrick.width+1);
    //set random y within screen height minus sprite height so it doesn't overflow on bottom
    redBrick.y = random(0, (63-redBrick.height));
  }
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

//check if s1 is withing s2
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

//check for overlapping pixels in bitmap within a rectangular range
bool testPixelCollision(ts_sprite *s1, ts_sprite *s2) {
  //return if s1 is inside s2
  if (!testBitmapCollision(s1, s2)) return false;
  //startingX is the left edge of sprt furthest from left of screen
  int startX = zmax(s1->x, s2->x);
  //endX is the right edge of sprt furthest from right of screen
  int endX = zmin(s1->x + s1->width, s2->x + s2->width);
  //startY the top edge of the sprt furthest from the top of screen
  int startY = zmax(s1->y, s2->y);
  //endY the bottom edge of the sprt furthest from the bottom of screen
  int endY = zmin(s1->y + s1->height, s2->y + s2->height);
  
  //for loops goes down reach row and the column inside rectangular range
  //iterate over vertical range (startY to endY)
  for (int y = startY; y < endY; y++) {
    //iterate over horizontal rage (startX to endX)
    for (int x = startX; x < endX; x++) {
      //if two non-transparent pixels overlap, there is a collision
      if (s1->bitmap[(y - s1->y)*s1->width + (x - s1->x)] != ALPHA && s2->bitmap[(y - s2->y)*s2->width + (x - s2 ->x)] != ALPHA)
        return true;
    }
  }
  //there was no overlap in the rectangular range
  return false;
}


