#ifndef ArduArm.h
#define ArduArm.h


#if ARDUINO>=100
  #include "Arduino.h"
#else
  #include "WProgram.h"
  #include "pins_arduino.h"
  #include "WCostants.h"

#endif

#include <WiFi.h>
#include <SPI.h>
#include <SD.h>

#include "codes.h"

#define DEBUG 1

#ifdef ADV_DEBUG
  #include <MemoryFree.h>
#endif

#define MAX_SSID_LENGTH 20
#define MAX_PSW_LENGTH 20

#define MAX_BUFFER_LENGTH 30
#define MAX_NAME_SIZE 30
#define MAX_CONTENT_SIZE 30
#define MAX_DATA_SIZE 15
#define MAX_ID_SIZE 20

#define MAX_PACKET_SIZE 60
#define TX_LATENCY 2000

struct Contents {
  char type = 0;
  char content[MAX_CONTENT_SIZE] = "";
};

struct Triple {
  char type; //type of message , R or A
  char fault; //type of fault : 0 (ok) , 1 ,2 ...
  char data[MAX_DATA_SIZE];
};


class node {
  public:

    node(char ID[] = "ArduArm");

    void begin(char SSID[MAX_SSID_LENGTH], char psw[MAX_PSW_LENGTH], short p = 10010, byte a = 0, byte b = 0, byte c = 0, byte d = 0); //Connect to WiFi network , then to

    //void setID(char id[]); //random initialization at creation , unless explicitly defined in the instance call
    short getTR();
    byte getState();//returns the current state of the machine (error code)

    void updateTask(Triple info);
    void reportFault(Triple info);
    bool incomingData();

  private:
    char _nodeID[MAX_ID_SIZE];


    short _port; //port used by the SIB server
    byte _ip[4];

    byte _status;

    WiFiClient _comm; //socket used for main communication

    void receive();
    void store();
    void composeMessage(char code, Triple t);
    void analyzeMessage();
    Contents create(char type, char *state , Triple t);

    void transaction(char type, Triple c);

};



#include "utils.h"
#include "wrappers.h"

#endif
