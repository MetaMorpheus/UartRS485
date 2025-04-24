#pragma once

#include <Arduino.h>
#include <HardwareSerial.h>

typedef void (*UartRS485_Handler)(int key, int value);

class UartRS485 {
public:
    UartRS485(HardwareSerial& serial, int8_t rxPin = -1, int8_t txPin = -1);

    void begin(uint32_t baud);
    void dispatch();
    bool registerHandler(int classifier, UartRS485_Handler handler);
    void inject(const char* message);  // For test messages

private:
    static constexpr size_t BUFFER_SIZE = 128;
    static constexpr size_t MAX_HANDLERS = 10;
    static constexpr char EOL = '#';

    struct Handler {
        int classifier;
        UartRS485_Handler callback;
    };

    HardwareSerial& _serial;
    int8_t _rxPin;
    int8_t _txPin;

    char _buffer[BUFFER_SIZE];
    size_t _pos = 0;
    volatile bool _ready = false;

    Handler _handlers[MAX_HANDLERS];
    size_t _handlerCount = 0;

    void parseBuffer();
};
