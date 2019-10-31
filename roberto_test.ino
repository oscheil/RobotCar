double NummerN;
int _output_LedRot = 11;
int _output_LedGelb = 10;
int _led_L = 13;
void setup()
{
    Serial.begin(9600); 
    pinMode(_output_LedRot, OUTPUT);
    pinMode(_output_LedGelb, OUTPUT);
    pinMode(_led_L, OUTPUT);
    NummerN = 10;
}

void loop()
{
    digitalWrite(_led_L, HIGH);
    delay(500);
    digitalWrite(_led_L, LOW);
    delay(3500);
    digitalWrite(_output_LedRot, 1);
    delay(500);
    digitalWrite(_output_LedRot, 0);
    delay(500);
}
