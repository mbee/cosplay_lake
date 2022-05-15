#include <FastLED.h>


//---------------------------------------------------
// Here are the parameters to customize
//---------------------------------------------------

// define the PIN where the LED will be piloted
#define LED_PIN              4
// define the switch to start the sequence
#define SWITCH_PIN           6

#define NUM_LEDS             124 
#define LED_TYPE             WS2812
#define COLOR_ORDER          GRB
// Here are defined the brighest BLUE and GREEN. Before they were set to CRGB(6,  181, 213) and CRGB(45, 198, 28)
#define BLUE                 CRGB(0,   0, 255)
#define GREEN                CRGB(255, 0, 0)
// Duration in ms for the blue sequence
#define SEQUENCE_BLUE_MS     18000
// Duration in ms for the green sequence
#define SEQUENCE_GREEN_MS    6000
// Duration in ms for the fade out sequence
#define SEQUENCE_FADE_OUT_MS 5000
// Warning: (SEQUENCE_BLUE_MS + SEQUENCE_GREEN_MS) module PERIOD_MS must be equal to 0, to have the best fade_out
#define PERIOD_MS            3000

//---------------------------------------------------
// Nothing to update after this line
//---------------------------------------------------

CRGB leds[NUM_LEDS];

static int switchValue;     // get the initial swith value (as it's not a push-button, but a switch, we don't know it)
static int currentStep = 0; // 0 => waiting the switch, 1 => glowing. To reset the sequence => arduino RESET button
static int lastGreenLumin = -1;


void showAll(CRGB myColor) {
  Serial.print("Showing them all!\n");
  for(int i = 0; i < NUM_LEDS; i++ ){
    leds[i] = myColor;
  }  
  FastLED.show();
}

void setup() {
  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
  // initialize serial:
  Serial.begin(9600);
  FastLED.clear();
  pinMode(SWITCH_PIN, INPUT_PULLUP);
  showAll(BLUE);
  switchValue = digitalRead(SWITCH_PIN);
}

void resetCycle() {
   Serial.print("Resetting cycle\n");
   currentStep = 0;
   switchValue = digitalRead(SWITCH_PIN);
   lastGreenLumin = -1;
}

int getCurrentLumin(unsigned long elapsed_ms) {
  if (currentStep == 0) {
    return 255;
  }
  float lumin = (cos(( 2 * 3.1415926 * elapsed_ms) / PERIOD_MS) + 1) / 2 * 255;
  if (elapsed_ms < SEQUENCE_BLUE_MS + SEQUENCE_GREEN_MS) {
    return int(lumin);
  }
  if (lastGreenLumin == -1) {
    lastGreenLumin = int(lumin);
    Serial.print("Setting last lumin to ");
    Serial.print(lastGreenLumin);
    Serial.print("\n");
  }
  lumin = (cos(((elapsed_ms - SEQUENCE_BLUE_MS - SEQUENCE_GREEN_MS) * 3.141592) / SEQUENCE_FADE_OUT_MS) + 1) / 2 * lastGreenLumin;
  return int(lumin);
}

CRGB getCurrentColor(unsigned long elapsed_ms) {
  if (elapsed_ms < SEQUENCE_BLUE_MS) {
    return BLUE;
  }
  return GREEN;
}

void loop() {
  static long startTime = 0;
  if (currentStep == 0) {
     int newValue = digitalRead(SWITCH_PIN);
     if (newValue == switchValue) {
        return;
     }
     Serial.print("Switch detected\n");
     switchValue = newValue;
     startTime = millis();
     currentStep = 1;
  }
  unsigned long elapsed_ms = millis() - startTime;
  if (elapsed_ms > SEQUENCE_BLUE_MS + SEQUENCE_GREEN_MS + SEQUENCE_FADE_OUT_MS) {
    resetCycle();
    return;
  }
  int fade = 255 - getCurrentLumin(elapsed_ms);
  CRGB myColor = getCurrentColor(elapsed_ms);
  for(int i = 0; i < NUM_LEDS; i++ ){
    leds[i] = myColor;
    leds[i].fadeLightBy(fade);
  }
  FastLED.show();
  delay(10);
}
