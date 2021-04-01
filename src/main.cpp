#include <Arduino.h>
#include <BasicStepperDriver.h>
#include <servo.h>



//rpm per motor
#define StepperHeadRPM 200
#define StepperEndRPM 200
#define StepperBaseRPM 200

#define TrigPin 4  //sonics sensor pins defined at random, Need changing when testing!!
#define EchoPin 5

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


// met de klok mee of er tegen in
int CW[5] = {41, 58, 142, 158, 191};   //clockwise
int CCW[5] = { 142, 158, 41, 58, 83};  //counterclockwise
int GDirection[5];                     // de huidige volgorde van het spel
// de laaste is rust positie 
// 0 is rust positie van de diabolo


int HdiabolD[5];     // de richting van H diabol
int LdiabolD[5];     // de richting van L diabol
// 1 is met klok mee 0 is tegen de klok in 


//servo
Servo Gripper;

//distance
int distanceX;
int distanceY;




//laatste position is 0 aan het begin
int EndPos = zone[1];
int StartPos; //is de positie nadat hij de diabolo's heeft gevonden en hij start met het spel
int StartVlak;   // welke hij van de 4 belangerijke vakken als eerst heeft gekozen om het spel mee te beginnen
int Steps;
float SDelay;  //delay voor tussen de steppen
int STlocation; // memory slot van de stepdistancefunctie voor later gebruik 


//geven de posities van de diabolen aan
int PDiabolH;
int PDiabolL;


// geeft de state van die machine aan 
int StateDiabolL;
int StateDiabolH;
/////////
int GripperS = 0;
// 0 is dicht 1 is open

int HeadS;
// 0 is center  1 is vlak voor de head 

//klok die op de achtergrond de tijd bijhoud
unsigned long Clock;

int tijdverlopen; 

int SonicDistance; 

bool Diabololaying;  
bool Diabolostanding;



//////////////////////////////////////////////////////////////////////////////////////////////////////////
// calculation functions
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
int calculateDistanceSteps(int Distance){ // in cm
   float MmStep = 0.18849;
   int StepDistance = Distance/MmStep; 
   STlocation = StepDistance; 
  return(StepDistance);  
  
}



/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//position functions

void SwitchHead(){
 if(HeadS == 0){
 //head is in base position 
   StepperHead.move(calculateDistanceSteps(20));
  calculateSDelay( calculateDistanceSteps(20), StepperHeadRPM);
   HeadS = 1; 
 }


 if(HeadS == 1){
   StepperEnd.move(-calculateDistanceSteps(20));
   calculateSDelay(-calculateDistanceSteps(20), StepperHeadRPM);
   //head is in middle position 
    HeadS = 0; 
 }
}



void SwitchEnd(){
  int distanceGripDH;
  int distanceGripDL; 
  int GP; 
  if(StateDiabolH == true){
    StepperEnd.move(distanceGripDH);
    GP = 1;    // gripper is in low positition for high diabolo 
    if(GP == 1 ){
      StepperEnd.move(-distanceGripDH);
    }
    else{
       GP = 0; 
    }
  }
  else{
    StepperEnd.move(distanceGripDL);
    GP = 2 ; 
    if(GP == 2){
       StepperEnd.move(-distanceGripDL);
    }
    else{
      GP = 0 ; 
    }
  }
  // if state diabolo is H 
  // hoe laag moet de grijper zakken 
  // else hoe laag moet de grijper zakken voor de diabolo L
  
 }




void SwitchGripper(){
// weer een check als hij open staat dicht doen als hij dicht is open doen 
// code voor het naar het midden gaan van de Head stepper of naar buiten afhankelijk van de huidige positie
// 0 is dicht 1 is open
if (GripperS == 0){ // hij is dicht dus nu open
Gripper.write(180);
GripperS = 1;
}
if (GripperS == 1){ // is open dus nu dicht 
Gripper.write(0);
GripperS = 0;

}
else{
  Serial.println("error");
}




  
  
}



////////////////////////////////////////////////////////////////////////////////////////////
// sensor funties
void sonic(){
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
}




///////////////////////////////////////////////////////////////////////////////////////////
//safety functions
void EndTimer(){
// kijken of hij de 5 min overschrijd 
int seconds;  
int totaltime = 300; 

seconds = Clock/ 1000; 

if(seconds == totaltime){
  tijdverlopen = 1 ;
}
else{
  tijdverlopen = 0 ;
}

}

////////////////////////////////////////////////////////////////////////////////////////////
//game functions

/*een van de blangerijskte functies moeten heel veel testen of dit wel werkt en of we bepaalde dingen over het hoofd hebben gezien 
mijn vermoeden is ook dat er iets fout gaat bij het uilezen van de sensor omdat hij dit maar een keer odet waneer hij over een vlak heen gaat*/


