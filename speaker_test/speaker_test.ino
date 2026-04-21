#include <Arduino.h>
#include "driver/i2s.h"
#include <math.h>

// ✅ YOUR ACTUAL WIRING
#define I2S_BCLK 2    // BCLK
#define I2S_LRC  14   // LRC (WS)
#define I2S_DOUT 13   // DIN

#define I2S_PORT I2S_NUM_0
#define SAMPLE_RATE 16000
#define AMPLITUDE 20000   // slightly reduced for stability

double phase = 0.0;

// Some musical notes
#define NOTE_C4 262.0
#define NOTE_D4 294.0
#define NOTE_E4 330.0
#define NOTE_G3 196.0

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

  i2s_driver_install(I2S_PORT, &config, 0, NULL);
  i2s_set_pin(I2S_PORT, &pins);
  i2s_set_clk(I2S_PORT, SAMPLE_RATE, I2S_BITS_PER_SAMPLE_16BIT, I2S_CHANNEL_STEREO);
}

// 🎵 Play a tone
void playTone(double frequency, int durationMs) {
  const int frames = 256;
  int16_t buffer[frames * 2];

  double phaseIncrement = 2.0 * PI * frequency / SAMPLE_RATE;

  int totalSamples = (SAMPLE_RATE * durationMs) / 1000;
  int samplesWritten = 0;

  while (samplesWritten < totalSamples) {
    int samplesThisChunk = min(frames, totalSamples - samplesWritten);

    for (int i = 0; i < samplesThisChunk; i++) {
      int16_t sample = (int16_t)(AMPLITUDE * sin(phase));

      phase += phaseIncrement;
      if (phase >= 2.0 * PI) phase -= 2.0 * PI;

      buffer[2 * i]     = sample; // left
      buffer[2 * i + 1] = sample; // right
    }

    size_t bytesWritten;
    i2s_write(
      I2S_PORT,
      buffer,
      samplesThisChunk * 2 * sizeof(int16_t),
      &bytesWritten,
      portMAX_DELAY
    );

    samplesWritten += samplesThisChunk;
  }
}

// 🔇 Silence between notes
void playSilence(int durationMs) {
  const int frames = 256;
  int16_t buffer[frames * 2] = {0};

  int totalSamples = (SAMPLE_RATE * durationMs) / 1000;
  int samplesWritten = 0;

  while (samplesWritten < totalSamples) {
    int samplesThisChunk = min(frames, totalSamples - samplesWritten);

    size_t bytesWritten;
    i2s_write(
      I2S_PORT,
      buffer,
      samplesThisChunk * 2 * sizeof(int16_t),
      &bytesWritten,
      portMAX_DELAY
    );

    samplesWritten += samplesThisChunk;
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("Playing musical tones...");
  setupI2S();
}

void loop() {
  // 🎶 Simple melody (low + musical)
  playTone(NOTE_G3, 400);
  playSilence(100);

  playTone(NOTE_C4, 400);
  playSilence(100);

  playTone(NOTE_E4, 400);
  playSilence(200);

  delay(1500);
}
