
#include "ESP32TimerInterrupt.h"
#include <esp_now.h>
#include <WiFi.h>
#include "Audio.h"
#include "SD.h"
#include "FS.h"
#include <SPI.h>
#include <Wire.h>
#include <DFRobot_MLX90614.h>
#include <ADXL345.h>
#include <MFRC522.h>


#define SLAVE_MAC "64:E8:33:8C:FF:1A"  //for your master device

#define TIMER0_INTERVAL_MS 200

// KEYs
#define KEY 5
#define DEBOUNCE 200  // in ms

//LEDs
#define BLUE 2
#define RED 13

// microSD Card Reader connections
#define SD_CS 18
#define SPI_SCK 19
#define SPI_MOSI 4
#define SPI_MISO 16

// I2S Connections
#define I2S_DOUT 25
#define I2S_BCLK 27
#define I2S_LRC 26

//rfid
#define I2C_SDA 21
#define I2C_SCL 22
#define BLU 2
#define RED 13
#define GRN 27

// Defining the ports for RFID Tag module
#define SCK 19  //SCK
#define MISO 16
#define MOSI 4
#define SS_PIN 14  //SDA
#define RST_PIN 17

MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance.

//pressure sensor
#define PRESSURE_SENSOR_PIN 39   // ESP32 pin GIOP36 (ADC0)
#define PRESSURE_SENSOR_PIN1 34  // ESP32 pin GIOP36 (ADC0)

#define MAX_NUM_SOUND 4

uint8_t broadcastAddress[] = { 0x74, 0x4D, 0xBD, 0xE1, 0x92, 0xD8 };

ADXL345 accel(0x53);  // Default address = 0x53
float x = 0;
float y = 0;
float z = 0;
float prev_x = 0;
float prev_y = 0;
float prev_z = 0;
float THRESHOLD = 2;
int analogValue = 0;
int analogValue1 = 0;
bool times_up = false;
bool dum = false;
int counter = 0;
int card_number = 0;
int read_card = 0;

long unsigned int currentTime = 0;
long unsigned int last_key_pressed = 0;
byte num_sound = 0;
byte deviceID = accel.readDeviceID();
bool end_mp3 = false;  //required by library
bool keypressed = true;

float ambientTemp = 0;
float objectTemp = 0;

// Structure example to receive data
// Must match the sender structure
int count1;
bool mother = false;
bool hasExecuted = false;

// instantiate an object to drive our sensor
DFRobot_MLX90614_I2C sensor;

// Create Audio object
Audio audio;

// Init ESP32 timer 0
ESP32Timer ITimer0(0);

//core v2.0.0+, you can't use Serial.print/println in ISR or crash.
// and you can't use float calculation inside ISR
// Only OK in core v1.0.6-
bool IRAM_ATTR TimerHandler0(void *timerNo) {
  times_up = true;
  counter++;
  return true;
}

// ESP_NOW receiving routine
// callback function that will be executed when data is received
void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len) {
  memcpy(&count1, incomingData, 1);
  Serial.print("New Count: ");
  Serial.println(count1);
  mother = true;
}

void read_audio(const char *info) {
  audio.connecttoFS(SD, info);
  Serial.println();
  Serial.println("success!");
}

void playRandomPressureAudio() {
  // Array of pressure audio file names
  const char *pressureAudioFiles[] = {
    "pressure_final.mp3",
    "pressure_final2.mp3",
    "pressure_final3.mp3"
  };
  // Number of audio files in the array
  const int numberOfFiles = sizeof(pressureAudioFiles) / sizeof(pressureAudioFiles[0]);

  // Generate a random index to select a file
  int randomIndex = random(0, numberOfFiles);

  // Select the audio file
  const char *selectedFile = pressureAudioFiles[randomIndex];

  // Play the selected audio file
  Serial.print("Playing random pressure audio file: ");
  Serial.println(selectedFile);

  // Use your existing read_audio function to play the file
  read_audio(selectedFile);

  // Wait for playback to finish
  while (end_mp3 == false) {
    audio.loop();
  }
  end_mp3 = false;  // Reset the flag for future use
}

