#include "ArduArm.h"
#include "meArm.h"
#include <Servo.h>

node n;
meArm arm;

#define COMM_INTERVAL 5000
#define POS_N 10  //max n# of positions to be stored for the arm

byte state = 0;

void setup() {
  Serial.begin(9600);

  //setup Timer0, may move this inside the node class object initialization

  OCR0A = 0xAF;
  TIMSK0 |= _BV(OCIE0A);

  do {
    n.begin("GATEWAY", "abcd1234", 10010, 192, 168, 43, 52);
  } while (n.getState() != 0);

  Triple info;
  info.type = 'R';
  info.fault = '0';
  strcpy(info.data, "null");
  n.updateTask(info);


  //setup Timer0, may move this inside the node class object initialization


}

void loop() {
  short x[POS_N], y[POS_N], z[POS_N], t[POS_N];
  static byte pos = 0;
  static Triple info;

  switch (state) {

    case 't': //task

      arm.goDirectlyTo(x[pos], y[pos], z[pos]);
      pos++;
      if (pos > POS_N) {
        pos = 0;
      }

      break;

    case 's': //stop
      arm.rest();
      break;

    case 'u': //update task , followed by polling whether there's data available
      n.updateTask(info);
      if (!n.incomingData()) { //if there's no data incoming (so there is no change in the arm task)
        state = 't'; //continue to do the task
        break;
      }
    //otherwise go to receive the message @ the next state
    case 'r': //receive data


    case 'f': //fault
      break;




  }

}


SIGNAL(TIMER0_COMPA_vect)
{
  static short counter = 0;
  counter++;
  if (counter > COMM_INTERVAL) {
    state = 'c';
    counter = 0;
  }
}
