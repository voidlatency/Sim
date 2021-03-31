#include <Arduino.h>
#include <BasicStepperDriver.h>
#include <servo.h>

//rpm per motor
#define StepperHeadRPM 200
#define StepperEndRPM 200
#define StepperBaseRPM 200

const int TrigPin = 4;   //sonics sensor pins defined at random, Need changing when testing!!
const int EchoPin = 5;

//example : BasicStepperDriver stepper(MOTOR_STEPS, DIR, STEP);
#include "DRV8825.h"
DRV8825 StepperBase(200, 8, 9);



BasicStepperDriver StepperHead(200, 1, 2);
BasicStepperDriver StepperEnd(200, 12, 13);

//zet zones op en geeft deze waardes op opgeroepen te worden zie foto verdeling voor veder uitleg
int zone[24] = {0, 8, 16, 25, 33, 41, 50, 58, 66, 75, 83, 91, 100, 108, 116, 125, 133, 142, 150, 158, 166, 175, 183, 191};
// belangerijke zones
// 20 punten zone[5]
// 14 punten zone[7]
// 16 punten zone[19]
// 18 punten zone[17]


//servo

Servo Gripper;





//laatste position is 0 aan het begin
int EndPos = zone[1];
int StartPos; //is de positie nadat hij de diabolo's heeft gevonden en hij start met het spel
int StartVlak;   // welke hij van de 4 belangerijke vakken als eerst heeft gekozen om het spel mee te beginnen
int Steps;
float SDelay;  //delay voor tussen de steppen
int STlocation; // memory slot van de stepdistancefunctie voor later gebruik 

int PDiabolH;
int PDiabolL;



int StateDiabolL;
int StateDiabolH;

int SonicDistance; 

bool Diabololaying;  
bool Diabolostanding;

int calculateSteps(int StartP, int Target){
//berekening stappen
int End1 = StartP;
int End2 = StartP + 200;      // 2e variant van dezelfde position
int StepOption1 = Target - End1;
int StepOption2 = Target - End2;
if (abs(StepOption1) <= abs(StepOption2)){
  Steps = StepOption1;
}
else{
  Steps = StepOption2;
}
// uiteindelijke steps
  return(Steps);
}


float calculateSDelay(int Step, int StepperRPM){
SDelay = Step/(StepperRPM * 0.006);

return(SDelay);
}
int calculateDistanceSteps(int positie){
   float MmStep = 0.18849;
   int StepDistance = positie/MmStep; 
   STlocation = StepDistance; 
  return(StepDistance);  
  
}



void SwitchHead(){
// code voor het naar het midden gaan van de Head stepper of naar buiten afhankelijk van de huidige positie



}

/*void SwitchEnd(H, L){
  int distanceGripDH;
  int distanceGripDL; 

  if(H == true){
    StepperEnd.move(distanceGripDH);
    char GP = 'L'; 
    if(){

    }
    
  }
  else{
    StepperEnd.move();
  }
  // if state diabolo is H 
  // hoe laag moet de grijper zakken 
  // else hoe laag moet de grijper zakken voor de diabolo L
  
 }

*/




