//=======================================DECLARATION ETHERNET =================================

#include <SPI.h>
#include <HttpClient.h>
#include <Ethernet.h>
#include <EthernetClient.h>

// assign a MAC address for the ethernet controller.
// fill in your address here:
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};

const char myLocalIP[] = "192.168.1.158";
const char route[] = "/fire";
byte ip[] = { 192, 168, 1, 241};
byte server[] = { 192, 168, 1, 122 };
byte myDns[] = { 192, 168, 1, 1 };

String data;
EthernetClient client;
HttpClient http(client);

// ================================== SETUP ======================================

const int sensorPin = 8;
const int buzzer = 2;
const int G_led = 6;
const int R_led = 7;

int read_value;
void setup() {
  SPI.begin();      // Initiate  SPI bus
  
//=================================ETHERNET SETUP ===============================

//  Ethernet.init(10);  // Most Arduino shields
  Ethernet.begin(mac, ip) ;
  Serial.begin(9600);   // Initiate a serial communication

  // start the Ethernet connection:
  Serial.println("Initialize Ethernet with DHCP:");
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // Check for Ethernet hardware present
    if (Ethernet.hardwareStatus() == EthernetNoHardware) {
      Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
      while (true) {
        delay(1); // do nothing, no point running without Ethernet hardware
      }
    }
    if (Ethernet.linkStatus() == LinkOFF) {
      Serial.println("Ethernet cable is not connected.");
    }
    // try to congifure using IP address instead of DHCP:
    Ethernet.begin(mac, ip, myDns);
    Serial.print("My IP address: ");
    Serial.println(Ethernet.localIP());
  } else {
    Serial.print("  DHCP assigned IP ");
    Serial.println(Ethernet.localIP());
    Serial.println("connecting...");
  }
  // give the Ethernet shield a second to initialize:
  delay(1000); //Change it later
  
  Serial.println("WELCOME TO FIRE DETECTION AND ALARM SYSTEM");
  delay(1000);
  pinMode(sensorPin, INPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(G_led, OUTPUT);
  pinMode(R_led, OUTPUT);
}

void loop() {
  readFlame();
}

void readFlame()
{
  read_value = digitalRead(sensorPin);
  if(read_value==0)
  {
    digitalWrite(buzzer, HIGH);
    digitalWrite(R_led, HIGH);
    digitalWrite(G_led, LOW);
    Serial.println("\nALERT!!! Fire Detected");
    Serial.println("Sending a message to your discord channel....");
    apiCall();
  }
  else {
    digitalWrite(buzzer, LOW);
    digitalWrite(R_led, LOW);
    digitalWrite(G_led, HIGH);
    Serial.println("No Fire Detected");
  }
  delay(1000);
}

void apiCall() {
  int err =0;
   
  err = http.get(myLocalIP, route);
  if (err == 0)
  {
    Serial.println("startedRequest ok");

    err = http.responseStatusCode();
    if (err >= 0)
    {
      Serial.print("Got status code: ");
      Serial.println(err);

      // Usually you'd check that the response code is 200 or a
      // similar "success" code (200-299) before carrying on,
      // but we'll print out whatever response we get

      err = http.skipResponseHeaders();
      if (err >= 0)
      {
        int bodyLen = http.contentLength();
      
        // Now we've got to the body, so we can print it out
        unsigned long timeoutStart = millis();
        char c;
        // Whilst we haven't timed out & haven't reached the end of the body
        while ( (http.connected() || http.available()) &&
               ((millis() - timeoutStart) < 30*1000) )
        {
            if (http.available())
            {
                c = http.read();
                // Print out this character
                // Serial.print(c);
               
                bodyLen--;
                // We read something, reset the timeout counter
                timeoutStart = millis();
            }
            else
            {
                // We haven't got any data, so let's pause to allow some to
                // arrive
                delay(1000);
            }
        }
      }
      else
      {
        Serial.println("Failed to skip response headers: ");
        Serial.println(err);
      }
    }
    else
    {    
      Serial.println("Getting response failed: ");
      Serial.println(err);
    }
  }
  else
  {
    Serial.println("Connect failed: ");
    Serial.println(err);
  }
  http.stop();
  return;
}
