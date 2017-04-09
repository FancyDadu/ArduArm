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

bool node::incomingData(){
  if(_comm.available()>0) return true;
  return false;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------
void node::receive() {

  #ifdef DEBUG
  Serial.println(F("-----------RECEIVING----------"));
  #endif

  if (!initializeSD()) {
    _status = SD_ERR_INI;
    return;
  }

  SD.remove(F("store"));
  File storage;
  storage = SD.open(F("store"), FILE_WRITE);

  if (storage) {

    char buffer[MAX_BUFFER_LENGTH] = {""};
    short bufCount = 0, totCount = 0;

    while (_comm.available() > 0) {

      #ifdef DEBUG
      Serial.println(F("Receiving..."));
      #endif

      while (bufCount < MAX_BUFFER_LENGTH && _comm.available() > 0 ) { //while there's still stuff to read and available space , store it in buffer
        buffer[bufCount++] = _comm.read();
        #ifdef DEBUG
        Serial.print(buffer[bufCount - 1]);
        #endif
      }

      if (bufCount > 0) { //if there's anything inside the buffer
        storage.write(buffer, bufCount);
        storage.flush();
        strcpy(buffer, "");
        totCount += bufCount;
        bufCount = 0;
      }
    }

    #ifdef DEBUG
    Serial.println(F("Communication Ended. Closing file now.."));
    #endif

    storage.close();
    if (totCount > 10) {
      _status = OK;
      return;
    }

    _status = TOO_SHORT;
    #ifdef DEBUG
    Serial.println(F("TOO SHORT!"));
    #endif
    return;
  }


  #ifdef DEBUG
  Serial.println(F("Error on Opening File!"));
  #endif

  _status = SD_ERR_FILE;
}

//---------------------------------------------------------------------------------------------------------------------------------------------------------------
void node::store() {

  #ifdef DEBUG
  Serial.println(F("-----------STORING----------"));
  #endif

  if (!initializeSD()) {
    _status = SD_ERR_INI;
    return;
  }

  File storage;
  storage = SD.open(F("store"), FILE_READ);

  if (storage) {

    char c, last, type;
    char tagName[MAX_NAME_SIZE] = {""}, id[3]; //max possible id : 999
    bool foundT = false, foundI = false, readN = false, readT = false, readI = false;
    byte i = 0;

    while (storage.available() && !foundI) {

      c = storage.read();

      if (last == '<') {
        if (c != '/') readN = true;
      }

      if (readN) {
        if (c != '>') tagName[i++] = c;
        else {
          if (!foundT && strncmp_P(tagName, PSTR("transaction_type"), i) == 0) readT = true;
          else if (!foundI && strncmp_P(tagName, PSTR("transaction_id"), i) == 0) readI = true;
          readN = false;
          strncpy(tagName, "", sizeof(tagName));
          i = 0;
        }
      }

      else {
        if (readT) {
          type = c;
          readT = false;
          foundT = true;
        }

        if (readI) {

          i = 0;
          id[i] = c;

          c = storage.read();
          while (c != '<') {
            id[++i] = c;
            c = storage.read();
          }
          if (i == 0) {
            id[2] = id[0];
            id[1] = '0';
            id[0] = '0';
          }
          if (i == 1) {
            id[2] = id[1];
            id[1] = id[0];
            id[0] = '0';
          }
          readI = false;
          foundI = true;
        }
      }
      last = c;
    }

    if (foundT && foundI) {

      char dName[4] = {""}; //type+id
      dName[0] = type;
      //for (int j = 0; j < 3; j++) dName[j + 1] = id[j];
      strncat(dName, id, sizeof(id));

      SD.remove(dName);

      File dest = SD.open(dName, FILE_WRITE);

      if (dest) {
        storage.seek(0);
        while (storage.available()) {
          dest.write(storage.read());
        }
        dest.close();
        storage.close();
        _status = OK;
        return;
      }

      else {
        #ifdef DEBUG
        Serial.println("cant open dest");
        #endif
        _status = SD_ERR_FILE;
      }

    }

    else {
      #ifdef DEBUG
      Serial.println(F("T/I no found"));
      #endif
      _status = BAD_STRUCT;
    }

  }

  else {
    #ifdef DEBUG
    Serial.println(F("Error on reading store!"));
    #endif
    _status = SD_ERR_FILE;
  }

}
//---------------------------------------------------------------------------------------------------------------------------------------------------------------

void node::analyzeMessage() {

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
      if (type == 'r') strcpy(c.content, "null");
      else  strcpy(c.content, t.data);
      *state = 'e';
      break;

    case 'e':
    case 'z':
      *state = 'z';

  }
  Serial.println(c.type);
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
  _comm.connect(_ip, _port);
  if (xtemp) {

    #ifdef DEBUG
    Serial.println(F("File successfully opened"));
    #endif

    char c, cState = 'n';
    char state, lastState ; // used by the sequence reader

    char tag, buffer[MAX_PACKET_SIZE] = {""};

    short bytecount = 0, count = 0;
    bool readingName = false, last = true;

    Contents curr = create(code, &cState, t);

    while (xtemp.available() > 0) {

      c = xtemp.read();
      state = c;
      buffer[count++] = c;

      if (count > MAX_PACKET_SIZE - 2 || xtemp.peek() <= 0) {
        _comm.write(buffer);
        strncpy(buffer, "", sizeof(buffer));
        count = 0;
        delay(TX_LATENCY);
      }


      if (cState != 'z') { //z means no more content to be added

        if (lastState == '<' && state != '/' ) { //that is , a tag has been opened but it's not a closing one

          readingName = true;
          #ifdef DEBUG
          Serial.println(F("Tag opened "));
          #endif
        }

        if (state == '>') {

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

              if (count > 0 && count + strlen(curr.content) > MAX_PACKET_SIZE - 2) {
                _comm.write(buffer);
                count = 0;
                strncpy(buffer, "", sizeof(buffer));
                delay(TX_LATENCY);
              }

              strcat(buffer, curr.content);
              count += strlen(curr.content);
              curr = create(code, &cState, t);

            }

            tag = 0;
          }
          readingName = false;
        }

      }

      if (readingName) tag = c;

      lastState = state;
    }
    _comm.println();
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


