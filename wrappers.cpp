#include "ArduArm.h"

void node::transaction(char type,Triple c){
  byte i=0;
  
  while(_status==OK && i<3){
    
    switch(i){
      
      case 0:
        composeMessage(type,c);
        delay(250);
        break;
        
      case 1:
        receive();
        
        break;

      case 2:
        store();
        break;
    }
    
    i++;   
  }
  
  //if(_comm.connected()) _comm.stop();
  
}

void node::join() {
  Triple foo;
  transaction('j',foo);
  
}

//------------------------------------------------------------------------------------------------------------------------------------

void node::rdfInsert(Triple cont) {
  transaction('i',cont);
}

//------------------------------------------------------------------------------------------------------------------------------------

void node::rdfRemove(Triple cont) {
  transaction('r', cont);
}

//------------------------------------------------------------------------------------------------------------------------------------

void node::rdfQuery(Triple cont) {
   transaction('q', cont);
}

//------------------------------------------------------------------------------------------------------------------------------------

void node::rdfSubscribe(Triple cont) {
   transaction('s', cont);
}

//------------------------------------------------------------------------------------------------------------------------------------

void node::leave(){
  Triple foo;
  transaction('l',foo);
}

