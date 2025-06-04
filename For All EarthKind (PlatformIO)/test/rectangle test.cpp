#include <Arduino.h>
#include <DEV_Config.h>
#include <EPD_7in5_V2.h>
#include <GUI_Paint.h>

void setup() {
  Serial.begin(115200);
  DEV_Module_Init();

  // 1) Full-screen init & white-clear
  EPD_7IN5_V2_Init();
  EPD_7IN5_V2_Clear();
  delay(500);

  // 2) Allocate a Paint buffer for the entire 800×480
  uint32_t bufSize = (EPD_7IN5_V2_WIDTH / 8) * EPD_7IN5_V2_HEIGHT;
  UBYTE *buffer = (UBYTE *)malloc(bufSize);
  if (!buffer) {
    Serial.println("OOM");
    while (1);
  }

  // 3) Initialize Paint on that buffer
  Paint_NewImage(buffer,
                 EPD_7IN5_V2_WIDTH,
                 EPD_7IN5_V2_HEIGHT,
                 0,    // rotation
                 WHITE);
  Paint_SelectImage(buffer);
  Paint_Clear(WHITE);

  // 4) Draw a filled black rectangle
  Paint_DrawRectangle(20, 20,
                      EPD_7IN5_V2_WIDTH  - 20,
                      EPD_7IN5_V2_HEIGHT - 20,
                      BLACK,
                      DOT_PIXEL_1X1,
                      DRAW_FILL_FULL);

  // 5) **Push your Paint buffer** with WritePicture
  EPD_7IN5_V2_WritePicture(buffer);
  delay(5000);  // wait for the e-ink waveform to finish

  // 6) Sleep so you’re not leaving the panel at high voltage
  // EPD_7IN5_V2_Sleep();

  // 7) (optional) free(buffer);
}

void loop() {
  // nothing
}
