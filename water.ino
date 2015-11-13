
//Created by Alejandro Garcia for atmega2560
// The system controlls the level of a house water tank, 
// the water is extracted from 35 meters deep by a lectric pump controlled by this system throw a 5v Realy 
// In addition the systems irrigates over 4 zones using timers. 
// All Events are logged on SDCARD (note: sdcard library uses 512 Bytes of RAM memory)
// A lc1602 Display shows the current status of the system and it is requiered for Time/Date setup 
// A matrix 4x4 Keyboard is required for time setup
// The system includes a remote control using HT12 E/D (which activates the pump and a solenoid valve send flow trougth a hose 5th zone)
// Solenoid valves are requiered and 7 relays ( 1 pump, 4 zones, 1 house water tank, hose )
//
// mesihas@gmail.com
// The comments will be traslated to English on request
// I am aware that the code is not the best, but works fine ! and it could be helpful to someone doing something similar



#include <Wire.h>  
#include <LiquidCrystal.h>
#include <Time.h>
#include <TimeAlarms.h>
#include <DS1307RTC.h>
#include <avr/wdt.h>
#include <Keypad.h>
#include <SPI.h>
#include <SD.h>

// RELAYS
int v_riegoManual = A6;
int v_tanque = A5;
int zona4 = A4;
int zona3 = A3;
int zona2 = A2;
int zona1 = A1;
int pumprelay = A0;
int buzzer = 1;

// botones y Switches
int btnSetHora = 30;
int Z1M=1;
int Z2M=1;
int Z3M=1;
int Z4M=1;
int TnqM=1;
int RCsi = 44;
int RCno = 46;
int floatSwitch = 42;
int snrlluvia = 1;
// SPI
const int chipSelect = 53; ////SD card

int pantalla = 0;
int i=0;
int intZona=0;


char tiempo[2];
String hora;
String minu;
String segu;
String dia;
String mes;
String anio;

String dataString; // para log SD

const int numRows = 2;
const int numCols = 16;

LiquidCrystal lcd(14, 15, 16, 17, 18, 19);
//LiquidCrystal_I2C lcd(0x26,16,2);  

//########### KeyPad ####################
const byte ROWS = 4; // Four rows
const byte COLS = 3; // Three columns
// Define the Keymap
char keys[ROWS][COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'#','0','*'}
};
// Connect keypad ROW0, ROW1, ROW2 and ROW3 to these Arduino pins.
byte rowPins[ROWS] = { 9, 8, 7, 6 };
// Connect keypad COL0, COL1 and COL2 to these Arduino pins.
byte colPins[COLS] = { 12, 11, 10 }; 
// Create the Keypad
Keypad kpd = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );
//#define ledpin 13 // Keypad

//########### KeyPad #######################
//########### RAIN SENSOR ##################
// lowest and highest sensor readings:
const int sensorMin = 0;     // sensor minimum
const int sensorMax = 1024;  // sensor maximum
//########### RAIN SENSOR ####################


//int calibrationTime = 30;
boolean runningRiego = false;
boolean runningTanque = false;
boolean runningRC = false;
boolean dspRiego = false;
boolean dspRiegoManual = false;
boolean dspTanque = false;
boolean crSI= false;
boolean crNO= false;
//


int valorSwitchTanque  = digitalRead(floatSwitch);
int valorRCsi= digitalRead(RCsi);
int valorRCno= digitalRead(RCno);

