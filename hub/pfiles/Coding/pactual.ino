//include the various required libraries for the DHT11
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

//include the libraries for LCD
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27,16,2);

#define DHTPIN 3                            //DHT Pin requires PWM

#define DHTTYPE    DHT11                    //Define DHT 11 as DHT sensor being used here

DHT_Unified dht(DHTPIN, DHTTYPE);

uint32_t delayMS;

//functions, listing them in order

void waterlevel();
void buttoncheck();
void pages();
void action();
void autowatering();
void smartwatering();

//global variable checks
int tank; // a variable checking for water tank level, 1 for sufficient water and 0 for no water

//my inputs and output names

//LED segment

const int ledsegmentpin1=13;
const int ledsegmentpin2=12;
const int ledsegmentpin3=11;
const int ledsegmentpin4=10;
const int ledsegmentpin5=9;

//PUSH BUTTON

const int button1=4;
const int button2=5;
const int button3=2;

//buzzer
const int buzzer=6;

//analog capacitative water sensor
const int watersensor=A0;               //using analog inputs to get a value between 0 and 5 V using the ADC
const int moisturesensor=A1;

//water pump
const int pump=8;

//millis
unsigned long currentMillis = 0;

//segment intervals
const int segmentinterval=200;          //how often the 5 segment LED updates
unsigned long previousSegmentMillis=0;
int segment=0; 

//button intervals
const int buttoninterval=250;              //how often the buttons are checked, helps with debouncing
unsigned long previousButtonMillis=0;

//pages
int page=1;
unsigned long previousPageMillis=0;

int pagecheck=1;
int pageinterval=200;                       //how often the pages are checked
//action
unsigned long previousActionMillis=0;
const int ActionInterval=150;               //cycle time for the 'action' function, in this case every 150ms
unsigned long wateringtime;

//autowatering
unsigned long previousAutowateringMillis=0;
int pumpinterval=500;
int masteron=0;
unsigned long previouspumpMillis=0;

//smart watering
unsigned long previousSmartWateringMillis=0;
int smartmasteron=0;

double input;
double voltage;

void setup() {
  // put your setup code here, to run once:

//LED SEGMENT
pinMode(ledsegmentpin1,OUTPUT);
pinMode(ledsegmentpin2,OUTPUT);
pinMode(ledsegmentpin3,OUTPUT);
pinMode(ledsegmentpin4,OUTPUT);
pinMode(ledsegmentpin5,OUTPUT);

//buzzer

pinMode(buzzer,OUTPUT);

//INPUT PUSH BUTTON, uses the onboard pullups, as the name implies the push buttons are pulled up to 5V and as a result are active low,producing 0V when pressed.
pinMode(button1,INPUT_PULLUP);
pinMode(button2,INPUT_PULLUP);
pinMode(button3,INPUT_PULLUP);

//pump config
pinMode(pump,OUTPUT);


Serial.begin(9600);                     //serial for debugging, helps with checking variable values
dht.begin();                            
sensor_t sensor;
dht.temperature().getSensor(&sensor);
dht.humidity().getSensor(&sensor);
delayMS = sensor.min_delay / 1000;

lcd.init(); //INITIALISE THE LCD
lcd.backlight(); //SWITCH ON THE LCD BACKLIGHT
lcd.clear(); //CLEAR THE LCD 
}

void loop() {
  // put your main code here, to run repeatedly:

currentMillis=millis();                 //MILLIS to help being a non-polling based program
waterlevel();                           //check for water
if(tank==1)                             //if there is water, proceed with the modes and pages below
{
  buttoncheck();                        //calling functions
  pages();
  action();
  autowatering();
  smartwatering();
}


  
}

