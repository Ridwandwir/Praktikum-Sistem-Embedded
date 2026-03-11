#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define BUTTON_PIN  PB1
#define LED_PIN     PC13

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

enum State { ACTIVE, IDLE };
State currentState = ACTIVE;

unsigned long previousMillis = 0;
int timerCount = 10; 
bool isLedOn = false; 
bool lastButtonReading = HIGH;

void updateDisplay(String mode, int timer, bool ledStatus, bool bIsPressed) {
    display.clearDisplay();
    
    // Header Judul "STM32"
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(48, 0); // Di tengah atas
    display.print("STM32");
    display.drawFastHLine(0, 10, 128, SSD1306_WHITE); // Garis pemisah
    
    // Status Mode
    display.setCursor(0, 15);
    display.print("MODE: ");
    display.print(mode);
    
    // Timer Besar di Tengah
    display.setCursor(45, 28);
    display.setTextSize(3);
    if(timer < 10) display.print("0");
    display.print(timer);
    
    // Footer Status
    display.setTextSize(1);
    display.drawFastHLine(0, 52, 128, SSD1306_WHITE); // Garis bawah
    display.setCursor(0, 56);
    display.print("LED: "); display.print(ledStatus ? "ON" : "OFF");
    display.print(" | BTN: ");
    
    // Logika tampilan status tombol
    if (currentState == IDLE) {
        display.print("LOCKED");
    } else {
        display.print(bIsPressed ? "REL" : "PRSD");
    }
    
    display.display();
}

void setup() {
    // Inisialisasi Pin
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(LED_PIN, OUTPUT);
    
    // Test LED saat mulai (Blink 1x)
    digitalWrite(LED_PIN, LOW); delay(300);
    digitalWrite(LED_PIN, HIGH);

    // Inisialisasi OLED
    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { 
        for(;;); 
    }
    display.clearDisplay();
}

void loop() {
    unsigned long currentMillis = millis();
    bool currentButtonReading = digitalRead(BUTTON_PIN);

    // 1. LOGIKA TIMER DAN PERPINDAHAN MODE
    if (currentMillis - previousMillis >= 1000) {
        previousMillis = currentMillis;
        timerCount--;

        if (timerCount < 0) {
            if (currentState == ACTIVE) {
                currentState = IDLE;
                timerCount = 15;
                isLedOn = true; // Mode IDLE: LED menyala terus
            } else {
                currentState = ACTIVE;
                timerCount = 10;
                isLedOn = false; // Kembali ke ACTIVE: LED mulai dari mati
            }
        }
    }

    // 2. LOGIKA TOMBOL (Hanya bekerja di Mode ACTIVE)
    if (currentState == ACTIVE) {
        // Deteksi saat tombol ditekan (Edge Trigger)
        if (currentButtonReading == LOW && lastButtonReading == HIGH) {
            isLedOn = !isLedOn; // Toggle status lampu
            delay(50); // Debounce sederhana
        }
    } else {
        // Paksa LED ON selama mode IDLE
        isLedOn = true;
    }
    lastButtonReading = currentButtonReading;

    // 3. KONTROL HARDWARE LED (Active Low PC13)
    if (isLedOn) {
        digitalWrite(LED_PIN, LOW);  // Menyala
    } else {
        digitalWrite(LED_PIN, HIGH); // Mati
    }

    // 4. UPDATE TAMPILAN LCD
    String modeLabel = (currentState == ACTIVE) ? "ACTIVE" : "IDLE";
    updateDisplay(modeLabel, timerCount, isLedOn, currentButtonReading);
}