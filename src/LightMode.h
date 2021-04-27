//
// Created by Jan on 27.12.2020.
//

#ifndef LIGHTMODE_LIGHTMODE_H
#define LIGHTMODE_LIGHTMODE_H

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <Scheduler.h>

#define NUM_LEDS 10
#define NORMAL  1
#define RAINBOW_ALL  2
#define RAINBOW_WHEEL  3
#define THEATER_CHASE_RAINBOW_EACH_PIXEL_SAME  4
#define THEATER_CHASE_RAINBOW_EACH_PIXEL_DIFFERENT  5
#define THEATER_CHASE  6
#define COLOR_WHIPE  7
#define FIRE  8
#define RUNNING_LIGHTS  9
#define SPARKLE  10
#define SPARKLE_RANDOM  11
#define TWINKLE  12
#define TWINKLE_RANDOM  13
#define CYLON_BOUNCE  14
#define BLINKING  15
typedef struct dataTrans {
    uint8 mode;
    uint8 red;
    uint8 green;
    uint8 blue;
    uint8 wait;
    uint8 brightness;
}trans;

class LightMode : public Task {
    bool modus_changed = false;
    uint8 mode = 1;
    uint8 red = 0;
    uint8 green = 0;
    uint8 blue = 0;
    uint8 wait = 50;
    uint8 brightness = 100;
    bool running = false;



public:
    LightMode();
    void begin();
    void setBrightness(uint8_t brightness);
    void setSpeed(uint8_t speed);
    void setMode(uint8 mode);
    void setColor(uint8_t red, uint8_t green, uint8_t blue);
    void loop();
    void stop();
    trans getDataForTransfer();
    String toString();

private:
    void setPixel(int Pixel, byte red2, byte green2, byte blue2);

    void setAll(uint8 red, uint8 green, uint8 blue);
    void normal();
    void rainbowWheel();
    void rainbowAll();
    void theaterChaseRainbowEachPixelSame();
    void theaterChaseRainbowEachPixelDifferent();
    void theaterChase();
    void colorWipe();
    void fire();
    void runningLights();
    void sparkle();
    void sparkleRandom();
    void twinkle();
    void twinkleRandom();
    void cylonBounce();
    void blinking();

    void error();

    byte * Wheel(byte WheelPos);
};

#endif //LIGHTMODE_LIGHTMODE_H
