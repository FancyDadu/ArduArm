#include "ArduArm.h"

node::node(char ID[]) {
  strcpy(_nodeID, ID);
  _status = OK;
}

void node::begin(char SSID[MAX_SSID_LENGTH], char psw[MAX_PSW_LENGTH], short p, byte a, byte b, byte c, byte d) { //a,b,c,d : bytes of the IP address,to be put in order
  if (!WiFiConnect(SSID, psw)) _status = WIFI_CONN_ERR;
  _port = p;
  _ip[0] = a;
  _ip[1] = b;
  _ip[2] = c;
  _ip[3] = d;
  
}

byte node::getState() {
  return _status;
}

void node::pingServer(){
  _comm.connect(_ip, _port);
  _comm.write("hey");
}

bool node::incomingData() {
  if (_comm.available()) {
    #ifdef DEBUG
    Serial.println(F("DATA!!"));
    #endif
    return true;
  }
  return false;
}

void node::stopConn() {
  _comm.stop();
  _connected = false;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------
char node::receive() {

  #ifdef DEBUG
  Serial.println(F("-----------RECEIVING----------"));
  #endif

  char buffer[MAX_BUFFER_LENGTH] = {""}, c = 0;
  short bufCount = 0, totCount = 0;

  while (_comm.available() > 0) {

    #ifdef DEBUG
    Serial.println(F("Receiving..."));
    #endif

    while (bufCount < MAX_BUFFER_LENGTH && _comm.available() > 0 ) { //while there's still stuff to read and available space , store it in buffer
      buffer[bufCount++] = _comm.read();
      totCount++;
      #ifdef DEBUG
      Serial.print(buffer[bufCount - 1]);
      #endif
    }

    if (bufCount > 0) { //if there's anything inside the buffer
      if (strstr(buffer, "<tx>") != NULL && strstr(buffer, "</tx>") != NULL) {
        char *p = strstr(buffer, "<t>");
        if (p != NULL) {
          #ifdef DEBUG
          Serial.println(F("New Task"));
          #endif
          c = *(p + sizeof(char) * 3);
        }
      }
    }

  }
  
  _comm.stop();
  #ifdef DEBUG
  Serial.println(F("Communication Ended."));
  #endif


  if (totCount > 5) {
    _status = OK;
  }
  else {
    _status = TOO_SHORT;
    #ifdef DEBUG
    Serial.println(F("TOO SHORT!"));
    #endif
  }
  
  return c;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------

Contents node::create(char type, char *state , Triple t) {

  Contents c;

  switch (*state) {

    case 'n': //node
      c.type = 'i';
      strcpy(c.content, _nodeID);
      *state = 't';
      break;

    case 't': //type
      c.type = 't';
      if (type == 'r') strcpy(c.content, "R");
      else strcpy(c.content, "A");
      *state = 'f';

      break;

    case 'f': //fault
      c.type = 'f';
      if (type == 'r') strcpy(c.content, "0");
      else strcpy(c.content, t.fault);
      *state = 'd';
      break;

    case 'd': //data
      c.type = 'd';
      if (type == 'r') strcpy(c.content, t.data);
      else  strcpy(c.content, t.data);
      *state = 'e';
      break;

    case 'e':
    case 'z':
      *state = 'z';

  }

  return c;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------

void node::composeMessage(char code, Triple t) {

  #ifdef DEBUG
  Serial.println(F("-----------COMPOSING----------"));
  #endif

  if (!initializeSD()) {
    _status = SD_ERR_INI;
    return;
  }

  File xtemp;
  char name[5] = "";
  name[0] = code;

  strcat(name, "temp");
  #ifdef DEBUG
  Serial.println(name);
  #endif

  xtemp = SD.open(name, FILE_READ);
  if (!_connected) _comm.connect(_ip, _port);
  if (xtemp) {

    #ifdef DEBUG
    Serial.println(F("File successfully opened"));
    #endif

    char c, tag, lastChar, cState = 'n';

    short bytecount = 0, count = 0;
    bool readingName = false, last = true;

    Contents curr = create(code, &cState, t);

    while (xtemp.available() >0) {

      c = xtemp.read();
      _comm.write(c);
      count++;
      if (count > MAX_PACKET_SIZE) {
        delay(TX_LATENCY);
      }

      if (cState != 'z') { //z means no more content to be added

        if (lastChar == '<' && c != '/' ) { //that is , a tag has been opened but it's not a closing one

          readingName = true;
          #ifdef DEBUG
          Serial.println(F("Tag opened "));
          #endif
        }

        if (c == '>') {

          if (readingName) { //finished reading tagName

            #ifdef DEBUG
            Serial.print(F("Found tag: -"));
            Serial.print(tag);
            Serial.print(F("- Searching for: -"));
            Serial.print(curr.type);
            Serial.println("-");
            #endif

            if (tag == curr.type) {

              #ifdef DEBUG
              Serial.println(F("Found content!: "));
              Serial.println(tag);
              #endif


              _comm.write(curr.content);
              curr = create(code, &cState, t);

            }

            tag = 0;
          }
          readingName = false;
        }

      }

      if (readingName) tag = c;

      lastChar = c;
    }

    xtemp.close();

    _status = OK;
    return;
  }


  #ifdef DEBUG
  Serial.print(F("Error File: "));
  Serial.println(code);
  #endif

  _status = SD_ERR_FILE;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------


