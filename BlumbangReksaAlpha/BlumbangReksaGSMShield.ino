//This file is for GSM Shield functions

//Initialize GSM Shield
bool blumbangGSMInit(void)
{
  unsigned char i = 0;
  bool notConnected = true;
   
  #ifdef DEBUG
    Serial.println();
    Serial.println("--------------------");
    Serial.println("blumbangGSMInit()");
    Serial.println("--------------------");
  #endif

  while (notConnected)
  {
    if (blumbangGSMAccess.begin(PINNUMBER) == GSM_READY)
      notConnected = 0;
    else delay(1000);
  }
  
  
    
  return !notConnected;
}

//Initialize GPRS of GSM Shield
bool blumbangClientInit(char* server, int port)
{
  bool notConnected = true;
  
#ifdef DEBUG
  Serial.println();
  Serial.println("--------------------");
  Serial.println("blumbangClientInit()");
#endif

  while (notConnected)
  {
    if ((blumbangGSMAccess.begin(PINNUMBER) == GSM_READY) &&
        (blumbangClientGPRS.attachGPRS(GPRS_APN, GPRS_LOGIN, GPRS_PASSWORD) == GPRS_READY))
      notConnected = false;
    else delay(1000);
  }
  
  while(blumbangSMS.available()) //Sometimes error when there is an sms, so flush them out
    blumbangSMS.flush();
  
  return !notConnected;
}

void blumbangGSMReconnecting(void)
{
  int i;
  long t;
  
  t = millis();
  while(1)
  {
    if(blumbangGSMAccess.begin(PINNUMBER) == GSM_READY &&
            blumbangClientGPRS.attachGPRS(GPRS_APN, GPRS_LOGIN, GPRS_PASSWORD) == GPRS_READY)
      break;
  }
}

//Call this function to check signal strength
String blumbangSignalCheck(String in)
{
  int i, inStrength;
  String outBarSignal = "";
  char blockCharacter = 255;
  
  inStrength = in.toInt();
  inStrength = (inStrength + 1) / 8;  //convert [0-31] to [1-4]
  for(i=1;i<=inStrength;i++)
  {
    outBarSignal += String(blockCharacter);
  }
  for(i=1;i<=(4-inStrength);i++)
  {
    outBarSignal += "-";
  }
  outBarSignal += "\0";
  return outBarSignal;
}

//Call this function to post data to server
bool blumbangClientPost(char* server, int port, char* page, String data)
{
  unsigned char i;
  bool state = false;
  
  #ifdef DEBUG
    Serial.println();
    Serial.println("--------------------");
    Serial.println("blumbangClientPost()");
  #endif

  blumbangClient.flush();
  if (!blumbangClient.connected())
  {
    blumbangClient.stop();
  }
  for(i = 0; i < 3; i++)
  {
    if (blumbangClient.connect(server, port))
    {
      data = "POST " + String(page) + " HTTP/1.1" + "\r\n" + 
             "Host: " + String(server) + "\r\n" + 
             "Connection: close" + "\r\n" +
             "Content-Type: application/x-www-form-urlencoded" + "\r\n" +
             "User-Agent: Arduino" + "\r\n" +
             "Content-Length: " + String(data.length()) + "\r\n" +
             "\r\n" +
             String(data);
      blumbangClient.print(data);
      #ifdef DEBUG             
        Serial.println(data);
      #endif

      state = true;
      break;
    }
    else
        blumbangClient.stop();
  }     
  
  #ifdef DEBUG             
    if(!state)
      Serial.println("Cannot connect to Server!");
    Serial.println("--------------------");
    Serial.println();
  #endif

  return state;
}

//Call this to post data to server and get write status
bool blumbangClientPostBlockCall(char* server, int port, char* page, String data, unsigned long timeOut)
{
  unsigned long receivedTime;
  unsigned char stateMachine = 0;
  char receivedChar[4] = {' ', ' ', ' ', ' '};
  bool state = false;
  
  #ifdef DEBUG
    Serial.println();
    Serial.println("--------------------");
    Serial.println("blumbangClientPostBlockCall()");
  #endif

  if (blumbangClientPost(server, port, page, data))
  {
    receivedTime = millis();
    // Check if available
    while(!blumbangClient.available())
    {
      if(millis() - receivedTime > timeOut)
      {
        #ifdef DEBUG
          Serial.println("No Response Timeout!");
        #endif
        break;
      }
    }
    if(blumbangClient.available()){
      receivedTime = millis();
      while(1)
      {
        if(millis() - receivedTime > 100) break;
        if(blumbangClient.available())
        {
          receivedTime = millis();
          receivedChar[3] = receivedChar[2];
          receivedChar[2] = receivedChar[1];
          receivedChar[1] = receivedChar[0];
          receivedChar[0] = (char)blumbangClient.read();
          if(receivedChar[3] == 13 && receivedChar[2] == 10 && receivedChar[1] == 13 && receivedChar[0] == 10)
            stateMachine++;
          else if(receivedChar[1] == '?' && receivedChar[0] == '0')
            state = true;
          
          if(stateMachine == 2)
          {
            #ifdef DEBUG
              Serial.print("Server Response: ");
              Serial.print(receivedChar[0]);
              Serial.println(receivedChar[1]);
            #endif
            serverResponseDebug = receivedChar[1];
            break;
          }
        }
      }
    }
    blumbangClient.stop();
  }
  
  #ifdef DEBUG
    Serial.println("--------------------");
    Serial.println();
  #endif
  
  return state;
}

//Call this to check if there is a message
//If the incoming message is 'check', then send sensor data to the incoming number
void blumbangGSMCheckSMS(String sendData)
{
  char incomingNumber[20];
  char receivedText[200];
  String receivedString;
  
  char getSMSType = 0;
  char bufferText[200];
  String bufferString;
  
  #ifdef DEBUG
    Serial.println();
    Serial.println("--------------------");
    Serial.println("checkBlumbangSMS()");
  #endif

  if(blumbangSMS.available())
  {
    //Read text/number/position of sms
    blumbangSMS.remoteNumber(incomingNumber, 20);
    for(unsigned int i = 0; i < 200; i++)
    {
      char c = blumbangSMS.read();
      if(c == 0xFF)
        break;
      else
        receivedText[i] = c;
    }
    receivedString = String(receivedText);
    blumbangSMS.flush();

    #ifdef DEBUG
      Serial.println("Last SMS deleted");
    #endif
    
    #ifdef DEBUG
      Serial.println(incomingNumber);
      Serial.println(receivedText);
    #endif
    
    if(receivedString.equalsIgnoreCase("CHECK")) 
    {
      sendData.toCharArray(bufferText, sendData.length()+1);
      // send the message
      blumbangSMS.beginSMS(incomingNumber);
      blumbangSMS.print(bufferText);
      blumbangSMS.endSMS();
      
      #ifdef DEBUG
        Serial.println("CHECK Debug Response: ");
        Serial.println(bufferText);
      #endif
    }
    else if(receivedText[0] == '+' || receivedText[0] == '0') 
    {
      // send the message
      blumbangSMS.beginSMS(incomingNumber);
      blumbangSMS.print("Type: CHECK");
      blumbangSMS.endSMS();

      #ifdef DEBUG
        Serial.println("Wrong Command debug response: ");
        Serial.println("Type: CHECK");
      #endif
    }
    
    while (blumbangSMS.ready() == 0);
    blumbangSMS.flush();
  }
  
  #ifdef DEBUG
    Serial.println("--------------------");
    Serial.println();
  #endif
}
