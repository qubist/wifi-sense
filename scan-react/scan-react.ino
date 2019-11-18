
#include "ESP8266WiFi.h"

// code modified from: https://gist.github.com/tablatronix/497f3b299e0f212fc171ac5662fa7b42https://gist.github.com/tablatronix/497f3b299e0f212fc171ac5662fa7b42

const int ledPin = 0;

#define SCAN_PERIOD 7000
long lastScanMillis;

#define PRINT_PERIOD 1000
long lastPrintMillis;

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
//   Serial.setDebugOutput(true);
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
}

// declare the variables for unique networs and network strength.
int n;
int u;
int32_t s;

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
  // WiFi.scanNetworks will return the number of networks found
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
      
      Serial.print("Sum of Strengths: ");
      Serial.println(s);
      
      Serial.println();
    }
  // delete the scan we just printed, but we can still access n, u, and s.
  WiFi.scanDelete();
  }

  

  if (currentMillis - lastPrintMillis > PRINT_PERIOD)
  {
  lastPrintMillis = currentMillis;
  // we have n, u, and s from the previous scan
  //   for n, -2 means scan not started, -1 means scan in progress, positive int means that many networks found.
  //   We'll never see positive int here because the moment it's positive, we print things and then delete the scan.
  Serial.println(n);
  Serial.println(u);
  Serial.println(s);
  }


}
