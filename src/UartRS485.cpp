#include "UartRS485.h"

UartRS485::UartRS485(HardwareSerial& serial, int8_t rxPin, int8_t txPin)
    : _serial(serial), _rxPin(rxPin), _txPin(txPin) {}

void UartRS485::begin(uint32_t baud) {
    _serial.begin(baud, SERIAL_8N1, _rxPin, _txPin);
}

bool UartRS485::registerHandler(int classifier, UartRS485_Handler handler) {
    if (_handlerCount >= MAX_HANDLERS) return false;
    _handlers[_handlerCount++] = { classifier, handler };
    return true;
}

void UartRS485::inject(const char* message) {
    while (*message) {
        char c = *message++;
        if (c == EOL) {
            _buffer[_pos] = '\0';
            _ready = true;
        } else if (_pos < BUFFER_SIZE - 1) {
            _buffer[_pos++] = c;
        }
    }
}

void UartRS485::dispatch() {
    while (_serial.available()) {
        char c = _serial.read();
        if (c == EOL) {
            _buffer[_pos] = '\0';
            _ready = true;
        } else if (_pos < BUFFER_SIZE - 1) {
            _buffer[_pos++] = c;
        }
    }

    if (_ready) {
        parseBuffer();
        _pos = 0;
        _ready = false;
    }
}

void UartRS485::parseBuffer() {
    int classifier, key, value;
    if (sscanf(_buffer, "%d|%d|%d", &classifier, &key, &value) == 3) {
        for (size_t i = 0; i < _handlerCount; ++i) {
            if (_handlers[i].classifier == classifier) {
                _handlers[i].callback(key, value);
                return;
            }
        }
    }
}
