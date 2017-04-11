#include "ArduArm.h"
#include "meArm.h"
#include <Servo.h>

node n;
meArm arm(179, 86, -pi / 2, 0, //Base (‐90 to 90 degrees)
          180, 80, 0, pi / 2, //Shoulder ( 45 to 135 degrees)
          45, 130 , -pi / 2, 0, //Elbow ( 0 to ‐45 degrees)
          95, 70, 0, pi / 2);

#define COMM_INTERVAL 10000
#define POS_N 5  //max n# of positions to be stored for the arm

char state = 't', lastState = state;

int basePin = 3;
int shoulderPin = 6;
int elbowPin = 9;
int gripperPin = 5;

void setup() {
  Serial.begin(9600);

  arm.begin(basePin, shoulderPin, elbowPin, gripperPin);

  arm.closeGripper();
  arm.rest();

  /*
    n.begin("GATEWAY", "abcd1234", 10010, 192, 168, 43, 52);

    Triple info;
    info.type = 'R';
    info.fault = '0';
    strcpy(info.data, "null");
    n.updateTask(info);
    n.stopConn();
  */

  arm.getUp();

  //setup Timer0, may move this inside the node class object initialization
  OCR0A = 0xAF;
  TIMSK0 |= _BV(OCIE0A);

  Serial.println("Finished initialization");

}

static int pause = 0; //global variable used to control pauses for the arm,set by the t (time) array in the loop , decremented by timer0

void loop() {


  static short x1[POS_N] = {0, 0, 50, 50, 0},
                           y1[POS_N] = {100, 150, 150, 200, 200},
                                       z1[POS_N] = {40, 40, 20, 20, 20},
                                           t1[POS_N] = {100, 250, 400, 400, 500};

  static short x2[POS_N] = {0, 0, -50, -50, -70},
                           y2[POS_N] = {150, 150, 200, 200, 150},
                                       z2[POS_N] = {20, 60, 60, 20, 20},
                                           t2[POS_N] = {100, 250, 400, 400, 500};

  static short *x = x1, *y = y1, *z = z1, *t = t1;

  static char task = '1';

  static byte pos = 0;
  static Triple info;

  switch (state) {


    case 't': //task
      if (pause <= 0) {
        arm.gotoPoint(x[pos], y[pos], z[pos]);
        pause = t[pos];
        pos++;

      }

      if (pos >= POS_N) {
        pos = 0;
      }

      break;

    case 's': //stop
      arm.rest();
      break;

    case 'r': //receive data
      Serial.println("PING");
      n.pingServer();

      if (n.incomingData()) {
        Serial.println("RECEIVING");
        task = n.receive();
        Serial.print("Now the task is ");
        Serial.println(task);
        state = 'h';
      }
      else  state = lastState;
      n.stopConn();
      Serial.println("RESUME");
      break;


    case 'f': //fault
      state = 's';
      break;

    case 'h': //helper to switch tasks without wifi [[DEBUG]]
      arm.rest();

      switch (task) {
        case '1':
          task = '2';
          x = x2;
          y = y2;
          z = z2;
          t = t2;
          state = 't';
          break;



        case  '2':
          state = 's';
          task = 's';
          break;


        case 's':
          task = '1';
          x = x1;
          y = y1;
          z = z1;
          t = t1;
          state = 't';

      }

      arm.getUp();

      break;

  }

}


SIGNAL(TIMER0_COMPA_vect)
{
  static short counter = 0;

  if (state != 'h') {
    counter++;
    pause--;
  }
  if (counter > COMM_INTERVAL) {
    lastState = state;
    state = 'h';
    counter = 0;
  }
}
