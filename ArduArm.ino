#include "ArduArm.h"

node n;

void setup() {
  Serial.begin(57600);

  do {
    n.begin("SIB", "abcd1234", 10010, 192, 168, 43, 52);
  } while (n.getState() != 0);

  n.join();

  Triple t;
  strcpy(t.subject, "http://ns#a");
  strcpy(t.predicate, "http://ns#b");
  strcpy(t.object, "http://ns#c");
  n.rdfInsert(t);
  n.leave();

  if (n.getState() == 0) Serial.println("ok");
  else Serial.println("nok");

  Serial.println("finished");
  while (1);
}

void loop() {

}

