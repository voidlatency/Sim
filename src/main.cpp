#include <Arduino.h>
#include <servo.h>
#include <Stepper.h>

//rpm per motor
#define StepperHeadRPM 60
#define StepperEndRPM 60
#define StepperBaseRPM 40

// afstand tot de grond voor het bereken van de werkelijk hoogte omgedraait 
#define distancetoground 10



#define TrigPin 14  //sonics sensor pins defined at random, Need changing when testing!!
#define EchoPin 15

//example : BasicStepperDriver stepper(MOTOR_STEPS, DIR, STEP);


// de end stepper 
Stepper StepperEnd = Stepper(200, 100, 100, 900, 1100);
Stepper StepperBase = Stepper(200,8 ,9 ,10 ,11 );
Stepper StepperHead = Stepper(200,6 ,7 ,8 ,9 );

//

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

//laatste position is 0 aan het begin
int EndPos = zone[1];
int StartPos; //is de positie nadat hij de diabolo's heeft gevonden en hij start met het spel
int StartVlak;   // welke hij van de 4 belangerijke vakken als eerst heeft gekozen om het spel mee te beginnen
int Steps;
float SDelay;  //delay voor tussen de steppen
int STlocation; // memory slot van de stepdistancefunctie voor later gebruik 
int BasePos; // de basis waar hij op dit moment zich bevind 

//geven de posities van de diabolen aan
int PDiabolH;
int PDiabolL;


// geeft de state van die machine aan 
int StateDiabolL;
int StateDiabolH;
/////////
// 0 is dicht 1 is open
int GripperS = 0;
//head 0 is center 
int HeadS = 0;

//endS 0 is dicht
int EndS = 0;


//klok die op de achtergrond de tijd bijhoud
unsigned long Clock;


/////////sensor integers
//sensor array

float SensorReading[40];




int tijdverlopen; 

// sensor afstand 
float SonicDistanceS1;  
float SonicDistanceS2; 
float SonicDistanceS3; 


float TotalAverage; 

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
   StepperHead.step(calculateDistanceSteps(20));
  calculateSDelay( calculateDistanceSteps(20), StepperHeadRPM);
   HeadS = 1; 
 }


 if(HeadS == 1){
   StepperHead.step(-calculateDistanceSteps(20));
   calculateSDelay(-calculateDistanceSteps(20), StepperHeadRPM);
   //head is in middle position 
    HeadS = 0; 
 }
}



