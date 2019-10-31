int ip = 4;

int val = 0;

int led = 13;

void setup() {

  Serial.begin(9600);

  pinMode (ip, INPUT);

  pinMode (led, OUTPUT);
  digitalWrite(led, HIGH);
}

void loop() {

  val = digitalRead(ip);

  Serial.println(val, DEC);

  if(val >0)

  {

    digitalWrite(led, HIGH);

  }

  else

  {

    digitalWrite(led, LOW);

  }

  delay(400);

}


