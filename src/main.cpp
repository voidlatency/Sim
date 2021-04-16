#include <Arduino.h>
#include <servo.h>
#include <Stepper.h>

//rpm per motor
#define StepperHeadRPM 60
#define StepperEndRPM 60
#define StepperBaseRPM 40

// afstand tot de grond voor het bereken van de werkelijk hoogte omgedraait 
#define distancetoground 10


// pins voor de sensoren
#define TrigPinS1 43  
#define TrigPinS2 47 
#define TrigPinS3 51 

#define EchoPinS1 45
#define EchoPinS2 49
#define EchoPinS3 53
//example : BasicStepperDriver stepper(MOTOR_STEPS, DIR, STEP);



#define EndDelay 1000
#define HeadDelay 1000
#define GripperDelay 1000

// de end stepper
Stepper StepperEnd = Stepper(200, 2, 3, 4, 5);
Stepper StepperBase = Stepper(200,8 ,9 ,10 ,11 );
Stepper StepperHead = Stepper(200, 25, 27 , 29, 31);
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

// 0 is dicht 1 is open
int GripperS = 0;
//head 0 is center 
int HeadS = 0;

//endS 0 is dicht
int EndS = 0;


//klok die op de achtergrond de tijd bijhoud
unsigned long Clock;

// sensor array
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
int calculateSteps(int Target){
//berekening stappen
int End1 = BasePos;
int End2 = BasePos + 200;      // 2e variant van dezelfde position
int StepOption1 = Target - End1;
int StepOption2 = Target - End2;
if (abs(StepOption1) <= abs(StepOption2)){
  Steps = StepOption1;
}
else{
  Steps = StepOption2;
}
// uiteindelijke steps + de tagret zetten als de nieuwe pos 
BasePos = Target;
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



// hij moet ook ng kijken in wat voor stat hij zit 
// hiervoor zijn er dus 3 eigelijk helemaal open dicht l en dicht h 




// dit moet er ook nog ingeprogrameerd worden 
if (GripperS == 0){ // hij is dicht dus nu open
Gripper.write(180);
GripperS = 1;
}
if (GripperS == 1){ // is open dus nu dicht 
Gripper.write(90);
GripperS = 0;
}
else{
  Serial.println("error");
}
}



////////////////////////////////////////////////////////////////////////////////////////////
// sensor funties

// nog niet af 
int HighLowScan(){
  // de functie voor het bepaalen of hij ligt op staat 
  int duration;
  // 1 is h 0 is l
  int Position = 1;


// lees sonic sensor
  StepperHead.step(-40);
  for(int i = 0; i < 40; i++){
  //delay om in positie te komen kan nog verandert worden
  delay(10);

  // Clears the trigPin condition        //kan er meschien uit 
  digitalWrite(TrigPinS2, LOW);
  delayMicroseconds(2);
  // Sets de trigPin HIGH (ACTIVE) voor 10 microseconds
  digitalWrite(TrigPinS2, HIGH);
  delayMicroseconds(10);
  digitalWrite(TrigPinS2, LOW);
  duration = pulseIn(EchoPinS2, HIGH);
  SensorReading[i]  = duration * 0.034 / 2;   // rekent afstand in cm
  // 2 stappen vooruit weer 
  StepperHead.step(2);
  // je hebt nu een array
  StepperHead.step(-40);
 
 
  Serial.println(SensorReading[i]);
  Serial.println("hij is weer terug in de center nadat hij heeft gescanned");
}
// hij moet nu een conclusie gaan trekken uit de sensor array 
// eerst meot hij de sensor array aflopen en kijken wat de eerste is die 
// de grens overschreid en tegelijk hierna moet hij de daibol oppakken en naar het midden zetten als dat niet al is 

/*for(int i = 0; i<40; i++){
if (SensorReading[i] > 0){
   
}
}
voor later meot hier de code staan voor het oppakken en dan in het midden setten van de diabol en het 
berekenen waar de daibol nou precies ligt in opzichte van het middelpunt van de circel

*vraag boyd voor vedere uitleg hierover*
*/



// hier moet hij een conclusie gaan trekken nadat het gemiddelde is bertekend
int total;
//eerst berekent hij het gemmidelde uit met wat hij heeft 
for(int i = 0; i < 40; i++){
total = SensorReading[i] + total;
}
int average = total/40;
// hij is nu klaar met het gemmidelde te bereken
if (average <= 7.9){
// hij ligt 
Position = 0;
}
if (average >= 8){
// hij staat recht op
Position = 1;
}
else{
  Serial.println("error diabol h/l onbekent");
}
StepperBase.step(-40);
return(Position);
}




