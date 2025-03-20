#include <Adafruit_Fingerprint.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <EEPROM.h>
#include <SoftwareSerial.h>

SoftwareSerial espSerial(A0, A1);  // RX, TX

// Define the LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Define the fingerprint sensor
SoftwareSerial mySerial(2, 3);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

// Define Keypad
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte rowPins[ROWS] = {11, 10, 9, 8};
byte colPins[COLS] = {7, 6, 5, 4};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// Save the PIN in EEPROM, max 4 chars 
#define PIN_ADDRESS 0
char pinCode[5] = "1234";

// Define the invalid count
int invalidCount = 0;

// Configure for LCD
void setup() {
  Serial.begin(9600);
  espSerial.begin(9600);
  espSerial.println("AT");
  lcd.init();
  lcd.backlight();
  //Define output pin for Lock
  pinMode(12, OUTPUT);
  //Define output pin for buzzer
  pinMode(13, OUTPUT);
  finger.begin(57600);
  
  if (finger.verifyPassword()) {
    Serial.println("Fingerprint sensor found!");
  } else {
    Serial.println("Fingerprint sensor not found!");
    while (1);
  }
  EEPROM.get(PIN_ADDRESS, pinCode);
}

//Loop that the main screen  
void loop() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("1.Fingerprint");
  lcd.setCursor(0, 1);
  lcd.print("2.PIN | 3.Menu");
  
  char choice = waitForInput();
  if (choice == '1') {
    fingerprintLogin();
  } else if (choice == '2') {
    pinLogin();
  } else if (choice == '3') {
    menu();
  }
}

// Wait for input 1 -> 3
char waitForInput() {
  char key;
  while (1) {
    key = keypad.getKey();
    if (key) {
      if (key == '1' || key == '2' || key == '3' || key == '#') {
        return key;
      }
    }
  }
}

// Recv the input for PIN, * for confirm, # for decline
void inputPIN(char input[5], boolean *successInput) {
  int i = 0;
  while (i < 4) {
    char key = keypad.getKey();
    if (key) {
      if (!isdigit(key) && key != '#') { // if the input not # --> incorrect format
        lcd.clear();
        lcd.print("PIN must be...");
        lcd.setCursor(6,1);
        lcd.print("...number!");
        delay(3000);
        return;
      }
      if (key == '#') { // # to cancel and go back to the main screen 
        lcd.clear();
        lcd.print("Canceled!");
        delay(3000);
        return;
      }
      input[i++] = key;
      lcd.setCursor(i, 1);
      lcd.print('*');
    }
  }
  input[i] = '\0';
  *successInput = true;
}

//Recv the input for fingerprint ID, * for confirm, # for decline, max 3 numbers
void inputIDFinger(int *id) {
  int numDigits = 0;
  char key;
  while (1) {
    key = keypad.getKey();
    if (key) {
      if (key == '#') { // # to cancel and go back to main screen
        lcd.clear();
        lcd.print("Canceled!");
        delay(2000);
        return;
      }
      if (isdigit(key) && numDigits < 3) { //limit the input only 3 digits
        *id = *id * 10 + (key - '0');
        numDigits++;
        lcd.setCursor(0, 1);
        lcd.print(*id);
      }
      if (key == '*') break; // * to confirm
    }
  }
}

void invalidVerify() {
  invalidCount++;
  if (invalidCount >= 3) {
    lcd.clear();
    lcd.print("Access Denied!");
    lcd.setCursor(0, 1);
    lcd.print("Block in 10s!");
    digitalWrite(13, HIGH);
    sendLog("Login failed, try again in 10s");
    delay(10000);
    digitalWrite(13, LOW);
  } else {
    lcd.clear();
    lcd.print("Access Denied!");
    lcd.setCursor(0, 1);
    lcd.print("#");
    lcd.print(invalidCount);
    lcd.print(" wrong time");
    sendLog("Login failed, try again in 3s");
    delay(3000);
  }
}

// login with fingerprint
void fingerprintLogin() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Place Finger...");

  int attempts = 5; // can try 5 times, each time 1 second
  while (attempts--) { // must for 5 seconds in total or have to use another finger
    if (finger.getImage() == FINGERPRINT_OK) {
      if (finger.image2Tz() == FINGERPRINT_OK && finger.fingerFastSearch() == FINGERPRINT_OK) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Access Granted!");
 
        lcd.setCursor(0, 1);
        lcd.print("User ID: ");
        lcd.print(finger.fingerID);
        invalidCount = 0;
        digitalWrite(12, HIGH);
        sendLog("Login fingerprint, id: " + String(finger.fingerID));
        delay(5000);
        digitalWrite(12, LOW);
        return;
      }
    }
    delay(1000);
  }
  lcd.clear();
  invalidVerify();
}

