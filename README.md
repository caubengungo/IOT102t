# 🔐 IOT102c - Smart Lock  

- Welcome to our **Smart Lock** project! This project is developed as part of the IOT102c course and focuses on creating a secure and efficient Arduino-based smart lock system using various hardware components.
- The Smart Lock system using Arduino and ESP8266 enhances security and convenience for personal use.
- Users can unlock through multiple methods, including PIN code and fingerprint authentication.
- Access data is logged and sent to Google Spread Sheet via Wi-Fi, allowing remote monitoring.

## ✨ Features

🔢 **PIN & Fingerprint Authentication** – Unlock the door using a secure PIN code or fingerprint sensor.

🔊 **Buzzer** – Audio feedback for successful and failed authentication attempts.

📟 **LCD Display** – Real-time status updates and user interaction messages.

🔄 **Relay & Solenoid Lock** – Controls the locking and unlocking mechanism securely.

🔌 **I2C Communication** – Efficient data transmission between components.

🔐 **Admin Access** – Manage PIN codes and stored fingerprints securely.

## 🛠 Hardware Components

🔢 **4x4 Keypad** – User input for PIN authentication.

🔌 **Arduino Board** – Central controller for the system.

🔊 **Buzzer** – Alerts for authentication success/failure.

📟 **LCD 16x2 with I2C** – Displays system status and user prompts.

🔒 **Solenoid Lock** – Electronic locking mechanism.

🔁 **Relay Module** – Controls the power to the solenoid lock.

📡 **ESP8266** – Enhanced connectivity options.

## Deploying
- After successfully connecting the wires based on the circuit_image.png, first, upload the code from the file IOTAssignmentSpring25.ino to the Arduino R3 board. Then, upload the code for the ESP8266 module from the file ESP8266_Config.ino.
- Note: Before uploading the ESP8266 code, update the WiFi name and password according to the network you are using. Next, create a Google Spreadsheet, then go to the Extensions menu and select App Script. Once the App Script editor opens, copy the following code into a file with a .gs extension:

'''
  System.out.println('Test');
'''

- After that, click "Deploy" and select "New deployment". In the "Select type" section, choose "Web app" and set "Who has access" to "Anyone", then click "Deploy".
- Once the deployment is complete, copy the URL and paste it into the scriptURL variable inside the ESP8266 code.
