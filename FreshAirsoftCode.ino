#include <Tone.h>
#include <LiquidCrystal.h>
#include <Keypad.h>

#define SETUP 0
#define RUNNING 1
#define ENDGAME 2
#define POSTGAME 3

Tone tone1;

int ledPin = 12; //red light
int ledPin2 = 11; //green light

long GameState = 0; // To keep track of where we are in the game.

// Timer Vars
long interval = 1000; // interval for seconds
int Scount = 0; // count seconds
int Mcount = 0; // count minutes
int Hcount = 0; // count hours
int Dcount = 0; // count days
int val = 0;
long secMillis = 0; // store last time for second add

// Keypad vars
const byte ROWS = 4; 
const byte COLS = 4; 

char hexaKeys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte rowPins[ROWS] = {8, 9, A0, A1}; 
byte colPins[COLS] = {A2, A3, A4, A5};

// LCD Vars
int RSPin = 3;
int EnablePin = 2;
int RegD4 = 4;
int RegD5 = 5;
int RegD6 = 6;
int RegD7 = 7;

// create screen object.
LiquidCrystal lcd(RSPin, EnablePin, RegD4, RegD5, RegD6, RegD7);

// create keypad object.
Keypad customKeypad = Keypad(makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS); 

// Passcode Vars
char codeEntered[4] = {NULL, NULL, NULL, NULL};
char setCode[4];
int codeLength = 0;
int setCodeLength = 0;
bool codeCorrect = false;
int checkCodeAttempts = 0;

unsigned long timeout;

void setup() {
  // put your setup code here, to run once:
  int lcdWidth = 16;
  int lcdHeight = 2;
  lcd.begin(lcdWidth, lcdHeight);
  Serial.begin(9600);

  pinMode(ledPin, OUTPUT); // sets the digital pin as

  tone1.begin(13);
  
  customKeypad.addEventListener(keypadEvent);
  timeout = millis();
  
  welcomeMessage();

  GameState = SETUP;
}

void loop() {
    char key = customKeypad.getKey();
    switch (GameState) {
      case SETUP:
        // statements
        enterSetCode();
        clearEnteredCode();
        GameState = RUNNING;
        break;
      case RUNNING:
        // Verify Code Entered
        verifyCode();
        // Display Code Entered
        displayEnteredCode();
        // Display Timer
        displayTimer();
        //Check if end condition is met.
        checkEndGameCondition();
        break;
      case ENDGAME:
        if(codeCorrect == true){
          endGame(true);
        } else {
          endGame(false);
        }
        break;
      case POSTGAME:
        lcd.setCursor (0,0);
        lcd.print("Would you like to play again?");
        char key = customKeypad.getKey();
        if( key == 'A' || key == 'a' ){
          setup(); 
        }
        // TODO:
        // If player answers yes, then count down from 15 seconds
        // start the game over.
        // Else
        // Power the box off to save battery power. 
        break;
      default:
        Serial.print("Inside DEFAULT");
        break;
    }
}

void checkEndGameCondition(){
  if(checkCodeAttempts >= 3 || codeCorrect == true){
    GameState = ENDGAME;
  }
}

void displayTimer(){
  if ((millis() - timeout) >= 1000){
    lcd.setCursor (0,0); // sets cursor to 2nd line
    lcd.print ("Timer: ");
    lcd.print (timer());
  }
}

void displayEnteredCode(){
  lcd.setCursor(0,1);
  lcd.print("Enter Code: ");
  for(int i = 0; i < sizeof(codeEntered); i++){
    if(codeEntered[i] != NULL){
      lcd.setCursor(i + 12, 1);
      lcd.cursor();
      lcd.print(codeEntered[i]);
    }
  }
}

void welcomeMessage(){
  lcd.print("Airsoft Timer v1.0.2"); // Prints "Arduino" on the LCD 
  delay(3000); // 3 seconds delay 
  lcd.noCursor();
  lcd.clear();
  lcd.home();
}