int range; //rain sensor
/*
//the time when the sensor outputs a low impulse
long unsigned int lowIn;         

//the amount of milliseconds the sensor has to be low 
//before we assume all motion has stopped
long unsigned int pause = 5000;  

boolean lockLow = true;
boolean takeLowTime;  

int pirPin = 4;    //the digital pin connected to the PIR sensor's output
int ledPin = 13;
// the follow variables are long's because the time, measured in miliseconds,
// will quickly become a bigger number than can be stored in an int.

*/
//int clockAddress = 0x68; // direccion I2C
void setup()
{
  
  pinMode(pumprelay, OUTPUT);
  pinMode(zona1, OUTPUT);
  pinMode(zona2, OUTPUT);
  pinMode(zona3, OUTPUT);
  pinMode(zona4, OUTPUT);
  pinMode(v_tanque, OUTPUT);
  pinMode(v_riegoManual, OUTPUT);
  pinMode(buzzer, OUTPUT); 
  
  pinMode(btnSetHora, INPUT_PULLUP);
  pinMode(floatSwitch, INPUT_PULLUP); 
  pinMode(RCsi, INPUT);
  pinMode(RCno, INPUT);
  
  digitalWrite(pumprelay, HIGH);
  digitalWrite(zona1, HIGH);
  digitalWrite(zona2, HIGH);
  digitalWrite(zona3, HIGH);
  digitalWrite(zona4, HIGH);
  digitalWrite(v_tanque, HIGH);
  digitalWrite(v_riegoManual, HIGH);
  //########### KeyPad ####################
//  pinMode(ledpin,OUTPUT);
//  digitalWrite(ledpin, HIGH);
  //########### KeyPad ####################
 // lcd.init();
  Serial.begin(9600);
  // Used to type in characters
  setSyncProvider(RTC.get);   // ***the function to get the time from the RTC***
  lcd.begin(numCols, numRows);
  
//########### SD Card ####################  
   // Serial.print("Initializing SD card...");
  // make sure that the default chip select pin is set to
  // output, even if you don't use it:
    pinMode(10, OUTPUT);

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
 //   Serial.println("Card failed, or not present");
    // don't do anything more:
    return;
  }
 // Serial.println("card initialized.");
//###########  SD Card  ####################

/*
  //----------Inicializo el sensor de movimiento-------------
  //pinMode(pirPin, INPUT);
 // pinMode(ledPin, OUTPUT);
 // digitalWrite(pirPin, LOW);

  //give the sensor some time to calibrate
  /*Serial.print("calibrating sensor ");
    for(int i = 0; i < calibrationTime; i++){
      Serial.print(".");
      delay(1000);
      }
    Serial.println(" done");
    Serial.println("SENSOR ACTIVE");
    delay(50);
 

  //------------- Timers riego-----------  
  Alarm.alarmRepeat(22,00,0, zona1_on); 
  Alarm.alarmRepeat(22,05,30, apagar);
  //
  Alarm.alarmRepeat(22,10,0, zona2_on); 
  Alarm.alarmRepeat(22,15,30, apagar);
  //
  Alarm.alarmRepeat(22,20,0, zona3_on); 
  Alarm.alarmRepeat(22,25,30, apagar);
  //
  Alarm.alarmRepeat(22,30,0, zona4_on); 
  Alarm.alarmRepeat(22,35,30, apagar);
  
  ////// MANIANA >>>>>>>>>>>>>>
  Alarm.alarmRepeat(9,00,00, zona1_on); 
  Alarm.alarmRepeat(9,15,00, apagar);
   
  Alarm.alarmRepeat(9,20,00, zona2_on); 
  Alarm.alarmRepeat(9,35,00, apagar);
   
  Alarm.alarmRepeat(9,40,00, zona3_on); 
  Alarm.alarmRepeat(9,55,00, apagar);
   
  Alarm.alarmRepeat(10,00,00, zona4_on); 
  Alarm.alarmRepeat(10,15,00, apagar);
*/
  dataString="Arranca";
  sdlog();
}

void loop(){ 
  Alarm.delay(100);
  //pir(); 
  automaticoTanque();
  RC();
  i=i+1;
  
     if (i==100000){
       softwareReset();
     } 
   
 // valorSwitchRiego = digitalRead(btn_riegoManual);
 int  valorSwitchTanque = digitalRead(floatSwitch);
 display();
 setHora();
 //setSyncProvider(RTC.get);
 //lluvia();
}

// ########### Software Reset timer just in case the systems Stability #######################
  /*
  15mS    WDTO_15MS
  30mS    WDTO_30MS
  60mS    WDTO_60MS
  120mS   WDTO_120MS
  250mS   WDTO_250MS
  500mS   WDTO_500MS
  1S      WDTO_1S
  2S      WDTO_2S
  4S      WDTO_4S
  8S      WDTO_8S
  */
void softwareReset(){
  delay(50);
  if (runningRiego==true or runningTanque == true && runningRC == true){
  }else{
      wdt_enable(WDTO_15MS);
      while(1){}     
  } 
} 

// ########### BOMBA #######################
void bomba_on() {   
  delay(500);
  digitalWrite(pumprelay, LOW); 
}


