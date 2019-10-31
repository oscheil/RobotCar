int g_1 = 10;
int y_1 = 9;
int r_1 = 8;

// the setup function runs once when you press reset or power the board
void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(g_1,OUTPUT);
  pinMode(y_1,OUTPUT);
  pinMode(r_1,OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);   // turn the LED on (HIGH is the voltage level)     
}

// the loop function runs over and over again forever
void loop() {
   // wait for a second
  digitalWrite(r_1, HIGH);
  delay(5000);
  digitalWrite(y_1, HIGH);
  delay(1000);
  digitalWrite(r_1, LOW);
  digitalWrite(y_1, LOW);
  digitalWrite(g_1, HIGH);
  delay(5000);
  blink(g_1, 4);
  digitalWrite(g_1, LOW);
  digitalWrite(y_1, HIGH);
  delay(1000);
  digitalWrite(y_1, LOW);
  digitalWrite(r_1, HIGH);
  delay(1000);
  
}
void blink(int pin, int times){
  for(int i = 0; i < times; i++){
    digitalWrite(pin, LOW);
    delay(500);
    digitalWrite(pin, HIGH);
    delay(500);
  }
}
