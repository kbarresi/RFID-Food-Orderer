#include <SPI.h>         // needed for Arduino versions later than 0018
#include <LiquidCrystal.h>

//digital pin designations
#define RFID_ENABLE 2

byte checkRFID(void);        //check for an RFID card, and compare against hardcoded master keys
byte buttonCtl(void);
void sendPacket(void);
int priceSelect(void);

void moneyPacketAdd(int code);
void restPacketAdd(int code);

void getPacket(void);
void parseInPacket(void);
void addPacketFood(int selection);
int mealSelect(void);

void hardReset(void);

char packetBuffer[27];
char inPacketBuffer[54];

char restaurantCode = 0;

char option0[16], option1[16], option2[16];

LiquidCrystal lcd(9, 8, 6, 5, 4, 3);

void setup() 
{
  //set up the pinmodes for various pins
  pinMode(RFID_ENABLE, OUTPUT);

  Serial.begin(2400);
  digitalWrite(RFID_ENABLE, LOW);    //enable to RFID reader


  for (int i = 0; i < 16; i++)
  {
    option0[i] = 0;
    option1[i] = 0;
    option2[i] = 0;
  }

  lcd.begin(16, 2);
  lcd.print("-RFID Menu Scan-");
}


void loop() 
{
  if (checkRFID())
  {
    Serial.end();
    lcd.clear();
    switch (restaurantCode)
    {
    case 0:
      break;   //shouldn't get here...
    case 1:
      {
        lcd.clear();
        lcd.print("~Chipotle Menu");
      }
    default:
      break;
    }
    delay(1000);
    char select = priceSelect();
    lcd.clear();
    lcd.print("Selected: ");
    switch (select)
    {
    case 0:
      {
        lcd.print("$1-5");
        break;
      }
    case 1:
      {
        lcd.print("$5-10");
        break;
      }
    case 2:
      {
        lcd.print("$10-15");
        break;
      }
    case 3:
      {
        lcd.print("$15+up");
        break;
      }
    default:
      break; //shouldnt get here;
    }
    delay(1000);
    lcd.clear();
    lcd.print("at Chipotle");
    delay(2000);
    moneyPacketAdd(select);
    restPacketAdd(restaurantCode);
    sendPacket();
    lcd.clear();
    lcd.print("Request sent!");
    getPacket();
    parseInPacket();
    lcd.clear();
    int mealSelection = mealSelect();
    if (mealSelection == 100)
    {
      lcd.clear();
      lcd.print("Com Errors!");
      hardReset();
    }
    addPacketFood(mealSelection);
    sendPacket();
    lcd.clear();
    lcd.print("Order sent!");
    delay(3000);
    lcd.clear();
    lcd.print("-RFID Menu Scan-");
    digitalWrite(RFID_ENABLE, LOW);
    Serial.begin(2400);
    Serial.flush();
  }
  delay(500);
}

/////////////////////////////////////////////
/////////////////////////////////////////////

byte checkRFID(void)
{
  Serial.flush();
  int  val = 0;    
  char readBuffer[10] = { 
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0     };
  int bytesread = 0;
  byte chipotle[10] = { 
    '0', '4', '1', '5', 'E', '9', 'C', '4', '1', '1'     };
  if(Serial.available() > 0) 
  {          // if data available from reader 
    if((val = Serial.read()) == 10) 
    {   // check for header 
      bytesread = 0; 
      while(bytesread<10) 
      {              // read 10 digit code 
        if( Serial.available() > 0) 
        { 
          val = Serial.read(); 
          if((val == 10)||(val == 13)) 
          { // if header or stop bytes before the 10 digit reading 
            break;                       // stop reading 
          } 
          readBuffer[bytesread] = val;         // add the digit           
          bytesread++;                   // ready to read next digit  
        } 
      }
      if(bytesread == 10) 
      {
        boolean restCorrect = true;

        for (int checkPosition = 0; checkPosition < 10; checkPosition++)

        {

          if (readBuffer[checkPosition] == chipotle[checkPosition] && restCorrect == true) 
          {   
            restCorrect = true; 
            restaurantCode = 0;
          } 
          else 
          {
            restCorrect = false; 
            restaurantCode = 1;
          }
        } 
        if (restCorrect)
        {
          digitalWrite(RFID_ENABLE, HIGH);
          return 1;
        }
        else 
        {
          digitalWrite(RFID_ENABLE, HIGH);
          return 0;
        }        
      } 
      digitalWrite(RFID_ENABLE, HIGH);                  // deactivate the RFID reader for a moment so it will not flood          
    }
    return 0;
  } 
  return 0;
}

