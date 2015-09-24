#include <OneWire.h>

OneWire  ds(A8);  // on pin 10 (a 4.7K resistor is necessary)

float blumbangSensorDSTemperatureRead(void)
{
#ifdef DEBUG
  Serial.println();
  Serial.println("--------------------");
  Serial.println("blumbangSensorDSTemperatureRead()");
  Serial.println();
#endif

  byte i;
  byte present = 0;
  byte data[12];
  byte addr[8];
  float celsius, fahrenheit;
  
  if ( !ds.search(addr)) {
    ds.reset_search();
    delay(250);
  }
  
  ds.reset();
  ds.select(addr);
  ds.write(0x44, 1);
  
  delay(800);     // maybe 750ms is enough, maybe not
  // we might do a ds.depower() here, but the reset will take care of it.
  
  present = ds.reset();
  ds.select(addr);    
  ds.write(0xBE);         // Read Scratchpad

  for ( i = 0; i < 9; i++) {           // we need 9 bytes
    data[i] = ds.read();
  }

  // Convert the data to actual temperature
  // because the result is a 16 bit signed integer, it should
  // be stored to an "int16_t" type, which is always 16 bits
  // even when compiled on a 32 bit processor.
  int16_t raw = (data[1] << 8) | data[0];
 
  byte cfg = (data[4] & 0x60);
  // at lower res, the low bits are undefined, so let's zero them
  if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
  else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
  else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
  // default is 12 bit resolution, 750 ms conversion time
  
  celsius = (float)raw / 16.0;
  fahrenheit = celsius * 1.8 + 32.0;
  
#ifdef DEBUG
  Serial.print("  Temperature = ");
  Serial.print(celsius);
  Serial.print(" Celsius, ");
  Serial.print(fahrenheit);
  Serial.println(" Fahrenheit");
  Serial.println("--------------------");
  Serial.println();
#endif

  return (float)celsius;
}

dht DHT;
unsigned char DHTPin = A15;
void blumbangSensorHumidityInit(void)
{
#ifdef DEBUG
  Serial.println();
  Serial.println("--------------------");
  Serial.println("blumbangSensorHumidityInit()");
  Serial.println("--------------------");
  Serial.println();
#endif

  pinMode(DHTPin, INPUT);  
}

float blumbangSensorHumidityRead(void)
{
#ifdef DEBUG
  Serial.println();
  Serial.println("--------------------");
  Serial.println("blumbangSensorHumidityRead()");
  Serial.println("--------------------");
  Serial.println();
#endif

  DHT.read(DHTPin);
  return DHT.humidity;
}

/*------------------------------ Sensor DO -----------------------------------*/

float DOSaturationValue = 0.0f;
float DOValue = 0.0f;

String blumbangSensorDOProbeSingleReading(void)
{
  unsigned char charBufferIndex = 0;
  char charBuffer[30];
  static char *DOSaturationBuffer;
  static char *DOBuffer;
  String DOString;
 
#ifdef DEBUG
  Serial.println();
  Serial.println("--------------------");
  Serial.println("blumbangSensorDOProbeSingleReading()");
#endif
 
  while(Serial3.available())
    Serial3.read();
  Serial3.print("C\r");
  charBufferIndex = Serial3.readBytesUntil('\r', charBuffer, 30);
  charBuffer[charBufferIndex] = '\0';
  Serial3.print("E\r");
  
#ifdef DEBUG
  Serial.println(charBuffer);
#endif

  DOString = String(charBuffer);
  
  DOSaturationBuffer = strtok(charBuffer, ",\r");  
  DOBuffer = strtok(NULL, " ,\r"); //Yang terakhir "[space][comma][return car]"
  
  DOSaturationValue = atof(DOSaturationBuffer);
  DOValue = atof(DOBuffer);
  
#ifdef DEBUG
  Serial.println("--------------------");
  Serial.println();
#endif

  return DOString;
}

float blumbangSensorDOSaturationRead(void)
{
  return DOSaturationValue;
}

float blumbangSensorDORead(void)
{
  return DOValue;
}

void blumbangSensorDOProbeInit(void)
{
#ifdef DEBUG
  Serial.println();
  Serial.println("--------------------");
  Serial.println("blumbangSensorDOProbeInit()");
#endif

  Serial3.begin(9600);
  Serial3.setTimeout(2000);
  Serial3.print("E\r");
//  pinMode(15, INPUT_PULLUP);
  
#ifdef DEBUG
  Serial.println("--------------------");
  Serial.println();
#endif
}

/*----------------------------------------------------------------------------*/

/*------------------------------ Sensor pH -----------------------------------*/

float pHValue = 0.0f;

String blumbangSensorPHProbeSingleReading(void)
{
  unsigned char charBufferIndex = 0;
  char charBuffer[30];
  String pHString;
  
#ifdef DEBUG
  Serial.println();
  Serial.println("--------------------");
  Serial.println("blumbangSensorPHProbeSingleReading()");
#endif

  while(Serial2.available())
    Serial2.read();
  Serial2.print("R\r");
  charBufferIndex = Serial2.readBytesUntil('\r', charBuffer, 30);
  charBuffer[charBufferIndex] = '\0';
  
#ifdef DEBUG
  Serial.println(charBuffer);
#endif
  
  pHString = String(charBuffer);
  if('0' <= charBuffer[0] && charBuffer[0] <= '9')
    pHValue = atof(charBuffer);
  
  return pHString;
}

float blumbangSensorPHRead(void)
{
  return pHValue;
}

