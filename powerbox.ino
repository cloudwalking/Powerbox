#include <stdint.h>
#include <Adafruit_NeoPixel.h>

#define PIN_ONBOARD_LED 13
#define PIN_NEOPIXEL 6
#define MV_PER_AMP 136

Adafruit_NeoPixel _strip = Adafruit_NeoPixel(250, PIN_NEOPIXEL, NEO_GRB + NEO_KHZ800);

// Sensors start at A0.
#define NUM_SENSORS 6
#define BUFFER_SIZE 16

int _bufferFrame = 0;
double _buffer[NUM_SENSORS][BUFFER_SIZE] = { 0 };

void setup() {
  pinMode(PIN_ONBOARD_LED , OUTPUT);
  digitalWrite(PIN_ONBOARD_LED , HIGH);

  _strip.begin();
  _strip.show();
  
  Serial.begin(9600);
}

void loop() {
  for(int i = 0; i < _strip.numPixels(); i++) {
    _strip.setPixelColor(i, 255);
  }
  _strip.show();

  // Fill the current frame of the buffer.
  for (int sensor = 0; sensor < NUM_SENSORS; sensor++) {
    _buffer[sensor][_bufferFrame] = readACS711(sensor);
  }
  _bufferFrame += 1;
  if (_bufferFrame > BUFFER_SIZE - 1) { _bufferFrame = 0; }

  // Average the buffer and read it out.
  for (int sensor = 0; sensor < NUM_SENSORS; sensor++) {
    double average = 0;
    for (int frame = 0; frame < BUFFER_SIZE; frame++) {
       average += _buffer[sensor][frame];
    }
    average = average / BUFFER_SIZE;
    Serial.printf("A%d: %f\t", sensor, average);
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

