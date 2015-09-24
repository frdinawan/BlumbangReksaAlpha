#include <LiquidCrystal.h> // Header untuk LCD
#include <EEPROM.h>

// Koneksi LCD:
LiquidCrystal blumbangLCD(5, 6, 8, 11, 12, 13);
unsigned char blumbangLCDLedPin = 4;

//Push Button
unsigned char blumbangButton[4] = {0, 1, A0, A1};
char blumbangIDCharacter[5];

char flagPool = 0;
char tombol = 0;

#define BUTTON_DELAY_LONG 250

/** @brief Initialization LCD and Button
 *
 * Function to initialization
 * - Turn ON LCD Led
 * - Init LCD
 * - Pull-Up Button
 */
void blumbangLCDButtonInit()
{
  unsigned char i; 
#ifdef DEBUG
  Serial.println();
  Serial.println("--------------------");
  Serial.println("blumbangLCDButtonInit()");
#endif
  pinMode(blumbangLCDLedPin, OUTPUT);
  digitalWrite(blumbangLCDLedPin, LOW);
 
  blumbangLCD.begin(16, 2);
//  blumbangLCD.print("<Blumbang Reksa>");
  
  for(i = 0; i < 4; i++)
  {
    pinMode(blumbangButton[i], INPUT_PULLUP);
  }
  
#ifdef DEBUG
  Serial.println("--------------------");
  Serial.println();
#endif
}

//Modification by Ataka, 1 February 2015
unsigned char blumbangLCDCheckButton()
{
  unsigned char FlagLCD;
  unsigned char i;

  for(i = 0; i < 4; i++)
  {
    if(digitalRead(blumbangButton[i])==LOW)
    {
      delay(BUTTON_DELAY_LONG);
      
      FlagLCD=1;
      if(i==0)tombol++;
      else if(i==1)tombol--;
      else tombol++;
      
      if(tombol>4)tombol=0;
      else if(tombol<0)tombol=4;
      
      blumbangLCD.clear();
    }
    else FlagLCD = 0; //biar balik 0.Aku edit ya mas Ataka.   
  }

  return FlagLCD;
}

//Modification by Imad, 1 March 2015
void blumbangLCDDisplay(){
  static int menu = 0;
  int hapus;
  hapus++;
  if(hapus>5000){
    hapus=0;
    blumbangLCD.clear();
  }
  
  if(digitalRead(blumbangButton[0])==LOW)
  {
    delay(BUTTON_DELAY_LONG);
    if(menu == 4)
      menu = 0;
    else
      menu++;
  }
  else if(digitalRead(blumbangButton[1])==LOW)
  {
    delay(BUTTON_DELAY_LONG);
    if(menu == 0)
      menu = 4;
    else
      menu--;
  }
  
  blumbangLCD.setCursor(0,0);
  blumbangLCD.print("ID: " +String( ID));
  blumbangLCD.setCursor(0,1);
  switch(menu)
  {
    case 0:
      blumbangLCD.print("Temp.: " + String(temperature, 2));  
    break;
    case 1:
      blumbangLCD.print("Humidity: " + String(humidity, 2));
    break;
    case 2:
      blumbangLCD.print("DO: " + String(DO, 2));
    break;
  //  case 3:
    //  blumbangLCD.print("pH: " + String(pH, 2)); 
    break;
    case 3:
      blumbangLCD.print("TDS: " + String(TDS, 2));
    break;
    case 4:
      blumbangLCD.print("Salinity: " + String(salinity, 2));
    break;
  }
}

