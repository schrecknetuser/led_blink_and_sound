#include <Arduino.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <Adafruit_NeoPixel.h>
#include <SoftwareSerial.h>
#include <DFPlayerMini_Fast.h>

#include "HttpControl.h"
#include "secrets.h"

#define NEOPIXEL_LED_COUNT 24

#define NEOPIXEL_LED_PIN 26
#define NEOPIXEL_BRIGHTNESS 255

#define THRESHOLD 10*1000
#define MAIN_LOOP_DELAY 100  // Base delay in main loop to prevent excessive CPU usage

static const uint8_t PIN_MP3_TX = 25;
static const uint8_t PIN_MP3_RX = 27;
SoftwareSerial softwareSerial(PIN_MP3_RX, PIN_MP3_TX);

Adafruit_NeoPixel rgbWS = Adafruit_NeoPixel(NEOPIXEL_LED_COUNT, NEOPIXEL_LED_PIN, NEO_GRB + NEO_KHZ800);


#define INTERVAL_MILLIS 1000

WiFiClientSecure client;
unsigned long lastMillis;
HttpControl* httpControl;

int currentSectorPercentEnd = 100;
int currentSectorCycles = 0;

DFPlayerMini_Fast myPlayer;


void setup() {
  Serial.begin(115200);
  while(!Serial);
  Serial.println("Setup started");

  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, WIFIPASSWORD);

  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }

  Serial.println("WIFI Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  rgbWS.begin();
  rgbWS.setBrightness(NEOPIXEL_BRIGHTNESS);

  lastMillis = millis();

  httpControl = new HttpControl();
  //randomSeed(analogRead(0));

  

  softwareSerial.begin(9600);
  delay(3000);
  if (myPlayer.begin(softwareSerial, false)) {
    Serial.println("Connection successful.");
    delay(1000);
    // set initial volume 0-30
    myPlayer.volume(30);
  } else {
    Serial.println("Connection failed.");
  }
  

  Serial.println("Setup finished");
}

bool isActive()
{
  auto arguments = httpControl->getLedProfileFullParameters();
  
  // Add debug logging occasionally to help with troubleshooting
  static unsigned long lastDebugLog = 0;
  if (millis() - lastDebugLog > 30000) { // Log every 30 seconds
    Serial.println("Status check - useSecondary: " + String(arguments.useSecondary ? "true" : "false"));
    lastDebugLog = millis();
  }
  
  return arguments.useSecondary;
  //return true;
}


