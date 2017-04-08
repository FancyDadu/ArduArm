#include "ArduArm.h"

void node::transaction(char type, Triple c) {

  if (_status == OK )composeMessage(type, c);


}

void node::updateTask(Triple info) {
  transaction('r', info); //Request of new task (or stopping)
}

//------------------------------------------------------------------------------------------------------------------------------------

void node::reportFault(Triple info) { //Triple with fault type , sensor data & 1 more possible field
  transaction('a', info); //Alert
}