void enterSetCode(){
  lcd.setCursor(0,1);
  lcd.print("Set Code: ");
  while (codeLength < 4){
    lcd.setCursor(codeLength + 12, 1);
    lcd.cursor();
    char key = customKeypad.getKey();
    lcd.print(key);
  }
  clearScreen();
  lcd.setCursor(0,0);
  lcd.print("You've entered: ");
  lcd.setCursor(0,1);
  saveCode();
  displaySetCode();
  delay(5000);
  clearScreen();
  codeLength = 0;
}

void clearScreen(){
  lcd.noCursor();
  lcd.clear();
  lcd.home();
}

void displaySetCode(){
  for(int i = 0; i < sizeof(codeEntered); i++){
    if(codeEntered[i] != NULL){
      lcd.setCursor(i, 1);
      lcd.cursor();
      lcd.print(setCode[i]);
    }
  }
}

void saveCode(){
  for(int i = 0; i < sizeof(codeEntered); i++){
    setCode[i] = codeEntered[i];
  }
}

void clearEnteredCode(){
  for(int i = 0; i < sizeof(codeEntered); i++){
    codeEntered[i] = (char)NO_KEY;
  }
}

void verifyCode(){
  if(codeLength == 4){
    String strSetCode = setCode;
    String strEnteredCode = codeEntered;
    Serial.print(strSetCode);
    Serial.print(strEnteredCode);
    if (setCode[0] == codeEntered[0] && 
        setCode[1] == codeEntered[1] &&
        setCode[2] == codeEntered[2] &&
        setCode[3] == codeEntered[3] ){
      codeCorrect = true;
    }else{
      checkCodeAttempts += 1;
      returnWrongMessage();
      resetEnteredCode();
      // TODO: Code didn't match. Reset the code for another try.
      codeCorrect = false;
    }
  }
}

void resetEnteredCode(){
  for(int i = 0; i < sizeof(codeEntered); i++){
    codeEntered[i] = NULL;
  }
  codeLength = 0;
}

void returnWrongMessage(){
  clearScreen();
  lcd.print("Wrong!");
  delay(1000);
}

void endGame(bool win){
  clearScreen();
  if(win){
    lcd.print("Disarmed");
    delay(2000);
    // TODO: Play Again Message and Input.
    GameState = POSTGAME;
  } else {
    lcd.print("Failed..");
    delay(1000);
    digitalWrite(ledPin, HIGH); // sets the LED on
    tone1.play(NOTE_A2, 200);
    delay(10);
    digitalWrite(ledPin, LOW); // sets the LED on
    tone1.play(NOTE_B1, 200);
    delay(10); // waits for a second
    tone1.play(NOTE_A2, 200);
    delay(10);
    tone1.play(NOTE_B1, 200);
    delay(10);
    // TODO: Play Again Message and Input.
  }
}

String timer(){
  while ( Mcount >= 15 ){
    GameState = ENDGAME;
  }
  
  if ( Mcount == 60){ // if Mcount is 60 do this operation
    Mcount = 0; // reset Mcount
    Hcount++;
  }
  
  if (Hcount > 23){
    Dcount++;
    Hcount = 0; // have to reset Hcount to "0" after 24hrs
  }

  if (Scount >= 59){ // if 60 do this operation
    Mcount++; // add 1 to Mcount
    Scount = 0;
  } 
  else { 
    unsigned long currentMillis = millis();
    if(currentMillis - secMillis > interval){
      secMillis = currentMillis;
      Scount++;

      tone1.play(NOTE_G5, 200);

      digitalWrite(ledPin2, HIGH); // sets the LED on
      delay(10); // waits for a second
      digitalWrite(ledPin2, LOW); // sets the LED off
      delay(10); // waits for a second
    }
  }
  
  String Day = String(Dcount), Hour = String(Hcount), Minute = String(Mcount), Second = String(Scount);
  String currentTime = String(Day + ":" + Hour + ":" + Minute + ":" + Second);
  
  return currentTime;
}

void keypadEvent(KeypadEvent key){
  switch (customKeypad.getState()){
    case PRESSED:
      codeEntered[codeLength] = key;
      tone1.play(NOTE_DS4, 100);
      codeLength++;
    break;
    case RELEASED:
      break;
    case HOLD:
      break;
  }
}