void SwitchEnd(){
 
  if(StateDiabolH == true){
    StepperEnd.step(342);
    EndS = 1;    // gripper is in low positition for high diabolo 
    if(EndS == 1 ){
      StepperEnd.step(-342);
    }
    else{
       EndS = 0; 
    }
  }

  else{
    StepperEnd.step(343);
    EndS = 0 ; 
    if(EndS == 0){
       StepperEnd.step(-343);
    }
    else{
      EndS = 1 ; 
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

// af 
void ReadSensorForward(){



  // de functie voor het bepaalen of hij ligt op staat 
  int duration;
// lees sonic sensor
StepperHead.step(-40);
for(int i = 0; i < 40; i++){
  //delay om in positie te komen kan nog verandert worden
  delay(10);

  // Clears the trigPin condition        //kan er meschien uit 
  digitalWrite(TrigPin, LOW);
  delayMicroseconds(2);
  // Sets de trigPin HIGH (ACTIVE) voor 10 microseconds
  digitalWrite(TrigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(TrigPin, LOW);
  duration = pulseIn(EchoPin, HIGH);
  SensorReading[i]  = duration * 0.034 / 2;   // rekent afstand in cm
  // 2 stappen vooruit weer 
  StepperHead.step(2);
  // je hebt nu een array
  StepperHead.step(-40);
  Serial.println("hij is weer terug in de center nadat hij heeft gescanned");
}
}


// nog niet af 
void PingSensors(){
// hier mot hij een pint uitstuuren en daaruit 3 outputs hebben en kijken wat de afstand is tot de sensor 
SonicDistanceS3 = abs(SonicDistanceS3 - distancetoground);
SonicDistanceS2 = abs(SonicDistanceS2 - distancetoground);
SonicDistanceS1 = abs(SonicDistanceS2 - distancetoground);
}


// nog niet af
int HighLowScan(){
// 1 is h 0 is l
//int Position;

// hier moet hij de strategie toepassen zoals op het whiteboard zie fotos op whatsapp


//return (Position);
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
//eerste functie hij gaat ronddeaaien dit is een verzameling van meerdere functies uiteindelijk
int unknown = 2; // hij heeft dus nog geen van beide gevonden
//de eerste diabol zoeken en bepaalen of hij licht of staat.

// het gemiddelde nodig om te berekenen of een van de 3 sensors iets ziet 
int averagesensor;
int loop;
int i;

for(i = 0 ; unknown == 2; i++){

PingSensors();


// berekent of een iets ziet door ze gewoon bij elkaar op te tellen wel gevoelig voor foute lezingen
averagesensor = SonicDistanceS1 + SonicDistanceS2 + SonicDistanceS3;
if (averagesensor > 0){
  unknown++;
  Serial.println("hij heeft de eerste Diabol gevonden ga nu bepaalen of hij staan of liggend is ");

  if(HighLowScan() == 1){
Serial.println("de 1e diabol is Hoog");
// hij moet nu de array overschrijven 
    for(loop = 0; loop < 5; loop++) {
      HdiabolD[loop] = CW[loop];

      // en nog onthouden dat de 1e diabol op deze positie ligt
      PDiabolH = zone[i];
    }
  } 
  if(HighLowScan() == 0){
Serial.println("de 1e diabol is Laag");
// hij moet nu de array voor de stappen overschrijven
for(loop = 0; loop < 5; loop++) {
      LdiabolD[loop] = CW[loop];

      // en nog onthouden dat de 1e diabol op deze positie ligt
      PDiabolL = zone[i];
    }
  }
}


Serial.println("hij heeft niks gevonden in de vak gaat naar de volgende ");
StepperBase.step(calculateSteps(zone[i], zone[i+1]));
}
/////////
Serial.println("hij gaat nu zoeken naar de 2e diabol");



for(i = 0 ; unknown == 1; i++){
PingSensors();
// berekent of een iets ziet door ze gewoon bij elkaar op te tellen wel gevoelig voor foute lezingen
averagesensor = SonicDistanceS1 + SonicDistanceS2 + SonicDistanceS3;
if (averagesensor > 0){
  unknown++;
  Serial.println("hij heeft de eerste Diabol gevonden ga nu bepaalen of hij staan of liggend is ");

  if(HighLowScan() == 1){
Serial.println("de 2e diabol is Hoog");
// hij moet nu de array overschrijven 
    for(loop = 0; loop < 5; loop++) {
      HdiabolD[loop] = CCW[loop];


           // en nog onthouden dat de 1e diabol op deze positie ligt
      PDiabolH = zone[i];
    }
  } 
  if(HighLowScan() == 0){
Serial.println("de 2e diabol is Laag");
// hij moet nu de array voor de stappen overschrijven
for(loop = 0; loop < 5; loop++) {
      LdiabolD[loop] = CCW[loop];

           // en nog onthouden dat de 1e diabol op deze positie ligt
      PDiabolL = zone[i];
    }
  }
}
Serial.println("hij heeft niks gevonden in de vak gaat naar de volgende ");
StepperBase.step(calculateSteps(zone[i], zone[i+1]));
}






}








void SwitchState(){
  // hij switched van gamestate

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


StepperBase.step(calculateSteps(EndPos, zone[19]));
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


 StateDiabolH = 1;
 HeadS = 1;


  //example stepper.begin(RPM, MICROSTEPS); Start de steppers
  StepperBase.setSpeed(StepperBaseRPM);
  StepperHead.setSpeed(StepperHeadRPM);
  StepperEnd.setSpeed(StepperEndRPM);
  //pinmode aangeven

  pinMode(TrigPin, OUTPUT);
  pinMode(EchoPin, INPUT);


// servo aan pin 10 koppelen
  Gripper.attach(80);
  Begin();
}





void loop() {

// voor testredenen doen we statediabol h aanhoudoen en de headS is 1


//SwitchEnd();
//SwitchHead();
StepperBase.step(200);
delay(2000);







}