// ########### Apagar Todo #######################
void apagar() {

  digitalWrite(pumprelay, HIGH);
  delay(500);
  digitalWrite(zona1, HIGH);
  digitalWrite(zona2, HIGH);
  digitalWrite(zona3, HIGH);
  digitalWrite(zona4, HIGH);
  digitalWrite(v_riegoManual, HIGH);
  digitalWrite(v_tanque, HIGH);
  runningRiego = false;
  pantalla=0;
  dataString="Apaga bomba";
  sdlog();
}


void zona1_on() {
    if (runningTanque==false && runningRC==false) {
        runningRiego = true;
        digitalWrite(zona1, LOW);
        delay(500);
        digitalWrite(pumprelay, LOW);
//        Serial.println("SI senior, zona 1");
//        serialPrintHora();
        dataString="zona 1";
        sdlog();
        pantalla=3;
        intZona  =1;   
    }
    else {}

}
  
void zona2_on() {
    if (runningTanque==false && runningRC==false) {
        runningRiego = true;
        digitalWrite(zona2, LOW);
        delay(500);
        digitalWrite(pumprelay, LOW);
      //  Serial.println("SI senior, zona 2");
      //  serialPrintHora();
        dataString="zona 2";
        sdlog();
        pantalla=4;
        intZona  =2;     
    }
    else {}  
  
}

void zona3_on() {
    if (runningTanque==false && runningRC==false) {
      runningRiego = true;
      digitalWrite(zona3, LOW);
      delay(500);
      digitalWrite(pumprelay, LOW);
    //  Serial.println("SI senior, zona 3");
   //   serialPrintHora();
      dataString="zona 3";
      sdlog();
      pantalla=5;  
      intZona  =3;   
    }
    else {} 
}


void zona4_on() {
  
    if (runningTanque==false && runningRC==false) {
      runningRiego = true;
      digitalWrite(zona4, LOW);
      delay(500);
      digitalWrite(pumprelay, LOW);
      pantalla=6;
//      Serial.println("SI senior, zona 4");
//      serialPrintHora();
      dataString="zona 4";
      sdlog();
      intZona  =4; 
    }
    else {}
}

// ########### AUTOMATICO TANQUE #######################
void automaticoTanque(){
    valorSwitchTanque = digitalRead(floatSwitch);
    
    if (valorSwitchTanque == LOW && runningRiego==false && runningRC==false) {
      runningTanque=true;
      digitalWrite(v_tanque, LOW);
      delay(500);
      digitalWrite(pumprelay, LOW);
      pantalla=1;
  //    Serial.println("SI senior, Tanque");
  //    serialPrintHora();
      if (dataString="Llena tanque") {
      }else{
        dataString="Llena tanque";
        sdlog();
      }
    } 
     if(runningRiego==false && valorSwitchTanque == HIGH && runningRC == false){
          if (valorSwitchTanque == HIGH && runningTanque == true) {
              delay(250);
              digitalWrite(pumprelay, HIGH);
              delay(500);
              digitalWrite(v_tanque, HIGH);
              runningTanque=false;
              pantalla=0;
       //       Serial.println("Apaga, Tanque");
      //        serialPrintHora();
              dataString="Apaga tanque";
              sdlog();

              
             
          } else {}
     }else{}
}

// ########### CONTRO REMORTO #######################

void RC(){
 
  valorRCsi = digitalRead(RCsi);
  valorRCno = digitalRead(RCno);
  
  
  if (valorRCsi == LOW && valorRCno == LOW){
 
 //   Serial.println("Kilombo RF");
 //   serialPrintHora();
 
  }
  else {
      
          if (runningRiego==false && runningTanque==false ){  
              
                    if (runningRC == true){
                      
                    }else{  
                            
                           if (valorRCsi == LOW){
                              runningRC == true;
                              digitalWrite(v_riegoManual, LOW);
                              delay(500);
                              digitalWrite(pumprelay, LOW);
                             // analogWrite(buzzer,128);
                              delay(50); 
                           //   analogWrite(buzzer, 0);
              //                Serial.println("Arranca  CR");
              //                serialPrintHora();             
                              pantalla=2;
                              dataString="Arranca  CR";
                              sdlog();
                       
                            }else{}
                    
                            if (valorRCno == LOW){
                              runningRC == false;
                              delay(250);
                              digitalWrite(pumprelay, HIGH);
                              delay(500);
                              digitalWrite(v_riegoManual, HIGH);  
            //                  Serial.println("Stop CR");
            //                  serialPrintHora();
                              pantalla=0;
                              dataString="Apaga CR";
                              sdlog();
                            }else{ }
                    }          
                  
          }else{}
  }
}


