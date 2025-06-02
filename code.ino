#include <PZEM004Tv30.h>
#include <SoftwareSerial.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

#define UPDATE_INTERVAL 500    
#define DISPLAY_INTERVAL 100    
#define MAX_ERRORS 5           
#define WATCHDOG_TIMEOUT 900000 
#define SERIAL_CLEAR_INTERVAL 2000
#define DISPLAY_RESET_INTERVAL 300000

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
SoftwareSerial pzemSerial(2, 3);
PZEM004Tv30 pzem(pzemSerial);

float currentPower = 0.0;
float lastValidPower = 0.0;
unsigned long lastUpdate = 0;
unsigned long lastDisplay = 0;
unsigned long lastValidReading = 0;
unsigned long lastSerialClear = 0;
unsigned long lastResetTime = 0;
unsigned long lastDisplayReset = 0;
int consecutiveErrors = 0;
bool isConnected = false;
bool displayNeedsUpdate = true;

void setup() {
    Serial.begin(9600);
    Serial.println(F("=== PZEM Power Monitor v2.0 ==="));
    
    Wire.begin();
    Wire.setClock(100000);
    
    if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
        Serial.println(F("SSD1306 Display Error"));
        while(1);
    }
    
    Serial.println(F("OLED Display Initialized"));
    
    showStartupScreen();
    
    lastUpdate = millis();
    lastDisplay = millis();
    lastValidReading = millis();
    lastSerialClear = millis();
    lastResetTime = millis();
    lastDisplayReset = millis();
    
    Serial.print(F("Start timestamp: "));
    Serial.println(lastValidReading);
    
    Serial.println(F("System ready - Reading every 1000ms"));
    delay(2000);
}

void loop() {
    unsigned long currentTime = millis();
    
    if (currentTime - lastDisplayReset >= DISPLAY_RESET_INTERVAL) {
        Serial.println(F("Performing display reset..."));
        resetDisplay();
    }
    
    if (currentTime - lastSerialClear >= SERIAL_CLEAR_INTERVAL) {
        clearSerialBuffers();
        lastSerialClear = currentTime;
    }
    
    if (currentTime - lastUpdate >= UPDATE_INTERVAL) {
        readPowerData();
        lastUpdate = currentTime;
        displayNeedsUpdate = true;
    }
    
    if (currentTime - lastDisplay >= DISPLAY_INTERVAL) {
        updateDisplay();
        lastDisplay = currentTime;
    }
    
    checkWatchdog(currentTime);
    delay(5);
}

void readPowerData() {
    float power = pzem.power();
    
    if (!isnan(power) && power >= 0 && power <= 25000) {
        currentPower = power;
        lastValidPower = power;
        if (millis() - lastValidReading > 10000) {
            lastValidReading = millis();
        }
        
        if (consecutiveErrors > 0) {
            Serial.println(F("✓ Connection restored"));
            consecutiveErrors = 0;
        }
        
        isConnected = true;
        displayNeedsUpdate = true;
        

        unsigned long pzemResetIn = (WATCHDOG_TIMEOUT - (millis() - lastResetTime)) / 1000;
        unsigned long displayResetIn = (DISPLAY_RESET_INTERVAL - (millis() - lastDisplayReset)) / 1000;
        
        Serial.print(F("Power: "));
        Serial.print(currentPower, 1);
        Serial.print(F("W | PZEM reset in: "));
        Serial.print(pzemResetIn);
        Serial.print(F("s | Display reset in: "));
        Serial.print(displayResetIn);
        Serial.println(F("s"));
        
    } else {
        handleReadingError();
    }
}

void handleReadingError() {
    consecutiveErrors++;
    isConnected = false;
    displayNeedsUpdate = true;
    
    Serial.print(F("✗ Reading error #"));
    Serial.println(consecutiveErrors);
    
    if (consecutiveErrors >= MAX_ERRORS) {
        Serial.println(F(">>> TOO MANY ERRORS - RESETTING COMMUNICATION <<<"));
        resetCommunication();
        consecutiveErrors = 0;
    }
}

void resetCommunication() {
    Serial.print(F("Serial reset in progress - Time: "));
    Serial.print(millis() / 1000);
    Serial.println(F("s"));
    
    clearSerialBuffers();
    
    pzemSerial.end();
    delay(200);
    
    while(pzemSerial.available()) {
        pzemSerial.read();
    }
    
    pzemSerial.begin(9600);
    delay(300);
    
    Serial.println(F("Serial reset completed"));
}

void clearSerialBuffers() {
    while(pzemSerial.available()) {
        pzemSerial.read();
        delay(1);
    }
    
    while(Serial.available()) {
        Serial.read();
        delay(1);
    }
}

void checkWatchdog(unsigned long currentTime) {
    if (currentTime > WATCHDOG_TIMEOUT && 
        (currentTime - lastValidReading) > WATCHDOG_TIMEOUT) {
        
        unsigned long elapsedMinutes = (currentTime - lastResetTime) / 60000;
        Serial.print(F(">>> SCHEDULED SERIAL RESET - Time elapsed: "));
        Serial.print(elapsedMinutes);
        Serial.println(F(" minutes <<<"));
        
        resetCommunication();
        lastValidReading = currentTime;
        lastResetTime = currentTime;
        consecutiveErrors = 0;
    }
}

void updateDisplay() {
    if (isConnected) {
        showPowerReading();
    } else {
        showErrorScreen();
    }
}

void showPowerReading() {
    display.clearDisplay();
    
    display.setTextSize(3);
    display.setTextColor(SSD1306_WHITE);
    
    String powerStr = String((int)currentPower) + "W";
    int16_t x1, y1;
    uint16_t w, h;
    display.getTextBounds(powerStr, 0, 0, &x1, &y1, &w, &h);
    
    int x = (SCREEN_WIDTH - w) / 2;
    int y = (SCREEN_HEIGHT - h) / 2;
    
    display.setCursor(x, y);
    display.print((int)currentPower);
    display.print("W");
    
    display.display();
}

void showErrorScreen() {
    display.clearDisplay();
    
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    
    display.setCursor(35, 8);
    display.print("ERROR");
    
    display.setCursor(25, 20);
    display.print("PZEM FAIL");
    
    if ((millis() / 250) % 2) {
        display.setCursor(120, 0);
        display.print("!");
    }
    
    display.display();
}

void showStartupScreen() {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    
    display.setCursor(25, 8);
    display.println("PZEM");
    display.setCursor(20, 20);
    display.println("READY");
    
    display.display();
}

void resetDisplay() {
    Serial.println(F("Performing display reset..."));
    
    display.ssd1306_command(0xAE);
    delay(100);
    
    if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
        Serial.println(F("Display reset failed"));
        return;
    }
    
    display.clearDisplay();
    display.display();
    
    Serial.println(F("Display reset completed"));
    lastDisplayReset = millis();
    
    displayNeedsUpdate = true;
}
