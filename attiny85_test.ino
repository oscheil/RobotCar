    int irPin=2;
    int onoff = 0;
    void setup()
    {
      pinMode(irPin,INPUT);
      pinMode(1,OUTPUT);
    }
     
    void loop()
    {
      if(pulseIn(irPin,LOW))
      {
        delay(300); // the debounce time 300ms;
        if(onoff == 0)
        {
          digitalWrite(1,HIGH);
          onoff =1;
        }
        else
        {
          digitalWrite(1,LOW);
          onoff =0;
        }
      }
    }
