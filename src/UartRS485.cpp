#include "UartRS485.h"

UartRS485::UartRS485(uart_port_t port) : _port(port) {
    active_buf = buffer1;
    ready_buf = nullptr;
    active_pos = 0;
    handler_count = 0;
}

void UartRS485::begin(uint32_t baud, int8_t rxPin, int8_t txPin) {
    uart_config_t uart_config = {
        .baud_rate = (int)baud,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };

    uart_param_config(_port, &uart_config);

    // Если пины не заданы — использовать дефолтные
    if (rxPin < 0 || txPin < 0) {
        switch (_port) {
            case UART_NUM_0: rxPin = 3; txPin = 1; break;
            case UART_NUM_1: rxPin = 9; txPin = 10; break;
            case UART_NUM_2: rxPin = 16; txPin = 17; break;
        }
    }

    uart_set_pin(_port, txPin, rxPin, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(_port, BUF_SIZE, 0, 0, NULL, 0);

    uart_disable_intr_mask(_port, 0xFFFFFFFF);
    uart_clear_intr_status(_port, 0xFFFFFFFF);
    uart_enable_intr_mask(_port, UART_RXFIFO_FULL_INT_ENA_M);

    esp_intr_alloc(uart_periph_signal[_port].irq, ESP_INTR_FLAG_IRAM, onUartISR, this, NULL);
}

void IRAM_ATTR UartRS485::onUartISR(void* arg) {
    UartRS485* self = static_cast<UartRS485*>(arg);
    uart_dev_t* uart = nullptr;

    switch (self->_port) {
        case UART_NUM_0: uart = &UART0; break;
        case UART_NUM_1: uart = &UART1; break;
        case UART_NUM_2: uart = &UART2; break;
    }

    if (!uart) return;

    while (uart->status.rxfifo_cnt) {
        uint8_t c = uart->fifo.rw_byte;
        self->handleByte(c);
    }

    uart->int_clr.rxfifo_full = 1;
}

void IRAM_ATTR UartRS485::handleByte(uint8_t c) {
    if (c == CUSTOM_EOL) {
        if (ready_buf == nullptr) {
            if (active_buf == buffer1) {
                ready_buf = buffer1;
                active_buf = buffer2;
            } else {
                ready_buf = buffer2;
                active_buf = buffer1;
            }
            active_pos = 0;
        } else {
            active_pos = 0;
        }
    } else {
        if (active_pos < BUF_SIZE) {
            active_buf[active_pos++] = c;
        } else {
            active_pos = 0;
        }
    }
}

bool UartRS485::available() {
    return ready_buf != nullptr;
}

ParsedMessage UartRS485::read() {
    ParsedMessage result = {0, 0, 0, false};
    if (ready_buf == nullptr) return result;

    volatile uint8_t* msg_ptr = ready_buf;
    size_t len = 0;
    while (len < BUF_SIZE && msg_ptr[len] != CUSTOM_EOL) len++;
    if (len >= BUF_SIZE) return result;

    ((volatile uint8_t*)msg_ptr)[len] = 0;

    char* saveptr;
    char* token = strtok_r((char*)msg_ptr, "|", &saveptr);
    if (token) result.classifier = atoi(token); else return result;
    token = strtok_r(NULL, "|", &saveptr);
    if (token) result.key = atoi(token); else return result;
    token = strtok_r(NULL, "#", &saveptr);
    if (token) result.value = atoi(token); else return result;

    result.valid = true;
    ready_buf = nullptr;
    return result;
}

void UartRS485::inject(const char* testMessage) {
    for (size_t i = 0; testMessage[i]; ++i) {
        handleByte(testMessage[i]);
    }
    handleByte(CUSTOM_EOL);
}

bool UartRS485::registerHandler(int classifier, UartRS485_Handler handler) {
    if (handler_count >= MAX_HANDLERS) return false;

    for (size_t i = 0; i < handler_count; ++i) {
        if (handlers[i].classifier == classifier) {
            handlers[i].handler = handler;
            return true;
        }
    }

    handlers[handler_count++] = { classifier, handler };
    return true;
}

void UartRS485::dispatch() {
    if (!available()) return;
    ParsedMessage msg = read();
    if (!msg.valid) return;

    for (size_t i = 0; i < handler_count; ++i) {
        if (handlers[i].classifier == msg.classifier) {
            handlers[i].handler(msg.key, msg.value);
            return;
        }
    }

    Serial.printf("[UartRS485] Нет обработчика для %d\n", msg.classifier);
}
