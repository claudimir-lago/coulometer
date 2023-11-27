#include "Arduino.h"
#include <Serine.h>
#include <LiquidCrystal.h>

LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

boolean idle = true;
SerineChronometer couloChrono, keyChrono, buttonChrono, displayChrono;
SerineBuffer buf;
unsigned long elapsedTime = 0;

const int buzzerPin = 18;
const int buttonPin = 3;  // the port for the pushbutton pin
int buttonState = 0;
int buttonStateOld = 0;
boolean apply = false;

//                   0123456789012345
char lineTop[] = "0000000ms";
char lineBottom[] = "option:         ";
struct OptionPar {
  byte relayPinNumber;
  char message[9];
};
const int NumberOfOptions = 4;
OptionPar optionList[NumberOfOptions] = {
  { 2, " 1.04mA" },
  { 1, " 9.70mA" },
  { 0, " 24.3mA" },
  { 19, " 47.3mA" }
};
int option = 1;
int opt = 1;

void buzz(int del) {
  delayMicroseconds(del);
  digitalWrite(buzzerPin, true);
  delayMicroseconds(del);
  digitalWrite(buzzerPin, false);
}

void setup() {
  for (int i = 0; i < NumberOfOptions; i++) {
    digitalWrite(optionList[i].relayPinNumber, HIGH);
    pinMode(optionList[i].relayPinNumber, OUTPUT);
  };
  lcd.begin(16, 2);
  buf.putUL(elapsedTime, 0, 6, lineTop);
  lcd.print(lineTop);
  lcd.print(optionList[option].message);
  lcd.setCursor(0, 1);
  lcd.print(lineBottom);
  lcd.setCursor(8, 1);
  lcd.print(optionList[opt].message);
  couloChrono.zero();
  keyChrono.zero();
  pinMode(buttonPin, INPUT_PULLUP);
  buttonStateOld = digitalRead(buttonPin);
  digitalWrite(buzzerPin, LOW);
  pinMode(buzzerPin, OUTPUT);
  buttonChrono.zero();
  displayChrono.zero();
}

void loop() {
  if (keyChrono.expired(10)) {
    int key = analogRead(0) >> 7;
    // 7 - no key; 5 - select; 3 - left; 2 - down; 1 - up; 0 - right
    if (idle && (key < 7)) {
      switch (key) {
        case 7:
          break;
        case 5:  // select
          digitalWrite(optionList[option].relayPinNumber, HIGH);
          option = opt;
          lcd.noDisplay();
          buzz(10000);
          delay(500);
          lcd.display();
          apply = false;
          elapsedTime = 0;
          buf.putUL(elapsedTime, 0, 6, lineTop);
          lcd.setCursor(0, 0);
          lcd.print(lineTop);
          lcd.print(optionList[option].message);
          break;
        case 3:
          break;
        case 2:  // down
          opt = constrain(opt + 1, 0, NumberOfOptions - 1);
          break;
        case 1:  // up
          opt = constrain(opt - 1, 0, NumberOfOptions - 1);
          break;
        case 0:
          break;
      }
      lcd.setCursor(8, 1);
      lcd.print(optionList[opt].message);
    }
    idle = key == 7;
    keyChrono.zero();
  }
  buttonState = digitalRead(buttonPin);
  if (buttonState != buttonStateOld && buttonChrono.expired(10)) {
    if (buttonState == LOW) {
      if (apply) {
        digitalWrite(optionList[option].relayPinNumber, HIGH);
        elapsedTime = elapsedTime + couloChrono.now();
        buf.putUL(elapsedTime, 0, 6, lineTop);
        lcd.setCursor(0, 0);
        lcd.print(lineTop);
      } else {
        digitalWrite(optionList[option].relayPinNumber, LOW);
        couloChrono.zero();
      }
      apply = !apply;
    };
    buttonStateOld = buttonState;
    buttonChrono.zero();
  }
  if (displayChrono.expired(300) && apply) {
    buf.putUL(elapsedTime + couloChrono.now(), 0, 6, lineTop);
    lcd.setCursor(0, 0);
    lcd.print(lineTop);
  };
  if (apply) buzz(100);
}
