#include <Arduino.h>
#include <DEV_Config.h>
#include <EPD_7in5_V2.h>
#include <Adafruit_NeoPixel.h>

constexpr uint8_t  NEO_DATA_PIN = 4;       // any free GPIO, NOT used by E-Paper
constexpr uint16_t NEO_PIXELS   = 8;      // length of your strip

Adafruit_NeoPixel strip(NEO_PIXELS, NEO_DATA_PIN, NEO_GRBW + NEO_KHZ800);

/* ---  Put your bitmaps in PROGMEM  ---------------- */
#include "moonbit1.h"     //  const unsigned char bitmap1[] PROGMEM = { … }
#include "moonbit2.h"
#include "moonbit3.h"

/*  Array of image pointers for easy cycling          */
const unsigned char * const images[] = {
  bitmap1,
  bitmap2,
  bitmap3
};
const uint8_t NUM_IMAGES = sizeof(images) / sizeof(images[0]);

/*  How long you want each image to stay on-screen    */
const uint32_t DISPLAY_MS = 180UL * 1000UL;   // 30 s  (keep ≥180 s if you want to be extra-safe)

void setup() {
  Serial.begin(115200);
  DEV_Module_Init();          //  power & SPI pins
  strip.begin();

// pick a colour:  R, G, B, W  (0-255 each)
strip.fill(strip.Color(255, 255, 255, 255));   // pure white at full brightness
strip.show();                            // latch data
}

void loop() {
  static uint8_t idx = 0;     //  which image to show next

  /* -------- 1. Wake panel (full refresh mode) ----- */
  EPD_7IN5_V2_Init();         //  full init brings high-voltage up

  /* -------- 2. Optional clear for best contrast --- */
  EPD_7IN5_V2_Clear();
  delay(500);                 //  let the clear waveform finish

  /* -------- 3. Push the new bitmap --------------- */
  EPD_7IN5_V2_Display((UBYTE*)images[idx]);

  /* -------- 4. Sleep immediately after refresh ---- */
  EPD_7IN5_V2_Sleep();        //  high-voltage off, RAM on panel is static

  /* -------- 5. Keep panel asleep while we wait ---- */
  delay(DISPLAY_MS);          //  image stays visible; electronics are idle

  /* -------- 6. Advance to next bitmap ------------- */
  idx = (idx + 1) % NUM_IMAGES;
}
