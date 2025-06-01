# PZEM-004T Power Monitor with OLED Display

A real-time power monitoring system using PZEM-004T sensor and Arduino with 0.91" OLED display.

## 📊 Features

- **Real-time Power Monitoring**: Displays current power consumption in Watts
- **Fast Updates**: 250ms refresh rate for responsive monitoring
- **Serial Output**: Shows both voltage and power readings via serial monitor
- **Error Handling**: Visual and serial error reporting with automatic recovery
- **Compact Display**: Optimized for 0.91" SSD1306 OLED displays
- **Connection Status**: Visual indicator for sensor connectivity

## 🛠️ Hardware Requirements

### Components
- Arduino (Uno, Nano, ESP32, etc.)
- PZEM-004T v3.0 Power Meter
- 0.91" SSD1306 OLED Display (128x32, I2C)
- Jumper wires

### PZEM-004T Specifications
- **Input Voltage**: AC 80-260V
- **Current Range**: 0-100A
- **Power Range**: 0-23kW
- **Communication**: TTL Serial (9600 baud)

### OLED Display Specifications
- **Size**: 0.91 inch
- **Resolution**: 128x32 pixels
- **Driver**: SSD1306
- **Interface**: I2C
- **Voltage**: 3.3V - 5V

## 🔌 Wiring Diagram

### PZEM-004T to Arduino
```
PZEM-004T    Arduino
---------    -------
TX      →    Pin 2 (RX)
RX      →    Pin 3 (TX)
GND     →    GND
```

### OLED Display to Arduino
```
OLED Display    Arduino
------------    -------
VCC        →    5V (or 3.3V)
GND        →    GND
SCL        →    A5 (Uno) / 21 (Mega)
SDA        →    A4 (Uno) / 20 (Mega)
```

### PZEM-004T AC Connection
```
⚠️ WARNING: High voltage AC connection required!

AC Line   →   PZEM Input Terminal
AC Load   →   PZEM Output Terminal
```

## 📚 Required Libraries

Install these libraries through Arduino IDE Library Manager:

```cpp
#include <PZEM004Tv30.h>     // by Jakub Mandula
#include <SoftwareSerial.h>  // Arduino Core
#include <Wire.h>            // Arduino Core
#include <Adafruit_GFX.h>    // by Adafruit
#include <Adafruit_SSD1306.h> // by Adafruit
```

## 🚀 Installation

1. **Clone this repository**:
   ```bash
   git clone https://github.com/Giovanni-Lovison/Power_meter.git
   ```

2. **Install required libraries** in Arduino IDE

3. **Wire the components** according to the wiring diagram

4. **Upload the code** to your Arduino

5. **Connect PZEM-004T to AC power source** (⚠️ Be careful with high voltage!)

## 💻 Usage

### Serial Monitor Output
Open serial monitor at **9600 baud** to see:
```
=== PZEM Power Monitor ===
Aggiornamento ogni 250ms
Display OLED 0.91" inizializzato
V: 230.2V  |  P: 150.3W
V: 230.1V  |  P: 148.7W
V: 229.9V  |  P: 152.1W
```

### OLED Display
The display shows:
- **Large centered power reading** in Watts
- **Connection indicator** (*) in top-right corner
- **Error messages** if communication fails

### Display States
- **Normal**: Shows power consumption (e.g., "150W")
- **Error**: Shows "ERROR" and "PZEM FAIL" messages
- **Startup**: Shows "PZEM" and "READY" during initialization

## ⚙️ Configuration

### Update Interval
Change the update frequency by modifying:
```cpp
const unsigned long UPDATE_INTERVAL = 250; // milliseconds
```

### Serial Debug
Enable/disable verbose serial output by commenting/uncommenting debug lines in the main loop.

### Display Settings
For different OLED displays, adjust:
```cpp
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32
#define SCREEN_ADDRESS 0x3C
```

## 🔧 Troubleshooting

### Common Issues

**Display shows random pixels**
- Check I2C wiring (SDA/SCL)
- Verify power supply voltage
- Try reducing I2C clock speed

**PZEM sensor not responding**
- Verify PZEM has AC power connected
- Check TX/RX wiring (may need to swap)
- Ensure PZEM-004T v3.0 compatibility

**Compilation errors**
- Install all required libraries
- Check Arduino board selection
- Verify library versions compatibility

### Error Messages

| Error | Cause | Solution |
|-------|-------|----------|
| `✗ Errore lettura #X` | PZEM communication failure | Check wiring and AC power |
| `Errore inizializzazione display` | OLED not detected | Verify I2C wiring and address |
| `>>> TROPPI ERRORI - RESET <<<` | Persistent communication issues | Check all connections |

## 📈 Performance

- **Update Rate**: 250ms (4 Hz)
- **Accuracy**: Depends on PZEM-004T specifications
- **Response Time**: Near real-time power monitoring
- **Memory Usage**: Optimized for Arduino Uno/Nano

## 🛡️ Safety Warnings

⚠️ **HIGH VOLTAGE WARNING**
- PZEM-004T connects to AC mains voltage
- Ensure proper electrical isolation
- Use appropriate safety measures
- Consider using a qualified electrician for AC connections

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

---

**Made with ❤️ for the Arduino community**