void display(){
  
      if ((minute()==00 || minute()==10  || minute()==20  || minute()==30  || minute()==40  || minute()==50) && second()==00 )
      {lcd.clear();}else{}
  
    switch (pantalla) {
          case 0: 
              lcd.setCursor(0,0); 
              lcd.print("RIEGO Y TANQUE ");
              lcdPrintHora();
              break;
          case 1:
                lcd.setCursor(0,0); 
                lcd.print("LLENANDO TANQUE");
                lcdPrintHora();
                break;
          case 2:
                lcd.setCursor(0,0); 
                lcd.print("CONTROL REMOTO");
                lcdPrintHora();
                break;
            
            case 3:
                lcd.setCursor(0,0); 
                lcd.print("RIEGO ZONA 1");
                lcdPrintHora();            
                break;          
            case 4:
                lcd.setCursor(0,0); 
                lcd.print("RIEGO ZONA 2");
                lcdPrintHora();
            break;
             case 5:
                lcd.setCursor(0,0); 
                lcd.print("RIEGO ZONA 3");
                lcdPrintHora();
            break;
            case 6:
                lcd.setCursor(0,0); 
                lcd.print("RIEGO ZONA 4");
                lcdPrintHora();
          default:;

    }

 
}

void lcdPrintHora(){
                
                lcd.setCursor(0,1); 
                lcd.print(hour());
                lcd.print(":");
                lcd.print(minute());
                lcd.print(":");
                lcd.print(second());
}
        

void serialPrintHora(){
//               Serial.print(hour());
//               Serial.print(":");           
//               Serial.print(minute());
//               Serial.print(":");             
//               Serial.println(second());
//               Serial.println("-----------------------------");
}

