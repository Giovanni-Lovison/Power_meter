#include <PZEM004Tv30.h>
#include <SoftwareSerial.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>


#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


SoftwareSerial pzemSerial(2, 3); // RX, TX
PZEM004Tv30 pzem(pzemSerial);


float voltage = 0.0;
float power = 0.0;


unsigned long lastUpdate = 0;
const unsigned long UPDATE_INTERVAL = 250;


int consecutiveErrors = 0;
bool isConnected = false;


void setup() {
    Serial.begin(9600);
  
    Wire.begin();
    Wire.setClock(100000);
  
    if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
        Serial.println(F("Errore inizializzazione display SSD1306"));

    } else {
        Serial.println("Display OLED 0.91\" inizializzato");
    
        // Pulizia completa del buffer
        display.clearDisplay();
        display.fillScreen(SSD1306_BLACK);
        display.display();
        delay(100);
    
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
        display.setCursor(25, 8);
        display.println("PZEM");
        display.setCursor(20, 20);
        display.println("READY");
        display.display();
  }
  
  Serial.println("=== PZEM Power Monitor ===");
  Serial.println("Aggiornamento ogni 250ms");
  
  delay(2000);
}

void loop() {
    if (millis() - lastUpdate >= UPDATE_INTERVAL) {
        lastUpdate = millis();
    
        float v = pzem.voltage();
        delay(50);
        float p = pzem.power();
    
        if (!isnan(v) && !isnan(p) && v >= 0 && p >= 0) {
            voltage = v;
            power = p;
      
            if (consecutiveErrors > 0) {
                Serial.println("✓ Connessione ripristinata!");
                consecutiveErrors = 0;
            }

        isConnected = true;
        displayPower();
      
        Serial.print("V: ");
        Serial.print(voltage, 1);
        Serial.print("V  |  P: ");
        Serial.print(power, 1);
        Serial.println("W");
      
    } else {
        consecutiveErrors++;
        isConnected = false;
      
        displayError();
      
        Serial.print("✗ Errore lettura #");
        Serial.println(consecutiveErrors);
      
        if (consecutiveErrors >= 15) {
            Serial.println(">>> TROPPI ERRORI - RESET <<<");
            consecutiveErrors = 0;
            delay(1000);
            }
        }
    }
}

void displayPower() {
    display.clearDisplay();
    display.fillScreen(SSD1306_BLACK);
  
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
  
    String powerStr = String((int)power) + "W";
    int16_t x1, y1;
    uint16_t w, h;
    display.getTextBounds(powerStr, 0, 0, &x1, &y1, &w, &h);
  
    int x = (SCREEN_WIDTH - w) / 2;
    int y = (SCREEN_HEIGHT - h) / 2 - 2;
  
    display.setCursor(x, y);
    display.print((int)power);
    display.print("W");
  
    if (isConnected) {
        display.setTextSize(1);
        display.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
        display.setCursor(120, 0);
        display.print("*");
    }
  
    display.display();
}

void displayError() {
    display.clearDisplay();
    display.fillScreen(SSD1306_BLACK);
  
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE, SSD1306_BLACK);
  
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
