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
