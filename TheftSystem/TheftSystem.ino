#include <EEPROM.h>
#include <SPI.h>
#include <MFRC522.h>
#include <SoftwareSerial.h>

#define DEBUG       true

#define LOCKED      true
#define UNLOCKED    false

#define RED_PIN     5
#define GREEN_PIN   4
#define BUZZER_PIN  3
#define IR_PIN      2
#define SS_PIN      10
#define RST_PIN     9
#define SIM900_RST  6
#define SIM900_RX   7
#define SIM900_TX   8
#define LIGHT_PIN   A0

#define ENV_LIGHT   80

char strPhone[13] = "+40746171700";

bool bSystemSecurity;
int iEEPROMAddress = 0;
MFRC522 oRFID(SS_PIN, RST_PIN);
SoftwareSerial oSIM900(SIM900_RX, SIM900_TX); 

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

  //Turn on SIM900 GSM shield
  digitalWrite(SIM900_RST, HIGH);
  delay(1000);
  digitalWrite(SIM900_RST, LOW);
  delay(5000);
  // Initialize SIM900 GSM shield
  oSIM900.begin(19200);
  if(DEBUG) Serial.println("SIM900 loading..");
  delay(20000);
  if(DEBUG) Serial.println("SIM900 logged on to network!");

  // Initialize RFID for lock / unlock
  SPI.begin();
  oRFID.PCD_Init();

  // Initialize buzzer and LEDs for user interface
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  // Initialize infrared sensor for door
  pinMode(IR_PIN, INPUT);

  // Initialize light sensor for environment
  pinMode(LIGHT_PIN, INPUT);

  // Get the system state from EEPROM: LOCKED / UNLOCKED
  bSystemSecurity = getEEPROMSystemState();
}

bool getSensorsStatus() {
  bool bStatus = false;

  delay(100);
  int iDoorSensor = digitalRead(IR_PIN);
  if (iDoorSensor == 1) {
    if(DEBUG) Serial.println("The door is open!");
    bStatus = true;
  }

  int iEnvironmentLight = analogRead(LIGHT_PIN);
  if (iEnvironmentLight > ENV_LIGHT) {
    if(DEBUG) Serial.print("Unknown light in room! Value: ");
    if(DEBUG) Serial.println(iEnvironmentLight);
    bStatus = true;
  }
  
  return bStatus;
}

void StartAlarm() {
  if(DEBUG) Serial.println("Alarm started!");
  // add code for external alarm
  tone(BUZZER_PIN, 5000);
}

void StartAlarm_Call() {
  char buffer[20];
  if(DEBUG) Serial.println("Call started!");
  sprintf(buffer, "ATD%s;\r", strPhone);
  if(DEBUG) Serial.println(buffer);
  oSIM900.print(buffer); 
  delay(3000);
}

void StopAlarm() {
  if(DEBUG) Serial.println("Alarm stopped!");
  // add code for external Alarm
  noTone(BUZZER_PIN);
}

inline void GreenSignal() {
  tone(BUZZER_PIN, 1000);
  digitalWrite(GREEN_PIN, HIGH);
  delay(1000);
  noTone(BUZZER_PIN);
  digitalWrite(GREEN_PIN, LOW);
}

inline void RedSignal() {
  tone(BUZZER_PIN, 100);
  digitalWrite(RED_PIN, HIGH);
  delay(1000);
  noTone(BUZZER_PIN);
  digitalWrite(RED_PIN, LOW);
}

bool RFID_Authetificated() {
  if (oRFID.PICC_IsNewCardPresent()) {
    if (oRFID.PICC_ReadCardSerial()) {
      if(DEBUG) Serial.print("UID tag :");
      String content= "";
      byte letter;
      for (byte i = 0; i < oRFID.uid.size; i++) {
         if(DEBUG) Serial.print(oRFID.uid.uidByte[i] < 0x10 ? " 0" : " ");
         if(DEBUG) Serial.print(oRFID.uid.uidByte[i], HEX);
         content.concat(String(oRFID.uid.uidByte[i] < 0x10 ? " 0" : " "));
         content.concat(String(oRFID.uid.uidByte[i], HEX));
      }
      if(DEBUG) Serial.println("");
      content.toUpperCase();
      if (content.substring(1) == "99 03 72 9E") {
        if(DEBUG) Serial.println("Authorized access");
        GreenSignal();
        return true;
      } else {
        if(DEBUG) Serial.println("Access denied");
        RedSignal();
        return false;
      }
    }
  }
  return false;
}

void SystemState_Locked() {
  while(bSystemSecurity == LOCKED) {
    if (getSensorsStatus()) {
      StartAlarm();
      StartAlarm_Call();
      while(true) {
        if(RFID_Authetificated()) {
          StopAlarm();
          break;
        }
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
      if(DEBUG) Serial.println("RFID success authetificated.");
      if (!getSensorsStatus()) {
        if(DEBUG) Serial.println("Status of the sensors: OK");
        bSystemSecurity = LOCKED;
        setEEPROMSystemState(bSystemSecurity);
        break;
      } else {
        if(DEBUG) Serial.println("Trying to lock with sensor status: NOT OK");
        if(DEBUG) Serial.println("Stop alarm with the key!");
        while (!RFID_Authetificated()) {
          StartAlarm();
          delay(500);
          StopAlarm();
          delay(500);
        }
      }
    }
  }
}

void loop() {
  SystemState_Locked();
  SystemState_Unlocked();
}
