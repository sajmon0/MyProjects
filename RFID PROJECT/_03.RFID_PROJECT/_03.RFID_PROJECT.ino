/**
 * -----------------------------------------------------------------------------------------
 * MFRC522.h ==> https://github.com/miguelbalboa/rfid
 * -----------------------------------------------------------------------------------------
 *               MFRC522      Arduino       
 *               Reader/PCD   Zero           
 * Signal        Pin          Pin           
 * -----------------------------------------------------------------------------------------
 * RST/Reset     RST           9             
 * SPI SS        SDA(SS)      10            
 * IRQ           IRQ           6            
 * SPI MOSI      MOSI         ISP_MOSI  
 * SPI MISO      MISO         ISP_MISO   
 * SPI SCK       SCK          ISP_SCK   
 * -----------------------------------------------------------------------------------------
 */

#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN         9           // Configurable, see typical pin layout above
#define SS_PIN          10           // Configurable, see typical pin layout above
#define IRQ_PIN         6           

MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.
MFRC522::MIFARE_Key key;

volatile boolean irq_rfid_flag = false;

unsigned char regVal = 0x7F;

byte PK_ID[2]         = {0xC, 0xB};
byte RFID_ID[4];
byte TIME[6]          = {16, 10, 23, 19, 47, 23};
    
byte dataHttp [16]; // PK_ID[2], RFID_ID[4], TIME[6], 0000
//byte dataRfid [16] = { 0xFF, 66, 00, 15,   00, 23, 00,   00, 35, 00,   00, 47, 00,   00, 59, 00 }; 
byte dataRfid [16] = { 0xA, 0xB, 0xC, 0,   0, 0, 0,   0, 0, 0,   0, 0, 0,   0, 0, 0xFF }; 
byte clearData [16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
byte blockData [16]; 

byte blockAddr[45] = { 4,  5,  6,    8,  9, 10,   12, 13, 14,   16, 17, 18,   20, 21, 22,   
                      24, 25, 26,   28, 29, 30,   32, 33, 34,   36, 37, 38,   40, 41, 42,   
                      44, 45, 46,   48, 49, 50,   52, 53, 54,   56, 57, 58,   60, 61, 62 };

byte sector[15]    = { 1,  2,  3,    4,  5,  6,    7,  8,  9,   10, 11, 12,   13, 14, 15 };
        
byte trailer[15]   = { 7, 11, 15,   19, 23, 27,   31, 35, 39,   43, 47, 51,   55, 59, 63 };



void setup()  {
  Serial.begin(115200); // Initialize serial communications with the PC
  
  // initialize digital pin 13 as an output.
  pinMode(13, OUTPUT);
  digitalWrite(13, HIGH);   // turn the LED on (HIGH is the voltage level)    
 
//  mainSetup(); // Read parameters form eeprom  
//  gprsSetup();
//  bluetoothSetup();
//  ds3231Setup();
  rfidSetup();
//  rtcSetup();
//  watchdogSetup();
//  irqRfidSetup(); // IRQ Rfid Setup
} // end Setup 


void loop() {
  boolean status;

//      clearRfid();
//      Serial.println("Clear");

      if (rfidDetected()) {
        mfrc522.PICC_DumpToSerial(&(mfrc522.uid));
        Serial.println("OK!");
        confirmMessage();

      } else {
        
      }  
      
 //     delay(100);
      closeRfid();
    
      delay(20);
  
} // end loop


boolean readRfid(byte blockAddr, byte *blockData, byte bufferSize) {
  byte trailerBlock   = (blockAddr/4)*4 + 3;
  MFRC522::StatusCode status;
  byte buffer[18];
  byte size = sizeof(buffer);

  status = (MFRC522::StatusCode) mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {return false;}
  status = (MFRC522::StatusCode) mfrc522.MIFARE_Read(blockAddr, buffer, &size);
  if (status != MFRC522::STATUS_OK) {return false;}
  
  byte i;
  blockData[i] = buffer[i];   
  return true;
}



boolean writeRfid(byte blockAddr, byte *blockData, byte bufferSize) {
  byte trailerBlock   = (blockAddr/4)*4 + 3;
  MFRC522::StatusCode status;
  byte buffer[18];
  byte size = sizeof(buffer);

  status = (MFRC522::StatusCode) mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK) {return false;}
  status = (MFRC522::StatusCode) mfrc522.MIFARE_Write(blockAddr, blockData, bufferSize);
  if (status != MFRC522::STATUS_OK) {return false;}
  return true;
}











boolean rfidClear() {
  // Look for new cards
  if ( ! mfrc522.PICC_IsNewCardPresent())
    return false;

  // make sure nuid has been read
  if ( ! mfrc522.PICC_ReadCardSerial())
    return false;

  for (byte i = 0; i < 4; i++) {
    RFID_ID[i] = mfrc522.uid.uidByte[i];
  }
  
  boolean status;    
  byte i = 0;
  byte blockAddrId = 0; // Block data index
  boolean foundBlockAddrId = false;
  
  do{
    status = writeRfid(blockAddr[i], clearData, 16);
    if (! status) {return false;}    
    i++;
  } while (i != 45);
  return true;


} // end rfidClear




