void blumbangSensorPHProbeInit(void)
{
#ifdef DEBUG
  Serial.println();
  Serial.println("--------------------");
  Serial.println("blumbangSensorPHProbeInit()");
#endif

  Serial2.begin(9600);
  Serial2.setTimeout(1000);
  Serial2.print("E\r");
//  pinMode(17, INPUT_PULLUP);
  
#ifdef DEBUG
  Serial.println("--------------------");
  Serial.println();
#endif
}

/*----------------------------------------------------------------------------*/

/*------------------------------ Sensor EC -----------------------------------*/

float ECValue = 0.0f;
long TDSValue = 0;
float salinityValue = 0.0f;
float specificGravityValue = 0.0f;

bool ECFlagSleep = false;

String blumbangSensorECProbeSingleReading(void)
{
  unsigned char charBufferIndex = 0;
  char charBuffer[30];
  char charResponse[5];
  static char *ECBuffer;
  static char *TDSBuffer;
  static char *salinityBuffer;
  static char *specificGravityBuffer;
  String ECString;
 
#ifdef DEBUG
  Serial.println();
  Serial.println("--------------------");
  Serial.println("blumbangSensorECProbeSingleReading()");
#endif
 
  while(Serial1.available())
    Serial1.read();
  Serial1.print("R\r");
  charBufferIndex = Serial1.readBytesUntil('\r', charBuffer, 30);
  charBuffer[charBufferIndex] = '\0';
  if(charBuffer[0] != '*')
  {
    charBufferIndex = Serial1.readBytesUntil('\r', charResponse, 5); //Dummy to remove *OK because using EZO board
    charResponse[charBufferIndex] = '\0';
  }
  
#ifdef DEBUG
  Serial.println(charBuffer);
  if(charResponse[0] == '*')
    Serial.println(charResponse);
#endif
  ECString = String(charBuffer) + String("\0");
  
  ECBuffer = strtok(charBuffer, ",\r");
  TDSBuffer = strtok(NULL, ",\r");
  salinityBuffer = strtok(NULL, ",\r");  
  specificGravityBuffer = strtok(NULL, " ,\r"); //Yang terakhir "[space][comma][return car]"
  
  ECValue = atof(ECBuffer);
  TDSValue = atol(TDSBuffer);
  salinityValue = atof(salinityBuffer);
  specificGravityValue = atof(specificGravityBuffer);
    
#ifdef DEBUG
  Serial.println("--------------------");
  Serial.println();
#endif

  ECFlagSleep = false;
  return ECString;
}

float blumbangSensorECRead(void)
{
  return ECValue;
}

long blumbangSensorTDSRead(void)
{
  return TDSValue;
}

float blumbangSensorSalinityRead(void)
{
  return salinityValue;
}

float blumbangSensorSpecificGravityRead(void)
{
  return specificGravityValue;
}

void blumbangSensorECProbeWakeUp()
{
  String getUp = "";
  char inputCh = '\0';
  Serial1.print("R\r");
    getUp = "";
  ECFlagSleep = false;
  delayMicroseconds(50000);
//  while(ECFlagSleep)
//  { 
//    while(1)
//    {
//      if(Serial1.available())
//      {
//        inputCh = (char)Serial1.read();
//        getUp += inputCh;
//        if(inputCh == '\r')
//          break;
//      }
//    }
//    getUp += "\0";
//    if(getUp.equals("*WA\r"))
//    {
//      ECFlagSleep = false;
//      break;
//    }
//  }
}

void blumbangSensorECProbeSleep(void)
{
  char bufferChar = '\0';
  String bufferString = "";
  
#ifdef DEBUG
    Serial.println();
    Serial.println("--------------------");
    Serial.println("blumbangSensorECProbeSleep()");
#endif
  
  while(!ECFlagSleep)
  {
    Serial1.print("B\r");
    while(1)
    {
      if(Serial1.available())
      {
        if(Serial1.read() == '\r')
          break;
      }
    }
    Serial1.print("SLEEP\r");
    bufferString = "";
    while(1)
    {
      if(Serial1.available())
      {
        bufferChar = (char)Serial1.read();
        bufferString += bufferChar;
        if(bufferChar == '\r')
          break;
      }
    }
    bufferString += "\0";
    if(bufferString.equals("*SL\r") || bufferString.equals("*OK\r"))
    {
      ECFlagSleep = true;
      break;
    }
#ifdef DEBUG
    Serial.println(bufferString);
#endif
  }

#ifdef DEBUG
  Serial.println(bufferString);
  Serial.println("--------------------");
  Serial.println();
#endif
}

void blumbangSensorECProbeInit(void)
{
#ifdef DEBUG
  Serial.println();
  Serial.println("--------------------");
  Serial.println("blumbangSensorECProbeInit()");
#endif
  
  Serial1.begin(38400);
  Serial1.setTimeout(1000);
  Serial1.print("C,0\r");
//  pinMode(19, INPUT_PULLUP);
  
#ifdef DEBUG
  Serial.println("--------------------");
  Serial.println();
#endif
}

/*-----------------------------------------------------------------*/

void blumbangSensorInit(void)
{
#ifdef DEBUG
  Serial.println();
  Serial.println("--------------------");
  Serial.println("blumbangSensorInit()");
#endif

//  blumbangSensorTemperatureInit();
  blumbangSensorHumidityInit();
  blumbangSensorDOProbeInit();
  blumbangSensorPHProbeInit();
  blumbangSensorECProbeInit();
  
#ifdef DEBUG
  Serial.println("--------------------");
  Serial.println();
#endif
}
