#define enable_right 6
#define motor_in1_right 5
#define motor_in2_right 7
#define enable_left 10
#define motor_in3_left 9
#define motor_in4_left 8
const int rpm=10;
const int wheelRadius=3.8; //cm 
const int time=100; // milliseconds
const int turnTime=5000;
int motorDistance=rpm*2*PI*wheelRadius*time/60;
/*
  From https://www.youtube.com/watch?v=e-ePKrBu6bY. 
  Forward and break tells the motors how to move
  To get backward(), just switch HIGH->LOW and LOW->HIGH in forward()
  I think the analogWrite gives us RPM but I might be wrong because the numbers aren't exact***
  To move left or right, the left motor has to go in one direction while the right motor goes in the other direction
*/
void Movement(int rpm, int in1, int in2, int in3, int in4)
{
 	analogWrite(enable_right,rpm);
  	digitalWrite(motor_in1_right, in1);
  	digitalWrite(motor_in2_right, in2);
  	analogWrite(enable_left,rpm);
  	digitalWrite(motor_in3_left, in3);
  	digitalWrite(motor_in4_left, in4);
}
void Forward(){ Movement(rpm,0,1,0,1); }
void Backward(){ Movement(rpm,1,0,1,0); }
void Break(){ Movement(rpm,0,0,0,0); }
void UturnLeft(){ Movement(rpm,0,1,1,0); delay(turnTime); } 
void UturnRight(){ Movement(rpm,1,0,0,1); delay(turnTime); } 

const int leftSensor=4;
const int midSensor=3;
const int rightSensor=2;
int leftDistance;
int midDistance;
int rightDistance;

