
#include <stdint.h>

#define DEBUG false
#define LOG(message) if (DEBUG) { Serial.print(message); }
#define LOGLN(message) if (DEBUG) { Serial.println(message); }

#define PIN_IN A6
#define PIN_OUT_R 9
#define PIN_OUT_B 11

#define SAMPLE_COUNT 100
#define DEFAULT_INPUT_SCALE 4.0
#define WINDOW_SIZE_MS 2000.0
#define WINDOW_MAX_TARGET 0.95

// global variables
double inputScale;
unsigned long lastScaleTimeMs;
double inputWindowMax;

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
  inputWindowMax = 0;
  lastScaleTimeMs = 0;

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
  // sum a number of samples
  uint32_t sum = 0;
  for (int i = 0; i < SAMPLE_COUNT; i++) {
    int inValue = analogRead(PIN_IN);
    
    int zeroCentered = inValue - 512;

    sum += convertAbs(zeroCentered);
  }

  // convert the sum to a float
  double sumF = (double) sum;

  // map the sum onto [0..1]
  double mapped = sumF / (512.0 * SAMPLE_COUNT);

  return constrain(mapped, 0.0, 1.0);
}

double convertGamma(double x) {
  return x * x * x;
}

void adjustScaleForInput(double normalizedInput) {
  uint32_t nowMs = millis();
  uint32_t timeSinceLastScaleUpdateMs = nowMs - lastScaleTimeMs;

  inputWindowMax = max(inputWindowMax, normalizedInput);

  if (DEBUG) {
    LOG("; wMax: ")
    char str[16];
    dtostrf(inputWindowMax, 7, 5, str);
    LOG(str)
  }
  
  if (timeSinceLastScaleUpdateMs > WINDOW_SIZE_MS) {
    inputScale = WINDOW_MAX_TARGET / inputWindowMax;

    inputWindowMax = 0;
    
    lastScaleTimeMs = nowMs;
  }
}

// converts into an unsigned 16-bit int equal to the magnitude of the input
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

