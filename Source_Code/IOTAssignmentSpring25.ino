#include <Adafruit_Fingerprint.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <Servo.h>
#include <EEPROM.h>

// Khai báo LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Khai báo cảm biến vân tay
SoftwareSerial mySerial(2, 3);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

// Khai báo Keypad
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

// Mã PIN lưu trong EEPROM, max mã PIN chỉ được 4 ký tự
#define PIN_ADDRESS 0
char pinCode[5] = "1234";

//khai báo biến đếm số lần sai
int invalidCount = 0;

//tiến hành cấu hình cho các thiết bị ngoại vi như lcd, servo,...
void setup() {
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();
  //khai báo chân 12 output cho lock
  pinMode(12, OUTPUT);
  //khai báo chân 13 output cho buzzer
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

//vòng loop luôn hiển thị màn hình chính
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

// Hàm chờ cho đến khi người dùng nhập từ 1 -> 3
char waitForInput() {
  char key;
  while (1) {
    key = keypad.getKey();
    if (key) {
      if (key == '1' || key == '2' || key == '3' || key == '3') {
        return key;
      }
    }
  }
}

//hàm nhận input cho mã PIN, không cần bấm * để hoàn tất, bấm # để huỷ
void inputPIN(char input[5], boolean *successInput) {
  int i = 0;
  while (i < 4) {
    char key = keypad.getKey();
    if (key) {
      if (!isdigit(key) && key != '#') { //nếu các ký tự khác ngoài số nhưng khác # thì là sai format của PIN
        lcd.clear();
        lcd.print("PIN must be...");
        lcd.setCursor(6,1);
        lcd.print("...number!");
        delay(3000);
        return;
      }
      if (key == '#') { //bấm # để huỷ quá trình và về màn hình chính
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

//hàm nhập ID của cảm biến vân tay, # để huỷ và * để hoàn tất, max 3 số
void inputIDFinger(int *id) {
  int numDigits = 0;
  char key;
  while (1) {
    key = keypad.getKey();
    if (key) {
      if (key == '#') { //bấm # để huỷ quá trình và về màn hình chính
        lcd.clear();
        lcd.print("Canceled!");
        delay(2000);
        return;
      }
      if (isdigit(key) && numDigits < 3) { // Giới hạn input là chỉ cho số và max là 3 ký tự số
        *id = *id * 10 + (key - '0');
        numDigits++;
        lcd.setCursor(0, 1);
        lcd.print(*id);
      }
      if (key == '*') break; // Bấm * để xác nhận ID
    }
  }
}

void invalidVerified() {
  invalidCount++;
  if (invalidCount >= 3) {
    lcd.clear();
    lcd.print("Access Denied!");
    lcd.setCursor(0, 1);
    lcd.print("Block in 10s!");
    digitalWrite(13, HIGH);
    delay(10000);
    digitalWrite(13, LOW);
  } else {
    lcd.clear();
    lcd.print("Access Denied!");
    lcd.setCursor(0, 1);
    lcd.print("#");
    lcd.print(invalidCount);
    lcd.print(" wrong time");
    delay(3000);
  }
}

// Đăng nhập bằng vân tay
void fingerprintLogin() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Place Finger...");

  int attempts = 5; //cho phép thử 5 lần, nếu không đặt vân tay, hoặc failed thì mỗi lần mất 1s
  while (attempts--) { //tổng cộng là sẽ chờ user tầm 5s để đặt tay hoặc sửa ngón khác ;v
    if (finger.getImage() == FINGERPRINT_OK) {
      if (finger.image2Tz() == FINGERPRINT_OK && finger.fingerFastSearch() == FINGERPRINT_OK) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("Access Granted!");
 
        lcd.setCursor(0, 1);
        lcd.print("User ID: ");
        lcd.print(finger.fingerID);

        doorLock.write(90);
        delay(5000);
        doorLock.write(0);
        return;
      }
    }
    delay(1000);
  }
  lcd.clear();
  lcd.print("Access Denied!");
  delay(3000);
}

// Đăng nhập bằng mã PIN
void pinLogin() {
  lcd.clear();
  lcd.print("Enter PIN:");
  char input[5] = "";
  boolean successInput = false;
  inputPIN(input, &successInput); //gọi hàm chỉ nhập mã PIN
  
  if (successInput) {
    if (strcmp(input, pinCode) == 0) {
      lcd.clear();
      lcd.print("Access Granted!");
      doorLock.write(90);
      delay(5000);
      doorLock.write(0);
    } else {
      lcd.clear();
      lcd.print("Access Denied!");
      delay(3000);
    }
  }
}

// Menu quản lý
void menu() {
  lcd.clear();
  lcd.print("Enter PIN:");
  char input[5] = "";
  boolean successInput = false;
  inputPIN(input, &successInput); //gọi hàm chỉ nhập mã PIN

  if (successInput) {
    // Kiểm tra mã PIN với mã PIN trong EEPROM
    if (strcmp(input, pinCode) != 0) {
      lcd.clear();
      lcd.print("Access Denied!");
      delay(3000);
      return; // Return về màn hình chính nếu mã PIN sai
    }

    // Nếu đúng, vào menu
    lcd.clear();
    lcd.print("1.Modify Finger");
    lcd.setCursor(0, 1);
    lcd.print("2.Change PIN");

    char choice = waitForInput();
    if (choice == '1') modifyFinger();
    else if (choice == '2') changePin();
  }
}

// Thêm hoặc xóa vân tay
void modifyFinger() {
  lcd.clear();
  lcd.print("1.Add finger");
  lcd.setCursor(0,1);
  lcd.print("2.Delete finger");
  char choice = waitForInput();
  if (choice == '1') addFinger();
  else if (choice == '2') deleteFinger();
}

// Thêm dấu vân tay
void addFinger() {
  lcd.clear();
  lcd.print("Enter ID (0-127):");
  int id = 0;
  
  inputIDFinger(&id); //gọi hàm nhập ID

   // Kiểm tra ID hợp lệ
  if (id < 0 || id >= 127) {
    lcd.clear();
    lcd.print("Invalid ID!");
    delay(3000);
    return;
  }

  lcd.clear();
  lcd.print("Place Finger...");
  while (finger.getImage() != FINGERPRINT_OK);
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
  while (finger.getImage() != FINGERPRINT_OK);
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
  delay(3000);
}

// Xóa dấu vân tay
void deleteFinger() {
  lcd.clear();
  lcd.print("Enter ID to Del:");
  int id = 0;

  inputIDFinger(&id);

  // Kiểm tra ID hợp lệ
  if (id < 0 || id >= 127) {
    lcd.clear();
    lcd.print("Invalid ID!");
    delay(3000);
    return;
  }

  // Xóa dấu vân tay
  lcd.clear();
  lcd.print("Deleting...");
  if (finger.deleteModel(id) == FINGERPRINT_OK) {
    lcd.clear();
    lcd.print("Deleted!");
  } else {
    lcd.clear();
    lcd.print("Error!");
  }
  delay(3000);
}

// Thay đổi mã PIN
void changePin() {
  lcd.clear();
  lcd.print("Enter new PIN:");
  char newPin[5] = "";
  boolean successInput = false;
  inputPIN(newPin, &successInput); //gọi hàm chỉ nhập mã PIN
  if (successInput) {
    strcpy(pinCode, newPin);
    EEPROM.put(PIN_ADDRESS, pinCode);
    lcd.clear();
    lcd.print("PIN Changed!");
    delay(3000);
  }
}