String blumbangIDSetting(String InputString)
{
  String complete;
  unsigned char in;
  
  for(in=0; in<5; in++)
  {
    blumbangIDCharacter[in]=InputString.charAt(in);
  }
  flagPool = InputString.charAt(5);

  
  if(digitalRead(blumbangButton[0]) == LOW)
  {
      delay(BUTTON_DELAY_LONG);
      flagPool++;
      if(flagPool > 4) flagPool = 0;
  }
  else if(digitalRead(blumbangButton[1]) == LOW)
  {
    delay(BUTTON_DELAY_LONG);
      flagPool--;
      if(flagPool < 0) flagPool = 4;
  }
  else if(digitalRead(blumbangButton[2]) == LOW)
  {
    delay(BUTTON_DELAY_LONG);
      switch(flagPool)
      {
      case 0:
        blumbangIDCharacter[0] = blumbangIDCharacter[0]+1;
        if(blumbangIDCharacter[0] > 90) blumbangIDCharacter[0] = 65;
      break;
      case 1:
        blumbangIDCharacter[1]++;
        if(blumbangIDCharacter[1] > 90) blumbangIDCharacter[1] = 65;
        break;
      case 2:
        blumbangIDCharacter[2]++;
        if(blumbangIDCharacter[2] > 57) blumbangIDCharacter[2] = 48;
        break;
      case 3:
        blumbangIDCharacter[3]++;
        if(blumbangIDCharacter[3] > 57) blumbangIDCharacter[3] = 48;
        break;
      case 4:
        blumbangIDCharacter[4]++;
        if(blumbangIDCharacter[4] > 57) blumbangIDCharacter[4] = 48;
        break;
      }
      
  }
    else if(digitalRead(blumbangButton[3]) == LOW)
  {
    delay(BUTTON_DELAY_LONG);
      switch(flagPool)
      {
      case 0:
        blumbangIDCharacter[0]--;
        if(blumbangIDCharacter[0] < 65) blumbangIDCharacter[0] = 90;
      break;
      case 1:
        blumbangIDCharacter[1]--;
        if(blumbangIDCharacter[1] < 65) blumbangIDCharacter[1] = 90;
        break;
      case 2:
        blumbangIDCharacter[2]--;
        if(blumbangIDCharacter[2] < 48) blumbangIDCharacter[2] = 57;
        break;
      case 3:
        blumbangIDCharacter[3]--;
        if(blumbangIDCharacter[3] < 48) blumbangIDCharacter[3] = 57;
        break;
      case 4:
        blumbangIDCharacter[4]--;
        if(blumbangIDCharacter[4] < 48) blumbangIDCharacter[4] = 57;
        break;
      }
      
  }
  
  for(in=0; in<5; in++)
  {
    EEPROM.write(21+in, blumbangIDCharacter[in]);
  }

  complete =   String(  String(blumbangIDCharacter[0]) +
                        String(blumbangIDCharacter[1]) +
                        String(blumbangIDCharacter[2]) +
                        String(blumbangIDCharacter[3]) +
                        String(blumbangIDCharacter[4]) +
                        String(flagPool));
  blumbangLCD.clear();
  blumbangLCD.setCursor(0, 0);
  blumbangLCD.print("Pool ID");
  blumbangLCD.setCursor(0,1);
  blumbangLCD.print(String(String(blumbangIDCharacter[0]) + String(blumbangIDCharacter[1]) + String(blumbangIDCharacter[2]) + String(blumbangIDCharacter[3]) + String(blumbangIDCharacter[4])));
  return complete;
}

void blumbangLCDClear()
{
  blumbangLCD.clear();
}

void blumbangLCDPrint(unsigned char x, unsigned char y, String s)
{
  blumbangLCD.setCursor(x, y);
  blumbangLCD.print(s);
}

int checkHang = 0;
void blumbangLCDPrintUpper()
{
  char blumbangGPRSStatus = ' ';
  if(blumbangClientGPRS.getStatus()==ERROR)
    blumbangGPRSStatus = 'E';
  else if(blumbangClientGPRS.getStatus()==IDLE)
    blumbangGPRSStatus = 'Y';
  else if(blumbangClientGPRS.getStatus()==CONNECTING)
    blumbangGPRSStatus = 'g';
  else if(blumbangClientGPRS.getStatus()==GPRS_READY)
    blumbangGPRSStatus = 'G';
  else if(blumbangClientGPRS.getStatus()==TRANSPARENT_CONNECTED)
    blumbangGPRSStatus = 'T';
  blumbangLCDPrint(0, 0, String(blumbangGPRSStatus) + blumbangSignalCheck(blumbangSignalStrength.getSignalStrength()));
}

void blumbangLCDPrintLower()
{
//  String words;
//  words = "T: " + String((int)temperature) + "| H: " + String((int)humidity);
//  blumbangLCDPrint(0, 1, words); delay(3000);
//  
//  words = "pH: " + String((int)pH) + "| DO: " + String((int)DO);
//  blumbangLCDPrint(0, 1, words); delay(3000);
//  
//  words = "TDS: " + String((int)TDS) + "| Sal: " + String((int)salinity);
//  blumbangLCDPrint(0, 1, words); delay(3000);
}
