#include <WiFi.h>
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

// TEST MODES
// 1 → Boot test
// 2 → Button test
// 3 → WiFi test
// 4 → WiFi + Button
// 5 → Speaker "hello"
// 6 → Full: WiFi + Button → Speaker



// ===== CHANGE THIS =====
#define TEST_MODE 1

// ===== CONFIG =====
#define BUTTON_PIN 13 // CHANGE IF NEEDED

// HALLIGAN WIFI - CHANGE IF NEEDED
const char *ssid = "tufts_eecs";
const char *password = "foundedin1883";

// ===== SPEAKER (I2S) =====
// (adjust pins if needed)
#define I2S_BCLK 26
#define I2S_LRC  25
#define I2S_DOUT 22

// ===== GLOBAL =====
bool lastButtonState = HIGH;

// ===== FUNCTION DECLARATIONS =====
void setupSpeaker();
void playHello();

// ================= SETUP =================
void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  Serial.begin(115200);
  delay(2000);

#if TEST_MODE == 1
  Serial.println("TEST 1: BOOT");
#endif

#if TEST_MODE == 2 || TEST_MODE == 4 || TEST_MODE == 6
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  Serial.println("Button initialized");
#endif

#if TEST_MODE == 3 || TEST_MODE == 4 || TEST_MODE == 6
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);

  int tries = 0;
  while (WiFi.status() != WL_CONNECTED && tries < 20) {
    delay(500);
    Serial.print(".");
    tries++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi connected");
    Serial.print("IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nWiFi FAILED");
  }
#endif

#if TEST_MODE == 5 || TEST_MODE == 6
  setupSpeaker();
#endif

#if TEST_MODE == 5
  Serial.println("Playing HELLO...");
  playHello();
#endif
}

// ================= LOOP =================
void loop() {

#if TEST_MODE == 1
  Serial.println("Running...");
  delay(1000);
#endif

#if TEST_MODE == 2
  Serial.println(digitalRead(BUTTON_PIN));
  delay(300);
#endif

#if TEST_MODE == 3
  Serial.println("WiFi OK");
  delay(2000);
#endif

#if TEST_MODE == 4
  bool current = digitalRead(BUTTON_PIN);

  if (lastButtonState == HIGH && current == LOW) {
    Serial.println("BUTTON PRESSED");
  }

  lastButtonState = current;
  delay(50);
#endif

#if TEST_MODE == 6
  bool current = digitalRead(BUTTON_PIN);

  if (lastButtonState == HIGH && current == LOW) {
    Serial.println("BUTTON PRESSED → PLAY HELLO");
    playHello();
  }

  lastButtonState = current;
  delay(50);
#endif
}

// ================= SPEAKER SETUP =================
#include "driver/i2s.h"

void setupSpeaker() {
  Serial.println("Initializing speaker...");

  i2s_config_t i2s_config = {
    .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
    .sample_rate = 16000,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
    .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
    .communication_format = I2S_COMM_FORMAT_I2S,
    .intr_alloc_flags = 0,
    .dma_buf_count = 8,
    .dma_buf_len = 64,
    .use_apll = false
  };

  i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_BCLK,
    .ws_io_num = I2S_LRC,
    .data_out_num = I2S_DOUT,
    .data_in_num = I2S_PIN_NO_CHANGE
  };

  i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_NUM_0, &pin_config);
}

// ================= SIMPLE HELLO SOUND =================
void playHello() {
  // simple tone (not real speech yet)
  const int samples = 16000;
  int16_t buffer[samples];

  for (int i = 0; i < samples; i++) {
    buffer[i] = 10000 * sin(2 * PI * 440 * i / 16000); // 440 Hz tone
  }

  size_t bytes_written;
  i2s_write(I2S_NUM_0, buffer, sizeof(buffer), &bytes_written, portMAX_DELAY);
}