void setHora(){
    if (digitalRead(btnSetHora) == LOW){
        //############################################# Hora
        lcd.clear();
        lcd.setCursor(0,0); 
        lcd.print("INGRESA PRIMER");
        lcd.setCursor(0,1);
        lcd.print("DIGITO HORA:");
        char key = kpd.waitForKey();
        if(key) { 
          lcd.print(key);
          tiempo[0]=key;
        }else{}
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("INGRESA SEGUNDO");
        lcd.setCursor(0,1);
        lcd.print("DIGITO HORA:");
        key = kpd.waitForKey();
        if(key) { 
          lcd.print(key);
          tiempo[1]=key;
        }else{}
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("USTED INGRESO:");
        lcd.setCursor(0,1);
        lcd.print(tiempo[0]);
        lcd.print(tiempo[1]);           
        hora +=(tiempo[0]);
        hora +=(tiempo[1]);             
        //############################################# Minutos
        lcd.clear();
        lcd.setCursor(0,0); 
        lcd.print("INGRESA PRIMER");
        lcd.setCursor(0,1);
        lcd.print("DIGITO MIBUTOS:");
        key = kpd.waitForKey();
        if(key) { 
          lcd.print(key);
          tiempo[0]=key;
        }else{}
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("INGRESA SEGUNDO");
        lcd.setCursor(0,1);
        lcd.print("DIGITO MINUTOS:");
        key = kpd.waitForKey();
        if(key) { 
        lcd.print(key);
        tiempo[1]=key;
        }else{}
        
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("USTED INGRESO:");
        lcd.setCursor(0,1);
        lcd.print(tiempo[0]);
        lcd.print(tiempo[1]);
        minu +=(tiempo[0]);
        minu +=(tiempo[1]);
        //############################################# Segundos
        segu= "00";
        //############################################# dia
        lcd.clear();
        lcd.setCursor(0,0); 
        lcd.print("INGRESA PRIMER");
        lcd.setCursor(0,1);
        lcd.print("DIGITO DIA:");
        key = kpd.waitForKey();
        if(key) { 
          lcd.print(key);
          tiempo[0]=key;
        }else{}
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("INGRESA SEGUNDO");
        lcd.setCursor(0,1);
        lcd.print("DIGITO DIA:");
        key = kpd.waitForKey();
        if(key) { 
          lcd.print(key);
          tiempo[1]=key;
        }else{}
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("USTED INGRESO:");
        lcd.setCursor(0,1);
        lcd.print(tiempo[0]);
        lcd.print(tiempo[1]);
        dia +=(tiempo[0]);
        dia +=(tiempo[1]);
        //############################################# Mes
        lcd.clear();
        lcd.setCursor(0,0); 
        lcd.print("INGRESA PRIMER");
        lcd.setCursor(0,1);
        lcd.print("DIGITO MES:");
        key = kpd.waitForKey();
        if(key) { 
          lcd.print(key);
          tiempo[0]=key;
        }else{}
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("INGRESA SEGUNDO");
        lcd.setCursor(0,1);
        lcd.print("DIGITO MES:");
        key = kpd.waitForKey();
        if(key) { 
          lcd.print(key);
          tiempo[1]=key;
        }else{}
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("USTED INGRESO:");
        lcd.setCursor(0,1);
        lcd.print(tiempo[0]);
        lcd.print(tiempo[1]);
        mes +=(tiempo[0]);
        mes +=(tiempo[1]);
       //############################################# Anio        
        lcd.clear();
        lcd.setCursor(0,0); 
        lcd.print("INGRESA TERCER");
        lcd.setCursor(0,1);
        lcd.print("DIGITO ANIO:");
        key = kpd.waitForKey();
        if(key) { 
          lcd.print(key);
          tiempo[0]=key;
        }else{}
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("INGRESA CUARTO");
        lcd.setCursor(0,1);
        lcd.print("DIGITO ANIO:");
        key = kpd.waitForKey();
        if(key) { 
          lcd.print(key);
          tiempo[1]=key;
        }else{}              
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("USTED INGRESO:");
        lcd.setCursor(0,1);
        lcd.print(tiempo[0]);
        lcd.print(tiempo[1]);
        anio="20";
        anio +=(tiempo[0]);
        anio +=(tiempo[1]);
        delay(4000);
        lcd.clear();
        lcd.setCursor(0,0); 
        lcd.print(hora);
        lcd.print(":");
        lcd.print(minu);
        lcd.print(":");
        lcd.print(segu);
        lcd.setCursor(0,1);
        lcd.print(dia);
        lcd.print(":");
        lcd.print(mes);
        lcd.print(":");
        lcd.print(anio);

        int horax=hora.toInt();
        int minux=minu.toInt();
        int segux=segu.toInt();
        int diax=dia.toInt();
        int mesx=mes.toInt();
        int aniox=anio.toInt();
        
        
        setTime(horax, minux, segux, diax ,mesx, aniox);     
        RTC.set(now());
        lcd.clear();

       
  }else{}
  
}


void lluvia() {
  // read the sensor on analog A0:
	int sensorReading = analogRead(snrlluvia);
  // map the sensor range (four options):
  // ex: 'long int map(long int, long int, long int, long int, long int)'
	int range = map(sensorReading, sensorMin, sensorMax, 0, 3);
  // range value:
  switch (range) {
 case 0:    // Sensor getting wet
 //   Serial.println("Flood");
    dataString="Flood";
    sdlog();
    break;
 case 1:    // Sensor getting wet
 //   Serial.println("Rain Warning");
     dataString="Rain Warning";
     sdlog();
    break;
 case 2:    // Sensor dry - To shut this up delete the " Serial.println("Not Raining"); " below.
    //Serial.println("Not Raining");
     //dataString="Not Raining";
    break;
  }
  delay(100);  // delay between reads
}

void sdlog(){
  
    // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  File dataFile = SD.open("datalog.txt", FILE_WRITE);


dataString = dataString + " // " + hour() + ":" + minute() + ":" + second() + " // " + day() + "-" + month() + "-" + year();

  // if the file is available, write to it:
  if (dataFile) {
    dataFile.println(dataString);
    dataFile.close();
    // print to the serial port too:
//    Serial.println(dataString);
  }
  // if the file isn't open, pop up an error:
  else {//Serial.println("error opening datalog.txt");
  }  
  
  
}