void waterlevel()
{
  if(currentMillis-previousSegmentMillis>=segmentinterval)
  {
  double waterlevel;
  double voltage;
  
  waterlevel=analogRead(watersensor);                       //the input is a 10 bit number due to the onboard analog to digital converter (ADC)
  voltage=waterlevel*5.0/1023.0;
  //Serial.println(voltage);
  if(voltage>3.68)                                          //in the case of my sensor, my sensor reduces in voltage when it is in contact with water , voltage values are callibrated.
  {
    if(segment==0)
    {
      tank=0;
      digitalWrite(ledsegmentpin1,LOW);
      digitalWrite(ledsegmentpin2,LOW);
      digitalWrite(ledsegmentpin3,LOW);
      digitalWrite(ledsegmentpin4,LOW);
      digitalWrite(ledsegmentpin5,HIGH);                    //high here
      segment++;
      tone(buzzer,3000);                                    //due to using millis, I can set the buzzer with no duration specified
      lcd.clear();                                          //clear lcd
      lcd.setCursor(0,0);                                   //set cursor at 0,0 which is first line first column
      lcd.print("LOW WATER");
      lcd.setCursor(0,1);
      lcd.print("REFILL TANK");
      
      
      
      
    }
    else if(segment==1)
    {
      tank=0;
      digitalWrite(ledsegmentpin1,LOW);                     
      digitalWrite(ledsegmentpin2,LOW);
      digitalWrite(ledsegmentpin3,LOW);
      digitalWrite(ledsegmentpin4,LOW);
      digitalWrite(ledsegmentpin5,LOW);                     //led segment low here, creates a flickering effect
      segment=0;
      noTone(buzzer);                                       //on and off buzzer results in a beeping effect
      
      
    } 
  }
  else if(voltage>3.62)
  {
      tank=1;
      digitalWrite(ledsegmentpin1,LOW);
      digitalWrite(ledsegmentpin2,LOW);
      digitalWrite(ledsegmentpin3,LOW);
      digitalWrite(ledsegmentpin4,HIGH);
      digitalWrite(ledsegmentpin5,HIGH);
      noTone(buzzer);                                       //no tone buzzer to switch off the buzzer when refilled
  }
  else if(voltage>3.57)
  {
      tank=1;
      digitalWrite(ledsegmentpin1,LOW);
      digitalWrite(ledsegmentpin2,LOW);
      digitalWrite(ledsegmentpin3,HIGH);
      digitalWrite(ledsegmentpin4,HIGH);
      digitalWrite(ledsegmentpin5,HIGH);
      noTone(buzzer);
  }
  else if(voltage>3.53)
  {
      tank=1;
      digitalWrite(ledsegmentpin1,LOW);
      digitalWrite(ledsegmentpin2,HIGH);
      digitalWrite(ledsegmentpin3,HIGH);
      digitalWrite(ledsegmentpin4,HIGH);
      digitalWrite(ledsegmentpin5,HIGH);
      noTone(buzzer);
  }
  else if(voltage>3.50)
  {
      tank=1;
      digitalWrite(ledsegmentpin1,HIGH);
      digitalWrite(ledsegmentpin2,HIGH);
      digitalWrite(ledsegmentpin3,HIGH);
      digitalWrite(ledsegmentpin4,HIGH);
      digitalWrite(ledsegmentpin5,HIGH);
      noTone(buzzer);
  }
 
  previousSegmentMillis+=segmentinterval;
  //Serial.println(tank);
  }
  
}

void buttoncheck()
{

  if(currentMillis-previousButtonMillis>=buttoninterval)
  { 
    Serial.println(page);
    if(digitalRead(button1)==0)                             //this is pretty much allows the program to cycle between the 6 pages
    {
      if(page<6)
      {
        page++;
      }
      else if(page=5)
      {
        page=1;
      }
      
    }


  previousButtonMillis+=buttoninterval;
  }
  
}

