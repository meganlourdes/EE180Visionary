#define BUTTON_PIN 13

void setup() {
  Serial.begin(115200);
  delay(1000);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  Serial.println("BOOT OK");
}

void loop() {
  Serial.println(digitalRead(BUTTON_PIN));
  delay(500);
}



// // #define BUTTON_PIN 13   // change if needed

// void setup() {
//   Serial.begin(115200);
//   pinMode(2, INPUT_PULLUP);  // uses internal pull-up resistor
// }

// void loop() {
//   int buttonState = digitalRead(2);

//   if (buttonState == LOW) {
//     Serial.println("Button Pressed");
//   } else {
//     Serial.println("Not Pressed");
//   }

//   delay(300);
// }




// #include <WiFi.h>

// #define BUTTON_PIN 2

// const char *ssid = "tufts_eecs";
// const char *password = "foundedin1883";

// bool lastButtonState = HIGH;

// void setup() {
//   Serial.begin(115200);
  
//   delay(1000);

//   pinMode(BUTTON_PIN, INPUT_PULLUP);
//   delay(50);
//   lastButtonState = digitalRead(BUTTON_PIN);

//   Serial.println("Starting...");
//   Serial.println("Connecting to WiFi...");

//   WiFi.mode(WIFI_STA);
//   WiFi.setSleep(false);
//   WiFi.begin(ssid, password);

//   int tries = 0;
//   while (WiFi.status() != WL_CONNECTED && tries < 30) {
//     delay(500);
//     Serial.print(".");
//     tries++;
//   }
//   Serial.println();

//   if (WiFi.status() == WL_CONNECTED) {
//     Serial.println("WiFi connected");
//     Serial.print("IP address: ");
//     Serial.println(WiFi.localIP());
//   } else {
//     Serial.println("WiFi connection failed");
//   }

//   Serial.println("Ready for button press");
// }

// void loop() {
//   bool currentButtonState = digitalRead(BUTTON_PIN);

//   if (lastButtonState == HIGH && currentButtonState == LOW) {
//     Serial.println("BUTTON_PRESSED");
//   }

//   if (lastButtonState == LOW && currentButtonState == HIGH) {
//     Serial.println("BUTTON_RELEASED");
//   }

//   lastButtonState = currentButtonState;
//   delay(50);
// }