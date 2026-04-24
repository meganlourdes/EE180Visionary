#include "esp_camera.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include "driver/i2s.h"
#include "board_config.h"
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

const char *ssid = "tufts_eecs";
const char *password = "foundedin1883";

// #define BUTTON_PIN 16 
#define I2S_BCLK 2
#define I2S_LRC  14
#define I2S_DOUT 13

#define I2S_PORT I2S_NUM_0
#define SAMPLE_RATE 16000

const char* audioURL = "http://10.5.9.65:8000/latest.wav";

bool initCameraOnly() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.frame_size = FRAMESIZE_QVGA;
  config.pixel_format = PIXFORMAT_JPEG;
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.jpeg_quality = 12;
  config.fb_count = 1;

  if (psramFound()) {
    config.jpeg_quality = 10;
    config.fb_count = 2;
    config.grab_mode = CAMERA_GRAB_LATEST;
  } else {
    config.frame_size = FRAMESIZE_QQVGA;
    config.fb_location = CAMERA_FB_IN_DRAM;
  }

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x\n", err);
    return false;
  }

  sensor_t *s = esp_camera_sensor_get();
  if (s && s->id.PID == OV3660_PID) {
    s->set_vflip(s, 1);
    s->set_brightness(s, 1);
    s->set_saturation(s, -2);
  }

  if (s) {
    s->set_framesize(s, FRAMESIZE_QVGA);
  }

  Serial.println("Camera init OK");
  return true;
}

void setupI2S() {
  i2s_config_t config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
    .sample_rate = SAMPLE_RATE,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
    .communication_format = I2S_COMM_FORMAT_STAND_I2S,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 8,
    .dma_buf_len = 256,
    .use_apll = false,
    .tx_desc_auto_clear = true,
    .fixed_mclk = 0
  };

  i2s_pin_config_t pins = {
    .bck_io_num = I2S_BCLK,
    .ws_io_num = I2S_LRC,
    .data_out_num = I2S_DOUT,
    .data_in_num = I2S_PIN_NO_CHANGE
  };

  esp_err_t err;

  err = i2s_driver_install(I2S_PORT, &config, 0, NULL);
  Serial.print("i2s_driver_install: ");
  Serial.println(err == ESP_OK ? "OK" : "FAILED");

  err = i2s_set_pin(I2S_PORT, &pins);
  Serial.print("i2s_set_pin: ");
  Serial.println(err == ESP_OK ? "OK" : "FAILED");

  err = i2s_set_clk(I2S_PORT, SAMPLE_RATE, I2S_BITS_PER_SAMPLE_16BIT, I2S_CHANNEL_STEREO);
  Serial.print("i2s_set_clk: ");
  Serial.println(err == ESP_OK ? "OK" : "FAILED");
}

void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  WiFi.setSleep(false);

  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void playWavFromURL(const char* url) {
  HTTPClient http;
  http.begin(url);

  int code = http.GET();
  Serial.print("HTTP code: ");
  Serial.println(code);

  if (code != 200) {
    http.end();
    return;
  }

  WiFiClient* stream = http.getStreamPtr();

  uint8_t header[44];
  if (stream->readBytes(header, 44) != 44) {
    Serial.println("Failed to read WAV header");
    http.end();
    return;
  }

  const int monoSamples = 256;
  int16_t monoBuffer[monoSamples];
  int16_t stereoBuffer[monoSamples * 2];
  size_t bytesWritten;

  while (http.connected() && (stream->available() || stream->connected())) {
    int bytesRead = stream->readBytes((uint8_t*)monoBuffer, sizeof(monoBuffer));

    if (bytesRead > 0) {
      int sampleCount = bytesRead / 2;

      for (int i = 0; i < sampleCount; i++) {
        stereoBuffer[2 * i] = monoBuffer[i];
        stereoBuffer[2 * i + 1] = monoBuffer[i];
      }

      i2s_write(
        I2S_PORT,
        stereoBuffer,
        sampleCount * 2 * sizeof(int16_t),
        &bytesWritten,
        portMAX_DELAY
      );
    } else {
      break;
    }
  }

  http.end();
  Serial.println("Playback done");
}

void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

  Serial.begin(115200);
  delay(1000);
  Serial.println("SERIAL TEST START");

  pinMode(16, INPUT_PULLUP);

  setupI2S();
  connectWiFi();
  initCameraOnly();

  Serial.println("Waiting for button press...");
}

void loop() {
  static bool lastButtonState = HIGH;
  bool currentButtonState = digitalRead(16);

  if (lastButtonState == HIGH && currentButtonState == LOW) {
    Serial.println("BUTTON_PRESSED");
    playWavFromURL(audioURL);
    delay(300);
  }

  lastButtonState = currentButtonState;
  delay(20);
}