void PingSensors(){
int duration;
// sensor 1 lezing 
// Clears the trigPin condition        //kan er meschien uit 
  digitalWrite(TrigPinS1, LOW);
  delayMicroseconds(2);
  // Sets de trigPin HIGH (ACTIVE) voor 10 microseconds
  digitalWrite(TrigPinS1, HIGH);
  delayMicroseconds(10);
  digitalWrite(TrigPinS1, LOW);
  duration = pulseIn(EchoPinS1, HIGH);
  SonicDistanceS1 = duration * 0.034 / 2;   // rekent afstand in cm
  // 2 stappen vooruit weer 




// sensor 2 lezing
// Clears the trigPin condition        //kan er meschien uit 
  digitalWrite(TrigPinS2, LOW);
  delayMicroseconds(2);
  // Sets de trigPin HIGH (ACTIVE) voor 10 microseconds
  digitalWrite(TrigPinS2, HIGH);
  delayMicroseconds(10);
  digitalWrite(TrigPinS2, LOW);
  duration = pulseIn(EchoPinS2, HIGH);
  SonicDistanceS2  = duration * 0.034 / 2;   // rekent afstand in cm
  // 2 stappen vooruit weer 


// sensor 3 lezing
// Clears the trigPin condition        //kan er meschien uit 
  digitalWrite(TrigPinS3, LOW);
  delayMicroseconds(2);
  // Sets de trigPin HIGH (ACTIVE) voor 10 microseconds
  digitalWrite(TrigPinS3, HIGH);
  delayMicroseconds(10);
  digitalWrite(TrigPinS3, LOW);
  duration = pulseIn(EchoPinS3, HIGH);
  SonicDistanceS3 = duration * 0.034 / 2;   // rekent afstand in cm
  // 2 stappen vooruit weer 

// hier mot hij een pint uitstuuren en daaruit 3 outputs hebben en kijken wat de afstand is tot de sensor 
SonicDistanceS3 = abs(SonicDistanceS3 - distancetoground);
SonicDistanceS2 = abs(SonicDistanceS2 - distancetoground);
SonicDistanceS1 = abs(SonicDistanceS2 - distancetoground);

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


void gatherpoint(){
// hij moet naar voren toe en weer te



// gaat dus eerst weer naar vooren toe 
SwitchHead();
delay(1000);
// de afstad naar beneden word al bepaald in de functie van switchend 
SwitchEnd();
delay(1000);
// daarna doet hij de servo draaien en doet hij de gripper dicht vergeet niet in het begin om de gripper altijd open te hebben
SwitchGripper();
delay(1000);
SwitchGripper();

//daarna alles weer in spiegelbeeld om weer terug in zijn center te komen 
SwitchEnd();
SwitchHead();
delay(2000);
}

void Begin(){
  SwitchGripper();
  StepperBase.step(40);
//eerste functie hij gaat ronddeaaien dit is een verzameling van meerdere functies uiteindelijk
int unknown = 2; // hij heeft dus nog geen van beide gevonden
//de eerste diabol zoeken en bepaalen of hij licht of staat.

// het gemiddelde nodig om te berekenen of een van de 3 sensors iets ziet 
int averagesensor;
int loop;
int i, S;


// i is hierbij de hoeveelheid vakjes 
for(i = 0 ; unknown == 2; i++){

PingSensors();


// berekent of een iets ziet door ze gewoon bij elkaar op te tellen wel gevoelig voor foute lezingen
averagesensor = SonicDistanceS1 + SonicDistanceS2 + SonicDistanceS3;
if (averagesensor > 0){
  unknown++;
  Serial.println("hij heeft de eerste Diabol gevonden ga nu bepaalen of hij staan of liggend is ");
S = HighLowScan();
if(S == 1){
Serial.println("de 1e diabol is Hoog");
// hij moet nu de array overschrijven 
    for(loop = 0; loop < 5; loop++) {
      HdiabolD[loop] = CW[loop];

      // en nog onthouden dat de 1e diabol op deze positie ligt
      PDiabolH = zone[i];
    }
  } 
  if(S == 0){
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
StepperBase.step(calculateSteps(zone[i+1]));
}
/////////
Serial.println("hij gaat nu zoeken naar de 2e diabol");



for(; unknown == 1; i++){
PingSensors();

// berekent of een iets ziet door ze gewoon bij elkaar op te tellen wel gevoelig voor foute lezingen
averagesensor = SonicDistanceS1 + SonicDistanceS2 + SonicDistanceS3;
if (averagesensor > 0){
  // hij ziet iets en gaat nu de scan functie uitvoeren
  S = HighLowScan();
  unknown++;
  Serial.println("hij heeft de eerste Diabol gevonden ga nu bepaalen of hij staan of liggend is ");

  if(S == 1){
Serial.println("de 2e diabol is Hoog");
// hij moet nu de array overschrijven 
    for(loop = 0; loop < 5; loop++) {
      HdiabolD[loop] = CCW[loop];


           // en nog onthouden dat de 1e diabol op deze positie ligt
      PDiabolH = zone[i];
      StateDiabolH = 1;     // hij begint nu dus bij h
      BasePos = zone[i];
    }
  } 
  if(S == 0){
Serial.println("de 2e diabol is Laag");
// hij moet nu de array voor de stappen overschrijven
for(loop = 0; loop < 5; loop++) {
      LdiabolD[loop] = CCW[loop];

           // en nog onthouden dat de 1e diabol op deze positie ligt
      PDiabolL = zone[i];
      StateDiabolL = 1;
      BasePos = zone[i];
    }
  }
}
Serial.println("hij heeft niks gevonden in de vak gaat naar de volgende ");
StepperBase.step(calculateSteps(zone[i+1]));
}
Serial.println("hij heeft beide diabolen gevonden en geindiceerd welke kant hij op moet draaien");

StepperBase.step(-40);
// is dit het eind deel van begin ?
// posities van de diabolen zijn al bekent nu en of ze hoog en laag zijn
// de begin game state is ook al bepaald 
// dit is alles wat nodig is om het spel te beginnen 






}










void SwitchState(){
  // hij switched van gamestate dit moet elke keer ndadat hij de gameS heeft afgelopen

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





// nog lang niet af 
void MaingameS(){

// kies eerst tussen de gamestate welke state zijn wij nu in ?
// vooraf conclusie om het makkelijker te maken voor kopieeren en plakken
// hij is nu bezig met het laage diabol
if (StateDiabolL == 1){
// hij moet eerst naar de l diabol toe 
StepperBase.step(calculateSteps(PDiabolL));
calculateSDelay(calculateSteps(PDiabolL),StepperBaseRPM);
// hij hangt nu boven de liggende diabol
Serial.println("ik hang nu boven de liggende diabol ik ga nu beginnen met mijn game strategie");


// hij gaat nu naar voren toe 
SwitchHead();
delay(HeadDelay);
// hij doet nu zijn gripper open 
SwitchGripper();
delay(GripperDelay);
// hij gaat nu naar beneden
SwitchEnd();
delay(EndDelay);
// hij gaat nu de diabol pakken
SwitchGripper();
delay(GripperDelay);
// hij moet nu weer naar boven en naar zijn hart toe 
SwitchEnd();
SwitchHead();
delay(HeadDelay);


// vanaf hier heeft hij de daibol in zijn hand
Serial.println("ik heb L diabol in mijn handen en ben weer terug in mijn hart");

int i;

for(i = 0; i < 4; i++){
  // hij gaat nu naar de volgende toe 
  StepperHead.step(calculateSteps(LdiabolD[i]));
  calculateSDelay(calculateSteps(LdiabolD[i]), StepperBaseRPM);  // de delay voor naar positie komen 
  // hij moet nu weer zijn standaart programma draaien door naar voren te gaan open te doen en snel neer ze zetten voor een paar seconde 
  Serial.println("ik ga nu beginnen met een punt te verzamelen voor de liggende diabol");
  gatherpoint();
  Serial.println("ik heb net een punt verzamelt veder naar de volgende !"); 
}
if (i == 4){
// L diabol moet nu naar zijn rust positie worden gebracht. dit is in de array de 5e positie 
  StepperHead.step(calculateSteps(LdiabolD[i]));
  calculateSDelay(calculateSteps(LdiabolD[i]), StepperBaseRPM);  // de delay voor naar positie komen 

  SwitchHead();
  delay(HeadDelay);
  SwitchEnd();
  delay(EndDelay);
  SwitchGripper();
  SwitchEnd();
  delay(EndDelay);
  // hier zorgt hij ervoor dat hij terug naar de kern kan en daarabij ook weer zijn gripper reset 
  SwitchGripper();
  SwitchHead();
  delay(HeadDelay);
// hij zit nu weer in zijn center na de strategie voor de l diabol te hebben uitgevoert

// hij onthoud waar hij de daibol heeft neergezet. eigelijk alleen maar nodig voor de eerste keer.
PDiabolL = LdiabolD[i];


Serial.println("ik ben klaar met puntjes verzamelen yay ! nu op naar de H diabol!");


// en hier schakelt hij weer om naar de volgende state. zodat hij weet wat hij moet doen
SwitchState();


}
// hij zit in de H gamestate
if (StateDiabolH == 1){
// hij moet eerst naar de H diabol toe 
StepperBase.step(calculateSteps(PDiabolL));
calculateSDelay(calculateSteps(PDiabolL),StepperBaseRPM);
// hij hangt nu boven de liggende diabol
Serial.println("ik hang nu boven de staande  diabol ik ga nu beginnen met mijn game strategie");


// hij gaat nu naar voren toe 
SwitchHead();
delay(HeadDelay);
// hij doet nu zijn gripper open 
SwitchGripper();
delay(GripperDelay);
// hij gaat nu naar beneden
SwitchEnd();
delay(EndDelay);
// hij gaat nu de diabol pakken
SwitchGripper();
delay(GripperDelay);
// hij moet nu weer naar boven en naar zijn hart toe 
SwitchEnd();
SwitchHead();
delay(HeadDelay);


// vanaf hier heeft hij de daibol in zijn hand
Serial.println("ik heb H diabol in mijn handen en ben weer terug in mijn hart");

int i;

for(i = 0; i < 4; i++){
  // hij gaat nu naar de volgende toe 
  StepperHead.step(calculateSteps(HdiabolD[i]));
  calculateSDelay(calculateSteps(HdiabolD[i]), StepperBaseRPM);  // de delay voor naar positie komen 
  // hij moet nu weer zijn standaart programma draaien door naar voren te gaan open te doen en snel neer ze zetten voor een paar seconde 
  Serial.println("ik ga nu beginnen met een punt te verzamelen voor de liggende diabol");
  gatherpoint();
  Serial.println("ik heb net een punt verzamelt veder naar de volgende !"); 
}
if (i == 4){
// L diabol moet nu naar zijn rust positie worden gebracht. dit is in de array de 5e positie 
  StepperHead.step(calculateSteps(HdiabolD[i]));
  calculateSDelay(calculateSteps(HdiabolD[i]), StepperBaseRPM);  // de delay voor naar positie komen 

  SwitchHead();
  delay(HeadDelay);
  SwitchEnd();
  delay(EndDelay);
  SwitchGripper();
  SwitchEnd();
  delay(EndDelay);
  // hier zorgt hij ervoor dat hij terug naar de kern kan en daarabij ook weer zijn gripper reset 
  SwitchGripper();
  SwitchHead();
  delay(HeadDelay);
// hij zit nu weer in zijn center na de strategie voor de l diabol te hebben uitgevoert

// hij onthoud waar hij de daibol heeft neergezet. eigelijk alleen maar nodig voor de eerste keer.
PDiabolH = HdiabolD[i];


Serial.println("ik ben klaar met puntjes verzamelen yay ! nu op naar de L diabol!");


// en hier schakelt hij weer om naar de volgende state. zodat hij weet wat hij moet doen
SwitchState();
}
}
}
}

























//////////////////////////////////////////////////////////////////////////////////////////////////////////
//start main code 
void setup(){
  //start serial met Baudrate
  Serial.begin(9600);


 StateDiabolH = 1;
 HeadS = 1;




  //example stepper.begin(RPM, MICROSTEPS); Start de steppers
  StepperBase.setSpeed(StepperBaseRPM);
  StepperHead.setSpeed(StepperHeadRPM);
  StepperEnd.setSpeed(StepperEndRPM);
  //pinmode aangeven

  pinMode(TrigPinS1, OUTPUT);
  pinMode(EchoPinS1, INPUT);
  pinMode(TrigPinS2, OUTPUT);
  pinMode(EchoPinS2, INPUT);
  pinMode(TrigPinS3, OUTPUT);
  pinMode(EchoPinS3, INPUT);

// servo aan pin 13 koppelen
  Gripper.attach(13);


  // de begin functie een van de grootste functies 
  Begin();
}





void loop() {



MaingameS();
// en hij moet ook nog na 5 min kijken of deze voorbij zijn maar deze funcie is een van de velen die gecshrapt is voor nu .
Serial.println("ik heb 4 punten verzameld naar de volgende toe ");
delay(2000);


}
// het is justin zijn schult als dit niet werkt