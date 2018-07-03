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

  Serial.print("A1: :");
  Serial.print(analogRead(A1));
  Serial.print("\t");

  int raw = analogRead(A0);
  double teensyVcc = 3.3;
  double sensorVcc = 5.15;
  double sensorZeroVcc = sensorVcc / 2.0;
  
  //double voltage = raw * internalVcc * 1000 / 1024;
  double pinVcc= raw / 1024.0 * teensyVcc;
  double amperage = (pinVcc - sensorZeroVcc) * 1000 / MV_PER_AMP;

  Serial.print("Raw: ");
  Serial.print(raw);
  Serial.print("\tVpin: ");
  Serial.print(pinVcc , 3);
  Serial.print("\tAh: ");
  Serial.print(amperage, 3);
  Serial.println("");
  delay(1000);
}

