#include <Wire.h>
#include <LCD.h>
#include <LiquidCrystal_I2C.h>
#include <LiquidCrystal.h>
#include <TimerOne.h> //An interrupt library. Executes timerlsr() every 1sec 
#include <SoftwareSerial.h>

LiquidCrystal lcd(7, 6, 2, 3, 4, 5); //RS, EN, D4, D5, D6, D7
SoftwareSerial GPRS(0,1);//TX, RX

int tempSense = A0;
int systolicSense = A1;
int diastolicSense = A2;
int heartSense = 8;
int greenPin = 10;
int yellowPin = 9;

float temp=0.00;
int mmHg = 0;
float Vo, Vs, errorP, factorT, E;

int timeInSecs = 0;
int hbCount = 0;
int hbCheck = 0;
int hbPerMin = 0;

float getTemp(int tempSensed);
int getPressure(int pressureSensed);
void getState(int systol, int diastol, int bpm, float temp);
void setLEDAndAlarm(int systol, int diastol, int bpm, float temp);

void setup() {
  // put your setup code here, to run once:
  pinMode(tempSense, INPUT);
  pinMode(systolicSense, INPUT);
  pinMode(diastolicSense, INPUT);
  pinMode(heartSense, INPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(yellowPin, OUTPUT);

  Timer1.initialize(800000);
  Timer1.attachInterrupt(everySec);

  //GSM shield initialisation
   GPRS.begin(19200);//GPRS baud rate
   Serial.begin(19200);
  //===========================

   //LCD INITIALISATION
  lcd.begin(20,4);
  lcd.clear();
  lcd.setCursor(5,0);
  lcd.print("HEALTH!!!");

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Temp : ");
  lcd.setCursor(0,1);
  lcd.print("BPM  : ");
  lcd.setCursor(0,2);
  lcd.print("mmHg : "); 
  lcd.setCursor(0,3);
  lcd.print("STATE: ");
  //----------------------------------------------------
}

void loop() {
  float tempVal, systolicVal, diastolicVal, theTemp;
  int theSystol, theDiastol;
  tempVal = analogRead(tempSense);
  theTemp = getTemp(tempVal);
  systolicVal = analogRead(systolicSense);
  theSystol = getPressure(systolicVal);
  diastolicVal = analogRead(diastolicSense);
  theDiastol = getPressure(diastolicVal);

  lcd.setCursor(8,0);
  lcd.print(theTemp);
  lcd.setCursor(14,0);
  lcd.print("C");

  lcd.setCursor(8,2);
  lcd.print(theSystol);
  lcd.setCursor(12,2);
  lcd.print("/");
  lcd.setCursor(14,2);
  lcd.print(theDiastol);

  if((digitalRead(heartSense) == HIGH) && (hbCheck == 0)){
    hbCount = hbCount+1;
    hbCheck = 1;
  }else if((digitalRead(heartSense) == LOW) && (hbCheck == 1)){
    hbCheck = 0;
  }

  if(timeInSecs == 2){ //Get for 2 seconds, times 30
    hbPerMin = hbCount * 30;
    lcd.setCursor(8,1);
    lcd.print(hbPerMin);
    lcd.setCursor(11,1);
    lcd.print(" BPM      ");
    getState(theSystol, theDiastol, hbPerMin, theTemp);
    delay(500);
    //Reset
    timeInSecs = 0;
    hbCount = 0;
    hbCheck = 0;
    hbPerMin = 0;
    lcd.clear();

     //LCD RE-INITIALISATION
  lcd.begin(20,4);
  lcd.clear();
  lcd.setCursor(5,0);
  lcd.print("HEALTH!!!");

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Temp : ");
  lcd.setCursor(0,1);
  lcd.print("BPM  : ");
  lcd.setCursor(0,2);
  lcd.print("mmHg : "); 
  lcd.setCursor(0,3);
  lcd.print("STATE: ");
  //----------------------------------------------------
  }
  else{
    lcd.setCursor(8,1);
    lcd.print("COMPUTING...");
    lcd.setCursor(8,3);
    lcd.print("COMPUTING...");
  }

}

//Return temperature in Celcius
float getTemp(int tempSensed){
  temp = (5.0 * tempSensed * 100.0) / 1024.0;
  return temp;  
}

//Return pressure in KPA
int getPressure(int pressureSensed){
  float pressure = 0.00;
  Vo= (5.0 * pressureSensed) /1024;
  errorP = 1.5;
  factorT= 1;
  Vs= 5;
  E= errorP*factorT*0.009*Vs;
  pressure = (Vo+0.475-E)/0.045;

  //1mmHg = 0.133322387415kPA
  mmHg = pressure /0.133322387415;
  return mmHg;  
}

//Advance the time in seconds using interrupts
void everySec(){
  timeInSecs = timeInSecs + 1;
}

//Print the current state of health and other short stories
void getState(int systol, int diastol, int bpm, float temp){
  if((systol>=130 || systol<120) || (diastol>=120 || diastol <110)){
    digitalWrite(greenPin, LOW);
    digitalWrite(yellowPin, HIGH);
    lcd.setCursor(8, 3);
    lcd.print("PRESSURE        ");
    delay(100);
    lcd.setCursor(8, 3);
    lcd.print("WARNING         ");
    delay(500); 
    setLEDAndAlarm(systol, diastol, bpm, temp);   
  }else if(bpm>=80 || bpm<60){
    digitalWrite(greenPin, LOW);
    digitalWrite(yellowPin, HIGH);
    lcd.setCursor(8, 3);
    lcd.print("BPM              ");
    delay(100);
    lcd.setCursor(8, 3);
    lcd.print("WARNING         ");
    delay(500);
    setLEDAndAlarm(systol, diastol, bpm, temp);
  }else if(temp >= 38 || temp < 36){
    digitalWrite(greenPin, LOW);
    digitalWrite(yellowPin, HIGH);
    lcd.setCursor(8, 3);
    lcd.print("TEMPERATURE     ");
    delay(100);
    lcd.setCursor(8, 3);
    lcd.print("WARNING         ");
    delay(500);
    setLEDAndAlarm(systol, diastol, bpm, temp);
  }else if((systol>=130 && diastol>=120 && bpm>=80 && temp >= 38) || (systol<120 && diastol <110 && bpm<60 && temp < 36)){
    digitalWrite(greenPin, LOW);
    digitalWrite(yellowPin, HIGH);
    lcd.setCursor(8, 3);
    lcd.print("CRITICAL     ");
    delay(100);
    lcd.setCursor(8, 3);
    lcd.print("WARNING         ");
    delay(500);
    setLEDAndAlarm(systol, diastol, bpm, temp);
  }else{
    lcd.setCursor(8,3);
    lcd.print("HEALTHY      ");
    digitalWrite(yellowPin, LOW);
    digitalWrite(greenPin, HIGH);
  }
}

void setLEDAndAlarm(int systol, int diastol, int bpm, float temp){
  lcd.setCursor(8,3);
  lcd.print("ALERTING DOC"); 
    //Send text to doctor here
  if(Serial.available()){
    GPRS.write(Serial.read());

    GPRS.print("AT+CMGF=1\r");//Command to send SMS in text mode
    Serial.println("AT+CMGF=1\r");
    delay(500);
    
    GPRS.print("AT+CMGS=\"+254718953619\"\r");//Command to set phone number
    Serial.println("AT+CMGS=\"+254718953619\"\r");
    delay(500);
    
    GPRS.print("ALERT!!! A patient is showing signs of unhealthiness. Please advise\r");
    GPRS.print("Patient Phone Number \r");
    GPRS.print(" +254725447764 \r");
    GPRS.print("VITALS \r");
    GPRS.print("BPM: \r");
    GPRS.print(bpm);
    GPRS.print("TEMPERATURE: \r");
    GPRS.print(temp);
    GPRS.print("SYSTOL: \r");
    GPRS.print(systol);
    GPRS.print("DIASTOL \r");
    GPRS.print(diastol);
    delay(500);
    
    GPRS.write(0x1A);//Ctrl+Z to send text
    delay(500);
    
    lcd.setCursor(8,3);
    lcd.print("DOC ALERTED"); 
  }
  delay(200);  
}

