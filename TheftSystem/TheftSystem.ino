#include <EEPROM.h>
#include <SPI.h>
#include <MFRC522.h>

#define DEBUG       true

#define LOCKED      true
#define UNLOCKED    false

#define RED_PIN     1
#define GREEN_PIN   2
#define BUZZER_PIN  3
#define IR_PIN      4
#define SS_PIN      5
#define RST_PIN     6

bool bSystemSecurity;
int iEEPROMAddress = 0;
MFRC522 oRFID(SS_PIN, RST_PIN);

bool getEEPROMSystemState() {
  int iState = EEPROM.read(iEEPROMAddress);
  if (DEBUG) Serial.print("The current state of the system is: ");
  if (iState == 0) {
    if (DEBUG) Serial.println("UNLOCKED");
    return UNLOCKED;
  }
  if (iState == 1) {
    if (DEBUG) Serial.println("LOCKED");
    return LOCKED;
  }
}

void setEEPROMSystemState(bool bState) {
  EEPROM.write(iEEPROMAddress, (int)bState);
  if (DEBUG) Serial.print("The current state of the system is: ");
  if (!bState)
    if (DEBUG) Serial.println("UNLOCKED");
  if (bState)
    if (DEBUG) Serial.println("LOCKED");
}

void setup() {
  Serial.begin(9600);

  // Initialize RFID for lock / unlock
  SPI.begin();
  oRFID.PCD_Init();

  // Initialize buzzer and LEDs for user interface
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  // Initialize infrared sensor for door
  pinMode(IR_PIN, INPUT);

  // Get the system state from EEPROM: LOCKED / UNLOCKED
  bSystemSecurity = getEEPROMSystemState();
}

bool getSensorsStatus() {
  return false;
}

void StartAlarm() {
  
}

void StartAlarm_Call() {
  
}

void StopAlarm() {
  
}

void StopAlarm_Call() {
  
}

bool RFID_Authetificated() {
  return false;
}

bool StopMessage_Received() {
  return false;
}

void SystemState_Locked() {
  while(bSystemSecurity == LOCKED) {
    if (getSensorsStatus()) {
      StartAlarm();
      StartAlarm_Call();
      while(true) {
        if(RFID_Authetificated() || StopMessage_Received())
          StopAlarm();
          StopAlarm_Call();
          break;
      }
      bSystemSecurity = UNLOCKED;
      setEEPROMSystemState(bSystemSecurity);
      break;
    }
    if (RFID_Authetificated()) {
      bSystemSecurity = UNLOCKED;
      setEEPROMSystemState(bSystemSecurity);
      break;
    }
  }
}

void SystemState_Unlocked() {
  while(bSystemSecurity == UNLOCKED) {
    if (RFID_Authetificated()) {
      if (getSensorsStatus()) {
        bSystemSecurity = LOCKED;
        setEEPROMSystemState(bSystemSecurity);
        break;
      } else {
        while (getSensorsStatus()) {
          StartAlarm();
          delay(1000);
          StopAlarm();
        }
      }
    }
  }
}

void loop() {
  SystemState_Locked();
  SystemState_Unlocked();
}
