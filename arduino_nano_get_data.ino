#include <SoftwareSerial.h>
#include <LiquidCrystal.h>

const int rs = 2,
          en = 3,
          d4 = 6,
          d5 = 7,
          d6 = 8,
          d7 = 9;

LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

SoftwareSerial mySerial(11,12);

void setup() {
  pinMode(11, INPUT);
  pinMode(12, OUTPUT);
  mySerial.begin(9600);
  Serial.begin(9600);
  lcd.begin(16, 2);

}

void loop() {
  if(mySerial.available()) {
    lcd.clear();
    String data = mySerial.readStringUntil('\n');

    Serial.println(data);
    lcd.setCursor(0,0);
    lcd.print(data);
  }

}