void playRandomDumAudio() {
  // Array of dum audio file names
  const char *dumAudioFiles[] = {
    "again.mp3",
    "again_1.mp3",
    "yun.mp3"
  };
  // Number of audio files in the array
  const int numberOfFiles = sizeof(dumAudioFiles) / sizeof(dumAudioFiles[0]);

  // Generate a random index to select a file
  int randomIndex = random(0, numberOfFiles);

  // Select the audio file
  const char *selectedFile = dumAudioFiles[randomIndex];

  // Play the selected audio file
  Serial.print("Playing random dum audio file: ");
  Serial.println(selectedFile);

  // Use your existing read_audio function to play the file
  read_audio(selectedFile);

  // Wait for playback to finish
  while (end_mp3 == false) {
    audio.loop();
  }
  end_mp3 = false;  // Reset the flag for future use
}

void playRandomGreetingAudio() {
  // Array of greeting audio file names
  const char *greetingAudioFiles[] = {
    "intro_1.mp3",
    "new_int.mp3"
  };
  // Number of audio files in the array
  const int numberOfFiles = sizeof(greetingAudioFiles) / sizeof(greetingAudioFiles[0]);

  // Generate a random index to select a file
  int randomIndex = random(0, numberOfFiles);

  // Select the audio file
  const char *selectedFile = greetingAudioFiles[randomIndex];

  // Play the selected audio file
  Serial.print("Playing random greeting audio file: ");
  Serial.println(selectedFile);

  // Use your existing read_audio function to play the file
  read_audio(selectedFile);

  // Wait for playback to finish
  while (end_mp3 == false) {
    audio.loop();
  }
  end_mp3 = false;  // Reset the flag for future use
}

void temp_check() {
  //Check temperature
  // PLEASE CHECK IF IT IS NECESSARY TO RECORD AN AUDIO TO GUIDE THE USER
  ambientTemp = sensor.getAmbientTempCelsius();
  objectTemp = sensor.getObjectTempCelsius();

  Serial.print("Ambient: ");
  Serial.println(ambientTemp);
  Serial.print("Object: ");
  Serial.println(objectTemp);

  if (abs(objectTemp - ambientTemp) > 0.1) {
    if (objectTemp > 30) {
      // Open music file for temperature > 30
      read_audio("temp_30.mp3");
      while (end_mp3 == false) {
        audio.loop();
      }
      end_mp3 = false;
    } else if (objectTemp < 22) {
      // Open music file for temperature < 25
      read_audio("temp_small26.mp3");
      while (end_mp3 == false) {
        audio.loop();
      }
      end_mp3 = false;
    }
    else {
      // Open music file otherwise
      read_audio("normal_temp.mp3");
      while (end_mp3 == false) {
        audio.loop();
      }
      end_mp3 = false;      
    }
  }
}

void dumDetection(int duration) {
  // Serial.println("Executing dumDetection()");
  Serial.print("Game: 1");
  counter = 0;
  int time_limit = duration;
  while (counter < time_limit) {
    Serial.print("Dum Game ");
    Serial.println(counter);
    if (accel.update()) {  // If update successful
      x = accel.getX();
      y = accel.getY();
      z = accel.getZ();
      // Serial.print("X = "); Serial.println(x);
      // Serial.print("Y = "); Serial.println(y);
      // Serial.print("Z = "); Serial.println(z);
      if (abs(x - prev_x) > THRESHOLD || abs(y - prev_y) > THRESHOLD || abs(z - prev_z) > THRESHOLD) {
        dum = true;
      } else {
        dum = false;
      }
      if (dum == true) {
        playRandomDumAudio();
        // while (end_mp3 == false) {
        //   audio.loop();
        // }
        end_mp3 = false;
        dum = false;
      }
      prev_x = x;
      prev_y = y;
      prev_z = z;
    }
  }
  hasExecuted = true;
}

void motherIOCheck(int duration) {
  // Serial.println("Executing motherIOCheck()");
  Serial.print("Game: 2");
  counter = 0;
  int time_limit = duration;
  while (counter < time_limit) {
    Serial.print("Mother Game ");
    Serial.println(counter);
    if (mother == true) {
      // Open music file
      read_audio("me_good.mp3");
      while (end_mp3 == false) {
        audio.loop();
      }
      end_mp3 = false;
      mother = false;
      hasExecuted = true;
      return;
    }
    mother = false;
  }
  hasExecuted = true;
}

