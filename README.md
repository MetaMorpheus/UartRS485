# UartRS485 for ESP32

Interrupt-driven RS485 UART receiver with message buffering, parsing, and classification for ESP32 (Arduino core).

## 📦 Features

- Non-blocking RX using UART interrupts
- Double-buffered message reception
- Custom message terminator (default `#`)
- Simple format: `classifier|key|value#` (e.g., `1|58|-20#`)
- External handler callbacks per classifier
- Works with all ESP32 UARTs (UART0, UART1, UART2)
- Test injection support for emulated messages

## 📐 Message Format

Messages must end with `#` and be structured like:

```
<classifier>|<key>|<value>#
```

Example:

```
1|5800|-20#
```

## 🚀 Quick Start

```cpp
#include <UartRS485.h>

UartRS485 uart1(UART_NUM_1);

void handleTemp(int key, int value) {
  Serial.printf("[Temp] Key: %d, Value: %d\n", key, value);
}

void setup() {
  Serial.begin(115200);
  uart1.begin(9600);
  uart1.registerHandler(1, handleTemp);
  uart1.inject("1|58|-20#");
}

void loop() {
  uart1.dispatch();
}
```

## 📁 Installation

###Arduino IDE

- Clone or download this repo as a `.zip`
- Place it in your `Arduino/libraries` folder
- Restart Arduino IDE
- See example under `File → Examples → UartRS485 → BasicUsage`

###PlatformIO

Add to `platformio.ini`:

```ini
lib_deps =
    https://github.com/MetaMorpheus/UartRS485.git
```

## 📚 API

```cpp
UartRS485(uart_port_t port);
void begin(uint32_t baud, int8_t rxPin = -1, int8_t txPin = -1);
bool available();
ParsedMessage read();
void dispatch();
bool registerHandler(int classifier, UartRS485_Handler handler);
void inject(const char* message);
```

## 🧩 Dependencies

- ESP32 Arduino core
- UART hardware (RS485 RX expected)

## 🛠 License

MIT License

## 🤝 Contributing

PRs welcome! If you find this useful — star the repo, fork it, or open an issue 🙌

---

## ⚙️ `library.json` (PlatformIO)

```json
{
  "name": "UartRS485",
  "version": "1.0.0",
  "description": "Interrupt-driven RS485 UART receiver with classification for ESP32 (Arduino).",
  "keywords": ["uart", "rs485", "esp32", "interrupt", "parser"],
  "authors": [
    {
      "name": "Andrew Timofeev",
      "email": "andrew.timofeev@gmail.com",
      "url": "https://github.com/MetaMorpheus",
      "maintainer": true
    }
  ],
  "license": "MIT",
  "repository": {
    "type": "git",
    "url": "https://github.com/MetaMorpheus/UartRS485.git"
  },
  "frameworks": ["arduino"],
  "platforms": ["espressif32"]
}
```
