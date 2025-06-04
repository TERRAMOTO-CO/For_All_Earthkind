/*
   ESP32-CAM  push-upload
   -- wakes → fires trigger → takes photo → POSTs to Mac → sleeps 30 s
   No SD card required.
*/
#include "esp_camera.h"
#include "WiFi.h"
#include "HTTPClient.h"
#include "driver/rtc_io.h"

#define CAMERA_MODEL_AI_THINKER
#include "camera_pins.h"

/* Wi-Fi credentials */
const char* WIFI_SSID = "KITBASH STUDIO";
const char* WIFI_PSK  = "********";

/* Receiver URL  (Mac’s IP) */
const char* POST_URL = "http://192.168.1.175:5050/upload";

/* Flash trigger pin (to UNO) */
constexpr gpio_num_t TRIGGER_PIN = GPIO_NUM_13;

/* Deep-sleep interval */
constexpr uint64_t SLEEP_US = 30ULL * 1'000'000ULL;

/* Disable on-board white LED */
constexpr gpio_num_t LED_ONBOARD = GPIO_NUM_4;

void setup()
{
  Serial.begin(115200);

  /* keep GPIO4 LOW */
  rtc_gpio_hold_dis(LED_ONBOARD);
  pinMode(LED_ONBOARD, OUTPUT);
  digitalWrite(LED_ONBOARD, LOW);
  rtc_gpio_hold_en(LED_ONBOARD);

  /* trigger pulse to UNO (3 ms) */
  pinMode(TRIGGER_PIN, OUTPUT);
  digitalWrite(TRIGGER_PIN, HIGH);
  delay(3);
  digitalWrite(TRIGGER_PIN, LOW);

  /* 25 ms for LEDs to settle */
  delay(25);

  /* camera init (UXGA, same as before) */
  camera_config_t cfg{};
  cfg.ledc_channel = LEDC_CHANNEL_0;
  cfg.ledc_timer   = LEDC_TIMER_0;
  cfg.pin_d0 = Y2_GPIO_NUM; cfg.pin_d1 = Y3_GPIO_NUM;
  cfg.pin_d2 = Y4_GPIO_NUM; cfg.pin_d3 = Y5_GPIO_NUM;
  cfg.pin_d4 = Y6_GPIO_NUM; cfg.pin_d5 = Y7_GPIO_NUM;
  cfg.pin_d6 = Y8_GPIO_NUM; cfg.pin_d7 = Y9_GPIO_NUM;
  cfg.pin_xclk = XCLK_GPIO_NUM;
  cfg.pin_pclk = PCLK_GPIO_NUM;
  cfg.pin_vsync = VSYNC_GPIO_NUM;
  cfg.pin_href  = HREF_GPIO_NUM;
  cfg.pin_sscb_sda = SIOD_GPIO_NUM;
  cfg.pin_sscb_scl = SIOC_GPIO_NUM;
  cfg.pin_pwdn = PWDN_GPIO_NUM;
  cfg.pin_reset = RESET_GPIO_NUM;
  cfg.xclk_freq_hz = 20'000'000;
  cfg.pixel_format = PIXFORMAT_JPEG;
  cfg.frame_size   = FRAMESIZE_UXGA;
  cfg.jpeg_quality = 10;
  cfg.fb_count     = 2;

  if (esp_camera_init(&cfg) != ESP_OK) {
    Serial.println("Camera init failed");
    sleepNow();
  }

  camera_fb_t *fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Capture failed");
    sleepNow();
  }

  /* connect Wi-Fi */
  WiFi.begin(WIFI_SSID, WIFI_PSK);
  uint32_t t0 = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - t0 < 8000)
    delay(100);

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Wi-Fi connect failed");
    esp_camera_fb_return(fb);
    sleepNow();
  }
  Serial.println("Wi-Fi connected");

  /* POST the JPEG */
  HTTPClient http;
  http.begin(POST_URL);
  http.addHeader("Content-Type", "image/jpeg");
  int rc = http.POST(fb->buf, fb->len);
  Serial.printf("HTTP rc=%d, bytes=%u\n", rc, fb->len);
  http.end();

  esp_camera_fb_return(fb);
  sleepNow();
}

void sleepNow() {
  WiFi.disconnect(true, true);
  Serial.println("Sleep");
  Serial.flush();
  esp_sleep_enable_timer_wakeup(SLEEP_US);
  esp_deep_sleep_start();
}

void loop() {}
