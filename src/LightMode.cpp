//
// Created by Jan on 27.12.2020.
//

#include "LightMode.h"
#include <Adafruit_NeoPixel.h>
#include <Arduino.h>


Adafruit_NeoPixel strip(NUM_LEDS, D8, NEO_GRB + NEO_KHZ800);


LightMode::LightMode() : Task() {
}

void LightMode::begin() {
    strip.begin();
    strip.setBrightness(this->brightness);
    setAll(10,10,10);
}

void LightMode::setMode(uint8 mode) {
    stop();
    this->mode = mode;
    modus_changed = true;
}

void LightMode::setSpeed(uint8_t speed) {
    if (speed < 256) wait = 256 - speed;
}

void LightMode::setBrightness(uint8_t brightness) {
    strip.setBrightness(brightness);
}

void LightMode::setPixel(int Pixel, byte red2, byte green2, byte blue2) {
#ifdef ADAFRUIT_NEOPIXEL_H
    // NeoPixel
    strip.setPixelColor(Pixel, strip.Color(red2, green2, blue2));
#endif
}


// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
void LightMode::setAll(uint8 red, uint8 green, uint8 blue) {
    for (int i = 0; i < NUM_LEDS; i++) {
        setPixel(i, red, green, blue);
    }
    strip.show();
}

byte *LightMode::Wheel(byte WheelPos) {
    static byte c[3];

    if (WheelPos < 85) {
        c[0] = WheelPos * 3;
        c[1] = 255 - WheelPos * 3;
        c[2] = 0;
    } else if (WheelPos < 170) {
        WheelPos -= 85;
        c[0] = 255 - WheelPos * 3;
        c[1] = 0;
        c[2] = WheelPos * 3;
    } else {
        WheelPos -= 170;
        c[0] = 0;
        c[1] = WheelPos * 3;
        c[2] = 255 - WheelPos * 3;
    }

    return c;
}

void LightMode::normal() {
    setAll(red, green, blue);
}

void LightMode::rainbowWheel() {
    // Hue of first pixel runs 5 complete loops through the color wheel.
    // Color wheel has a range of 65536 but it's OK if we roll over, so
    // just count from 0 to 5*65536. Adding 256 to firstPixelHue each time
    // means we'll make 5*65536/256 = 1280 passes through this outer loop:
    for (long firstPixelHue = 0; firstPixelHue < 5 * 65536 && modus_changed == false; firstPixelHue += 256) {

        for (int i = 0; i < strip.numPixels(); i++) { // For each pixel in strip...
            if (modus_changed) return;

            // Offset pixel hue by an amount to make one full revolution of the
            // color wheel (range of 65536) along the length of the strip
            // (strip.numPixels() steps):
            int pixelHue = firstPixelHue + (i * 65536L / strip.numPixels());
            // strip.ColorHSV() can take 1 or 3 arguments: a hue (0 to 65535) or
            // optionally add saturation and value (brightness) (each 0 to 255).
            // Here we're using just the single-argument hue variant. The result
            // is passed through strip.gamma32() to provide 'truer' colors
            // before assigning to each pixel:
            strip.setPixelColor(i, strip.gamma32(strip.ColorHSV(pixelHue)));
        }
        strip.show(); // Update strip with new contents
        delay(wait);
    }
}

void LightMode::rainbowAll() {
    byte *c;
    uint16_t i, j;

    for (j = 0; j < 256 * 5; j++) { // 5 cycles of all colors on wheel

        for (i = 0; i < NUM_LEDS; i++) {
            if (modus_changed) return;
            c = Wheel(((i * 256 / NUM_LEDS) + j) & 255);
            setPixel(i, *c, *(c + 1), *(c + 2));
        }
        strip.show();
        delay(wait);
    }
}

void LightMode::theaterChaseRainbowEachPixelSame() {
    byte *c;

    for (int j = 0; j < 256; j++) {     // cycle all 256 colors in the wheel

        for (int q = 0; q < 3; q++) {
            for (int i = 0; i < NUM_LEDS; i = i + 3) {
                if (modus_changed) return;
                c = Wheel((i + j) % 255);
                setPixel(i + q, *c, *(c + 1), *(c + 2));    //turn every third pixel on
            }
            strip.show();

            delay(wait);

            for (int i = 0; i < NUM_LEDS; i = i + 3) {
                setPixel(i + q, 0, 0, 0);        //turn every third pixel off
            }
        }
    }
}