/* From Ultrasonic Rangefinder Ping))) Sketch
  returns the distance to the
  closest object in range. To do this, it sends a
  pulse to the sensor to initiate a reading, then
  listens for a pulse to return.  The length of
  the returning pulse is proportional to the
  distance of the object from the sensor.
*/
long readUltrasonicDistance(int triggerPin, int echoPin)
{
  pinMode(triggerPin, OUTPUT);  // Clear the trigger
  digitalWrite(triggerPin, LOW);
  delayMicroseconds(2);
  // Sets the trigger pin to HIGH state for 10 microseconds
  digitalWrite(triggerPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(triggerPin, LOW);
  pinMode(echoPin, INPUT);
  // Reads the echo pin, and returns the sound wave travel time in microseconds
  return pulseIn(echoPin, HIGH);
}

void calculateDistance(){
  leftDistance = (0.01723 * readUltrasonicDistance(leftSensor, leftSensor));
  midDistance = (0.01723 * readUltrasonicDistance(midSensor, midSensor));
  rightDistance = (0.01723 * readUltrasonicDistance(rightSensor, rightSensor));
  leftDistance=leftDistance%336;
  midDistance=midDistance%336;
  rightDistance=rightDistance%336;
  if(leftDistance==0){ leftDistance+=337; }
  if(midDistance==0){ midDistance+=337; }
  if(rightDistance==0){ rightDistance+=337; }
}

const int arraySize=12; // max is 127
int traveled[12][12]={{3,3,3,3,3,3,3,3,3,3,3,3},
  				  {3,0,0,0,0,0,0,0,0,0,0,3},
   				  {3,0,0,0,0,0,0,0,0,0,0,3},
                  {3,0,0,0,0,0,0,0,0,0,0,3},
  				  {3,0,0,0,0,0,0,0,0,0,0,3},
  				  {3,0,0,0,0,0,0,0,0,0,0,3},
  				  {3,0,0,0,0,0,0,0,0,0,0,3},
  				  {3,0,0,0,0,0,0,0,0,0,0,3},
   				  {3,0,0,0,0,0,0,0,0,0,0,3},
                  {3,0,0,0,0,0,0,0,0,0,0,3},
  				  {3,0,0,0,0,0,0,0,0,0,0,3},
  				  {3,3,3,3,3,3,3,3,3,3,3,3}};
int lastRow=arraySize-2;
int lastCol=1;
int direction=1; // odd is up, even is down

// Checks if the table is cleaned
bool checkArrayFull(){
  return((traveled[lastRow+1][lastCol+1]==3)&&(lastCol==arraySize-2));
}
// Checks if there's an object ahead
bool checkObjectAhead(){
  if(leftDistance<=7&&midDistance<=7&&rightDistance<=7&&direction%2!=0){
  	traveled[lastRow-1][lastCol]=3;
  } else if (leftDistance<=7&&midDistance<=7&&rightDistance<=7&&direction%2==0){
  	traveled[lastRow+1][lastCol]=3;
  }
  return ((direction%2!=0&&traveled[lastRow-1][lastCol]!=3)||(direction%2==0&&traveled[lastRow+1][lastCol]!=3));
}
// Updates Array where it traveled
void updateArray(int motorDistance, int mode){
  switch(mode){
  	case 1: // Forward up
      for(int i=0; i<motorDistance&&lastRow>=2; i++){
        lastRow--;
        traveled[lastRow][lastCol]=1;
      }
      break;
    case 2: // Forward down
      for(int i=0; i<motorDistance&&lastRow<arraySize-2; i++){
        lastRow++;
        traveled[lastRow][lastCol]=1;
      }
      break;
    case 3: // Uturn Right
      if(lastCol<arraySize-1){
        lastCol++;
        traveled[lastRow][lastCol]=1;
      } 
      break;
    case 4: // Uturn Left
      if(lastCol<arraySize-1){
        lastCol++;
        traveled[lastRow][lastCol]=1;
      } 
      break;
  }
}
void printArray(){
  for(int i=0; i<arraySize; i++){
    for(int j=0; j<arraySize; j++){
      Serial.print(traveled[i][j]);
    }
    Serial.println();
  }
  Serial.println();
}

void setup()
{
  Serial.begin(9600);
  pinMode(enable_right,OUTPUT);
  pinMode(motor_in1_right, OUTPUT);
  pinMode(motor_in2_right, OUTPUT);
  pinMode(enable_left,OUTPUT);
  pinMode(motor_in3_left, OUTPUT);
  pinMode(motor_in4_left, OUTPUT);
  traveled[lastRow][lastCol]=1; //start at left corner
}

void loop()
{
  // Calculates the distance to object and records it in leftDistance, midDistance, rightDistance
  calculateDistance();

    Serial.print(leftDistance);
    Serial.print("\t");
    Serial.print(midDistance);
    Serial.print("\t");
    Serial.println(rightDistance);
  
  // Checks to see if the table cleaner went through the whole table
  if(!checkArrayFull()){
    Serial.println("Table not cleaned.");
    // Checks to see if there's an object or wall ahead
    if(checkObjectAhead()){
      Serial.println("No object ahead.");
      Forward();
      if(direction%2!=0){ updateArray(motorDistance%10,1); }
      else{ updateArray(motorDistance%10,2); }
    } else{
      if(direction%2!=0){
        Serial.println("Object ahead. Turning Right, Moving down. ");
      	UturnRight();
        updateArray(motorDistance%10,3);
        direction++;
        Forward();
      	updateArray(motorDistance%10,2);
      } else{
        Serial.println("Object ahead. Turning Left, Moving up. ");
      	UturnLeft();
        updateArray(motorDistance%10,4);
        direction++;
        Forward();
        updateArray(motorDistance%10,1);
      }
    } 
  } else{
    Serial.println("Table cleaned.");
  	playSong();
  	while(1){}
  }
  printArray();
  delay(time); // time between loop, also used to calculate distance traveled by cleaner
}

/* 
  Jigglypuff's Song
  Connect a piezo buzzer or speaker to pin 11 or select a new pin.
  More songs available at https://github.com/robsoncouto/arduino-songs                                            
                                              
  Robson Couto, 2019
*/
#define NOTE_B0  31
#define NOTE_C1  33
#define NOTE_CS1 35
#define NOTE_D1  37
#define NOTE_DS1 39
#define NOTE_E1  41
#define NOTE_F1  44
#define NOTE_FS1 46
#define NOTE_G1  49
#define NOTE_GS1 52
#define NOTE_A1  55
#define NOTE_AS1 58
#define NOTE_B1  62
#define NOTE_C2  65
#define NOTE_CS2 69
#define NOTE_D2  73
#define NOTE_DS2 78
#define NOTE_E2  82
#define NOTE_F2  87
#define NOTE_FS2 93
#define NOTE_G2  98
#define NOTE_GS2 104
#define NOTE_A2  110
#define NOTE_AS2 117
#define NOTE_B2  123
#define NOTE_C3  131
#define NOTE_CS3 139
#define NOTE_D3  147
#define NOTE_DS3 156
#define NOTE_E3  165
#define NOTE_F3  175
#define NOTE_FS3 185
#define NOTE_G3  196
#define NOTE_GS3 208
#define NOTE_A3  220
#define NOTE_AS3 233
#define NOTE_B3  247
#define NOTE_C4  262
#define NOTE_CS4 277
#define NOTE_D4  294
#define NOTE_DS4 311
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_FS4 370
#define NOTE_G4  392
#define NOTE_GS4 415
#define NOTE_A4  440
#define NOTE_AS4 466
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_CS5 554
#define NOTE_D5  587
#define NOTE_DS5 622
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_FS5 740
#define NOTE_G5  784
#define NOTE_GS5 831
#define NOTE_A5  880
#define NOTE_AS5 932
#define NOTE_B5  988
#define NOTE_C6  1047
#define NOTE_CS6 1109
#define NOTE_D6  1175
#define NOTE_DS6 1245
#define NOTE_E6  1319
#define NOTE_F6  1397
#define NOTE_FS6 1480
#define NOTE_G6  1568
#define NOTE_GS6 1661
#define NOTE_A6  1760
#define NOTE_AS6 1865
#define NOTE_B6  1976
#define NOTE_C7  2093
#define NOTE_CS7 2217
#define NOTE_D7  2349
#define NOTE_DS7 2489
#define NOTE_E7  2637
#define NOTE_F7  2794
#define NOTE_FS7 2960
#define NOTE_G7  3136
#define NOTE_GS7 3322
#define NOTE_A7  3520
#define NOTE_AS7 3729
#define NOTE_B7  3951
#define NOTE_C8  4186
#define NOTE_CS8 4435
#define NOTE_D8  4699
#define NOTE_DS8 4978
#define REST      0
// change this to make the song slower or faster
int tempo = 185;
// change this to whichever pin you want to use
int buzzer = 11;
// notes of the moledy followed by the duration.
// a 4 means a quarter note, 8 an eighteenth , 16 sixteenth, so on
// !!negative numbers are used to represent dotted notes,
// so -4 means a dotted quarter note, that is, a quarter plus an eighteenth!!
int melody[] = {
  // Jigglypuff's Song
  // Score available at https://musescore.com/user/28109683/scores/5044153
  NOTE_D5,-4, NOTE_A5,8, NOTE_FS5,8, NOTE_D5,8,
  NOTE_E5,-4, NOTE_FS5,8, NOTE_G5,4,
  NOTE_FS5,-4, NOTE_E5,8, NOTE_FS5,4,
  NOTE_D5,-2,
  NOTE_D5,-4, NOTE_A5,8, NOTE_FS5,8, NOTE_D5,8,
  NOTE_E5,-4, NOTE_FS5,8, NOTE_G5,4,
  NOTE_FS5,-1,
  NOTE_D5,-4, NOTE_A5,8, NOTE_FS5,8, NOTE_D5,8,
  NOTE_E5,-4, NOTE_FS5,8, NOTE_G5,4,
  NOTE_FS5,-4, NOTE_E5,8, NOTE_FS5,4,
  NOTE_D5,-2,
  NOTE_D5,-4, NOTE_A5,8, NOTE_FS5,8, NOTE_D5,8,
  NOTE_E5,-4, NOTE_FS5,8, NOTE_G5,4,
  NOTE_FS5,-1,
};
// sizeof gives the number of bytes, each int value is composed of two bytes (16 bits)
// there are two values per note (pitch and duration), so for each note there are four bytes
int notes = sizeof(melody) / sizeof(melody[0]) / 2;
int wholenote = (60000 * 4) / tempo; // this calculates the duration of a whole note in ms
int divider = 0, noteDuration = 0;
void playSong(){
  // iterate over the notes of the melody.
  // Remember, the array is twice the number of notes (notes + durations)
  for (int thisNote = 0; thisNote < notes * 2; thisNote = thisNote + 2) {
    // calculates the duration of each note
    divider = melody[thisNote + 1];
    if (divider > 0) {
      // regular note, just proceed
      noteDuration = (wholenote) / divider;
    } else if (divider < 0) {
      // dotted notes are represented with negative durations!!
      noteDuration = (wholenote) / abs(divider);
      noteDuration *= 1.5; // increases the duration in half for dotted notes
    }
    // we only play the note for 90% of the duration, leaving 10% as a pause
    tone(buzzer, melody[thisNote], noteDuration * 0.9);
    // Wait for the specief duration before playing the next note.
    delay(noteDuration);
    // stop the waveform generation before the next note.
    noTone(buzzer);
  }
}