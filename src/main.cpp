#include <Arduino.h>
#include <BasicStepperDriver.h>

//rpm per motor
#define StepperHeadRPM 200
#define StepperEndRPM 200
#define StepperBaseRPM 200

const int TrigPin = 4;   //sonics sensor pins defined at random, Need changing when testing!!
const int EchoPin = 5;

//example : BasicStepperDriver stepper(MOTOR_STEPS, DIR, STEP);
BasicStepperDriver StepperBase(200, 8, 9);
BasicStepperDriver StepperHead(200, 1, 2);
BasicStepperDriver StepperEnd(200, 12, 13);

//zet zones op en geeft deze waardes op opgeroepen te worden zie foto verdeling voor veder uitleg
int zone[24] = {0, 8, 16, 25, 33, 41, 50, 58, 66, 75, 83, 91, 100, 108, 116, 125, 133, 142, 150, 158, 166, 175, 183, 191};

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


//klaar 
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

//hij gaat hier voor een ronde punten verzamelen nadat hij van game state verandert naar de volgende diabolo toe gaat we gaan er van uit dat hij steeds begint met de diabolo in zijn hand

//naar boven en rekeninghouden met de tijd voor de servo om het op te pakken ook moet je rekening houden met of je te maken heb met liggend of staand
if (StateDiabolH == 1){
//staande pos minder naar beneden
StepperEnd.move(calculateDistanceSteps(200)); // hij moet 200mm naar beneden


}
if (StateDiabolL == 1){
// liggende pos meer naar beneden
StepperEnd.move(calculateDistanceSteps(400)); // hij moet 400mm naar beneden
}
else{
  //error
  Serial.println("Critical error Diabolo state Unknown");
}
  //beweeg de diabol naar de center to met de 2e Stepper
  //hierbij moeten we de afstand naar tanden gaan omzetten aan de hand met het tandwiel dus de positie waarin hij staat 
  //- de positie waarhij naar toe moet 
}
 


void switchState(){
// bepalen dat hij na 1 keer de LDiabolo heeft opgepakt switched naar de HDiabolo

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
calculateSDelay(8, StepperBaseRPM);                     //bereken delay 

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
// hij moet nu de diabol oppakken


// kijken hoe ver hij ligt ? moet dit of maken wij de grijparm groot genoeg dat het niks uit maakt 









}













void setup() {

  //start serial met Baudrate
  Serial.begin(9600);

  //example stepper.begin(RPM, MICROSTEPS); Start de steppers
  StepperBase.begin(StepperBaseRPM , 1);
  StepperEnd.begin(StepperEndRPM , 1);
  StepperHead.begin(StepperHeadRPM , 1);

  pinMode(TrigPin, OUTPUT);
  pinMode(EchoPin, INPUT);




}

void loop() {

sonic();
Serial.println(SonicDistance);
delay(200);
}
