#include <Wire.h>
#include <LCD.h>
#include <LiquidCrystal_I2C.h>
#include <LiquidCrystal.h>
#include <Servo.h>

/*FLOODS DETECTION AND AVERSION SYS
 * - Every 5 minutes, read data about water things from level sensor, soil moisture sensor, raindrop sensor
 * - Based on their outputs, compute danger level: 0-Warning, 1-Critical, 2-Healthy, 3-Indeterminable
 * - Open literal flood gates (Servo), light up LED, print on LCD, and sound buzzer based on the danger level AKA action
 * -> If danger i.e. action<2, check on a loop until action ==0 i.e. don't wait for 5 minutes
 * -> If action==0, wait for another 5 minutes
 */

LiquidCrystal_I2C lcd(0x27, 2, 1, 0,4,5,6,7);
Servo myServo;
//0: Warning, 1:Critical, 2:Healthy
int redPin = 11;
int greenPin = 9;
int bluePin = 10;
int buzzerPin = 8;
int servoPin = 7;
int raindropPin = A2;
int levelPin = A0;
int soilPin = A1;
int raindropstate, levelstate, soilstate;
unsigned long previousMillis = 0; // will store last time a check was done. millis()
long interval = 10000; //Interval in milliseconds
int looping = 0; //Is averting danger?


void setup() {
  Serial.begin(9600);
  
  //****LCD Initialisation
  //Vcc - Vcc, GND - GND. SDA - A4. SCL - A5
  lcd.setBacklightPin(3, POSITIVE);
  lcd.setBacklight(LOW);
  lcd.begin(16,2);
  //***********

  //***RGB pins setup
  //longest-GND. Sole to the left(RED) - redPin, Green- greenPin, Blue- bluePin
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  //***************

  //****Buzzer setup
  pinMode(buzzerPin, OUTPUT);
  //***********

  //****Servo setup
  //Red - Vcc. Black- GND. Yellow- servoPin
  myServo.attach(servoPin);
  //*********
}

void loop() {
  // Check time.
  unsigned long currentMillis = millis(); //Return milliseconds of sketch run
  
  if(currentMillis - previousMillis >= interval){//10 secs have passed since last check
    check:
    raindropstate = getRainDropState();
    levelstate = getLevelState();
    soilstate = getSoilState();
      
    int action = calculateTotalAction(raindropstate, levelstate, soilstate, looping);
    Serial.println("=====================================");
    Serial.print("Rain: ");Serial.println(raindropstate);
    Serial.print("Level: ");Serial.println(levelstate);
    Serial.print("Soil: ");Serial.println(soilstate);
    Serial.print("Action: "); Serial.println(action);
    //DISPLAY ACCORDING TO ACTION
    showOnLCD(action); 
    lightLED(action);
    playAlarm(action);
    gatePosition(action);  
    delay(5000); //So that you can see displayed values
    //===========
    previousMillis = currentMillis+5000; //Update time for checking next time

    //Repeat till action is healthy
    if(action < 2){ 
      goto check; 
      looping = 1; //Is looping
    }else if(action==2){ //Healthy
      //Re-initialise things i.e. all healthy
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("STANDBY");
      lcd.setCursor(0,1);
      lcd.print("READING");
      gatePosition(2); //Close gate
      delay(2000); //For reading
      previousMillis = previousMillis+2000;
      looping=0;
    }else if(action==3){//Indeterminable. Loop
      goto check; //Check if danger is over
      looping = 1; //Is looping     
    }
  }else{ //Still time
    lcd.setCursor(0,0);
    lcd.print("STANDBY");
    lcd.setCursor(0,1);
    lcd.print("READING");
  }
}


//For colouring the RGB LED. For warning(0), critical(1) or healthy(2).
void lightLED(int type){
  if(type==0){//Warning. Blue
    putColour(0,0,255);    
  }else if(type==1){//Critical. Red
    putColour(255,0,0);
  }else if(type==2){//Healthy. Green
    putColour(0,255,0);
  }
}

//Actually paint the LED
void putColour(int red, int green, int blue){ 
  analogWrite(redPin, red);
  analogWrite(greenPin, green);
  analogWrite(bluePin, blue);
}

//For playing alarm on buzzer. For warning or critical
void playAlarm(int type){
  if(type==0){ //Warning. Beep
    digitalWrite(buzzerPin, HIGH);
    delay(500);
    digitalWrite(buzzerPin, LOW);
    delay(500);
    
   }else if(type==1){ //Critical. Siren
    for(int hz=440; hz<1000;hz++){
    tone(buzzerPin, hz, 50);
    delay(5);
     }
    noTone(buzzerPin);
    for(int hz=1000; hz>440; hz--){
      tone(buzzerPin, hz, 50);
      delay(5);
     }
    noTone(buzzerPin);
    
  }else{//Healthy. Nothing
    noTone(buzzerPin);
  }
}

