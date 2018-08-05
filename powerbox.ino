#include <stdint.h>
#include <FastLED.h>

#define MV_PER_AMP 136
// Sensors start at A0.
#define NUM_SENSORS 6
#define BUFFER_SIZE 64

#define PIN_ONBOARD_LED 13
#define PIN_NEOPIXEL 6
// One LED per sensor.
CRGB _leds[NUM_SENSORS];

//////////////////////////////////////////////////////
// Arduino
//////////////////////////////////////////////////////

void setup() {
  pinMode(PIN_ONBOARD_LED , OUTPUT);
  digitalWrite(PIN_ONBOARD_LED , HIGH);

  FastLED.addLeds<WS2811, PIN_NEOPIXEL, RGB>(_leds, NUM_SENSORS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness(64);

  fillBuffer();
  
  Serial.begin(9600);
}

void loop() {
  EVERY_N_MILLISECONDS(50) {
    fillBuffer();
  }

  EVERY_N_MILLISECONDS(15) {
    updateLEDs();
  }
}

//////////////////////////////////////////////////////
// ACS711 current sensor
//////////////////////////////////////////////////////

int _bufferFrame = 0;
double _buffer[NUM_SENSORS][BUFFER_SIZE] = { 0 };
double _amperageRead[NUM_SENSORS] = { 0 };

void fillBuffer() {
  // Fill the buffer with a new reading.
  for (int sensor = 0; sensor < NUM_SENSORS; sensor++) {
    _buffer[sensor][_bufferFrame] = max(0.0, readACS711(sensor));
  }
  _bufferFrame += 1;
  if (_bufferFrame > BUFFER_SIZE - 1) { _bufferFrame = 0; }

  // Average the buffer and save it out.
  for (int sensor = 0; sensor < NUM_SENSORS; sensor++) {
    double total = 0;
    for (int frame = 0; frame < BUFFER_SIZE; frame++) {
       total += _buffer[sensor][frame];
    }
    _amperageRead[sensor] = total / BUFFER_SIZE;
//    Serial.printf("A%d: %f\t", sensor, _amperageRead[sensor]);
  }
//  Serial.println("");
}

double readACS711(int pin) {
  const double teensyVcc = 3.3;
  const double sensorVcc = 5.15;
  const double sensorZeroVcc = sensorVcc / 2.0;

  int raw = analogRead(pin);
  double pinVcc= raw / 1024.0 * teensyVcc;
  double amperage = (pinVcc - sensorZeroVcc) * 1000 / MV_PER_AMP;

  return amperage;
}

//////////////////////////////////////////////////////
// LEDs
//////////////////////////////////////////////////////

CRGBPalette16 _currentPallet[NUM_SENSORS];
unsigned long _nextLEDUpdateTime[NUM_SENSORS] = { 0 };

void updateLEDs() {
  const uint8_t minFrequency = 10;
  const uint8_t maxFrequency = 50;
  const uint8_t rangeFrequency = maxFrequency - minFrequency;
  const float maxAmps = 1.0; // 5.0 in reality.
  
  for (int sensor = 0; sensor < NUM_SENSORS; sensor++) {
    unsigned long currentTime = millis();
    if (currentTime < _nextLEDUpdateTime[sensor]) {
      // Not enough time has passed to update this sensor.
      continue;
    }

    // Get the current reading as a fraction between 0 amps and 5 amps.
    float amps = _amperageRead[sensor];
    float ampFraction = amps / maxAmps;
Serial.printf("A%d: %f\t", sensor, amps);
    // Anything below 10 mA we consider sensor noise.
    if (amps < 0.01) {
      _leds[sensor] = CRGB::Black;
      continue;
    }

    // Color range is Hue 359 (starting, low power) to Hue 300 (ending, highest power).
    // Translate into FastLED world where Hue max is 255, we get 255 to 213.
    uint8_t hue = 213 + (42 * (1 - ampFraction));
//    Serial.printf("A%d: %d\t", sensor, hue);
    _leds[sensor] = CHSV(hue, 255, random(100, 200));

    // Schedule the next update based on our frequency.
    float ampInterpolation =  minFrequency + (ampFraction * rangeFrequency);
    int flickerFrequency = 1000 / ampInterpolation;
    _nextLEDUpdateTime[sensor] = currentTime + flickerFrequency; 
  }
  Serial.println("");
  
  FastLED.show();
}

