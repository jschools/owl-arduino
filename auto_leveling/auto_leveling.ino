
#include <stdint.h>

#define DEBUG true
#define LOG(message) if (DEBUG) { Serial.print(message); }
#define LOGLN(message) if (DEBUG) { Serial.println(message); }

#define PIN_IN A6
#define PIN_OUT_R 9
#define PIN_OUT_B 11

#define SAMPLE_COUNT 100

#define OUTPUT_SCALE 4.0f

void setup(){
  // Set the REF pin to output 1.1V
  analogReference(INTERNAL);

  // Set input mode
  pinMode(PIN_IN, INPUT);

  // Set output mode
  pinMode(PIN_OUT_R, OUTPUT);
  pinMode(PIN_OUT_B, OUTPUT);

  if (DEBUG) {
    // initialize debug logs
    Serial.begin(115200);
  }
}

void loop() {
  float normalized = readNormalized();
  LOG("norm: ")
  LOGLN(normalized)

  // scale
  normalized *= OUTPUT_SCALE;

  // correct the gamma
  float output = convertGamma(normalized);
  LOG("transf: ")
  LOGLN(output)

  // set the PWM output
  writeOutputPwm(output);
}

float readNormalized() {
  // sum a number of samples
  uint16_t sum = 0;
  for (int i = 0; i < SAMPLE_COUNT; i++) {
    int inValue = analogRead(PIN_IN);
    
    int zeroCentered = inValue - 512;

    sum += convertAbs(zeroCentered);
  }

  // convert the sum to a float
  float sumF = (float) sum;

  // map the sum onto [0..1]
  float mapped = sumF / (512.0f * SAMPLE_COUNT);

  return constrain(mapped, 0.0f, 1.0f);
}

float convertGamma(float x) {
  return x * x * x;
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

void writeOutputPwm(float duty) {
  // map onto and constrain to [0..255]
  int output = (int) (duty * 255);
  output = constrain(output, 0, 255);
  LOG("out: ")
  LOGLN(output)
  
  // set the output PWM
  analogWrite(PIN_OUT_R, output);
  analogWrite(PIN_OUT_B, output);
}

