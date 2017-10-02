
/*
  Stepper Motor Control - speed control
  By Ville Kukko-Liedes For Platonics Oy

  Based on work by Tom Igoe

  Setup Config variables. The tester has 3 modes: E, XY, and Z. Default is E.
  When booting the tester, wait for light to turn solid and press UL for XY or UR for Z.
  If nothing is pressed within 2sec the tester remains in E-mode

*/

#define UL 18
#define UR 19
#define LL 17
#define LR 16

#define DEBUG 1

// Config

int speedE[] = {5,10,20,30}; // mm/s, 4 different speeds can be set and selected with LR-button
int speedXY[] = {20,40,80,300}; // mm/s
int speedZ[] = {1,2,3,4}; // mm/s

int stepsPerRev[] = {200,200,200}; //motor steps per revolution E, XY, Z
int mmPerRev[] = {5.16, 16*2.5, 8};
byte uSteps[] = {16,16,16};  //uSteps E, XY, Z

// END of Config



byte mode = 0; //0 = extruder, 1 = X/Y, 2 = Z
bool dir = true;
float curDelay = 0;
float newDelay = 0;
byte curSpeed = 0;

byte startUp = 0;

void setup() {
  if(DEBUG)Serial.begin(9600);
  pinMode(11, INPUT_PULLUP); // Enable internal pull-up resistor on pin 2
  pinMode(10, INPUT_PULLUP); // Enable internal pull-up resistor on pin 3
  pinMode(9, INPUT_PULLUP); // Enable internal pull-up resistor on pin 4
  
  pinMode(6, OUTPUT); //Direction
  pinMode(8, OUTPUT); //Step
  pinMode(7, OUTPUT); //Enable
  
  pinMode(LL, INPUT_PULLUP); //Direction
  pinMode(LR, INPUT_PULLUP); //Speed-up
  pinMode(UL, INPUT_PULLUP); //Do 1 revolution
  pinMode(UR, INPUT_PULLUP); //Constant revolution
  
  pinMode(13, OUTPUT); //LED indicator
  digitalWrite(13, HIGH);
  for(int i = 0; i<200; i++){
    if (digitalRead(UR) == LOW) { // Z-mode (default Extruder)
      mode = 2;
      i = 200;
    }else if (digitalRead(UL) == LOW) { // XY-mode (default Extruder)
      mode = 1;
      i = 200;
    }
    delay(10);
  }
  digitalWrite(13, LOW);
}


void loop() {
  if(startUp == 0){
    startUp=1;
    delay(700);
    blinkTimes(mode+1);
    curDelayUpdate();
  }
  
  if(!digitalRead(UR)){  // Continous rotation when UR pressed
    if (dir == true) digitalWrite(6, HIGH);
    else digitalWrite (6, LOW);

    digitalWrite (13, HIGH);

    byte byteLOW = PORTB;
    byte byteHIGH = PORTB ^ B00000001; 
  
    while (!digitalRead(UR)) {
      PORTB = byteHIGH;  //Pin 8 High
      delayMicroseconds(newDelay);
      PORTB = byteLOW;  //Pin 8 Low
      delayMicroseconds(newDelay);
  
    }
    digitalWrite (13, LOW);
  }
  else{
 
    if (!digitalRead(LR)) {
      if(curSpeed < 3) curSpeed++;
      else curSpeed = 0;
      blinkTimes(curSpeed+1);
      curDelayUpdate();
      while (!digitalRead(LR)){} // wait to release button
      delay(20); //debounce
    } else
    if (!digitalRead(LL)) {
      dir = !dir;
      blinkTimes(dir+1);
      while (!digitalRead(LL)){}; // wait to release button
      delay(20); //debounce
    } else
    if (!digitalRead(UL)) {
      doSteps();
      while (!digitalRead(UL)){} // wait to release button
      delay(20); //debounce
    }
    
  }
}

void doSteps() {
  digitalWrite(13,HIGH);
  if (dir == true) digitalWrite(6, HIGH);
  else digitalWrite (6, LOW);

  byte byteLOW = PORTB;
  byte byteHIGH = PORTB ^ B00000001; 
  long Time = micros();
  for (int n = 0; n < stepsPerRev[mode]*uSteps[mode]; n++){
    PORTB = byteHIGH;  //Pin 8 High
    delayMicroseconds(newDelay);
    PORTB = byteLOW;  //Pin 8 Low
    delayMicroseconds(newDelay);
  }
  long endTime = micros();
  
  Serial.print("Multiplier: ");
  Serial.print(float((endTime-Time)/(stepsPerRev[mode]*uSteps[mode]*curDelay)));
  Serial.print(" Diff: ");
  Serial.println(((endTime-Time)-(stepsPerRev[mode]*uSteps[mode]*curDelay))/(stepsPerRev[mode]*uSteps[mode]));
  Serial.print("Theory: ");
  Serial.print(curDelay);
  Serial.print(" Adjusted: ");
  Serial.print(newDelay*2);
  Serial.print(" Reality: ");
  Serial.println(float((endTime-Time)/(stepsPerRev[mode]*uSteps[mode])));
  
  digitalWrite(13,LOW);
}

void curDelayUpdate(){
  if(mode==0)curDelay = 1000000*(float(mmPerRev[mode]))/(float(uSteps[mode])*float(stepsPerRev[mode])*float(speedE[curSpeed]));
  else if(mode==1)curDelay = 1000000*(float(mmPerRev[mode]))/(float(uSteps[mode])*float(stepsPerRev[mode])*float(speedXY[curSpeed]));
  else if(mode==2)curDelay = 1000000*(float(mmPerRev[mode]))/(float(uSteps[mode])*float(stepsPerRev[mode])*float(speedZ[curSpeed]));

  if(DEBUG)Serial.println(curDelay);
  
  if(curDelay >= 12)newDelay = curDelay-10;
  else newDelay = 2;
  newDelay = newDelay/2;
}

void blinkTimes(int count){
  for (int n = 0; n < count; n++) {
    blink();
  }
}

void blink() {
  digitalWrite(13, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(150);                       // wait for a second
  digitalWrite(13, LOW);    // turn the LED off by making the voltage LOW
  delay(150);
}

