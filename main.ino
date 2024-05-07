#include <SPI.h> 
#include <MFRC522.h>
#include <Keypad.h> 
#include <SoftwareSerial.h> 


// define pins and variables for the libarys and the arduino 

// MFRC522 
#define SS_PIN 10 
#define RST_PIN 9
#define RST_PIN 5 
class FGRFIDHelper {
private:
  MFRC522 mfrc522;
  MFRC522::MIFARE_Key key;
  bool initialized = false;
  byte *buffer;
  byte *splitNumber;
  void stopRFID();
  byte bufferLength = 18; 

public:
  FGRFIDHelper();
  void init();
  bool ready();
  byte *read(int block, bool print = false, bool printData = false);
  bool write(int block, byte data[], bool print = false);
  bool writeNumber(int block, int number);
  int readNumber(int block);
  int fromSplitBytes(byte data[]);
  byte *toSplitBytes(int number);
};
 // Software Serial
SoftwareSerial mySerial(A5, A4);

// Keypad
const byte ROWS = 4;  // number of rows
const byte COLS = 3;  // number of columns
char keys[ROWS][COLS] = {
  { '1', '2', '3' },
  { '4', '5', '6' },
  { '7', '8', '9' },
  { '*', '0', '#' }
};
byte rowPins[ROWS] = { 8, 7, 6, 5 };  // row pinouts of the keypad R1 = D8, R2 = D7, R3 = D6, R4 = D5
byte colPins[COLS] = { 4, 3, 2 };     // column pinouts of the keypad C1 = D4, C2 = D3, C3 = D2
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

FGRFIDHelper helper;

// setup function
void setup() {

  Serial.begin(9600);
  mySerial.begin(9600);
  helper = FGRFIDHelper();
  helper.init();
  memset(numbers, 0, 6);
}

// define variables
bool read = false;
int arrIndex = 0;
int numbersToWriteReadOnRFID = 0;
char numbers[6];

// loop function
void loop() {
// get the inputs and save them in an array. # = confirmed, * = go back one index, and send data to the nano
  char key = keypad.getKey();
  if (key != NO_KEY) {
    
    if(key != '*' && key != '#') {
      if(arrIndex != 5) {
      numbers[arrIndex] = key;
      arrIndex++;

      mySerial.print("Betrag:");
      atoi(numbers);
      mySerial.print(numbers);
      mySerial.print("\n");
      mySerial.flush();
      }
    } else if(key == '*') {
      arrIndex--;
    } else if(key == '#') {
      numbersToWriteReadOnRFID = atoi(numbers);
      mySerial.print("Bestaetigt");
      delay(5000);
      mySerial.print("Ihr Betrag ");
      mySerial.print(numbers);
      mySerial.print("\n");
      mySerial.flush();
    }
  }
  if (helper.ready()) {
    if (!read) {
      // READ
      Serial.println("RFID Num:");
      mySerial.print("RFID Num:");
      int numbersToWriteReadOnRFID = helper.readNumber(2);
      Serial.println(numbersToWriteReadOnRFID);
      mySerial.println(numbersToWriteReadOnRFID);
      read = true;
    } else {
      //WRITE
      Serial.println("Writing RFID");
      mySerial.print("Writing RFID");
      helper.writeNumber(2, numbersToWriteReadOnRFID);
      read = false;
    }
  }
  delay(100);
}


// Siehe Code von FG
FGRFIDHelper::FGRFIDHelper() {
  mfrc522 = MFRC522(SS_PIN, RST_PIN);
}

void FGRFIDHelper::init() {
  SPI.begin();
  mfrc522.PCD_Init();

  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }

  initialized = true;
}

bool FGRFIDHelper::ready() {
  if (!initialized) {
    Serial.println("Helper has not yet been initialized");
    return false;
  }
  if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
    return false;
  }
  return true;
}

byte *FGRFIDHelper::read(int block, bool print = false, bool printData = false) {
  buffer = new byte[bufferLength];

  MFRC522::StatusCode status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    if (print) {
      Serial.print(F("Authentication failed: "));
      Serial.println(mfrc522.GetStatusCodeName(status));
    }
    this->stopRFID();
    return {};
  }
  status = mfrc522.MIFARE_Read(block, buffer, &bufferLength);
  if (status != MFRC522::STATUS_OK) {
    if (print) {
      Serial.print(F("Reading failed: "));
      Serial.println(mfrc522.GetStatusCodeName(status));
    }
    this->stopRFID();
    return {};
  }
  this->stopRFID();

  if (printData) {
    Serial.print(F("Data in block "));
    Serial.print(block);
    Serial.println(F(":"));
    for (byte i = 0; i < 16; i++) {
      Serial.print(buffer[i] < 0x10 ? " 0" : " ");
      Serial.print(buffer[i], HEX);
    }
    Serial.println();
  }
  return buffer;
}

bool FGRFIDHelper::write(int block, byte data[], bool print = false) {
  MFRC522::StatusCode status = mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, block, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {
    if (print) {
      Serial.print(F("Authentication failed: "));
      Serial.println(mfrc522.GetStatusCodeName(status));
    }
    this->stopRFID();
    return false;
  }

  // Write data to the block
  status = mfrc522.MIFARE_Write(block, data, 16);
  if (status != MFRC522::STATUS_OK) {
    if (print) {
      Serial.print(F("Writing failed: "));
      Serial.println(mfrc522.GetStatusCodeName(status));
    }
    this->stopRFID();
    return false;
  }

  if (print) {
    Serial.println(F("Data written to block successfully!"));
  }
  this->stopRFID();
  return true;
}

void FGRFIDHelper::stopRFID() {
  mfrc522.PICC_HaltA();
  mfrc522.PCD_StopCrypto1();
}

int FGRFIDHelper::fromSplitBytes(byte data[]) {
  int d0 = data[0];
  int d1 = data[1];
  return (d0 << 8) | d1;
}

byte *FGRFIDHelper::toSplitBytes(int number) {
  if (number > 65535) {
    return {};
  }
  byte superior = (number >> 8) & 0xFF;
  byte inferior = number & 0xFF;
  splitNumber = new byte[bufferLength];
  memset(splitNumber, 0, bufferLength);
  splitNumber[0] = superior;
  splitNumber[1] = inferior;
  return splitNumber;
}

bool FGRFIDHelper::writeNumber(int block, int number) {
  byte *data = this->toSplitBytes(number);
  return this->write(block, data);
}

int FGRFIDHelper::readNumber(int block) {
  return this->fromSplitBytes(this->read(block));
}
