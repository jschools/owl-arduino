// Prototype for Jonathan
// Code preloaded into arduino nano

void setup(){

	// Set the REF pin to output 1.1V
	analogReference(INTERNAL);

	// Transistor for LED is connected to this pin
	pinMode(11, OUTPUT);
}

void loop() {

	// The REF pin won't output the right voltage until the function 'analogRead' is called a few times.
	analogRead(0);

	// Wait 5 seconds and set the duty cycle to 86%
	delay(5000);
	analogWrite(11,220);

	// Wait 5 seconds and set the duty cycle to 50%
	delay(5000);
	analogWrite(11,128);

}