int sonic(){
  long duration;
  int distance;  
 // Clears the trigPin condition
  digitalWrite(TrigPin, LOW);
  delayMicroseconds(2);
  // Sets de trigPin HIGH (ACTIVE) voor 10 microseconds
  digitalWrite(TrigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(TrigPin, LOW);
  duration = pulseIn(EchoPin, HIGH);
  distance = duration * 0.034 / 2;   // rekent afstand in cm
  SonicDistance = distance; 
 
  return(SonicDistance);


  }







void GatherPoint(){
// eerst moet hij naar vlak 16 toe 
StepperBase.move(calculateSteps(EndPos, zone[19]));
calculateSDelay(calculateSteps(EndPos, zone[19]), StepperBaseRPM);

Serial.print("hij begint nu aan zijn rondje");
// begin spel strategie
// ga naar voren toe 
SwitchHead();
// hij is nu naar voren toe gegaan








}



void SwitchState(){
  //hier moet switchen van state
  if (StateDiabolH == 1){
    // hij gaat nu de andere diabol op pakken
    StateDiabolL = 1;
    StateDiabolH = 0;
}
  if (StateDiabolL == 1){
    // hij gaat nu de andere diabol op pakken
    StateDiabolL = 0;
    StateDiabolH = 1;
}
  else{
  //error
  Serial.println("Critical error Diabolo state Unknown");
}
}
 




void Begin(){
int unknown = 0;
int i;
// hij moet gaan draaien totdat hij iets vind het eerste wat hij doet het opstarten hij doet dit ook maar een keer
// beweeg grijparm naar het midden toe

for(unknown = 0 ; unknown == 0 ; i++){

StepperBase.move(calculateSteps(zone[i], zone[i+1]));
calculateSDelay(8, StepperBaseRPM);                     //bereken delay 

int sonic = 1000; // tijdelijke int ligt aan de toekomstige sensor


//trek conclusie uit waarde is dit de pos van de L of H diabol
  if (sonic == 1000){
    PDiabolH = zone[i];
    unknown = 1;
  }
  if (sonic == 2000){
    PDiabolL = zone[i];
    unknown = 1;
  }
}
//zoeken naar de 2e diabol
Serial.println("hij gaat nu de 2e diabol zoeken");
// niet i resetten

for(unknown = 1 ; unknown == 1; i++){

StepperBase.move(calculateSteps(zone[i], zone[i+1]));
delay(calculateSDelay(8, StepperBaseRPM));                     //bereken delay 

int sonic; // tijdelijke int ligt aan de toekomstige sensor

//trek conclusie uit waarde is dit de pos van de L of H diabol
  if (sonic == 1000){
    PDiabolH = zone[i];
    unknown = 2;
  }
  if (sonic == 2000){
    PDiabolL = zone[i];
    unknown = 2;
  }

}

// hij heeft nu beide diabolo's gevonden
EndPos = i;

// de afstand moet er nog in geprogrameerd worden en is nu hard coded dit kan opgelost worden met een gripper die het hele vak over komt
int distanceX;
int distanceY;

// hij gaat naar voren toe 
StepperHead.move(calculateDistanceSteps(distanceX));
delay(calculateSDelay(calculateDistanceSteps(distanceX), StepperHeadRPM));

// hij moet nu naar beneden
StepperHead.move(calculateDistanceSteps(distanceY));
delay(calculateSDelay(calculateDistanceSteps(distanceY), StepperHeadRPM));

// kijken hoe ver hij ligt ? moet dit of maken wij de grijparm groot genoeg dat het niks uit maakt 
// we gaan er van uit dat de gripper in een open positie is 

Gripper.write(180);       // hij moet dichtgaan

// hij moet naar boven

StepperHead.move(-calculateDistanceSteps(distanceY));
delay(calculateSDelay(calculateDistanceSteps(distanceY), StepperHeadRPM));

// hij gaat naar de kern
StepperHead.move(-calculateDistanceSteps(distanceX));
delay(calculateSDelay(calculateDistanceSteps(distanceX), StepperHeadRPM));
}













void setup() {

  //start serial met Baudrate
  Serial.begin(9600);

  //example stepper.begin(RPM, MICROSTEPS); Start de steppers
  StepperBase.begin(StepperBaseRPM , 1);
  StepperEnd.begin(StepperEndRPM , 1);
  StepperHead.begin(StepperHeadRPM , 1);

  //pinmode aangeven
  pinMode(TrigPin, OUTPUT);
  pinMode(EchoPin, INPUT);


// servo aan pin 10 koppelen
  Gripper.attach(10);
}



void loop() {

sonic();
Serial.println(SonicDistance);
delay(200);
}
