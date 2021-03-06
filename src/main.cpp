#include <Arduino.h>
#include <servo.h>
#include <Stepper.h>

//rpm per motor
#define StepperHeadRPM 60
#define StepperBaseRPM 5

// afstand tot de grond voor het bereken van de werkelijk hoogte omgedraait 
#define distancetoground 7

// pins voor de sensoren
#define TrigPinS1 41  
#define TrigPinS2 36
#define TrigPinS3 39

#define EchoPinS1 40
#define EchoPinS2 37
#define EchoPinS3 38

//example : BasicStepperDriver stepper(MOTOR_STEPS, DIR, STEP);
#define EndDelay 1000
#define HeadDelay 1000
#define GripperDelay 1000

// de grens als hij grooter is dan dit getal is hij hoog en als ij alger is dan dit getal dan hebben wij te maken met een liggende diabol of andersom dit moet nog getest worden 
#define BoundaryDiabolAverage 10     
#define Scanofset 3

//hoever de grijper naar beneden of naar boven moet dus stepper end 
#define EndStepsHigh 20
#define EndStepsLow 30

// de afstand die hij neemt voor het herkennen of er een diabol is hier gaat het steeds fout. 
#define distancetodabol 8.5 


// de end stepper
Stepper StepperBase = Stepper(200, 45 ,47 ,49 ,51 );
Stepper StepperHead = Stepper(200, 25, 27 , 29, 31);

//servo
Servo Gripper;
Servo RopeServo;

//pins voor de servo's
#define Gripperpin 3
#define RopeServoPin 4

//zet zones op en geeft deze waardes op opgeroepen te worden zie foto verdeling voor veder uitleg
int zone[24] = {0, 8, 16, 25, 33, 41, 50, 58, 66, 75, 83, 91, 100, 108, 116, 125, 133, 142, 150, 158, 166, 175, 183, 191};
// belangerijke zones
// -1 is dus het begin veld waar hij moet beginnen 
// 20 punten zone[5]
// 14 punten zone[7]
// 16 punten zone[19]
// 18 punten zone[17]


// met de klok mee of er tegen in
int CW[5] = {41, 58, 142, 158, 191} ;  //clockwise
int CCW[5] = { 142, 158, 41, 58, 83} ; //counterclockwise
// de laaste is rust positie 
// 0 is rust positie van de diabolo

int HdiabolD[5];     // de richting van H diabol
int LdiabolD[5];     // de richting van L diabol
// 1 is met klok mee 0 is tegen de klok in 

float SDelay;  //delay voor tussen de steppen
int BasePos = zone[1];; // de basis waar hij op dit moment zich bevind 

//geven de posities van de diabolen aan
int PDiabolH;
int PDiabolL;

// geeft de state van die machine aan 1 is H diabol 0 is L diabol
int Gamestate = 0; // de 0 moet later weg 

// 0 is dicht 1 is open
int GripperS = 0;
//head 0 is center 
int HeadS = 0;

//endS 0 is dicht
int EndS = 0;

// sensor array
float SensorReading[40];

int tijdverlopen; 

// sensor afstand en gemiddelde 
float SonicDistanceS1 = 10;  
float SonicDistanceS2 = 10; 
float SonicDistanceS3 = 10; 

//////////////////////////////////////////////////////////////////////////////////////////////////////////
// calculation functions
int calculateSteps(int Target){


//berekening stappen
int Steps;
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
SDelay = abs(Step/(StepperRPM * 0.006));

// voor veiligheid een extra delay als dit nodig blijkt te zijn. 
// delay(5);

return(SDelay);
}




/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//position functions


// hier gaat ook iets fout
void SwitchHead(){
 if(HeadS == 0){


 //head is in base position 
   StepperHead.step(80);
  calculateSDelay(80, StepperHeadRPM);
   HeadS = 1; 
 }


 else{
   StepperHead.step(-80);
   calculateSDelay(-80, StepperHeadRPM);
   //head is in middle position 
    HeadS = 0; 
 }

}



