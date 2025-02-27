#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <Adafruit_Fingerprint.h>
#include <Wire.h>
#include <Servo.h>
#include <EEPROM.h>

// declare lcd using I2C driver
LiquidCrystal_I2C lcd(0x27, 16, 2);

// Declare fingerprint using Adafruit_Fingerprint
#if (defined(__AVR__) || defined(ESP8266)) && !defined(__AVR_ATmega2560__)
SoftwareSerial mySerial(2, 3);
#else
#define mySerial Serial1
#endif
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);

// Declare servo using Servo
Servo myservo;

// Keypad 4x4
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
};
byte rowPins[ROWS] = {4, 5, 6, 7};
byte colPins[COLS] = {8, 9, 10, 11};
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// Save passcode into EEPROM
const int PASSCODE_ADDR = 0;
char storedPasscode[5] = "1234";

void setup() {
    myservo.attach(12);
    myservo.write(150); //Default the door will be closed
    
    lcd.init();
    lcd.backlight();
    lcd.clear();

    Serial.begin(9600);

    //Begin: khu vực của vân tay, không đụng đến
    while (!Serial);  // For Yun/Leo/Micro/Zero/...
    delay(100);
    Serial.println("\n\nAdafruit finger detect test");
    // set the data rate for the sensor serial port
    finger.begin(57600);
    delay(5);
    if (finger.verifyPassword()) {
      Serial.println("Found fingerprint sensor!");
    } else {
      Serial.println("Did not find fingerprint sensor :(");
      while (1) { delay(1); }
    }

    Serial.println(F("Reading sensor parameters"));
    finger.getParameters();
    Serial.print(F("Status: 0x")); Serial.println(finger.status_reg, HEX);
    Serial.print(F("Sys ID: 0x")); Serial.println(finger.system_id, HEX);
    Serial.print(F("Capacity: ")); Serial.println(finger.capacity);
    Serial.print(F("Security level: ")); Serial.println(finger.security_level);
    Serial.print(F("Device address: ")); Serial.println(finger.device_addr, HEX);
    Serial.print(F("Packet len: ")); Serial.println(finger.packet_len);
    Serial.print(F("Baud rate: ")); Serial.println(finger.baud_rate);

    finger.getTemplateCount();  // Get number of fingerprints from fgp sensor

    if (finger.templateCount == 0) {
        Serial.print("Sensor doesn't contain any fingerprint data. Please run the 'enroll' example.");
    } else {
        Serial.println("Waiting for valid finger...");
        Serial.print("Sensor contains "); 
        Serial.print(finger.templateCount); 
        Serial.println(" templates");
    }
    //END: khu vực của vân tay

    loadPasscode();
}

void loop() {
    lcd.clear();
    lcd.print("1: Fingerprint");
    lcd.setCursor(0, 1);
    lcd.print("2: Enter Code 3:Change");
    char key = keypad.getKey();
    
    if (key == '1') {
        loginFingerPrint();
    } else if (key == '2') {
        loginPasscode();
    } else if (key == '3') {
        manageMenu();
    }
}

//--------------------------------------------------------------------------------------------------------

void loadPasscode() {
    int len = EEPROM.read(PASSCODE_ADDR);  // Đọc độ dài của passcode từ EEPROM
    if (len > 0 && len <= 16) {  // Giới hạn độ dài passcode để tránh lỗi
        memset(storedPasscode, '\0', sizeof(storedPasscode));  // Xóa dữ liệu cũ
        for (int i = 0; i < len; i++) {
            storedPasscode[i] = EEPROM.read(PASSCODE_ADDR + 1 + i);  // Đọc từng ký tự passcode
        }
    }
}

//--------------------------------------------------------------------------------------------------------

void openDoor() {
    myservo.write(45);
    delay(3000);
    closeDoor();
}

void closeDoor() {
    myservo.write(150);
}

//--------------------------------------------------------------------------------------------------------

//Option 1: Khi vào đăng nhập bằng vân tay
void loginFingerPrint() {
    lcd.clear();
    lcd.print("Place Finger");
    lcd.setCursor(0, 1);
    lcd.print("Press * to Exit");
    int fingerprintID = -1; //khởi tạo giá trị ban đầu -1 nghĩa là vân tay không hợp lệ

    while (millis() - startTime < 5000) {  // Chờ max 5 giây
        char key = keypad.getKey();  // Kiểm tra nếu có phím bấm
        if (key == '#') {  // bấm '*' thùi user quay lại menu chính
            return;
        }
        fingerprintID = getFingerprintID();
        if (fingerprintID >= 0) break;  // Có vân tay hợp lệ -> thoát khỏi vòng lặp
    }

    if (fingerprintID >= 0) {
        lcd.clear();
        lcd.print("Access Granted");
        lcd.setCursor(0, 1);
        lcd.print("With ID: ");
        lcd.print(fingerprintID);
        openDoor();
    } else {
        lcd.clear();
        lcd.print("Access Denied");
        delay(2000);
    }
}

//--------------------------------------------------------------------------------------------------------