void pressureSensorCheck_1(int duration) {
  // Serial.println("Executing pressureSensorCheck_1()");
  Serial.print("Game: 3");
  counter = 0;
  int time_limit = duration;
  while (counter < time_limit) {
    analogValue = analogRead(PRESSURE_SENSOR_PIN);
    Serial.print("Pressure 1 Game ");
    Serial.println(counter);
    if (analogValue > 1500) {
      playRandomPressureAudio();
      hasExecuted = true;
      return;
    }
  }
  hasExecuted = true;
}

void pressureSensorCheck_2(int duration) {
  // Serial.println("Executing pressureSensorCheck_2()");
  Serial.print("Game: 4");
  counter = 0;
  int time_limit = duration;
  while (counter < time_limit) {
    analogValue1 = analogRead(PRESSURE_SENSOR_PIN1);
    Serial.print("Pressure 2 Game ");
    Serial.println(counter);
    if (analogValue1 > 1500) {
      playRandomPressureAudio();
      hasExecuted = true;
      return;
    }
  }
  hasExecuted = true;
}

void rfid_check(int duration) {
  // Show the card
  Serial.print("Game: 5");
  Serial.println("RFID devices check - Waiting for you");
  int time_limit = duration;
  counter = 0;
  card_number = rfid();
  while (counter < time_limit) {
    Serial.print("RFID game ");
    Serial.println(counter);
    if (card_number == 0) {
      for (int j = 0; j < 10; j++) {
        card_number = rfid();
      }
    }
    else if (card_number == 1 && card_number != read_card) {
      // Open music file for temperature > 30
      read_card = card_number;
      read_audio("nei_final.mp3");
      while (end_mp3 == false) {
        audio.loop();
      }
      end_mp3 = false;
      hasExecuted = true;
    } else if (card_number == 2 && card_number != read_card) {
      // Open music file for temperature < 25
      read_card = card_number;
      read_audio("schoolbag_final.mp3");
      while (end_mp3 == false) {
        audio.loop();
      }
      end_mp3 = false;
      hasExecuted = true;
    }
  }
  hasExecuted = true;
}

int rfid() {
  // Look for new cards
  if (!mfrc522.PICC_IsNewCardPresent()) {
    Serial.println("no card!");
    return 0;
  }
  // Select one of the cards
  if (!mfrc522.PICC_ReadCardSerial()) {
    Serial.println("no card!");
    return 0;
  }

  //Show UID on serial monitor
  Serial.print("UID tag :");
  String content = "";

  byte letter;  // Extract the unique ID of the card detected.
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
    content.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
    content.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  Serial.println();
  content.toUpperCase();

  // Milk Bottle
  if (content.substring(1) == "04 76 68 7A 74 1D 91") {
    return 1;
  }

  // School Bag
  else if (content.substring(1) == "04 76 69 7A 74 1D 91") {
    return 2;
  } else {
    Serial.println("Wrong Card!");
    return 0;
  }
}

void randomExecute() {
  // Randomly select one of the functions to execute
  int randomChoice = random(1, 6);  // Random number between 1 and 3 (inclusive)

  switch (randomChoice) {
    case 1:
      read_audio("dum_i.mp3");
      while (end_mp3 == false) {
        audio.loop();
      }
      end_mp3 = false;
      Serial.println("Lift up!");
      dumDetection(50);
      break;
    case 2:
      read_audio("neinei_i.mp3");
      while (end_mp3 == false) {
        audio.loop();
      }
      end_mp3 = false;
      Serial.println("Clothes buttoning!");
      motherIOCheck(50);
      break;
    case 3:
      read_audio("schoolbag_i.mp3");
      while (end_mp3 == false) {
        audio.loop();
      }
      end_mp3 = false;
      Serial.println("Gentle squeeze 1!");
      pressureSensorCheck_1(50);
      break;
    case 4:
      read_audio("play_i.mp3");
      while (end_mp3 == false) {
        audio.loop();
      }
      end_mp3 = false;
      Serial.println("Gentle squeeze 2!");
      pressureSensorCheck_2(50);
      break;
    case 5:
      Serial.println("Accessorizing!");
      rfid_check(50);
      break;
    default:
      Serial.println("No valid choice selected");
      break;
  }
}