void SwitchEnd(){
 // voor dit project is dit meschien niet nodig kijk of hij wel echt naar boven of naar beneden moet 

  if(Gamestate == 1){
    if(EndS == 0 ){
      // hij is dus nu hoog en moet naar beneden
     RopeServo.write(105);      // minder ver naar beneden voor de hooge diabol
      EndS = 1;
      Serial.println("mijn kopstuk gaat nu naar beneden toe voor de hooge diabol");
    }
    else{
      RopeServo.write(180);
       EndS = 0; 
       Serial.println("mijn kopstuk gaat nu naar boven toe voor de hooge diabol");
    }
  }

  else{
   //de gripper moet veder naar beneden omdat de gamestate l is  
    if(EndS == 0){
      // hij is dus nu hoog en moet naar beneden
     RopeServo.write(50);
      EndS = 1;
      Serial.println("mijn kopstuk gaat nu naar beneden toe voor de lage diabol");
    }
    else{
      RopeServo.write(180);
      EndS = 0 ; 
      Serial.println("mijn kopstuk gaat nu naar boven toe voor de lage diabol");
    }
  }
  }

void SwitchGripper(){
// 0 is dicht 1 is open
// de gripper heeft maar 2 standen open en dicht bij dicht kan hij de diabol in beide zijn posties op pakken
// hier hoeft dus niet meer voor geprogrameerd te worden. 
if (GripperS == 0){ // hij is dicht dus nu open
Gripper.write(180);
Serial.println("mijn gripper is nu open");
GripperS = 1;
}

else{ // is open dus nu dicht 
Gripper.write(75);
Serial.println("mijn gripper is nu dicht");
GripperS = 0;
}
}



////////////////////////////////////////////////////////////////////////////////////////////
// sensor funties

// nog niet af 
int HighLowScan(){
  // de functie voor het bepaalen of hij ligt op staat 
  int duration;
  // 1 is h 0 is l
  int Position;


// hij zit nog in het midden op dit moment en moet eerst dus nog 40 stappe nnaarachteren om echt het hele vak af te gaan
  StepperHead.step(-80);

  for(int i = 0; i < 40; i++){
  //delay om in positie te komen kan nog verandert worden
  delay(5);

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
  StepperHead.step(4);

 // test voor laag en hoog door te printen wat de average is voor een paar keer totdat met trial en error de average vast staan en kijk wat de marigin is als het goed is is het elke keer ongeveer dezelfde waarde 
  Serial.println(SensorReading[i]);

}

 StepperHead.step(-80);
  Serial.println("hij is weer terug in de center nadat hij heeft gescanned");

// hij moet nu een conclusie gaan trekken uit de sensor array 
// eerst meot hij de sensor array aflopen en kijken wat de eerste is die 
// de grens overschreid en tegelijk hierna moet hij de daibol oppakken en naar het midden zetten als dat niet al is 



// is geschrapt voor sim2 wel heel nodig voor sim 3 !

/*for(int i = 0; i<40; i++){
if (SensorReading[i] > 0){

}
}
voor later meot hier de code staan voor het oppakken en dan in het midden setten van de diabol en het 
berekenen waar de daibol nou precies ligt in opzichte van het middelpunt van de circel

*vraag boyd voor vedere uitleg hierover*




veder is het nog efficenter te maken door de average te gaan berekenen vanaf de eerste array waarde die kleiner is dan 10 dit zorgt ervoor dat je een nettere average krijgt zonder heel veel nullen er in
ook zorgt dit er voor dat je meer ziet wat er gebeurt. dit stuk moet dus voor sim 3 helemaal anders geprogrameerd worden. vergeet daarbij ook niet aan dat je moet onthouden wanneer de eerste avverag er is een van de beste manier is om een nieuwe 
array aan te maken voor de waardes boven 10 en hieruit veder te rekenen dus alles hierboven en onder veranderen tot een functie die een neiuwe array aanmaakt en hier uit de average rekent en onthoud bij welke step de eerste waarde kleiner is dan 10




veel suc6 !  hehe
*/




// hier moet hij een conclusie gaan trekken nadat het gemiddelde is bertekend
float total = 0;
for(int i = 0; i < 40; i++){
total = SensorReading[i] + total;
}

float average = total/40;

Serial.println(average);

if (average <= BoundaryDiabolAverage){
// hij ligt 
Position = 0;
}
if (average > BoundaryDiabolAverage){
// hij staat recht op
Position = 1;
}
else{
  Serial.println("error diabol h/l onbekent");
}
return(Position); // hier geeft hij dus door of hij H of L positie is 
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

delay(50);

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

delay(50);

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

delay(50);

Serial.println("de uiteindelijk afstand tot de grond is op volgorde 1,2,3 is:");
Serial.println(SonicDistanceS1);
Serial.println(SonicDistanceS2);
Serial.println(SonicDistanceS3);
 // voobeeld hij ziet 6 doet hij dus 10 - 5 is 4 betekend dat er iets op een hoogte van 4 staat weet niet of dit een heel handige manier is om het te berekenen
}





