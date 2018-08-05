#include <Adafruit_NeoPixel.h>

#define PIN_ONBOARD_LED 13
#define PIN_NEOPIXEL 6
#define MV_PER_AMP 136

Adafruit_NeoPixel strip = Adafruit_NeoPixel(250, PIN_NEOPIXEL, NEO_GRB + NEO_KHZ800);

void setup() {
  pinMode(PIN_ONBOARD_LED , OUTPUT);
  digitalWrite(PIN_ONBOARD_LED , HIGH);

  strip.begin();
  strip.show();
  
  Serial.begin(9600);
}

void loop() {
  for(int i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, 255);
  }
  strip.show();

  for (int sensor = 0; sensor < 6; sensor++) {
    double amperage = readACS711(sensor);
    Serial.printf("A%d: %f\t", sensor, amperage);
  }
  Serial.println("");

  delay(500);
}

double readACS711(int pin) {
  int raw = analogRead(pin);
  double teensyVcc = 3.3;
  double sensorVcc = 5.15;
  double sensorZeroVcc = sensorVcc / 2.0;
  
  double pinVcc= raw / 1024.0 * teensyVcc;
  double amperage = (pinVcc - sensorZeroVcc) * 1000 / MV_PER_AMP;

  return amperage;
}

