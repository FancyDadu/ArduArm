/* meArm IK test - York Hackspace May 2014
   Test applying Nick Moriarty's Inverse Kinematics solver
   to Phenoptix' meArm robot arm, to walk a specified path.

   Pins:
   Arduino    Base   Shoulder  Elbow    Gripper
      GND    Brown     Brown   Brown     Brown
       5V      Red       Red     Red       Red
       11    Yellow
       10             Yellow
        9                     Yellow
        6                               Yellow
*/
#include "meArm.h"
#include <Servo.h>

int basePin = 10;
int shoulderPin = 6;
int elbowPin = 9;
int gripperPin = 5;

meArm arm(179, 86, -pi / 2, 0, //Base (‐90 to 90 degrees)
          180, 80, 0, pi / 2, //Shoulder ( 45 to 135 degrees)
          45, 130 , -pi / 2, 0, //Elbow ( 0 to ‐45 degrees)
          95, 70, 0, pi / 2);

void setup() {
  arm.begin(basePin, shoulderPin, elbowPin, gripperPin);
  arm.closeGripper();
}

void loop() {

  
  arm.gotoPoint(100, 180, 50);
  arm.rest();
  while(1);


}
