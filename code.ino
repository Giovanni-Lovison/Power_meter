#include <PZEM004Tv30.h>
#include <SoftwareSerial.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

#define UPDATE_INTERVAL 200
#define DISPLAY_INTERVAL 200
#define MAX_ERRORS 10
#define WATCHDOG_TIMEOUT 1800000
#define SERIAL_CLEAR_INTERVAL 5000

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
SoftwareSerial pzemSerial(2, 3);
PZEM004Tv30 pzem(pzemSerial);

float currentPower = 0.0;
float lastValidPower = 0.0;
unsigned long lastUpdate = 0;
unsigned long lastDisplay = 0;
unsigned long lastValidReading = 0;
unsigned long lastSerialClear = 0;
int consecutiveErrors = 0;
bool isConnected = false;
bool displayNeedsUpdate = true;

void setup() {
    Serial.begin(9600);
    Serial.println(F("=== PZEM Power Monitor v2.0 ==="));
    
    Wire.begin();
    Wire.setClock(100000);
    
    if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
        Serial.println(F("Errore display SSD1306"));
        while(1);
    }
    
    Serial.println(F("Display OLED inizializzato"));
    
    showStartupScreen();
    
    lastUpdate = millis();
    lastDisplay = millis();
    lastValidReading = millis();
    lastSerialClear = millis();
    
    Serial.println(F("Sistema pronto - Lettura ogni 200ms"));
    delay(2000);
}

void loop() {
    unsigned long currentTime = millis();
    
    if (currentTime - lastSerialClear >= SERIAL_CLEAR_INTERVAL) {
        clearSerialBuffers();
        lastSerialClear = currentTime;
    }
    
    if (currentTime - lastUpdate >= UPDATE_INTERVAL) {
        readPowerData();
        lastUpdate = currentTime;
    }
    
    if (currentTime - lastDisplay >= DISPLAY_INTERVAL && displayNeedsUpdate) {
        updateDisplay();
        lastDisplay = currentTime;
        displayNeedsUpdate = false;
    }
    
    checkWatchdog(currentTime);
    delay(10);
}

void readPowerData() {
    float power = pzem.power();
    
    if (!isnan(power) && power >= 0 && power <= 25000) {
        currentPower = power;
        lastValidPower = power;
        lastValidReading = millis();
        
        if (consecutiveErrors > 0) {
            Serial.println(F("✓ Connessione ripristinata"));
            consecutiveErrors = 0;
        }
        
        isConnected = true;
        displayNeedsUpdate = true;
        
        static int logCounter = 0;
        if (++logCounter >= 10) {
            Serial.print(F("Potenza: "));
            Serial.print(currentPower, 1);
            Serial.println(F("W"));
            logCounter = 0;
        }
        
    } else {
        handleReadingError();
    }
}

void handleReadingError() {
    consecutiveErrors++;
    isConnected = false;
    displayNeedsUpdate = true;
    
    Serial.print(F("✗ Errore lettura #"));
    Serial.println(consecutiveErrors);
    
    if (consecutiveErrors >= MAX_ERRORS) {
        Serial.println(F(">>> TROPPI ERRORI - RESET COMUNICAZIONE <<<"));
        resetCommunication();
        consecutiveErrors = 0;
    }
}

void resetCommunication() {
    clearSerialBuffers();
    
    delay(1000);
    
    pzemSerial.end();
    delay(100);
    pzemSerial.begin(9600);
    delay(500);
    
    Serial.println(F("Comunicazione reinizializzata"));
}

void clearSerialBuffers() {
    while(pzemSerial.available()) {
        pzemSerial.read();
    }
    
    while(Serial.available()) {
        Serial.read();
    }
}

void checkWatchdog(unsigned long currentTime) {
    if (currentTime - lastValidReading > WATCHDOG_TIMEOUT) {
        Serial.println(F(">>> RESET AUTOMATICO 30 MINUTI <<<"));
        
        resetCommunication();
        
        lastValidReading = currentTime;
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
