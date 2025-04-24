#pragma once
#include <Arduino.h>
#include "driver/uart.h"

struct ParsedMessage {
    int classifier;
    int key;
    int value;
    bool valid;
};

typedef void (*UartRS485_Handler)(int key, int value);

class UartRS485 {
public:
    explicit UartRS485(uart_port_t port);

    void begin(uint32_t baud, int8_t rxPin = -1, int8_t txPin = -1);
    bool available();
    ParsedMessage read();
    void dispatch();
    void inject(const char* testMessage);
    bool registerHandler(int classifier, UartRS485_Handler handler);

private:
    static void IRAM_ATTR onUartISR(void* arg);
    void handleByte(uint8_t c);

    uart_port_t _port;

    static const int BUF_SIZE = 256;
    static const uint8_t CUSTOM_EOL = '#';
    static const int MAX_HANDLERS = 10;

    volatile uint8_t buffer1[BUF_SIZE];
    volatile uint8_t buffer2[BUF_SIZE];

    volatile uint8_t* active_buf;
    volatile uint8_t* ready_buf;
    volatile size_t active_pos;

    struct HandlerEntry {
        int classifier;
        UartRS485_Handler handler;
    };

    HandlerEntry handlers[MAX_HANDLERS];
    size_t handler_count;
};