////Option 2: Khi vào đăng nhập bằng vân tay
void loginPasscode() {
    lcd.clear();
    lcd.print("Enter Passcode:");
    lcd.setCursor(0, 1);
    String enteredPasscode = "";
    char key;
    while (true) {
        key = keypad.getKey();
        if (key) {
            if (key == '#') break;
            enteredPasscode += key;
            lcd.print('*');
        }
    }
    if (enteredPasscode == storedPasscode) {
        lcd.clear();
        lcd.print("Access Granted");
        openDoor();
    } else {
        lcd.clear();
        lcd.print("Wrong Passcode");
        delay(2000);
    }
}

//--------------------------------------------------------------------------------------------------------

////Option 3: Chọn chức năng phụ
void manageMenu() {
    lcd.clear();
    lcd.print("1:Add 2:Del 3:Pass");
    char key = keypad.getKey();
    if (key == '1') {
        registerFingerprint();
    } else if (key == '2') {
        deleteFingerprint();
    } else if (key == '3') {
        changePasscode();
    }
}

//Option 3 -> thêm vân tay
void registerFingerprint() {
    lcd.clear();
    lcd.print("Enter ID:");
    
    // Nhập ID từ Keypad
    String idStr = "";
    char key;
    while (true) {
        key = keypad.getKey();
        if (key == '#') break;  // Nhấn '#' để xác nhận ID
        if (key && isDigit(key)) {
            idStr += key;
            lcd.setCursor(idStr.length(), 1);
            lcd.print(key);
        }
    }
    
    int id = idStr.toInt();  // Chuyển chuỗi ID thành số nguyên
    if (id <= 0 || id > 127) {
        lcd.clear();
        lcd.print("Invalid ID");
        delay(2000);
        return;
    }

    // Kiểm tra xem ID đã tồn tại chưa
    if (finger.loadModel(id) == FINGERPRINT_OK) {
        lcd.clear();
        lcd.print("ID Exists!");
        delay(2000);
        return;
    }

    lcd.clear();
    lcd.print("Place Finger");

    // Lấy ảnh vân tay lần 1
    while (finger.getImage() != FINGERPRINT_OK) {
      if (millis() - startTime > 10000) {
          lcd.clear();
          lcd.print("Timeout!");
          delay(2000);
          return;
      }
    }
    finger.image2Tz(1);

    lcd.clear();
    lcd.print("Remove Finger");
    delay(2000);

    // Lấy ảnh vân tay lần 2
    lcd.clear();
    lcd.print("Place Again");
    while (finger.getImage() != FINGERPRINT_OK) {
      if (millis() - startTime > 10000) {
          lcd.clear();
          lcd.print("Timeout!");
          delay(2000);
          return;
      }
    }
    finger.image2Tz(2);

    // So sánh hai ảnh
    if (finger.createModel() != FINGERPRINT_OK) {
        lcd.clear();
        lcd.print("Match Failed");
        delay(2000);
        return;
    }

    // Lưu vân tay vào ID đã nhập
    if (finger.storeModel(id) == FINGERPRINT_OK) {
        lcd.clear();
        lcd.print("Fingerprint Saved");
    } else {
        lcd.clear();
        lcd.print("Save Failed");
    }
    delay(2000);
}

//--------------------------------------------------------------------------------------------------------

//Option 3 -> xoá vân tay
void deleteFingerprint() {
  lcd.clear();
    lcd.print("Enter ID to Del:");

    // Nhập ID từ Keypad
    String idStr = "";
    char key;
    while (true) {
        key = keypad.getKey();
        if (key == '#') break;  // Nhấn '#' để xác nhận ID
        if (key && isDigit(key)) {
            idStr += key;
            lcd.setCursor(idStr.length(), 1);
            lcd.print(key);
        }
    }

    int id = idStr.toInt();  // Chuyển chuỗi ID thành số nguyên
    if (id <= 0 || id > 127) {
        lcd.clear();
        lcd.print("Invalid ID");
        delay(2000);
        return;
    }

    // Kiểm tra xem ID có tồn tại không trước khi xóa
    if (finger.loadModel(id) != FINGERPRINT_OK) {
        lcd.clear();
        lcd.print("ID Not Found!");
        delay(2000);
        return;
    }

    // Xóa vân tay nếu ID hợp lệ
    if (finger.deleteModel(id) == FINGERPRINT_OK) {
        lcd.clear();
        lcd.print("Fingerprint Deleted");
    } else {
        lcd.clear();
        lcd.print("Delete Failed!");
    }
    delay(2000);
}

//--------------------------------------------------------------------------------------------------------

void changePasscode() {
    lcd.clear();
    lcd.print("New Passcode:");
    String newPasscode = "";
    char key;
    while (true) {
        key = keypad.getKey();
        if (key) {
            if (key == '#') break;
            newPasscode += key;
            lcd.print('*');
        }
    }
    newPasscode.toCharArray(storedPasscode, 5);
    EEPROM.put(PASSCODE_ADDR, storedPasscode);
    lcd.clear();
    lcd.print("Passcode Updated");
}