void loop() {

  // Add base delay to prevent excessive CPU usage and allow other tasks
  delay(MAIN_LOOP_DELAY);
  yield();

  if(!isActive())
  {
    rgbWS.fill(rgbWS.Color(0, 0, 0), 0, NEOPIXEL_LED_COUNT);
    rgbWS.show();
    return;
  }

  int volMin = 15;
  int volMax = 31;
  int randomVol = random(volMin, volMax);  

  // upper value should be one more than total tracks
  int randomTrack = 1;//random(1, 2);

  /*for(auto i = 0; i < 10; i++)
  {
    myPlayer.volume(30);
    myPlayer.play(randomTrack);

    delay(2000);
  }*/

  // lightning variables
  // use rgbw neopixel adjust the following values to tweak lightning base color
  int r = random(0, 0);
  int g = random(120, 255);
  int b = random(0, 0);

  uint32_t color = rgbWS.Color(r, g, b, 50);

  // number of flashes
  //int flashCount = random (5, 15);
  int flashCount = 50;
  // flash white brightness range - 0-255
  int flashBrightnessMin =  10;
  int flashBrightnessMax =  255;
  // flash duration range - ms
  int flashDurationMin = 3;
  int flashDurationMax = 9;
  // flash off range - ms
  int flashOffsetMin = 1;
  int flashOffsetMax = 75;
  // time to next flash range - ms
  int nextFlashDelayMin = 1;
  int nextFlashDelayMax = 50;
  // map white value to volume - louder is brighter
  int flashBrightness = map(randomVol, volMin, volMax, flashBrightnessMin, flashBrightnessMax);

  // map flash to thunder delay - invert mapping
  int thunderDelay = map(randomVol,  volMin, volMax, 1000, 250);

  // randomize pause between strikes
  // longests track length - ms
  int longestTrack = 2000;
  // intensity - closer to longestTrack is more intense
  int stormIntensity = 30000;
  long strikeDelay = random(longestTrack, stormIntensity);

  // debug serial print
  /*Serial.println("FLASH");
  Serial.print("Track: ");
  Serial.println(randomTrack);
  Serial.print("Volume: ");
  Serial.println(randomVol);
  Serial.print("Brightness: ");
  Serial.println(flashBrightness);
  Serial.print("Thunder delay: ");
  Serial.println(thunderDelay);
  Serial.print("Strike delay: ");
  Serial.println(strikeDelay);
  Serial.print("-");*/

  for(auto i = 0; i < 4; i++)
  {
    for (int flash = 0 ; flash <= flashCount/4; flash += 1) {
    // add variety to color
      int colorV = random(0, 50);
      if (colorV < 0) colorV = 0;
      // flash segments of neopixel strip
      //color = rgbWS.Color(r + colorV, g + colorV, b + colorV, flashBrightness);
      color = rgbWS.Color(0, g + colorV, 0, flashBrightness);
      rgbWS.fill(color, 0, NEOPIXEL_LED_COUNT/4);
      rgbWS.show();
      yield();
      delay(random(flashOffsetMin, flashOffsetMax));
      
      rgbWS.fill(color, NEOPIXEL_LED_COUNT/4*2, NEOPIXEL_LED_COUNT/4);
      rgbWS.show();
      yield();
      delay(random(flashOffsetMin, flashOffsetMax));
      
      rgbWS.fill(color, NEOPIXEL_LED_COUNT/4, NEOPIXEL_LED_COUNT/4);
      rgbWS.show();
      yield();
      delay(random(flashOffsetMin, flashOffsetMax));
      
      rgbWS.fill(color, NEOPIXEL_LED_COUNT/4*2, NEOPIXEL_LED_COUNT/4*2);
      rgbWS.show();
      yield();
      delay (random(flashDurationMin, flashDurationMax));
      
      rgbWS.clear();
      rgbWS.show();
      yield();
      delay (random(nextFlashDelayMin, nextFlashDelayMax));
      yield();
      
      // Check if we should exit early due to LED state change
      if(!isActive())
      {
        rgbWS.fill(rgbWS.Color(0, 0, 0), 0, NEOPIXEL_LED_COUNT);
        rgbWS.show();
        return;
      }
    }

    if(!isActive())
    {
      rgbWS.fill(rgbWS.Color(0, 0, 0), 0, NEOPIXEL_LED_COUNT);
      rgbWS.show();
      return;
    }
    yield();
  }
    
  yield();

  //myPlayer.volume(randomVol);
  //myPlayer.volume(30);
  ulong startMillis = millis();
  myPlayer.play(randomTrack);
  
  // Improved audio player handling with better timeout and error checking
  while(myPlayer.isPlaying())
  {
    delay(10);  // Increased delay to reduce CPU usage
    yield();    // Allow other tasks to run
    
    if(millis() - startMillis > THRESHOLD)
    {
      Serial.println("Audio playback timeout, stopping player");
      myPlayer.stop();
      break;
    }
    
    // Check if we should exit early due to LED state change
    if(!isActive())
    {
      myPlayer.stop();
      break;
    }
  }

  //delay(longestTrack);

  //for(int i = 0; i < NEOPIXEL_LED_COUNT; i++) {
  //  rgbWS.setPixelColor(i, arguments.red, arguments.green, arguments.blue);
  //}

}