void LightMode::theaterChaseRainbowEachPixelDifferent() {
    int firstPixelHue = 0;     // First pixel starts at red (hue 0)
    for (int a = 0; a < 30; a++) {  // Repeat 30 times...

        for (int b = 0; b < 3; b++) { //  'b' counts from 0 to 2...
            strip.clear();         //   Set all pixels in RAM to 0 (off)
            // 'c' counts up from 'b' to end of strip in increments of 3...
            for (int c = b; c < strip.numPixels(); c += 3) {
                if (modus_changed) return;
                // hue of pixel 'c' is offset by an amount to make one full
                // revolution of the color wheel (range 65536) along the length
                // of the strip (strip.numPixels() steps):
                int hue = firstPixelHue + c * 65536L / strip.numPixels();
                uint32_t color = strip.gamma32(strip.ColorHSV(hue)); // hue -> RGB
                strip.setPixelColor(c, color); // Set pixel 'c' to value 'color'
            }
            strip.show();                // Update strip with new contents
            delay(wait);                 // Pause for a moment
            firstPixelHue += 65536 / 90; // One cycle of color wheel over 90 frames
        }
        yield();
    }
}

void LightMode::theaterChase() {
    for (int j = 0; j < 10; j++) {  //do 10 cycles of chasing
        for (int q = 0; q < 3; q++) {
            if (modus_changed) return;

            for (int i = 0; i < NUM_LEDS; i = i + 3) {
                setPixel(i + q, red, green, blue);    //turn every third pixel on
            }
            strip.show();
            delay(wait);
            for (int i = 0; i < NUM_LEDS; i = i + 3) {
                setPixel(i + q, 0, 0, 0);        //turn every third pixel off
            }
        }
    }
}

void LightMode::colorWipe() {
    for (uint16_t i = 0; i < NUM_LEDS; i++) {
        if (modus_changed) return;

        setPixel(i, red, green, blue);
        strip.show();
        delay(wait);
    }

    setAll(0 ,0 ,0);
}

void LightMode::fire() {
    for (int j = 0; j < 2; j++) {
        int r = 255, g = 90, b = 35;

        for (int i = 0; i < strip.numPixels() / 2; i++) {
            int flicker = random(0, 150);
            int r1 = r - flicker * i * 0.8;
            int g1 = g - flicker * i * 0.7;
            int b1 = b - flicker * i * 0.7;
            if (g1 < 0) g1 = 0;
            if (r1 < 0) r1 = 0;
            if (b1 < 0) b1 = 0;

            if (j == 0) strip.setPixelColor(i, r1, g1, b1);
            else strip.setPixelColor(NUM_LEDS - i - 1, r1, g1, b1);
        }
    }
    strip.show();

    //  Adjust the this->delay here, if you'd like.  Right now, it randomizes the
    //  color switch this->delay to give a sense of realism
    this->delay(random(10, wait));
}

void LightMode::runningLights() {
    int Position = 0;
    for (int i = 0; i < NUM_LEDS * 2; i++) {
        if (modus_changed) return;

        Position++; // = 0; //Position + Rate;
        for (int i = 0; i < NUM_LEDS; i++) {
            setPixel(i, ((sin(i + Position) * 127 + 128) / 255) * red,
                     ((sin(i + Position) * 127 + 128) / 255) * green,
                     ((sin(i + Position) * 127 + 128) / 255) * blue);
        }
        strip.show();
        delay(wait);
    }
}

void LightMode::sparkle() {
    int Pixel = random(NUM_LEDS);
    setPixel(Pixel, red, green, blue);
    strip.show();
    delay(wait);
    setPixel(Pixel, 0, 0, 0);
}

