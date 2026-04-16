#include "esp_camera.h"
#include <WiFi.h>
#include "board_config.h"
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

const char *ssid = "tufts_eecs";
const char *password = "foundedin1883";

// const char *ssid = "Tufts_Guest";
// const char *password = "";

// #define BUTTON_PIN 2

bool lastButtonState = HIGH;

void startCameraServer();
void setupLedFlash();

void printWiFiStatus(wl_status_t status) {
  Serial.print("WiFi status code: ");
  Serial.println((int)status);

  switch (status) {
    case WL_IDLE_STATUS: Serial.println("Idle"); break;
    case WL_NO_SSID_AVAIL: Serial.println("SSID not found"); break;
    case WL_SCAN_COMPLETED: Serial.println("Scan completed"); break;
    case WL_CONNECTED: Serial.println("Connected"); break;
    case WL_CONNECT_FAILED: Serial.println("Connect failed"); break;
    case WL_CONNECTION_LOST: Serial.println("Connection lost"); break;
    case WL_DISCONNECTED: Serial.println("Disconnected"); break;
    default: Serial.println("Unknown status"); break;
  }
}

void scanNetworks() {
  Serial.println("Scanning for WiFi...");
  int n = WiFi.scanNetworks();
  if (n == 0) {
    Serial.println("No networks found");
    return;
  }

  for (int i = 0; i < n; i++) {
    Serial.print(i + 1);
    Serial.print(": ");
    Serial.print(WiFi.SSID(i));
    Serial.print(" | RSSI: ");
    Serial.print(WiFi.RSSI(i));
    Serial.print(" | Encryption: ");
    Serial.println((int)WiFi.encryptionType(i));
  }
}

void setup() { 
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

  pinMode(2, INPUT_PULLUP);

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
  config.frame_size = FRAMESIZE_UXGA;
  config.pixel_format = PIXFORMAT_JPEG;
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.jpeg_quality = 12;
  config.fb_count = 1;

  if (config.pixel_format == PIXFORMAT_JPEG) {
    if (psramFound()) {
      config.jpeg_quality = 10;
      config.fb_count = 2;
      config.grab_mode = CAMERA_GRAB_LATEST;
    } else {
      config.frame_size = FRAMESIZE_SVGA;
      config.fb_location = CAMERA_FB_IN_DRAM;
    }
  } else {
    config.frame_size = FRAMESIZE_240X240;
  }

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x\n", err);
    return;
  }

  sensor_t *s = esp_camera_sensor_get();
  if (s->id.PID == OV3660_PID) {
    s->set_vflip(s, 1);
    s->set_brightness(s, 1);
    s->set_saturation(s, -2);
  }

  if (config.pixel_format == PIXFORMAT_JPEG) {
    s->set_framesize(s, FRAMESIZE_QVGA);
  }

  WiFi.mode(WIFI_STA);
  WiFi.disconnect(true, true);
  delay(1000);

  scanNetworks();

  Serial.print("Connecting to: ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  // WiFi.begin(ssid);
  WiFi.setSleep(false);

  int tries = 0;
  while (WiFi.status() != WL_CONNECTED && tries < 30) {
    delay(1000);
    Serial.print(".");
    tries++;
  }
  Serial.println();

  printWiFiStatus(WiFi.status());

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Failed to connect to WiFi.");
    return;
  }

  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  startCameraServer();

  Serial.print("Camera Ready! Use 'http://");
  Serial.print(WiFi.localIP());
  Serial.println("' to connect");
}

void loop() {
  static bool lastButtonState = HIGH;
  bool currentButtonState = digitalRead(2);

  if (lastButtonState == HIGH && currentButtonState == LOW) {
    Serial.println("BUTTON_PRESSED");
  }

  if (lastButtonState == LOW && currentButtonState == HIGH) {
    Serial.println("BUTTON_RELEASED");
  }

  lastButtonState = currentButtonState;
  delay(50);
}