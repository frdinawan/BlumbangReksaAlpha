#include <stdio.h>
#include "dht.h"
#include <GSM.h>
#include <Wire.h>
#include <Time.h>
#include "Timer.h"
#include <DS1307RTC.h>

#define DEBUG 1 //Comment this to turn on the debug mode

//GSM Shield's data
#define PINNUMBER ""
#define GPRS_APN       "indosatgprs" // replace your GPRS APN
#define GPRS_LOGIN     ""    // replace with your GPRS login
#define GPRS_PASSWORD  "" // replace with your GPRS password

#define PERIODUPDATE 300000

GSM blumbangGSMAccess;
GSM_SMS blumbangSMS;
GSMClient blumbangClient;
GPRS blumbangClientGPRS;
GSMScanner blumbangSignalStrength;

//variables for blumbangClient
String ID = "ZZ002";  //change this to match the server's ID
char serverName[] = "blumbangreksa.com";
char serverPage[] = "/server-side/editData.php";
int serverPort = 80;
String serverData;
int serverWriteState = 0;
bool sentFlag = false;
char g;

//sensor's data
float temperature = 20.0f;
float humidity = 0.0f;
float pH = 0.0f;
float DO = 0.0f;
long TDS = 0;
float salinity = 0.0f;

//================================================================
String bufferString;
String smsData;

unsigned long t;
unsigned long tForUpdate;

tmElements_t tm;
//Timer tLCDupper;
//Timer tLCDlower;

char serverResponseDebug = '0';
int counter;

struct Time
{
  int h;
  int m;
  int s;
};

struct Date
{
  int d;
  int m;
  int y;
};

Date DNow;
Time TNow;
Date DSaved;
Time TSaved;

void setup()
{
  String temp;
  unsigned char gsmFlag;
  
  //I this need to be deleted
  writeDate(__DATE__);
  writeTime(__TIME__);
  
  Serial.begin(9600);
  //int tickLCDUpper = tLCDupper.every(1000, blumbangLCDPrintUpper);
  //int tickLCDLower = tLCDlower.every(60000, blumbangLCDPrintLower);
  
  #ifdef DEBUG
    Serial.println("Debug mode is ON");
  #endif
  
  blumbangLCDButtonInit();
  
  blumbangLCDPrint(0, 0, "Init. . .       ");
  delay(1000);
  blumbangLCDPrint(0, 0, "Connecting. . . ");
  blumbangSignalStrength.begin();
  
  t = millis();
  blumbangLCDPrint(0, 0, "Init GSM . . .  ");
  blumbangLCDPrint(0, 1, "GSM Not Ready   "); 
  gsmFlag = 0;
  while(!blumbangGSMInit());
  blumbangLCDPrint(0, 1, "GSM Ready       ");
  gsmFlag = 1;
  t = millis() - t;
  temp = "GSM: " + String(t) + "ms\n";
  temp = temp + "NETWORK STATUS: " + String(gsmFlag);
  delay(1000); //Add this so you can see clearly the status on LCD
  
  #ifdef DEBUG
    Serial.println(temp);  //Check 3
  #endif

  t = millis();
  blumbangLCDClear();
  blumbangLCDPrint(0, 0, "Init GPRS . . . ");
  blumbangLCDPrint(0, 1, "GPRS Not Ready  ");
  while(!blumbangClientInit(serverName, serverPort));
  blumbangLCDPrint(0, 1, "GPRS Ready      ");
  t = millis() - t;
  temp = "Connect to GPRS: " + String(t) + "ms";
  delay(1000);

  #ifdef DEBUG
    Serial.println(temp);
    Serial.println("Input:");
  #endif
  
  blumbangSensorInit();

  t = millis();
  tForUpdate = millis();
  blumbangSensorECProbeSleep();
    
  TSaved.h = tm.Hour; TSaved.m = tm.Minute; TSaved.s = tm.Second;
  DSaved.d = tm.Day; DSaved.m = tm.Month; DSaved.y = tm.Year;
  blumbangLCDClear();
}

