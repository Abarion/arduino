#include <SoftwareSerial.h>
#include <LiquidCrystal.h>

// define variables for libaries
const int rs = 2,
          en = 3,
          d4 = 6,
          d5 = 7,
          d6 = 8,
          d7 = 9;

LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

SoftwareSerial mySerial(11,12);

// setup function
void setup() {
  pinMode(11, INPUT);
  pinMode(12, OUTPUT);
  mySerial.begin(9600);
  Serial.begin(9600);
  lcd.begin(16, 2);

}
// loop function
void loop() {
// get data from the uno and print them on the LCD display
  if(mySerial.available()) {
    lcd.clear();
    String data = mySerial.readStringUntil('\n');

    Serial.println(data);
    lcd.setCursor(0,0);
    lcd.print(data);
  }

}
