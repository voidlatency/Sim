#include <Arduino.h>
#include <Stepper.h>

//aantal stppen per rotatie
Stepper StepperBase = Stepper(200, 8, 9, 10, 11);
Stepper StepperHead = Stepper (200, 1, 2, 3, 4);
Stepper StepperEnd = Stepper(200, 12, 13, 14, 15);

//zet zones op en geeft deze waardes op opgeroepen te worden zie foto verdeling voor veder uitleg
int zone[24] = {191, 0, 8, 16, 25, 33, 41, 50, 58, 66, 75, 83, 91, 100, 108, 116, 125, 133, 142, 150, 158, 166, 175, 183};


//laatste position is 0 aan het begin
int EndPos = zone[1];
int StartPos; //is de positie nadat hij de diabolo's heeft gevonden en hij start met het spel
int StartVlak;   // welke hij van de 4 belangerijke vakken als eerst heeft gekozen om het spel mee te beginnen
int Steps;
int StepD;

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






int calculateSDelay(int SDelay){
SDelay = Steps * 1;   // moet nog een compleete formule voor komen - uitwerken


return(SDelay);
}


void ClosestTarget(){
  //eerst moet hij kijken waar hij zit de belangerijkste zones zijn:
  // 14 = zone[8]
  // 18 = zone[18]
  // 20 = zone[6]
  // 16 = zone[20]
  //bepalen wat het dichtste bij is en dat als start punt bepaalen voor de rest van het traject


// alle stappen berekend
int closestSteps[4];

closestSteps[0] = calculateSteps(StartPos, zone[8]);
closestSteps[1] = calculateSteps(StartPos, zone[18]);
closestSteps[2] = calculateSteps(StartPos, zone[6]);
closestSteps[3] = calculateSteps(StartPos, zone[20]);

//vind het kleinste getal

int closest, num ,i;


for (i = 0; i < num; i++){
      scanf("%d", &closestSteps[i]);
}
//Consider first element as smallest
closest = closestSteps[0];
 
for (i = 0; i < num; i++) {
  if (closestSteps[i] < closest) {
      closest = closestSteps[i];
    }
  }
 
   // Print out the Result
   printf("\nkleinste hoeveelheid stappen naar hoogwaardige vak%d", closest);



// nog een check om het Startvalk te defineeren 


}

void DiabolotoCenter(){
  //beweeg de diabol naar de center to met de 2e Stepper
  //hierbij moeten we de afstand naar tanden gaan omzetten aan de hand met het tandwiel dus de positie waarin hij staat 
  //- de positie waarhij naar toe moet 
  





}

void PickupDiabol(int position){
// we gaan er van uit dat hij al boven de diabol zit heirbij moeten wij dus hen naar beneden brengen ne dan weer 
//naar boven en rekeninghouden met de tijd voor de servo om het op te pakken ook moet je rekening houden met of je te maken heb met liggend of staand
if (StateDiabolH == 1){
//staande pos minder naar beneden


}
if (StateDiabolL == 1){
// liggende pos meer naar beneden

}
else{
  //error
  Serial.println("Critical error HL state Unknown");
  delay(200000000);
}
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

void DecideState(){



}

void setup() {
  Serial.begin(9600);
  StepperBase.setSpeed(60);
  StepperHead.setSpeed(60);








}

void loop() {
  //






  
}