byte buttonCtl(void)
{
  //we want this to run until the user makes a choice
  while(1)
  {
    int readVal = analogRead(0);

    if ((readVal > 270) && (readVal < 280))
    {
      delay(100);
      return 0;
    }
    else if((readVal > 130) && (readVal < 140))
    {
      return 1;
    }
    else if ((readVal > 220) && (readVal < 230))
    {
      delay(100);
      return 2;
    }
    delay(100);
  }
}

int priceSelect(void)
{
  char currentSelect = 0;
  lcd.clear();
  lcd.print("Select a price range");
  for (int positionCounter = 0; positionCounter < 7; positionCounter++)
  {
    lcd.scrollDisplayLeft(); 
    lcd.scrollDisplayLeft(); 
    lcd.scrollDisplayLeft(); 
    delay(500);
  }
  while(1)
  {
    if (currentSelect < 0)
    {
      currentSelect = 0;
    }
    if (currentSelect > 3)
    {
      currentSelect = 3;
    }
    switch (currentSelect)
    {
    case 0:
      {
        lcd.print("~$1 - $5     ->");
        break;
      }
    case 1:
      {
        lcd.print("~$5 - $10   <->");
        break;
      }
    case 2:
      {
        lcd.print("~$10 - $15  <->");
        break;
      }
    case 3:
      {
        lcd.print("~$15 +up    <- ");
        break;
      }
    default:
      break; //shouldnt get here
    }
    char response = buttonCtl();
    delay(250);
    switch (response)
    {
    case 0:
      {
        currentSelect--;
        lcd.clear();
        break;
      }
    case 1:
      {
        currentSelect++;
        lcd.clear();
        break;
      }
    case 2:
      {
        lcd.clear();
        return currentSelect;
        break;
      }
    default:
      {
        break; //shoudlnt get here
      }
    }
  }
}

void sendPacket()
{
  Serial.begin(2400);
  for (int i = 0; i < 27; i++)
  {
    Serial.print(packetBuffer);
  }
  Serial.flush();
  Serial.end();
  for (int i = 0; i < 27; i++)
  {
    packetBuffer[i] = 0;
  }
}

void moneyPacketAdd(int code)
{
  char price0[] = "price=|1-5|__";
  char price1[] = "price=|5-10|_";

  char price2[] = "price=|10-15|";
  char price3[] = "price=|15up|_";

  switch (code)
  {
  case 0:
    {
      for (int i = 0; i < 13; i++)
      {
        packetBuffer[i] = price0[i];
      }
      break;
    }
  case 1:
    {
      for (int i = 0; i < 13; i++)
      {
        packetBuffer[i] = price1[i];
      }
      break;
    }
  case 2:
    {
      for (int i = 0; i < 13; i++)
      {
        packetBuffer[i] = price2[i];
      }
      break;
    }
  case 3:
    {
      for (int i = 0; i < 13; i++)
      {
        packetBuffer[i] = price3[i];
      }
      break;
    }    
  default:
    break;
  }
}