//For servo movement (Gate opening) 0-Warning(90deg) 1-Critical(180deg) 2-Healthy(0deg)
void gatePosition(int type){
  if(type==0){
    myServo.write(90);
  }else if(type==1){
    myServo.write(180);
  }else if(type==2){
    myServo.write(0);    
  }
  
}

//Write to LCD the action
void showOnLCD(int type){
  if(type==0){ //Warning
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("WARNING");
    lcd.setCursor(0,1);
    lcd.print("AVERTING");    
  }else if(type==1){ //Critical
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("DANGER");
    lcd.setCursor(0,1);
    lcd.print("REACTING");  
  }else if(type==2){ //Healthy
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("HEALTHY"); 
    lcd.setCursor(0,1);
    lcd.print("CHILLING");   
  }
}

//Read and return things from raindrop sensor
int getRainDropState(){
  int sensed =  analogRead(raindropPin); //1023 when dry, 0 when soaked
  int sensedRange = map(sensed, 0, 1024, 0,3);
  switch(sensedRange){
    case 0: //Soaked
      return 1;
      Serial.println("Raindrops critical!!");
      break;
    case 1: //Warning
     return 0;
     Serial.println("Raindrops warning!!");
     break;
    case 2: //Dry
     return 2;
     Serial.println("Raindrops Healthy!!");
     break;
  }
  
}

//Read and return things from level sensor
int getLevelState(){
  int sensed =  analogRead(levelPin); //Output proportional to water level (0 to 700ish)
  if(sensed >0 && sensed <=233){
      return 2;
      Serial.println("Level Healthy!!");
  }else if(sensed >233 && sensed<=467){
     return 0;
     Serial.println("Level warning!!");
  }else if(sensed >468){
     return 1;
     Serial.println("Level Critical!!");
  }  
}

//Read and return things from soil moisture sensor
int getSoilState(){
  int sensed =  analogRead(soilPin); //1024 when dry, 0 when soaked
  int sensedRange = map(sensed, 0, 1024, 0,3);
  switch(sensedRange){
    case 0: //Soaked
      return 1;
      Serial.println("Soil critical!!");
      break;
    case 1: //Warning
     return 0;
     Serial.println("Soil warning!!");
     break;
    case 2: //Dry
     return 2;
     Serial.println("Soil Healthy!!");
     break;
  }
  
}

//Comparison every five minutes. Check change in state in last 5mins. Returns alert type
int calculateTotalAction(int rainstateNow, int levelstateNow, int soilstateNow, int averting){
  if(levelstateNow <0){levelstateNow=2;} //Because level sensor is forked!

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("CALCING");
  lcd.setCursor(0,1);
  lcd.print("ACTION");

if(averting==0){//Normal 5min computation
    //Same states
    if(rainstateNow==0 && levelstateNow==0 && soilstateNow==0){
      return 0;
    }else if(rainstateNow==1 && levelstateNow==1 && soilstateNow==1){
      return 1;
    }else if(rainstateNow==2 && levelstateNow==2 && soilstateNow==2){
      return 2;
    }
    //The six possible combinations
    else if(rainstateNow==0 && levelstateNow==1 && soilstateNow==2){//Level critical
      return 1;
    }else if(rainstateNow==1 && levelstateNow==0 && soilstateNow==2){//Rainy. High level
      return 1;
    }else if(rainstateNow==2 && levelstateNow==0 && soilstateNow==1){//Manageable
      return 2;
    }else if(rainstateNow==0 && levelstateNow==2 && soilstateNow==1){//High-ish rain. Soaked soil
      return 0;
    }else if(rainstateNow==1 && levelstateNow==2 && soilstateNow==0){//High rain with soaked soil
      return 1;
    }else if(rainstateNow==2 && levelstateNow==1 && soilstateNow==0){//Level critical
      return 1;
    }
    //Because sloppy code
    else if(rainstateNow!=1 && levelstateNow==1 && soilstateNow!=1){
      return 1;
    }
    //Because we need more health
    else if(rainstateNow==2 && levelstateNow==2){
      return 2;
    }else{
      return 3;
    }
}else if(averting==1){//Danger looming.
  //Check water level only
      if(levelstateNow ==0){
      return 0;    
      }else if(levelstateNow==1){
        return 1;
      }else if(levelstateNow==2){
        return 2;
      }
  
} 
}