// log in with PIN
void pinLogin() {
  lcd.clear();
  lcd.print("Enter PIN:");
  char input[5] = "";
  boolean successInput = false;
  inputPIN(input, &successInput); // call the func that the input mus PIN only
  if (successInput) {
    if (strcmp(input, pinCode) == 0) {
      lcd.clear();
      lcd.print("Access Granted!");
      invalidCount = 0;
      digitalWrite(12, HIGH);
      sendLog("Login PIN");
      delay(5000);
      digitalWrite(12, LOW);
    } else {
      invalidVerify();
    }
  }
}

// Menu 
void menu() {
  lcd.clear();
  lcd.print("Enter PIN:");
  char input[5] = "";
  boolean successInput = false;
  inputPIN(input, &successInput); //call PIN login func

  if (successInput) {
    // check the PIN with the one that have already stored  in  EEPROM
    if (strcmp(input, pinCode) == 0) {
      // if correct --> Menu
      lcd.clear();
      lcd.print("1.Modify Finger");
      lcd.setCursor(0, 1);
      lcd.print("2.Change PIN");

      char choice = waitForInput();
      if (choice == '1') modifyFinger();
      else if (choice == '2') changePin();
    } else {
      invalidVerify();
    }
  }
}

// add and delete fingerprints
void modifyFinger() {
  lcd.clear();
  lcd.print("1.Add finger");
  lcd.setCursor(0,1);
  lcd.print("2.Delete finger");
  char choice = waitForInput();
  if (choice == '1') addFinger();
  else if (choice == '2') deleteFinger();
}

// add Fingerprint
void addFinger() {
  lcd.clear();
  lcd.print("Enter ID (0-127):");
  int id = 0;
  int attempts = 0;

  inputIDFinger(&id); // call ID funcs

   // validate the ID
  if (id < 0 || id > 127) {
    lcd.clear();
    lcd.print("Invalid ID!");
    delay(3000);
    return;
  }

  lcd.clear();
  lcd.print("Place Finger...");
  while (finger.getImage() != FINGERPRINT_OK) {
    ++attempts;
    if (attempts == 5) {
      lcd.clear();
      lcd.print("Adding Failed!");
      sendLog("Adding new fingerprint failed!");
      delay(1000);
      return;
    }
    delay(1000);
  }
  attempts = 0;
  if (finger.image2Tz(1) != FINGERPRINT_OK) {
    lcd.clear();
    lcd.print("Try Again!");
    delay(3000);
    return;
  }

  lcd.clear();
  lcd.print("Remove Finger...");
  delay(2000);

  lcd.clear();
  lcd.print("Place Again...");
  while (finger.getImage() != FINGERPRINT_OK) {
    ++attempts;
    if (attempts == 5) {
      lcd.clear();
      lcd.print("Adding Failed!");
      sendLog("Adding new fingerprint failed!");
      delay(1000);
      return;
    }
    delay(1000);
  }
  if (finger.image2Tz(2) != FINGERPRINT_OK) {
    lcd.clear();
    lcd.print("Try Again!");
    delay(3000);
    return;
  }

  if (finger.createModel() != FINGERPRINT_OK) {
    lcd.clear();
    lcd.print("Error Creating!");
    delay(3000);
    return;
  }

  if (finger.storeModel(id) != FINGERPRINT_OK) {
    lcd.clear();
    lcd.print("Store Failed!");
    delay(3000);
    return;
  }

  lcd.clear();
  lcd.print("Successfully Added!");
  sendLog("Successfully add new fingerprint!");
  delay(3000);
}

// Del Finger
void deleteFinger() {
  lcd.clear();
  lcd.print("Enter ID to Del:");
  int id = 0;

  inputIDFinger(&id);

  // Validate 
  if (id < 0 || id > 127) {
    lcd.clear();
    lcd.print("Invalid ID!");
    delay(3000);
    return;
  }

  // Del finger
  lcd.clear();
  lcd.print("Deleting...");
  if (finger.deleteModel(id) == FINGERPRINT_OK) {
    lcd.clear();
    lcd.print("Deleted!");
    sendLog("Deleted! Finger no: " + String(finger.fingerID));
  } else {
    lcd.clear();
    lcd.print("Error!");
  }
  delay(3000);
}

// Change PIN
void changePin() {
  lcd.clear();
  lcd.print("Enter new PIN:");
  char newPin[5] = "";
  boolean successInput = false;
  inputPIN(newPin, &successInput); 
  if (successInput) {
    strcpy(pinCode, newPin);
    EEPROM.put(PIN_ADDRESS, pinCode);
    lcd.clear();
    lcd.print("PIN Changed!");
    sendLog("PIN Changed!", "PIN");
    delay(3000);
  }
}

void sendLog(String event) {
  espSerial.listen();
  delay(1000);
  String logData = "?event=" + event;
  espSerial.println(logData); // send log to ESP8266 to send to spreadsheet
  mySerial.listen();
}