void pages()
{
  char msg[16];                                             //initalises a string for sprintf
  if(currentMillis-previousPageMillis>=pageinterval)
  {
    if(page==1)
    {
      if(pagecheck!=1)                                      //i use this 'pagecheck' to check whether its the same page being printed on the lcd
      {                                                     //if it's the same page i dont need to clear it, but if its a different page it will clear the screen
        lcd.clear();
      }
      lcd.setCursor(0,0);
      
      if(masteron==0)
      {
        lcd.print("MASTER TIMER OFF");
      
      }
      else if(masteron==1)
      {
     
        lcd.print("MASTER TIMER ON ");
      
      }
      lcd.setCursor(0,1);
      lcd.print("OFF || ON");
      pagecheck=1;
    }
    if(page==2)
    {
      if(pagecheck!=2)
      {
        lcd.clear();
      }
      lcd.setCursor(0,0);
      lcd.print("WATERING TIME");
      lcd.setCursor(0,1);
      sprintf(msg,"<-- | --> T:%02ds",wateringtime/1000);   //sprintf combines the string and wateringtime divided by 1000
      lcd.print(msg);                                       //print the combined string
      pagecheck=2;
    }
    if(page==3)
    {
      if(pagecheck!=3)
      {
        lcd.clear();
      }
      lcd.setCursor(0,0);
      lcd.print("MANUAL WATER");
      lcd.setCursor(0,1);
      lcd.print("OFF  || ON");
      pagecheck=3;
      
    }
    if(page==4)
    {
      if(pagecheck!=4)
      {
        lcd.clear();
      }
      lcd.setCursor(0,0);
      lcd.print("PUMP RUN TIME");
      lcd.setCursor(0,1);
      sprintf(msg,"<-- | --> T:%02ds",pumpinterval/1000);
      lcd.print(msg);
      pagecheck=4;
      
    }
    if(page==5)
    {
      if(pagecheck!=5)
      {
        lcd.clear();
      }
      lcd.setCursor(0,0);
      if(smartmasteron==0)
      {
        lcd.print("SMART MASTER OFF");
      }
      else if(smartmasteron==1)
      {
        lcd.print("SMART MASTER ON");
      }
      lcd.setCursor(0,1);
      lcd.print("OFF  || ON");
      pagecheck=5;
      
    }
    


  previousPageMillis+=pageinterval;
  }
  
}

void action()
{
  if(currentMillis-previousActionMillis>=ActionInterval)
  {
    if(digitalRead(button2)==0)                         //button 2 is the left button and button 3 is the right button
    {                                                   //button2 used as off/decreasing whilst button 3 used as ON/increasing
      if(page==1)
      {
        masteron=0;
      }

      
      if(page==2)
      {
        wateringtime-=5000;
      }


      if(page==3)
      {
        digitalWrite(pump,LOW);
      }

      if(page==4)
      {
        pumpinterval-=1000;
      }

      if(page==5)
      {
        smartmasteron=0;
      }
      
    }
    if(digitalRead(button3)==0)
    {
      if(page==1)
      {
        masteron=1;
      }

      
      if(page==2)
      {
        wateringtime+=5000;
      }
      
      if(page==3)
      {
        digitalWrite(pump,HIGH);
      }

      if(page==4)
      {
        pumpinterval+=1000;
      }
      if(page==5)
      {
        smartmasteron=1;
      }


      
    }
  previousActionMillis+=ActionInterval;  
  }
}

void autowatering()
{ 

if(masteron==1)
{
  if(currentMillis-previousAutowateringMillis>=wateringtime)
  {
    digitalWrite(pump,HIGH);
    delay(pumpinterval);
    digitalWrite(pump,LOW);
    
    
    previousAutowateringMillis+=wateringtime;  
  }
}
}

void smartwatering()
{
  if(smartmasteron==1)
  {
  
  if(currentMillis-previousSmartWateringMillis>=wateringtime)
  {
    digitalWrite(pump,HIGH);
    Serial.println(voltage);
    do
    {
      input=analogRead(moisturesensor);
      voltage=input*5.0/1023.0;
      Serial.println(voltage);
    }while(voltage<3.8);
    digitalWrite(pump,LOW);
    previousSmartWateringMillis+=wateringtime;
  }
  }

}
