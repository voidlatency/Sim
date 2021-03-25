#include <Arduino.h>
#include <Stepper.h>

//rpm per motor
int StepperHeadRPM = 200;
int StepperEndRPM = 200;
int StepperBaseRPM = 200;


//aantal stppen per rotatie
Stepper StepperBase = Stepper(StepperHeadRPM, 8, 9);
Stepper StepperHead = Stepper(StepperHeadRPM, 1, 2);
Stepper StepperEnd = Stepper(StepperEndRPM, 12, 13);

//zet zones op en geeft deze waardes op opgeroepen te worden zie foto verdeling voor veder uitleg
int zone[24] = {191, 0, 8, 16, 25, 33, 41, 50, 58, 66, 75, 83, 91, 100, 108, 116, 125, 133, 142, 150, 158, 166, 175, 183};

//test
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



//test


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

void GatherPoint(){

// we gaan er van uit dat hij al boven de diabol zit heirbij moeten wij dus hen naar beneden brengen ne dan weer 
//naar boven en rekeninghouden met de tijd voor de servo om het op te pakken ook moet je rekening houden met of je te maken heb met liggend of staand
if (StateDiabolH == 1){
//staande pos minder naar beneden
StepperEnd.step(calculateDistanceSteps(200)); // hij moet 200mm naar beneden


}
if (StateDiabolL == 1){
// liggende pos meer naar beneden
StepperEnd.step(calculateDistanceSteps(400)); // hij moet 400mm naar beneden
}
else{
  //error
  Serial.println("Critical error Diabolo state Unknown");
}




  //beweeg de diabol naar de center to met de 2e Stepper
  //hierbij moeten we de afstand naar tanden gaan omzetten aan de hand met het tandwiel dus de positie waarin hij staat 
  //- de positie waarhij naar toe moet 
  
}
 





 
void SetupScan(){
  // de eerste scan hierbij moeten de posities van de 2 diabolen worden onhouden om oz later ze weer op te kunnen pakken 


  //1.beginnen met rond draaien
  //2. het uitlezen van de servo

 //test

}

void FirsttimePickup(){
// bij de eerste keer oppakken moet er ook rekening worden gehouden met de afstand van de parabol tot de kern van de arm



}

void switchState(){
// bepalen dat hij na 1 keer de LDiabolo heeft opgepakt switched naar de HDiabolo

}



void Begin(){
// eerst nadat hij de diabol heeft opgepakt zal hij naar pos 20 toe gaan en vanuit hier zijn spel strategie starten
StepperBase.step(calculateSteps(EndPos, zone[6]));


}

void setup() {
  Serial.begin(9600);
  StepperBase.setSpeed(60);
  StepperHead.setSpeed(60);








}

void loop() {


StepperBase.step(200);
delay(calculateSDelay(200, StepperBaseRPM));
StepperBase.step(-200);
delay(calculateSDelay(200, StepperBaseRPM));



}
