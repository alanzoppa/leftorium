
#include <Adafruit_NeoPixel.h>
#include "Keyboard.h"
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "leftorium_logo.h"
#include "keydefs.h"

#define PIN        10
#define NUMPIXELS 2
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(128, 32, &Wire, OLED_RESET);

int buttonState;
int modifierState;
int lastOutputState = 0;
int lastModifierState = 0;
int modifier = 0;
int mode = 0;

const int colorStates[4][3] = {
  {0, 0, 0},
  {150, 0, 0},
  {0, 150, 0},
  {0, 0, 150}
};

const char* stateNames[4] = {
  "okely",
  "DOKELY",
  "numberinos",
  "symbols"
};

const int NUM_STATES = 4;
const int NUM_PINS = 5;
const int inputs[] = {4,5,6,7,8};
const int modifier_pin = 9;
char lowercase[32];
char uppercase[32];

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 35;    // the debounce time; increase if the output flickers

void setup() {


  Serial.begin(9600);
  Keyboard.begin();
  pixels.begin();
  pixels.setBrightness(25);


  for (byte i = 0; i < NUM_PINS; i++) {
    pinMode(inputs[i], INPUT_PULLUP);
  }
  pinMode(modifier_pin, INPUT_PULLUP);

  assignKeyDefsLowercase(lowercase);
  assignKeyDefsUppercase(uppercase);

  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  display.clearDisplay();
  display.setTextSize(2); // Draw 2X-scale text
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(50, 0);

  display.clearDisplay();
  display.drawBitmap(0, 0, leftorium, 128, 32, 1);
  display.display(); 
  display.invertDisplay(true);
  delay(1000);
  display.invertDisplay(false);
  delay(1000);
  display.clearDisplay();

}

void loop() {
  byte read = 0;

  for (byte i=0; i<NUM_PINS; i++) {
    if (!digitalRead(inputs[i])) bitSet(read, i);
  }
  modifier = !digitalRead(modifier_pin);
  
  // If the switch changed, due to noise or pressing:
  if (read != lastOutputState || modifier != lastModifierState) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }


  if ((millis() - lastDebounceTime) > debounceDelay) {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:

    // if the button state has changed:

    if (modifier != modifierState) {
      modifierState = modifier;
      if (modifier != 0) {
        mode++;
        if (mode >= NUM_STATES) {mode = 0;}
        Serial.println(mode);
      }
    }

    if (read != buttonState) {
      buttonState = read;
      if (read != 0) {
        Serial.print(read, BIN);
        Serial.print(": ");
        Serial.print(read);    
        Serial.print(": ");

        switch (mode) {
          case 0:
            Serial.print(lowercase[read]);
            Keyboard.print(lowercase[read]); 
            break;
          case 1:
            Serial.print(uppercase[read]);
            Keyboard.print(uppercase[read]); 
            break;
          case 2:
            // statements
            break;
          case 3:
            // statements
            break;
        }


        Serial.print("\n");
      }
    }
  }

  for(int i=0; i<NUMPIXELS; i++) {
    pixels.setPixelColor(i, pixels.Color(
      colorStates[mode][0],
      colorStates[mode][1],
      colorStates[mode][2]
    ));
  }

  display.clearDisplay();
  display.setCursor(10, 16);
  display.println(stateNames[mode]);
  display.display();  
  pixels.show();

  // save the reading. Next time through the loop, it'll be the lastButtonState:
  lastOutputState = read;
  lastModifierState = modifier;
}