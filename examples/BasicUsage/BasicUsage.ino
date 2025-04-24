#include <UartRS485.h>

// RX: GPIO16, TX: GPIO17 (на TX можно повесить пустой пин)
UartRS485 uart2(Serial2, 16, 17);

void handleTemp(int key, int value) {
  Serial.printf("[Temp] Key: %d, Value: %d\n", key, value);
}

void handleStatus(int key, int value) {
  Serial.printf("[Status] Code: %d, Detail: %d\n", key, value);
}

void setup() {
  Serial.begin(115200);
  uart2.begin(9600);
  uart2.registerHandler(1, handleTemp);   // classifier 1
  uart2.registerHandler(2, handleStatus); // classifier 2

  uart2.inject("1|5800|-20#");
  uart2.inject("2|100|1#");
}

void loop() {
  uart2.dispatch();
}
