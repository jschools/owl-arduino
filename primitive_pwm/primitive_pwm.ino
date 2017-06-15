
#include <stdint.h>

#define PIN_IN A0
#define PIN_OUT_R 11

#define DEBUG

void setup(){
  // Set the REF pin to output 1.1V
  analogReference(INTERNAL);

  // Set input mode
  pinMode(PIN_IN, INPUT);

  // Set output mode
  pinMode(PIN_OUT_R, OUTPUT);

  #ifdef DEBUG
    // initialize debug logs
    Serial.begin(115200);
  #endif
}

void loop() {
  // sum a number of samples
  uint16_t sum = 0;
  for (int i = 0; i < 100; i++) {
    int inValue = analogRead(PIN_IN);
    int zeroCentered = inValue - 512;

    sum += convertAbs(zeroCentered);
  }

  // convert the sum to a float
  float fSum = (float) sum;

  // map the sum from an emperically-derived domain onto [0..1]
  float mapped = mapFloat(fSum, 360.0f, 3000.0f, 0.0f, 1.0f);

  // map the result onto m^(3/2)
  mapped = sqrt(mapped);
  mapped = mapped * mapped * mapped;

  // map onto [0..255] and convert to integer
  int iMapped = (int) (mapped * 255);

  // constrain to output range
  iMapped = constrain(iMapped, 0, 255);

  // set the output PWM
  analogWrite(PIN_OUT_R, iMapped);

  #ifdef DEBUG
    // print the output value
    Serial.println(iMapped);
  #endif
}

// converts into an unsigned 16-bit int equal to the magnitude of the input
uint16_t convertAbs(int val) {
  if (val < 0) {
    return (uint16_t) -val;
  }
  return (uint16_t) val;
}

// maps a float value from one range onto another
float mapFloat(float val, float inMin, float inMax, float outMin, float outMax) {
  return (val - inMin) * (outMax - outMin) / (inMax - inMin) + outMin;
}

