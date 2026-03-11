#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define BUTTON_PIN  5
#define LED_PIN     4

// Durasi dalam milidetik
const unsigned long ACTIVE_DURATION = 10000; // 10 detik
const unsigned long IDLE_DURATION   = 15000; // 15 detik
const unsigned long DEBOUNCE_DELAY  = 250;   // Debounce tombol

enum State { MODE_ACTIVE, MODE_IDLE };
State currentState = MODE_ACTIVE;

unsigned long lastStateChange = 0;
unsigned long lastDebounceTime = 0;
volatile bool buttonPressed = false;
bool ledStatus = false;

// Fungsi Interrupt dengan Debouncing
void IRAM_ATTR handleButton() {
    unsigned long currentTime = millis();
    if (currentTime - lastDebounceTime > DEBOUNCE_DELAY) {
        buttonPressed = true;
        lastDebounceTime = currentTime;
    }
}

void updateOLED(String mode, String ledState, int timeLeft) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    
    display.setCursor(0, 0);
    display.println("ESP32 CONTROLLER");
    display.drawLine(0, 10, 128, 10, SSD1306_WHITE);
    
    display.setCursor(0, 20);
    display.print("MODE: ");
    display.setTextSize(2);
    display.setCursor(0, 30);
    display.println(mode);
    
    display.setTextSize(1);
    display.setCursor(0, 50);
    display.print("LED: ");
    display.print(ledState);
    
    display.setCursor(80, 50);
    display.print("T-");
    display.print(timeLeft);
    display.print("s");
    
    display.display();
}

void setup() {
    Serial.begin(115200);
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(LED_PIN, OUTPUT);

    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
        Serial.println(F("SSD1306 allocation failed"));
        for(;;);
    }
    
    attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), handleButton, FALLING);
    lastStateChange = millis();
}

void loop() {
    unsigned long currentTime = millis();
    int timeLeft;

    if (currentState == MODE_ACTIVE) {
        timeLeft = (ACTIVE_DURATION - (currentTime - lastStateChange)) / 1000;
        
        // Logika Tombol di Mode Active
        if (buttonPressed) {
            ledStatus = !ledStatus; // Toggle LED
            digitalWrite(LED_PIN, ledStatus);
            buttonPressed = false;
        }

        updateOLED("ACTIVE", ledStatus ? "ON" : "OFF", timeLeft);

        // Cek perpindahan ke IDLE
        if (currentTime - lastStateChange >= ACTIVE_DURATION) {
            currentState = MODE_IDLE;
            lastStateChange = currentTime;
            ledStatus = true; // LED menyala terus saat IDLE
            digitalWrite(LED_PIN, HIGH);
            buttonPressed = false; // Reset klik yang mungkin masuk
        }
    } 
    else if (currentState == MODE_IDLE) {
        timeLeft = (IDLE_DURATION - (currentTime - lastStateChange)) / 1000;
        
        // Abaikan input tombol
        buttonPressed = false; 
        digitalWrite(LED_PIN, HIGH); // Pastikan tetap ON

        updateOLED("IDLE", "ALWAYS ON", timeLeft);

        // Cek perpindahan kembali ke ACTIVE
        if (currentTime - lastStateChange >= IDLE_DURATION) {
            currentState = MODE_ACTIVE;
            lastStateChange = currentTime;
            ledStatus = false; // Default awal ACTIVE adalah OFF (bisa diubah)
            digitalWrite(LED_PIN, LOW);
        }
    }
}