boolean rfidDetected() {
  // Look for new cards
  if ( ! mfrc522.PICC_IsNewCardPresent())
    return false;

  // make sure nuid has been read
  if ( ! mfrc522.PICC_ReadCardSerial())
    return false;

  for (byte i = 0; i < 4; i++) {
    RFID_ID[i] = mfrc522.uid.uidByte[i];
  }
  
  boolean status;    
  byte i = 0;
  byte blockAddrId = 0; // Block data index
  boolean foundBlockAddrId = false;
  
  do{
    if(i == 45){
      Serial.println("CARD IS FULL!");  
      return false; // Rfid card full and PK_ID not found, probably error      
    }
    
    // If PK_ID was found on rfid card or first empty block save blockData index in blockDataId   
    if(readRfid(blockAddr[i], blockData, 16)){
        if((blockData[0] == PK_ID[0]) && (blockData[1] == PK_ID[1]) || (blockData[0] == 0) && (blockData[1] == 0)){
          blockAddrId = i; 
          foundBlockAddrId = true;
        }      
    }else {
      return false; // Read data from rfid card error
    }
   
    i++;
  } while (! foundBlockAddrId);
  
  status = writeRfid(blockAddr[blockAddrId], dataRfid, 16);
  if (! status) {return false;}
  return true;


} // end rfidDetected




/*
//;lihfgjsopigjsoigjsoifjpid[jsfpsdkjflsdkjfl;ksdfl;ksdl;'fk'sdl;fk'l;dskf;lsdkf;ldkfl;'ksd;lksd;lfks;lk
void loop() {
   
   if(bNewInt){  //new read interrupt
      Serial.print("Interrupt. ");
      mfrc522.PICC_ReadCardSerial(); //read the tag data
      // Show some details of the PICC (that is: the tag/card)
      Serial.print(F("Card UID:"));
      debug_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);
      Serial.println();
      delay(100);
      clearInt(mfrc522);
      mfrc522.PICC_HaltA();
      bNewInt = false;
   }

// The receiving block needs regular retriggering (tell the tag it should transmit??)
// (mfrc522.PCD_WriteRegister(mfrc522.FIFODataReg,mfrc522.PICC_CMD_REQA);)
   activateRec(mfrc522);
   delay(100);
} //loop()
//;lihfgjsopigjsoigjsoifjpid[jsfpsdkjflsdkjfl;ksdfl;ksdl;'fk'sdl;fk'l;dskf;lsdkf;ldkfl;'ksd;lksd;lfks;lk
*/




/*
//@
 //@ Read parameters from uC eeprom memory
  //@
void mainSetup() {
 
} // end mainSetup

//@
 //@ Setup GPRS
  //@
void gprsSetup() {
 
} // end gprsSetup

//@
 //@ Setup Bluetooth
  //@
void bluetoothSetup() {
 
} // end bluetoothSetup

//@
 //@ Setup Watchdog, reset uC if there was an infinity loop error
  //@
void watchdogSetup() {
 
} // end watchdogSetup
*/








//@
 //@ Confirm Message
  //@
void confirmMessage() {
  digitalWrite(13, LOW);    // turn the LED off by making the voltage LOW
  delay(1000);
  digitalWrite(13, HIGH);   // turn the LED on (HIGH is the voltage level)
} // end confirmMessage



//&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&//
//&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&//
//&&&&&&&&&&&&&&&&&&&&&& Start DS3231 and uC RTC Functions &&&&&&&&&&&&&&&&&&&&&&//
//&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&//
//&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&//
//@
 //@ Setup DS3231 RTC
  //@
void ds3231Setup() {
   
} // end ds3231Setup


//@
 //@ Read time from ds3231 and setup uC RTC
  //@
void rtcSetup() {
 
} // end rtcSetup
//&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&//
//&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&//
//&&&&&&&&&&&&&&&&&&&&&&& End DS3231 and uC RTC Functions &&&&&&&&&&&&&&&&&&&&&&&//
//&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&//
//&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&//



//###############################################################################//
//###############################################################################//
//########################### START RFID FUNCTIONS ##############################//
//###############################################################################//
//###############################################################################//






//-------------------------------------------------------------------------------//
//------------------------- Start Data Rfid Functions ---------------------------//
//-------------------------------------------------------------------------------//
//@
 //@ End Rfid transmission
  //@
void closeRfid() {
  mfrc522.PICC_HaltA(); // Halt PICC
  mfrc522.PCD_StopCrypto1(); // Stop encryption on PCD
} // end closeRfid
//-------------------------------------------------------------------------------//
//-------------------------- End Data Rfid Functions ----------------------------//
//-------------------------------------------------------------------------------//


//-------------------------------------------------------------------------------//
//------------------------ Start RFID Setup Functions ---------------------------//
//-------------------------------------------------------------------------------//
//@
 //@ Setup rfid reader
  //@
void rfidSetup()
{
  SPI.begin(); // Init SPI bus
  mfrc522.PCD_Init();
  mfrc522.PCD_SetAntennaGain(mfrc522.RxGain_48dB); // RxGain_33dB RxGain_38dB RxGain_43dB RxGain_48dB

  // Default keyA & keyB setup 0xFF
  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }
}  // end rfidSetup

