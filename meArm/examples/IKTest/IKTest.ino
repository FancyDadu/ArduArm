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
int shoulderPin = 9;
int elbowPin = 6;
int gripperPin = 11;

meArm arm(85, 8, 0, pi / 2, //Base (‐90 to 90 degrees)
          172, 85, 0, pi / 2, //Shoulder ( 45 to 135 degrees)
          178 , 85 , 0, pi / 2, //Elbow ( 0 to ‐45 degrees)
          140, 180, 0, pi / 2);

void setup() {
  arm.begin(basePin, shoulderPin, elbowPin, gripperPin);
  arm.closeGripper();
}

void loop() {

  arm.openGripper();
  arm.gotoPoint(-80, 100, 140);
  arm.closeGripper();
  arm.gotoPoint(70, 200, 10);
  arm.openGripper();
  arm.gotoPoint(80, 100, 140);
  arm.gotoPoint(-70, 200, 10);



  /*
    arm.openGripper();
    //Go up and left to grab something
    arm.gotoPoint(-80, 100, 140);
    arm.closeGripper();
    //Go down, forward and right to drop it
    arm.gotoPoint(70, 200, 10);
    arm.openGripper();
    //Back to start position
    arm.gotoPoint(0, 100, 50);
    arm.closeGripper();
     arm.gotoPoint(80, 100, 140);
  */


  delay(300);

}
