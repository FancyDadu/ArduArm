#include "ArduArm.h"

node n;

void setup() {
  Serial.begin(57600);

  do {
    n.begin("GATEWAY", "abcd1234", 10010, 192, 168,43, 52);
  } while (n.getState() != 0);

  Triple info;
  info.type = 'R';
  info.fault='0';
  strcpy(info.data,"null");
  n.updateTask(info);

}

void loop() {

}

