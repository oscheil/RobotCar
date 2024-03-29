/*
  Blink

  Turns an LED on for one second, then off for one second, repeatedly.

 
*/

// io27 für geekworm
int BlueLEDPin = 32;
// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin BlueLEDPin as an output.
  pinMode(BlueLEDPin, OUTPUT);
}

// the loop function runs over and over again forever
void loop() {
  digitalWrite(BlueLEDPin, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(1000);                       // wait for a second
  digitalWrite(BlueLEDPin, LOW);    // turn the LED off by making the voltage LOW
  delay(1000);                       // wait for a second
}