void restPacketAdd(int code)
{
  char price0[] = "rest=*chipotle*_";
  char price1[] = "rest=*qdoba*____";
  char price2[] = "rest=*chilis*___";
  char price3[] = "rest=*somethin*_";

  switch (code)
  {
  case 0:
    {
      for (int i = 0; i < 13; i++)
      {
        packetBuffer[i+13] = price0[i];
      }
      break;
    }
  case 1:
    {
      for (int i = 0; i < 13; i++)
      {
        packetBuffer[i+13] = price1[i];
      }
      break;
    }
  case 2:
    {
      for (int i = 0; i < 13; i++)
      {
        packetBuffer[i+13] = price2[i];
      }
      break;
    }
  case 3:
    {
      for (int i = 0; i < 13; i++)
      {
        packetBuffer[i+13] = price3[i];
      }
      break;
    }    
  default:
    break;
  }
  packetBuffer[27] = '\n';
}

void getPacket(void)
{
    char index = 0;
    Serial.begin(2400);
    if (Serial.available() > 0)
    {
      while (Serial.available() && (index < 54))
      {
        inPacketBuffer[index++] = Serial.read();
	}
	inPacketBuffer[index++] = '\0';
    }
 
}


void parseInPacket(void)
{
  char temp[16];
  int tempIndex = 0;
  int messageIndex = 0;
  char open = 0;
  for (int i = 0; i < 54; i++)
  {
    char holder = inPacketBuffer[i];
    if (holder == '&')  //matched a value container
    {
      if (open == 0)  //its the opening brace
      {
        open = 1;
      }
      else  //this is the closing brace
      {
        open = 0;
        for (int j = 0; j < 16; j++)
        {
          switch (messageIndex)
          {
          case 0:
            {
              option0[j] = temp[j];
              break;
            }
          case 1:
            {
              option1[j] = temp[j];
              break;
            }
          case 2:
            {
              option2[j] = temp[j];
              break;
            }
          }
        }
        messageIndex++;
      }
    }
    else
    {
      if (open == 1)  //inside of a bracket
      {
        temp[tempIndex] = inPacketBuffer[i];  //copy it to the thing
        tempIndex++;
      }
    }
  }
  for (int i = 0; i < 54; i++)
  {
    inPacketBuffer[i] = 0;
  }
}

int mealSelect(void)
{
  byte tempCounter = 0;
  for (int i = 0; i < 16; i++)
  {
    if (option0[i] || option1[i] || option2[i])
    {
      tempCounter++;
    }
  }
  if (!tempCounter)
  {
    lcd.clear();
    lcd.print("No response...");
    delay(1000);
    return 100;
  }
  char currentSelect = 0;
  lcd.clear();
  lcd.print("Select a meal");
  for (int positionCounter = 0; positionCounter < 5; positionCounter++)
  {
    lcd.scrollDisplayLeft(); 
    lcd.scrollDisplayLeft(); 
    lcd.scrollDisplayLeft(); 
    delay(500);
  }
  while(1)
  {
    if (currentSelect < 0)
    {
      currentSelect = 0;
    }
    if (currentSelect > 2)
    {
      currentSelect = 2;
    }
    switch (currentSelect)
    {
    case 0:
      {
        lcd.print(option0);
        break;
      }
    case 1:
      {
        lcd.print(option1);
        break;
      }
    case 2:
      {
        lcd.print(option2);
        break;
      }
    default:
      break; //shouldnt get here
    }
    char response = buttonCtl();
    delay(250);
    switch (response)
    {
    case 0:
      {
        currentSelect--;
        lcd.clear();
        break;
      }
    case 1:
      {
        currentSelect++;
        lcd.clear();
        break;
      }
    case 2:
      {
        lcd.clear();
        return currentSelect;
        break;
      }
    default:
      {
        break; //shoudlnt get here
      }
    }
  }
}

void addPacketFood(int selection)
{
  char header[] = "meal=*";
  for (int i = 0; i < 6; i++)
  {
    packetBuffer[i] = header[i];
  }
  packetBuffer[7] = selection;
  packetBuffer[8] = '*';
  packetBuffer[9] = '\n';
}

void hardReset()
{
        asm volatile ("  jmp 0");
}
