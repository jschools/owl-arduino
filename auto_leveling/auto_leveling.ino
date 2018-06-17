
#include <stdint.h>

#define DEBUG false
#define LOG(message) if (DEBUG) { Serial.print(message); }
#define LOGLN(message) if (DEBUG) { Serial.println(message); }

#define PIN_IN A6
#define PIN_OUT_R 9
#define PIN_OUT_B 11

#define THRESHOLD_CUTOFF_VALUE 0.006
#define SAMPLE_WINDOW_MS 15
#define DEFAULT_INPUT_SCALE 30.0
#define AUTO_LEVEL_WINDOW_SIZE_MS 1000.0
#define AUTO_LEVEL_WINDOW_MAX_TARGET 0.95

// global variables
uint32_t lastSampleTimeMs;
double inputAccumulator;
double inputScale;
uint32_t lastScaleTimeMs;
double autoLevelWindowMax;

void setup(){
  // Set the REF pin to output 1.1V
  analogReference(INTERNAL);

  // Set input mode
  pinMode(PIN_IN, INPUT);

  // Set output mode
  pinMode(PIN_OUT_R, OUTPUT);
  pinMode(PIN_OUT_B, OUTPUT);

  // init global variables
  inputAccumulator = 0;
  inputScale = DEFAULT_INPUT_SCALE;
  autoLevelWindowMax = 0;
  lastSampleTimeMs = millis();
  lastScaleTimeMs = millis();

  if (DEBUG) {
    // initialize debug logs
    Serial.begin(115200);
  }
}

void loop() {
  double normalized = readNormalized();
  LOG("in: ")
  char str[32];
  dtostrf(normalized, 7, 5, str);
  LOG(str)

  // noise floor threshold
  normalized = thresholdFilter(normalized);

  // adjust scale
  adjustScaleForInput(normalized);

  // scale
  normalized *= inputScale;
  LOG("; scale: ")
  LOG(inputScale)
  LOG(" --> ")
  LOG(normalized)

  // correct the gamma
  double output = convertGamma(normalized);
  LOG("; out: ")
  LOG(output)

  // set the PWM output
  writeOutputPwm(output);
}

double readNormalized() {
  uint32_t nowMs = millis();
  uint32_t timeSinceLastSampleMs = nowMs - lastSampleTimeMs;
  
  // read the raw input and convert to magnitude from center
  int inValue = analogRead(PIN_IN);
  int zeroCentered = inValue - 512;
  double absValue = (double) convertAbs(zeroCentered);

  // normalize the magnitude (map onto [0..1])
  double normalized = absValue / 512.0;
  normalized = constrain(normalized, 0.0, 1.0);

  // update running average
  double sampleProportion = (double) timeSinceLastSampleMs / SAMPLE_WINDOW_MS;
  inputAccumulator = sampleProportion * normalized + (1.0 - sampleProportion) * inputAccumulator;

  lastSampleTimeMs = nowMs;

  return inputAccumulator;
}

double thresholdFilter(double input) {
  if (input < THRESHOLD_CUTOFF_VALUE) {
    return 0;
  }
  return input;
}

double convertGamma(double x) {
  return x * x * x;
}

void adjustScaleForInput(double normalizedInput) {
  uint32_t nowMs = millis();
  uint32_t timeSinceLastScaleUpdateMs = nowMs - lastScaleTimeMs;

  autoLevelWindowMax = max(autoLevelWindowMax, normalizedInput);

  if (DEBUG) {
    LOG("; wMax: ")
    char str[16];
    dtostrf(autoLevelWindowMax, 7, 5, str);
    LOG(str)
  }
  
  if (timeSinceLastScaleUpdateMs > AUTO_LEVEL_WINDOW_SIZE_MS) {
    inputScale = autoLevelWindowMax > 0 ? AUTO_LEVEL_WINDOW_MAX_TARGET / autoLevelWindowMax : DEFAULT_INPUT_SCALE;

    autoLevelWindowMax = 0;
    
    lastScaleTimeMs = nowMs;
  }
}

// converts into an unsigned 32-bit int equal to the magnitude of the input
uint32_t convertAbs(int val) {
  if (val < 0) {
    return (uint32_t) -val;
  }
  return (uint32_t) val;
}

void writeOutputPwm(double duty) {
  // map onto and constrain to [0..255]
  int output = (int) (duty * 255);
  output = constrain(output, 0, 255);
  LOG("; outPwm: ")
  LOGLN(output)
  
  // set the output PWM
  analogWrite(PIN_OUT_R, output);
  analogWrite(PIN_OUT_B, output);
}

