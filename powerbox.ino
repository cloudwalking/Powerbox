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
  delay(2000);
  pinMode(PIN_ONBOARD_LED , OUTPUT);
  digitalWrite(PIN_ONBOARD_LED , HIGH);

  FastLED.addLeds<WS2811, PIN_NEOPIXEL, RGB>(_leds, NUM_SENSORS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness(64);

  fillBuffer();
  
  Serial.begin(9600);
}

void loop() {
  EVERY_N_MILLISECONDS(15) {
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
double _sensorAmps[NUM_SENSORS] = { 0 };

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
    _sensorAmps[sensor] = total / BUFFER_SIZE;
    Serial.printf("+ A%d: %f\t", sensor, _sensorAmps[sensor]);
  }
  Serial.println("");
}

double readACS711(int pin) {
  // USB handicaps
//  const int handicap[NUM_SENSORS] = { 0, 1, 3, -2, 0, -11 };
  // 12V handicaps
  const int handicap[NUM_SENSORS] = { -8, -9, -6, -10, -9, -22 };
  
  const double teensyVcc = 3.34;
  const double sensorVcc = 5.08; //5.08 12v //5.15 usb
  const double sensorZeroVcc = sensorVcc / 2.0;

  int raw = analogRead(pin);
  int withHandicap = raw + handicap[pin];
  double pinVcc= (withHandicap / 1024.0) * teensyVcc;
  double amperage = (pinVcc - sensorZeroVcc) * 1000 / MV_PER_AMP;
//  Serial.printf("\t raw: %d\t with_handi: %d\t pin: %f\t amperage: %f\n", raw, withHandicap, pinVcc, amperage);

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
    float amps = _sensorAmps[sensor];
    float ampFraction = amps / maxAmps;
//Serial.printf("* A%d: %f\t", sensor, ampFraction);

    // Anything below 125 mA we consider sensor noise.
    if (amps < 0.125) {
      _leds[sensor] = CRGB::Black;
      continue;
    }

    // Pick a hue based on amperage.
    uint8_t hue = 0;
    uint8_t vibrance = 0;
    if (amps < 0.5) {
      // Green
      hue = 77;
      vibrance = 150;
    } else if (amps < 1.0) {
      // Teal
      hue = 122;
      vibrance = 175;
    } else if (amps < 2.0) {
      // Blue
      hue = 155;
      vibrance = 200;
    } else {
      // Purple
      hue = 224;
      vibrance = 230;
    }

    // For a nice sparkle effect -- pick colors from the bottom color up
    // to the color for current amperage.
    hue = random(max(77, hue - 10), hue + 10);
    vibrance = random(vibrance - 25, vibrance + 25);
    _leds[sensor] = CHSV(hue, 255, vibrance);

    // Schedule the next update based on our frequency.
    float ampInterpolation =  minFrequency + (ampFraction * rangeFrequency);
    int flickerFrequency = 1000 / ampInterpolation;
    _nextLEDUpdateTime[sensor] = currentTime + flickerFrequency; 
  }
//  Serial.println("");
  
  FastLED.show();
}