void loop()
{ 
  //tLCDupper.update();
  //tLCDlower.update();
  blumbangLCDPrintUpper();
  
  temperature = blumbangSensorDSTemperatureRead();
  if (millis() - t < 10000)
  {
    blumbangSensorECProbeWakeUp();delay(100);
    Serial1.print("T," + String(temperature) + "\r"); delay(100);  //compensating EC sensor
    bufferString = String("EC: ") + String(blumbangSensorECProbeSingleReading());
  }
  if ((millis() - t < 20000) && (millis() - t >= 10000))
  {
    blumbangSensorECProbeSleep();
    Serial2.print(String(temperature)+"\r"); delay(100);           //compensating PH sensor
    bufferString = String("pH: ") + String(blumbangSensorPHProbeSingleReading());
  }
  if ((millis() - t < 30000) && (millis() - t >= 20000))
  {
    blumbangSensorECProbeSleep();
    Serial3.print(String(temperature) + "," + blumbangSensorECRead() +"\r"); delay(100);
    bufferString = String("DO: ") + String(blumbangSensorDOProbeSingleReading());
  }
  if (millis() - t >= 30000)
  {
    t = millis();
  }
  blumbangLCDPrint(0, 1, bufferString);
  
  temperature = blumbangSensorDSTemperatureRead();
  humidity = blumbangSensorHumidityRead();
  DO = blumbangSensorDORead();
  pH = blumbangSensorPHRead();
  TDS = blumbangSensorTDSRead();
  salinity = blumbangSensorSalinityRead();
  
  smsData = "Time: " + String(tm.Hour) + ":" + String(tm.Minute) + ":" + String(tm.Second) + "\r\n" +
            "ID: " + ID + "\r\n" +
            "Temperature: " + String(temperature, 2) + "\r\n" +
            "Humidity: " + String(humidity, 2) + "\r\n" +
            "DO: " + String(DO, 2) + "\r\n" +
            "pH: " + String(pH, 2) + "\r\n" +            
            "TDS: " + String(TDS) + "\r\n" +
            "Salinity: " + String(salinity, 2);   
  
  serverData = "data_device_code=" + ID +
               "&data_temperature=" + String(temperature, 2) +
               "&data_humidity=" + String(humidity, 2) +
               "&data_DO=" + String(DO, 2) +
               "&data_pH=" + String(pH, 2) +
               "&data_TDS=" + String(TDS) +
               "&data_salinity=" + String(salinity, 2);

  blumbangGSMCheckSMS(smsData);
  
  TNow.h = tm.Hour; TNow.m = tm.Minute; TNow.s = tm.Second;
  DNow.d = tm.Day; DNow.m = tm.Month; DNow.y = tm.Year;
  
  g = Serial.read();
//  blumbangLCDPrint(0, 1, String(deltaTime()));
//  if(deltaTime() >= minToSec(2))  
  counter = 0;
  if(g == 'a' || (millis() - tForUpdate >= PERIODUPDATE))
  {               
    #ifdef DEBUG
      Serial.println("serverData: ");
      Serial.println(serverData);
    #endif

    while(1)
    {
//      sentFlag = blumbangClientPost(serverName, serverPort, serverPage, serverData);
      blumbangLCDPrint(0, 0, "Updating..  " + blumbangSignalCheck(blumbangSignalStrength.getSignalStrength()));
      sentFlag = blumbangClientPostBlockCall(serverName, serverPort, serverPage, serverData, 10000);
      if(sentFlag == true)
      {
        blumbangLCDPrint(0, 0, "Updated         ");
        #ifdef DEBUG
          Serial.println("Sent!");
        #endif
        delay(100);
        blumbangLCDClear();
        break;
      }
      else if(sentFlag == false)
      {
        blumbangLCDPrint(0, 0, "Not Updated     ");
        #ifdef DEBUG
          Serial.println("Not Sent!");
        #endif
        counter++;
      }
      if(counter >= 3) {
        blumbangLCDPrint(0, 0, "Reconnecting... "); 
        counter=0;
        blumbangGSMReconnecting();
        blumbangLCDClear(); 
        break;
      } //Try to resend 5 times
    }
    tForUpdate = millis();    
    g = 'd';
    
    TSaved.h = tm.Hour; TSaved.m = tm.Minute; TSaved.s = tm.Second;    
    DSaved.d = tm.Day; DSaved.m = tm.Month; DSaved.y = tm.Year;    
  }
}