void setup() {
  // Start Serial Port
  Serial.begin(115200);
  delay(100);
  Serial.println("System Start!");

  // Set microSD Card CS as OUTPUT and set HIGH
  pinMode(SD_CS, OUTPUT);
  digitalWrite(SD_CS, HIGH);

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);

  //void setup for rfid
  SPI.begin(SCK, MISO, MOSI, SS_PIN);  // Initiate SPI bus
  mfrc522.PCD_Init();                  // Initiate MFRC522

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info
  esp_now_register_recv_cb(OnDataRecv);
  // Init ESP_NOW ends
  // Initialize SPI bus for microSD Card
  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);

  // Start microSD Card
  if (!SD.begin(SD_CS)) {
    Serial.println("Error accessing microSD card!");
    while (true)
      ;
  }

  // initialize the sensor
  while (NO_ERR != sensor.begin()) {
    Serial.println("Communication with device failed, please check connection");
    delay(3000);
  }
  Serial.println("Begin ok!");

  sensor.enterSleepMode();
  delay(50);
  sensor.enterSleepMode(false);
  delay(200);

  // Setup I2S
  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);

  // Set Volume 0..21
  audio.setVolume(21);

  //AXL345 G sensor setting
  if (!accel.writeRate(ADXL345_RATE_200HZ)) {
    Serial.println("write rate: failed");
    while (1) {
      delay(100);
    }
  }

  if (!accel.writeRange(ADXL345_RANGE_16G)) {
    Serial.println("write range: failed");
    while (1) {
      delay(100);
    }
  }

  if (!accel.start()) {
    Serial.println("start: failed");
    while (1) {
      delay(100);
    }
  }

  // Greeting
  // read_audio("new_int.mp3");
  // while (end_mp3 == false) {
  //   audio.loop();
  // }
  // end_mp3 = false;
  playRandomGreetingAudio();
  Serial.println("finish greeting");

  // Check the temperature
  delay(1000);
  temp_check();
  delay(2000);

  // Start the timer
  // Interval in microsecs
  if (ITimer0.attachInterruptInterval(TIMER0_INTERVAL_MS * 1000, TimerHandler0)) {
    Serial.print(F("Starting  ITimer0 OK, millis() = "));
    Serial.println(millis());
  } else
    Serial.println(F("Can't set ITimer0. Select another freq. or timer"));
}

void loop() {
  // STORYLINE BEGINS
  if (hasExecuted == false) {
    randomExecute();  // Ensure this is re-enabled if needed
    Serial.println("Random execution has finished!");
  }

  // STORYLINE ENDS
  else {
    dumDetection(1);
    motherIOCheck(1);
    pressureSensorCheck_1(1);
    pressureSensorCheck_2(1);
    rfid_check(1);
    // Event-driven execution of sensing functions based on sensor changes
    // if (accel.update()) {  // Check for accelerometer updates
    //   x = accel.getX();
    //   y = accel.getY();
    //   z = accel.getZ();
    //   if (abs(x - prev_x) > THRESHOLD || abs(y - prev_y) > THRESHOLD || abs(z - prev_z) > THRESHOLD) {
    //     dumDetection(1);  // Execute dumDetection if significant movement is detected
    //   }
    //   prev_x = x;
    //   prev_y = y;
    //   prev_z = z;
    // }

    // analogValue = analogRead(PRESSURE_SENSOR_PIN);
    // if (analogValue > 1500) {
    //   pressureSensorCheck_1(1);  // Execute pressureSensorCheck_1 if pressure exceeds threshold
    // }

    // analogValue1 = analogRead(PRESSURE_SENSOR_PIN1);
    // if (analogValue1 > 1500) {
    //   pressureSensorCheck_2(1);  // Execute pressureSensorCheck_2 if pressure exceeds threshold
    // }

    // int card_number = rfid();
    // if (card_number == 1 || card_number == 2) {
    //   rfid_check(1);  // Execute rfid_check if a valid RFID card is detected
    // }
  }
}
