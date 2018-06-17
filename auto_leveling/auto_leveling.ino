
#include <stdint.h>

#define DEBUG true
#define LOG(message) if (DEBUG) { Serial.print(message); }
#define LOGLN(message) if (DEBUG) { Serial.println(message); }

#define PIN_IN A6
#define PIN_OUT_R 9
#define PIN_OUT_B 11

#define SAMPLE_COUNT 100
#define DEFAULT_INPUT_SCALE 4.0f
#define WINDOW_SIZE_MS 1000.0f
#define WINDOW_AVERAGE_TARGET 0.7f

// global variables
float inputScale;
unsigned long lastInputTimeMs;
float windowAverage;

void setup(){
  // Set the REF pin to output 1.1V
  analogReference(INTERNAL);

  // Set input mode
  pinMode(PIN_IN, INPUT);

  // Set output mode
  pinMode(PIN_OUT_R, OUTPUT);
  pinMode(PIN_OUT_B, OUTPUT);

  // init global variables
  inputScale = DEFAULT_INPUT_SCALE;
  lastInputTimeMs = millis();
  windowAverage = WINDOW_AVERAGE_TARGET;

  if (DEBUG) {
    // initialize debug logs
    Serial.begin(115200);
  }
}

void loop() {
  float normalized = readNormalized();
  LOG("in: ")
  LOGLN(normalized)

  // scale
  normalized *= inputScale;
  LOG("scale: ")
  LOG(inputScale)
  LOG(" --> ")
  LOGLN(normalized)

  // adjust scale
  adjustScaleForLastInput(normalized);

  // correct the gamma
  float output = convertGamma(normalized);
  LOG("out: ")
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

void adjustScaleForLastInput(float normalized) {
  unsigned long nowMs = millis();
  float diffMs = (float) (nowMs - lastInputTimeMs);
  float windowProportion = diffMs / WINDOW_SIZE_MS;

  LOG("diffMs: ")
  LOG(diffMs)
  LOG(" --> p: ")
  LOGLN(windowProportion)

  windowAverage = windowProportion * normalized + (1.0f - windowProportion) * windowAverage;
  inputScale = WINDOW_AVERAGE_TARGET / windowAverage;
  
  lastInputTimeMs = nowMs;

  LOG("A: ")
  LOGLN(windowAverage)
}

// converts into an unsigned 16-bit int equal to the magnitude of the input
uint16_t convertAbs(int val) {
  if (val < 0) {
    return (uint16_t) -val;
  }
  return (uint16_t) val;
}

void writeOutputPwm(float duty) {
  // map onto and constrain to [0..255]
  int output = (int) (duty * 255);
  output = constrain(output, 0, 255);
  LOG("outPwm: ")
  LOGLN(output)
  
  // set the output PWM
  analogWrite(PIN_OUT_R, output);
  analogWrite(PIN_OUT_B, output);
}