void Begin(){
int unknown = 0;
int i;
int loop;
// hij moet gaan draaien totdat hij iets vind het eerste wat hij doet het opstarten hij doet dit ook maar een keer
// beweeg grijparm naar het midden toe

for(unknown = 0 ; unknown == 0 ; i++){

StepperBase.move(calculateSteps(zone[i], zone[i+1]));
calculateSDelay(8, StepperBaseRPM);                     //bereken delay 

sonic();      //hij update de lengte waneer hij in positie is als er iets onder is  
delay(20);
Serial.println("hij is aan het zoeken naar de 1e diabol");

//trek conclusie uit waarde is dit de pos van de L of H diabol
  if (SonicDistance == 12){
    PDiabolH = zone[i];
    unknown = 1;
    Serial.println("de 1e diabolo is H");
// hier steld hij de ene array aan de andere
    for(loop = 0; loop < 5; loop++) {
      HdiabolD[loop] = CW[loop];
    }
    
  }
  if (SonicDistance == 9){
    PDiabolL = zone[i];
    unknown = 1;
    Serial.println("de 1e diabolo is L");
    // hier steld hij de ene array aan de andere
    for(loop = 0; loop < 5; loop++) {
      LdiabolD[loop] = CW[loop];
    }
  }
}
//zoeken naar de 2e diabol
Serial.println("hij gaat nu de 2e diabol zoeken");
// niet i resetten

for(unknown = 1 ; unknown == 1; i++){

StepperBase.move(calculateSteps(zone[i], zone[i+1]));
delay(calculateSDelay(8, StepperBaseRPM));                  //bereken delay 

sonic();      // hij update de lengte als er iets onder is 
delay(20);
Serial.println("hij is aan het zoeken naar de 2e diabol");

//trek conclusie uit waarde is dit de pos van de L of H diabol
  if (SonicDistance == 12){
    PDiabolH = zone[i];
    unknown = 2;
    Serial.println("de 2e diabolo is H");
    // hier steld hij de ene array aan de andere
    for(loop = 0; loop < 5; loop++) {
      HdiabolD[loop] = CCW[loop];
    }
    
  }
  if (SonicDistance == 9){
    PDiabolL = zone[i];
    unknown = 2;
    Serial.println("de 2e diabolo is L");
    // hier steld hij de ene array aan de andere
    for(loop = 0; loop < 5; loop++) {
      LdiabolD[loop] = CCW[loop];
    }
  }

}
// hij heeft nu beide diabolo's gevonden
EndPos = i;

// de afstand moet er nog in geprogrameerd worden en is nu hard coded dit kan opgelost worden met een gripper die het hele vak over komt
// hij gaat naar voren toe 
SwitchHead();

// hij moet nu naar beneden
SwitchEnd();

// kijken hoe ver hij ligt ? moet dit of maken wij de grijparm groot genoeg dat het niks uit maakt 
// we gaan er van uit dat de gripper in een open positie is 

SwitchGripper();       // hij moet dichtgaan // kijkt dus ook of hij te maken heeft met low en high

// hij moet naar boven
SwitchEnd();

// hij gaat naar de kern
SwitchHead();

// dit is het einde van de verzamel en registreer fase
}



void SwitchState(){
  //hier moet switchen van state en daarbij de ene diabol neerleggen en de ander op pakken



  if (StateDiabolH == 1){
    // hij gaat nu de andere diabol op pakken

    // de game direction moet vedanderen nadat hij de vorige heeft afgezet 
    // dus rewrite gamestate met Cw als Ddiabol h = 1 en andersom
    // switch van state

    StateDiabolL = 1;
    StateDiabolH = 0;





}
  if (StateDiabolL == 1){
    // hij gaat nu de andere diabol op pakken



    //switch van state
    StateDiabolL = 0;
    StateDiabolH = 1;








}
  else{
  //error
  Serial.println("Critical error Diabolo state Unknown");
}
}






void GatherPoint(){

// eerst moet hij naar vlak 16 toe nadat er is besloten of hij met de klok mee of tegen in gaat 


StepperBase.move(calculateSteps(EndPos, zone[19]));
calculateSDelay(calculateSteps(EndPos, zone[19]), StepperBaseRPM);



Serial.print("hij begint nu aan zijn rondje");
// begin spel strategie
// ga naar voren toe 
SwitchHead();
// hij is nu naar voren toe gegaan
}




 








//////////////////////////////////////////////////////////////////////////////////////////////////////////
//start main code 
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
  Begin();
}



void loop() {










}
