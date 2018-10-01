volatile unsigned long counter = 0;

void Timer_ISR(void) {
  // put your main code here, to run repeatedly:
  int i = 0;
  counter++;
  #if DEBUG==1
  Serial.print(counter);
  #endif

  if (counter > 30){
    for (i = 0; i<4; i++)
    knocks[i] = 3;
    counter = 0;
    index = -1;
    Timer1.stop();
    #if DEBUG==1
    Serial.println("3333");
    #endif
  }
}