//@
 //@ Setup interrupt from RFID 
  //@
void irqRfidSetup()  { 
  pinMode(IRQ_PIN, INPUT_PULLUP); // Setup the IRQ pin
   
  regVal = 0xA0; // RX irq mfrc522 register setup
  mfrc522.PCD_WriteRegister(mfrc522.ComIEnReg,regVal); // Send value to the interrupt register

  attachInterrupt(IRQ_PIN, irqRfid, FALLING); // Activate the interrupt

  irq_rfid_flag = false; // Clear interrupt flag  
} // end irqSetup
//-------------------------------------------------------------------------------//
//------------------------- End RFID Setup Functions ----------------------------//
//-------------------------------------------------------------------------------//


//-------------------------------------------------------------------------------//
//---------------------- Start RFID Interrupt Functions -------------------------//
//-------------------------------------------------------------------------------//
//@
 //@ The function sending to the MFRC522 the needed commands to activate the reception
  //@
void irqRfid() {
  irq_rfid_flag = true; // Clear interrupt flag
}

//@ 
 //@ The function sending to the MFRC522 the needed commands to activate the reception
  //@
void activateRec(MFRC522 mfrc522) {
  mfrc522.PCD_WriteRegister(mfrc522.FIFODataReg,mfrc522.PICC_CMD_REQA);
  mfrc522.PCD_WriteRegister(mfrc522.CommandReg,mfrc522.PCD_Transceive);  
  mfrc522.PCD_WriteRegister(mfrc522.BitFramingReg, 0x87);    
}

//@
 //@ The function to clear the pending interrupt bits after interrupt serving routine
  //@
void clearInt(MFRC522 mfrc522) {
  mfrc522.PCD_WriteRegister(mfrc522.ComIrqReg,0x7F); // Clear interrupt in rfid register
}
//-------------------------------------------------------------------------------//
//------------------------- End RFID Interrupt Functions ------------------------//
//-------------------------------------------------------------------------------//


//-------------------------------------------------------------------------------//
//-------------------------- Start RFID Debug Functions -------------------------//
//-------------------------------------------------------------------------------//
void debug_dataRfid () {
    Serial.print("Data Rfid:  ");
    for (byte i = 0; i < 2; i++) {
        Serial.print((char)dataRfid[i]);  // Print PK ID: A B
        Serial.print(' ');
    }
    for (byte i = 2; i < 16; i++) {
        Serial.print(dataRfid[i], HEX);  // Print Card ID: A B C D 
        Serial.print(' ');        
    }
    Serial.println();
}

void debug_dataHttp () {
    Serial.print("PK ID:      ");
    for (byte i = 0; i < 2; i++) {
        Serial.print((char)dataHttp[i]);  // Print PK ID: A B
        Serial.print(' ');
    }
    Serial.println();
    
    Serial.print("Rfid ID:    ");
    for (byte i = 2; i < 6; i++) {
        Serial.print(dataHttp[i], HEX);  // Print Card ID: A B C D 
        Serial.print(' ');        
    }
    Serial.println();
    
    Serial.print("Time:       ");
    for (byte i = 6; i < 9; i++) {
        Serial.print(dataHttp[i], HEX);  // Print Date: YY MM DD
          if ((i == 6)|(i == 7)) Serial.print('/');
    }
    Serial.print(' ');
    for (byte i = 9; i < 12; i++) {
        Serial.print(dataHttp[i], HEX);  // Print Time: HH MM SS
          if ((i == 9)|(i == 10)) Serial.print(':');
    }
    Serial.println();

    Serial.print("Data Http:  ");
    for (byte i = 0; i < 2; i++) {
        Serial.print((char)dataHttp[i]);  // Print PK ID: A B
        Serial.print(' ');
    }
    
    for (byte i = 2; i < 16; i++) {
        Serial.print(dataHttp[i], HEX);  // Print Card Rest 
        Serial.print(' ');        
    }
    Serial.println();
}

void debug_byte_array(byte *buffer, byte bufferSize) { 
    for (byte i = 0; i < bufferSize; i++) {
        Serial.print(buffer[i] < 0x10 ? " 0" : " ");
        Serial.print(buffer[i], HEX);
    }
    Serial.println();
}

void debug_start () {
    Serial.println();
    Serial.println("************************ Start Debug ************************");
    Serial.println();
}  

void debug_end () {
    Serial.println();
    Serial.println("************************* End Debug *************************");
    Serial.println();
}
//-------------------------------------------------------------------------------//
//-------------------------- End RFID Debug Functions ---------------------------//
//-------------------------------------------------------------------------------//


//###############################################################################//
//###############################################################################//
//############################ End RFID Functions ###############################//
//###############################################################################//
//###############################################################################//


//*******************************************************************************//
//*******************************************************************************//
//*******************************************************************************//
//*******************************************************************************//
//******************************** End Project **********************************//
//*******************************************************************************//
//*******************************************************************************//
//*******************************************************************************//
//*******************************************************************************//
