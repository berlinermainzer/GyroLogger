#include <FS.h>
#include <PrintEx.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ADXL345_U.h>

#define DATA_FILE "./data.txt"
#define TIME_OUT 10000L

//#define PWM_SOURCE_PIN 5 // Pin GPIO5, D1
//#define PWM_SOURCE_PIN 16 // Pin GPIO16, D0
#define PWM_SOURCE_PIN 12 // Pin GPIO12, D6
//#define PWM_SOURCE_PIN 0 // Pin GPIO0, D3
#define THRESH_LOW 1000
#define THRESH_MID 1500
#define THRESH_HIGH 2000
#define HIST 10
#define LED_PIN 0 // Pin GPIO0, D3//LED_BUILTIN

/* Assign a unique ID to this sensor at the same time */
Adafruit_ADXL345_Unified accel = Adafruit_ADXL345_Unified(42);



void blink(uint16_t count) {
  for (uint16_t i = 0; i < count; i++) {
    digitalWrite(LED_PIN, LOW);
    delay(70);
    digitalWrite(LED_PIN, HIGH);
    delay(250);
  }
}

void blinkFast(uint16_t count) {
  for (uint16_t i = 0; i < count; i++) {
    digitalWrite(LED_PIN, LOW);
    delay(30);
    digitalWrite(LED_PIN, HIGH);
    delay(100);
  }
}

void blinkError(uint16_t code) {
  while (true) {
    blink(code);
    delay(1500);
  }
}



void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(PWM_SOURCE_PIN, INPUT_PULLUP);
  
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);

  Serial.print("SPIFFS init... ");
  boolean initOk = false;
  for (uint16_t i = 1; i <= 3 && !initOk; i++){
    initOk = SPIFFS.begin();
    if(!initOk) {
      Serial.print("Failed #");
      Serial.println(i);
      SPIFFS.format();
    }
  }

  if (!initOk) {
    Serial.println("SPIFFS init failed. Giving up.");
    blinkError(2);
  } else {
    Serial.println("OK.");
  }


  unsigned long startTime = millis();
  unsigned long now = 0;
  
  Serial.println("Hit d to delete current data, l to list current data. Wait for 10 secs to continue with data collection.");
  do {
    if(Serial.available() > 0) {
        char x = Serial.read();

        if (x == 'd') {
          Serial.print("=== Deleting data... ");
          boolean deleteOk = SPIFFS.remove(DATA_FILE);
          if (!deleteOk) {
            Serial.println("Failed.");
            blinkError(2);
          }
          Serial.println("Ok.");
          
          Serial.print("Init file... ");
          File file = SPIFFS.open(DATA_FILE, "w");
          if (!file) {
            Serial.println("Failed.");
            blinkError(2);
          } else {
            file.println("t;x;y;z");
            file.close();
            Serial.println("Ok. Reset to continue.");
            blinkError(1);
          }
          while(true){delay(1000);};
        } else if (x == 'l') {
          Serial.println("=== Listing data:");
          File file = SPIFFS.open(DATA_FILE, "r");
          if (!file) {
            Serial.println("None available.");
            blinkError(2);
          } else {
            while(file.available()) {
              //Lets read line by line from the file
              String line = file.readStringUntil('\n');
              Serial.println(line);
            }
            file.close();
            Serial.println("=== Listing data done. Reset to continue");
            blinkError(1);
          }
          } else {
            Serial.println("Ignoring " + x);
        }
    }
    now = millis();
/*
    Serial.print("waitingForInput="); Serial.println(waitingForInput);
    Serial.print("startTime="); Serial.println(startTime);
    Serial.print("now="); Serial.println(now);
    Serial.print("now - startTime="); Serial.println(now - startTime);
  */
  } while((now - startTime) < TIME_OUT);
  Serial.println("Time out.");
  
}


uint16_t readCH3() {
  uint16_t pwmin = pulseIn(PWM_SOURCE_PIN, HIGH, 20000);
  //delay(30);
  //Serial.println(pwmin);
  return pwmin;
}

void loop() {
  File file = SPIFFS.open(DATA_FILE, "a");
  if (!file) {
    Serial.println("SPIFFS open failed.");
    blinkError(2);
  }
  
  if(!accel.begin()) {
    /* There was a problem detecting the ADXL345 ... check your connections */
    Serial.println("Ooops, no ADXL345 detected ... Check your wiring!");
    blinkError(3);
  }
  accel.setRange(ADXL345_RANGE_4_G);

  uint32_t flashSize = ESP.getFlashChipRealSize();
  uint32_t currentSize = file.size();
  Serial.print("Flash size: "); Serial.print(flashSize); Serial.print(", used: "); Serial.println(currentSize);

  StreamEx streamFile = file;
  streamFile.println();

  boolean separatorWritten = false;
  uint32_t bytesWritten = 1;
  uint16_t pwmin;
  uint16_t pwmhist = 0;

  Serial.println("Collecting data...");
  blinkFast(15);

  while(currentSize <= flashSize) {

    pwmin = readCH3();
    
    // Check if CH3 is on HOLD
    separatorWritten = false;

    while (abs(pwmin - THRESH_MID) < HIST) {
      if (!separatorWritten) {
        file.println("===");
        bytesWritten += 4;
        separatorWritten = true;
        Serial.println("Detected CH3 MID value, waiting...");
      }
      pwmin = readCH3();
    }

    sensors_event_t event; 
    accel.getEvent(&event);

    //Serial.print(event.acceleration.x); Serial.print(" ");Serial.print(event.acceleration.y);Serial.print(" ");Serial.println(event.acceleration.z);
    //uint32_t bytesWritten = streamFile.printf("%d;%.2f;%.2f\n", millis(), 42.5f, 17.9);
    bytesWritten = streamFile.printf("%d;%.3f;%.3f;%.3f\n", millis(), event.acceleration.x, event.acceleration.y, event.acceleration.z);
    currentSize += bytesWritten;
    
    delay(50);
  }


  Serial.println("Flash is full. Stop collecting.");
  file.close();
  
  blinkError(5);
}
