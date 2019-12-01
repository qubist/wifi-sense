
#include "ESP8266WiFi.h"
#include <Servo.h>

// code modified from: https://gist.github.com/tablatronix/497f3b299e0f212fc171ac5662fa7b42https://gist.github.com/tablatronix/497f3b299e0f212fc171ac5662fa7b42

const int ledPin = 0;

#define SCAN_PERIOD 7000
long lastScanMillis;

#define PRINT_PERIOD 1000
long lastPrintMillis;

// max speed serve should turn one way or the other
// do not set this over 90

#define SERVO_MAX 50

Servo servo1; // create servo object to control a servo
Servo servo2;
Servo servo3;
Servo servo4;

// aliasing high and low because they're switched for the LED on the Feather HUZZAH
#define ON LOW
#define OFF HIGH

void flicker(int pin, int rate) {
  if (millis() % rate == 0) {
    digitalWrite(pin, ON); // turn LED on
    delay(5); // tiny delay so the light can get bright, not enough to mess up timing of the rest of the code.
  } else {
    digitalWrite(pin, OFF); // turn LED off
  }
}

void setup() {
  Serial.begin(115200);
  // Serial.setDebugOutput(true);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, OFF); 

  // Set WiFi to station mode and disconnect from an AP if it was previously connected
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  delay(100);

  Serial.println("");
  Serial.println("Setup done");
  Serial.println("");
  Serial.println("MAC: " + WiFi.macAddress());

  // attatch servo object to pin 14
  // Position "90" (1.5ms pulse) is stop, "180" (2ms pulse) is full speed forward, "0" (1ms pulse) is full speed backwards.
  servo1.attach(14);
  servo2.attach(12);
  servo3.attach(13);
  servo4.attach(15);

  servo1.write(90); // stop servo
  servo2.write(90);
  servo3.write(90);
  servo4.write(90);
}





// declare the variables for unique networks and network strength.
int n; // number of networks
int u; // number of unique networks
int32_t s; // combined strengths of networks
int32_t last_s; // keep the last strength, too

bool flickering = false;

void loop() {

  if (flickering) {
    flicker(ledPin, 100);
  }
  
  // asynchronously scan and save results
  // https://arduino-esp8266.readthedocs.io/en/latest/esp8266wifi/scan-examples.html
  long currentMillis = millis();
  if (currentMillis - lastScanMillis > SCAN_PERIOD)
  {
    WiFi.scanNetworks(true);
    Serial.print("\nScan start ... ");
    lastScanMillis = currentMillis;
    /////// digitalWrite(ledPin, LOW); // LOW and HIGH reversed for some reason
    flickering = true;
  }

  // When async scan is complete, WiFi.scanNetworks will return the
  // number of networks found. Then we'll do calculations and
  // update motor speed
  n = WiFi.scanComplete();
  if (n >= 0) {
    ///// digitalWrite(ledPin, HIGH);
    flickering = false;
    Serial.println("scan done");
    
  
    u = n;
    int loops = 0;
    
    if (n == 0)
      Serial.println("no networks found");
    else
    {
      // sort by RSSI
      int indices[n];
      int skip[n];
      
      String ssid;
      
      for (int i = 0; i < n; i++) {
        indices[i] = i;
      }
  
      for (int i = 0; i < n; i++) {
        if(indices[i] == -1){
          --u;
          continue;
        }
        ssid = WiFi.SSID(indices[i]);
        for (int j = i + 1; j < n; j++) {
          loops++;
          if (ssid == WiFi.SSID(indices[j])) {
            indices[j] = -1;
          }
        }
      }
  
  //    Serial.println((String)loops);
      Serial.print(u); // u is number of unique networks found!
      Serial.println(" unique networks found of " + (String)n + " total networks.");
      
      Serial.println("00: (RSSI) SSID");
      for (int i = 0; i < n; ++i)
      {
        if(indices[i] != -1){
        // Print SSID and RSSI for each network found
        Serial.printf("%02d", i + 1);
        Serial.print(":");
  
        Serial.print(" (");
        Serial.print(WiFi.RSSI(indices[i]));
        Serial.print(")");
  
        Serial.print(" " + WiFi.SSID(indices[i]));
        // Serial.print((WiFi.encryptionType(indices[i]) == ENC_TYPE_NONE)?" ":"*");
  
  //      Serial.print(" WiFi index: " + (String)indices[i]);
  
        Serial.println();
        }
      }

      last_s = s; // save current strength into last before updating it
      // sum strengths
      s = 0;
      for (int i = 0; i < n; ++i) {
        if(indices[i] != -1){
          s += WiFi.RSSI(indices[i]);
        }
      }


      Serial.print("Unique networks: ");
      Serial.println(u);
      
      Serial.print("Total networks: ");
      Serial.println(n);

      Serial.print("Last Sum of Strengths: ");
      Serial.println(last_s);
      
      Serial.print("Sum of Strengths: ");
      Serial.println(s);
      
      // Serial.println();
    }
  // delete the scan we just printed, but we can still access n, u, and s.
  WiFi.scanDelete();

  // SET THE MOTOR SPEED! Once every scan time

  // difference between last strength and current strength
  int32_t d = s - last_s; // if strength got bigger since last time, d will be positive
  // int32_t is signed 
  
  Serial.print("Strength delta: ");
  Serial.println(d);

  // Set servo speed based on the strength delta
  // Recall: Position "90" (1.5ms pulse) is stop, "180" (2ms pulse) is full speed forward, "0" (1ms pulse) is full speed backwards.

  int servoSpeed;
  if (d < 0) {
    // d is negative          // slowest it can go down       fastest it can go down
    servoSpeed = map(d, 0, -400, 89,                          78);
  } else {
    // d is positive          // fastest it can go up         slowest it can go up
    servoSpeed = map(d, 400,  0, 100,                         95);
  }

  if ((servoSpeed >= 78) && (servoSpeed <= 100)){ // don't go too fast
    servo1.write(servoSpeed); // start servo moving at appropriate speed
    servo2.write(servoSpeed);
    servo3.write(servoSpeed); 
    servo4.write(servoSpeed);
    Serial.print("Servo speed: ");
    Serial.println(servoSpeed);
  }
  else {
    servo1.write(90); // stop servo
    servo2.write(90);
    servo3.write(90); 
    servo4.write(90);
    Serial.print("Servo STOPPED from speed: ");
    Serial.print(servoSpeed);
  }


  // calibration:
// servo1.write(90);
//  servo2.write(90);
//  servo3.write(90);
//  servo4.write(90);

  }

  

//  if (currentMillis - lastPrintMillis > PRINT_PERIOD) {
//    lastPrintMillis = currentMillis;
//  // we have n, u, and s from the previous scan
//  //   for n, -2 means scan not started, -1 means scan in progress, positive int means that many networks found.
//  //   We'll never see positive int here because the moment it's positive, we print things and then delete the scan.
//  Serial.println(n);
//  Serial.println(u);
//  Serial.println(last_s);
//  Serial.println(s);
//  }


}