void LightMode::sparkleRandom() {
    int Pixel = random(NUM_LEDS);
    setPixel(Pixel, random(0, 255), random(0, 255), random(0, 255));
    strip.show();
    delay(wait);
    setPixel(Pixel, 0, 0, 0);
}

void LightMode::twinkle() {
    setAll(0, 0, 0);
    for (int i = 0; i < 3; i++) {
        if (modus_changed) return;

        setPixel(random(NUM_LEDS), red, green, blue);
        strip.show();
        delay(wait);
    }
    delay(wait);
}

void LightMode::twinkleRandom() {
    setAll(0, 0, 0);
    for (int i = 0; i < 3; i++) {
        setPixel(random(NUM_LEDS), random(0, 255), random(0, 255), random(0, 255));
        strip.show();
        delay(wait);
    }
    delay(wait);
}

void LightMode::cylonBounce() {
    int EyeSize = 2;
    for (int i = 0; i < NUM_LEDS - EyeSize - 2; i++) {
        if (modus_changed) return;

        setAll(0, 0, 0);
        setPixel(i, red / 10, green / 10, blue / 10);
        for (int j = 1; j <= EyeSize; j++) {
            setPixel(i + j, red, green, blue);
        }
        setPixel(i + EyeSize + 1, red / 10, green / 10, blue / 10);
        strip.show();
        delay(wait);
    }
    delay(wait);

    for (int i = NUM_LEDS - EyeSize - 2; i > 0; i--) {
        if (modus_changed) return;

        setAll(0, 0, 0);
        setPixel(i, red / 10, green / 10, blue / 10);
        for (int j = 1; j <= EyeSize; j++) {
            setPixel(i + j, red, green, blue);
        }
        setPixel(i + EyeSize + 1, red / 10, green / 10, blue / 10);
        strip.show();
        delay(wait);
    }
    delay(wait);
}

void LightMode::blinking() {
    float r, g, b;

    for (int k = 0; k < 256; k = k + 1) {
        r = (k / 256.0) * red;
        g = (k / 256.0) * green;
        b = (k / 256.0) * blue;
        setAll(r, g, b);
        strip.show();;
    }

    for (int k = 255; k >= 0; k = k - 2) {
        r = (k / 256.0) * red;
        g = (k / 256.0) * green;
        b = (k / 256.0) * blue;
        setAll(r, g, b);
        strip.show();;
    }

    delay(wait);
}

void LightMode::error() {

    for (int i = 0; i < 3; i++) {
        setAll(255, 0, 0);
        delay(200);
        setAll(0, 0, 0);
        delay(200);
    }
    delay(200);
}

void LightMode::loop() {
    if (modus_changed) {
        modus_changed = false;
        yield();
    }

    yield();
    switch (mode) {
        case 1:
            normal();
            break;
        case 2:
            rainbowAll();
            break;
        case 3:
            rainbowWheel();
            break;
        case 4:
            theaterChaseRainbowEachPixelSame();
            break;
        case 5:
            theaterChaseRainbowEachPixelDifferent();
            break;
        case 6:
            theaterChase();
            break;
        case 7:
            colorWipe();
            break;
        case 8:
            fire();
            break;
        case 9:
            runningLights();
            break;
        case 10:
            sparkle();
            break;
        case 11:
            sparkleRandom();
            break;
        case 12:
            twinkle();
            break;
        case 13:
            twinkleRandom();
            break;
        case 14:
            cylonBounce();
            break;
        case 15:
            blinking();
            break;
        default :
            error();
            break;
    }
}

void LightMode::stop() {
    modus_changed = true;
    running = false;
}

void LightMode::setColor(uint8_t red, uint8_t green, uint8_t blue) {
    this->red = red;
    this->green = green;
    this->blue = blue;
}

String LightMode::toString() {
    String data = String(this->mode);
    data += "&";
    data += String(this->red);
    data += "&";
    data += String(this->green);
    data += "&";
    data += String(this->blue);
    data += "&";
    data += String(this->wait);
    data += "&";
    data += String(this->brightness);

    return data;
}

trans LightMode::getDataForTransfer() {
    trans translate{this->mode, this->red, this->green, this -> blue,
                     this->wait, this->brightness};
    return translate;
}