///////////////////////////////////////////////////////////////////////////////////////////
//safety functions

// bijna alle safety functies zijn geschrapt omdat er geen tijd meer was






























////////////////////////////////////////////////////////////////////////////////////////////
//game functions

// hij moet de diabol neerzetten voor een paar seconde en dan weer oppakken en naar zijn rust positie gaan dit kan ook getest worden los 


// hij moet nu wisselen van state 
void SwitchState(){
if (Gamestate == 1){
  Gamestate = 0;
}
else{
  Gamestate = 1;
}
}


void gatherpoint(){
// gaat dus eerst weer naar voren toe 
SwitchHead();
delay(1000);

// de afstad naar beneden word al bepaald in de functie van switchend 
SwitchEnd();
delay(1000);

// nu doet hij de servo open en na 2 seconde doet hij hem weer dicht om de diabol weer op te pakken 
SwitchGripper();
delay(1000);
SwitchGripper();

// nu moet hij weer omhoog en naar zijn hart toe it doet hij tegelijk en heeft hij 2 seconde voor 
SwitchEnd();
SwitchHead();
delay(2000);
}




void Begin(){

// pas op omdat de safety feature weg is betekend dat de machine onverwachte dingen kan doen wanneer er een diabol op vak 0 ligt dit is op te lossen met een functie later maar let hier bij op
  StepperHead.step(40);
  // onthoud hij denk dat hij nog bij de center is de heads state is 0 maar hij zit wel in midden
  // kijken of dit wel handig is want het midden is al switch maar dit kan voor nu worden genegeerd kijk of dit bij sim 3 van toepassing is




Serial.println("ik ze nu mijn grijper open ");
  Gripper.write(180);
  GripperS = 1;

 


delay(1000);

//eerste functie hij gaat ronddeaaien dit is een verzameling van meerdere functies uiteindelijk
int unknown = 0; // hij heeft dus nog geen van beide gevonden
//de eerste diabol zoeken en bepaalen of hij licht of staat.

// het gemiddelde nodig om te berekenen of een van de 3 sensors iets ziet 
int loop;
int i, S;


// i is hierbij de hoeveelheid vakjes 
for(i = 0 ; unknown == 0; i++){

  //eerst draait hij naar het volgende vak
  Serial.println("hij heeft niks gevonden in de vak gaat naar de volgende ");
  StepperBase.step(calculateSteps(zone[i+1] - Scanofset));
  delay(1000);

// nadat hij naar zijn vak heeft gedraait scant hij
  PingSensors();
  delay(100);


// om vals te spelen kan hier voor i een vak worden ingevult en zo de diabol worden ge ahrd coded dus bijvoor beeld i ==5 . 
if (SonicDistanceS1 < distancetodabol || SonicDistanceS2 < distancetodabol|| SonicDistanceS3 < distancetodabol){
  unknown++;
  Serial.println("hij heeft de eerste Diabol gevonden ga nu bepaalen of hij staan of liggend is ");


  // terug naar de kern toe 
  S = HighLowScan();
  // veder om vals t espelen moet je hier aangeven of in dit vlak de diabol hig hof low is. 
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


}

// bij sim 3 moet hier ook nog in gezet worden dat hi jde 1e diabol in het midden legt of maak het nog complexer dat naast h en laag en de positie ook onthouden word wat de afstand is dit is niet heel moeilijk als je nog een variable aanmaakt
// hier mogen jullie lekker naar kijken de volgende keer!
Serial.println("ik gaat nu zoeken naar de 2e diabol");




// hij gaat veder met de stappen die hij al had gedaan
for(; unknown == 1; i++){

// hij gaat eerst naar het volgende vak toe dit voorkomt ook dat hij 2x een diabol scant 
Serial.println("ik heb niks gevonden in het vak gaat naar de volgende ");

StepperBase.step(calculateSteps(zone[i+1] - Scanofset));
delay(2000);

PingSensors();



if (SonicDistanceS1 < distancetodabol || SonicDistanceS2 < distancetodabol || SonicDistanceS3 < distancetodabol){
  // hij ziet iets en gaat nu de scan functie uitvoeren
  S = HighLowScan();
  unknown++;
  Serial.println("ik heb de tweede Diabol gevonden ga nu bepaalen of hij staan of liggend is ");




  if(S == 1){
    Serial.println("de 2e diabol is Hoog");
    // hij moet nu de array overschrijven 
    for(loop = 0; loop < 5; loop++) {
      HdiabolD[loop] = CCW[loop];
           // en nog onthouden dat de 1e diabol op deze positie ligt
      PDiabolH = zone[i];
      Gamestate = 1;     
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
      Gamestate = 0;
      BasePos = zone[i];
    }
  }
}
}


Serial.println("hij heeft beide diabolen gevonden en geindiceerd welke kant hij op moet draaien");



StepperHead.step(-40);
// is dit het eind deel van begin ?
// posities van de diabolen zijn al bekent nu en of ze hoog en laag zijn
// de begin game state is ook al bepaald 
// dit is alles wat nodig is om het spel te beginnen 

}



// nog lang niet af 
void MaingameS(){


int i = 0;


// kies eerst tussen de gamestate welke state zijn wij nu in ?
// vooraf conclusie om het makkelijker te maken voor kopieeren en plakken
// hij is nu bezig met het laage diabol
if (Gamestate == 0){
    // hij moet eerst naar de l diabol toe 
    StepperBase.step(calculateSteps(PDiabolL));
    delay(2000);
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

  

    for(i = 0; i < 4; i++){
      // hij gaat nu naar de volgende toe 
      StepperHead.step(calculateSteps(LdiabolD[i]));
      delay(2000);  // de delay voor naar positie komen 
      // hij moet nu weer zijn standaart programma draaien door naar voren te gaan open te doen en snel neer ze zetten voor een paar seconde 
      Serial.println("ik ga nu beginnen met een punt te verzamelen voor de liggende diabol");
      gatherpoint();
      Serial.println("ik heb net een punt verzamelt veder naar de volgende !"); 
    }
    if (i == 4){
    // L diabol moet nu naar zijn rust positie worden gebracht. dit is in de array de 5e positie 
      StepperHead.step(calculateSteps(LdiabolD[i]));
      delay(2000);  // de delay voor naar positie komen 

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
}
// hij zit in de H gamestate
if (Gamestate == 1){
    // hij moet eerst naar de H diabol toe 
    StepperBase.step(calculateSteps(PDiabolH));
    delay(2000);  // de delay voor naar positie komen 
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
    delay(2000);
    SwitchHead();
    delay(HeadDelay);


    // vanaf hier heeft hij de daibol in zijn hand
    Serial.println("ik heb H diabol in mijn handen en ben weer terug in mijn hart");

    int i;

    for(i = 0; i < 4; i++){
      // hij gaat nu naar de volgende toe 
      StepperHead.step(calculateSteps(HdiabolD[i]));
      delay(2000); // de delay voor naar positie komen 
      // hij moet nu weer zijn standaart programma draaien door naar voren te gaan open te doen en snel neer ze zetten voor een paar seconde 
      Serial.println("ik ga nu beginnen met een punt te verzamelen voor de liggende diabol");
      gatherpoint();
      Serial.println("ik heb net een punt verzamelt veder naar de volgende !"); 
    }
    if (i == 4){
    // L diabol moet nu naar zijn rust positie worden gebracht. dit is in de array de 5e positie 
      StepperHead.step(calculateSteps(HdiabolD[i]));
      delay(2000);  // de delay voor naar positie komen 

      SwitchHead();
      delay(HeadDelay);
      SwitchEnd();
      delay(EndDelay);
      SwitchGripper();
      SwitchEnd();
      delay(EndDelay);
      // hier zorgt hij ervoor dat hij terug naar de kern kan en daarabij ook weer zijn gripper reset 
      SwitchGripper();
       delay(2000);
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


























//////////////////////////////////////////////////////////////////////////////////////////////////////////
//start main code 
void setup(){
  //start serial met Baudrate voor debugging
  Serial.begin(9600);
  
  //example stepper.begin(RPM, MICROSTEPS); Start de steppers
  StepperBase.setSpeed(StepperBaseRPM);
  StepperHead.setSpeed(StepperHeadRPM);
  
  //pinmode aangeven Sonic sensors
  pinMode(TrigPinS1, OUTPUT);
  pinMode(EchoPinS1, INPUT);
  pinMode(TrigPinS2, OUTPUT);
  pinMode(EchoPinS2, INPUT);
  pinMode(TrigPinS3, OUTPUT);
  pinMode(EchoPinS3, INPUT);


// geripper op x
  Gripper.attach(Gripperpin);
  RopeServo.attach(RopeServoPin);



Serial.println("ik zet nu mijn grijper dicht ");
  Gripper.write(150);
  GripperS = 1;

EndS = 0; // hij is naar boven 
Gamestate = 1;

}

//zet zones op en geeft deze waardes op opgeroepen te worden zie foto verdeling voor veder uitleg
//int zone[24] = {0, 8, 16, 25, 33, 41, 50, 58, 66, 75, 83, 91, 100, 108, 116, 125, 133, 142, 150, 158, 166, 175, 183, 191};
// belangerijke zones
// -1 is dus het begin veld waar hij moet beginnen 
// 20 punten zone[5]
// 14 punten zone[7]
// 16 punten zone[19]
// 18 punten zone[17]



void loop() {
delay(5000);


StepperBase.step(zone[1] - 5);
// doe alsof hij iets ziet 

SwitchHead();
delay(1000);
SwitchHead();

// hij ziet nu "iets"

StepperBase.step(zone[3] - zone[1]);


SwitchHead();
delay(1000);
SwitchHead();

StepperBase.step(5);


delay(2000);
SwitchEnd();
delay(1000);
SwitchGripper();
delay(1000);
SwitchEnd();
delay(1000);


// hij is nu weer omhoog

StepperBase.step(zone[5] - zone[3]);

delay(2000);
SwitchEnd();
delay(1000);
SwitchGripper();
delay(1000);
SwitchGripper();
delay(1000);
SwitchEnd();
delay(1000);

StepperBase.step(zone[7] - zone[5]-1);

delay(2000);
SwitchEnd();
delay(1000);
SwitchGripper();
delay(1000);


SwitchEnd();
delay(1000);
// hij heeft het nu bij 14 neergezet 
// hij moet nu weer terug naar de andere diabol. 

Gamestate = 0;
// hij hangt nu boven het eerste veld 
StepperBase.step(-zone[6]+1);
delay(2000);
SwitchEnd();
delay(1000);
SwitchGripper();
delay(1000);
SwitchEnd();

StepperBase.step(-zone[6]);

delay(2000);
SwitchEnd();
delay(1000);
SwitchGripper();
delay(1000);
SwitchGripper();
delay(1000);
SwitchEnd();
delay(1000);

StepperBase.step(-zone[2] - 4);


delay(2000);
SwitchEnd();
delay(1000);
SwitchGripper();
delay(1000);




// einde
}


// het is justin zijn schuld als dit niet werkt !!