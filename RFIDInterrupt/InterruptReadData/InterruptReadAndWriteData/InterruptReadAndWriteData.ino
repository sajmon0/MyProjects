/**
 * ----------------------------------------------------------------------------
 * This is a MFRC522 library example; see https://github.com/miguelbalboa/rfid
 * for further details and other examples.
 * 
 * NOTE: The library file MFRC522.h has a lot of useful info. Please read it.
 * 
 * Released into the public domain.
 * ----------------------------------------------------------------------------
 * Minimal example how to use the interrupts to read the UID of a MIFARE Classic PICC
 * (= card/tag).
 * 
 * 
 * Typical pin layout used:
 * -----------------------------------------------------------------------------------------
 *             MFRC522      Arduino       Arduino   Arduino    Arduino          Arduino
 *             Reader/PCD   Uno           Mega      Nano v3    Leonardo/Micro   Pro Micro
 * Signal      Pin          Pin           Pin       Pin        Pin              Pin
 * -----------------------------------------------------------------------------------------
 * RST/Reset   RST          9             5         D9         RESET/ICSP-5     RST
 * SPI SS      SDA(SS)      10            53        D10        3                10
 * IRQ         ?            ?             ?         ?          2                10
 * SPI MOSI    MOSI         11 / ICSP-4   51        D11        ICSP-4           16
 * SPI MISO    MISO         12 / ICSP-1   50        D12        ICSP-1           14
 * SPI SCK     SCK          13 / ICSP-3   52        D13        ICSP-3           15
 * 
 */

#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN         9           // Configurable, see typical pin layout above
#define SS_PIN          10           // Configurable, see typical pin layout above
#define IRQ_PIN         6           

MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.



volatile boolean bNewInt = false;
unsigned char regVal = 0x7F;
void activateRec(MFRC522 mfrc522);
void clearInt(MFRC522 mfrc522);

byte nuidPICC[4];

MFRC522::MIFARE_Key key;
MFRC522::StatusCode status;

    // In this sample we use the second sector,
    char PK_ID[2]         = {'A', 'B'};
    byte RFID_ID[4];
    byte TIME[6]          = {16, 10, 23, 19, 47, 23};
    
    // PK_ID[2], RFID_ID[4], TIME[6], 0000
    byte DATA_HTTP [16];
    // PK_ID[2], TIME[6], 00000000
    byte DATA_RFID [16];
 /*   
    byte RFID_1K [SECTOR][SECTOR][TRAILER] = {
                                             {  
                                               {4, 5, 6}  
                                            
                                            
                                            }   
 */    
    
    byte sector[15]    = { 1,  2,  3,    4,  5,  6,    7,  8,  9,   10, 11, 12,   13, 14, 15 };
    byte blockAddr[45] = { 4,  5,  6,    8,  9, 10,   12, 13, 14,   16, 17, 18,   20, 21, 22,   
                          24, 25, 26,   28, 29, 30,   32, 33, 34,   36, 37, 38,   40, 41, 42,   
                          44, 45, 46,   48, 49, 50,   52, 53, 54,   56, 57, 58,   60, 61, 62 };
        
    byte trailer[15]  = { 7, 11, 15, 19, 23, 27, 31, 35, 39, 43, 47, 51, 55, 59, 63 };
    byte trailerBlock = trailer[0];
    
    boolean flag_rfid = true;
    boolean flag_rfid_auth_a = true;
    boolean flag_rfid_auth_b = true;    
    boolean flag_rfid_read = true;
    boolean flag_rfid_write = true;
    
    
    byte buffer[18];
    byte size = sizeof(buffer);





/**
 * Initialize.
 */
void setup() {
    Serial.begin(9600); // Initialize serial communications with the PC
    while (!Serial);    // Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)
    SPI.begin();        // Init SPI bus
    
    mfrc522.PCD_Init(); // Init MFRC522 card
    
    // Prepare the key (used both as key A and as key B)
    // using FFFFFFFFFFFFh which is the default at chip delivery from the factory
    for (byte i = 0; i < 6; i++) {
      key.keyByte[i] = 0xFF;
    }

    /* read and printout the MFRC522 version (valid values 0x91 & 0x92)*/
    Serial.print("Ver: 0x");      
    byte readReg = mfrc522.PCD_ReadRegister(mfrc522.VersionReg);
    Serial.println(readReg, HEX);      

    /* setup the IRQ pin*/
    pinMode(IRQ_PIN, INPUT_PULLUP);
   
    /* 
     *  Allow the ... irq to be propagated to the IRQ pin
     *  For test purposes propagate the IdleIrq and loAlert
     */
    regVal = 0xA0; //rx irq
    mfrc522.PCD_WriteRegister(mfrc522.ComIEnReg,regVal);
    
    bNewInt = false; //interrupt flag

    /*Activate the interrupt*/
    attachInterrupt(IRQ_PIN, readCard, FALLING);
    
/*    do{ //clear a spourious interrupt at start
      ;
    }while(!bNewInt);
*/ 
    Serial.println("End setup"); 
}

/**
 * Main loop.
 */
void loop() {      
  
   if(bNewInt){  //new read interrupt
             
/*               
   // Look for new cards
   if ( ! mfrc522.PICC_IsNewCardPresent())
   return;

   // Verify if the NUID has been readed
   if ( ! mfrc522.PICC_ReadCardSerial())
   return;
*/   
/*
//-----------------------------------   
    // Store RFID_ID into array
    for (byte i = 0; i < 4; i++) {
      RFID_ID[i] = mfrc522.uid.uidByte[i];
    }
    
    create_data_http(RFID_ID, TIME);
    create_data_rfid(TIME);
//--------------------------------------  
  
  if (mfrc522.uid.uidByte[0] != nuidPICC[0] || 
    mfrc522.uid.uidByte[1] != nuidPICC[1] || 
    mfrc522.uid.uidByte[2] != nuidPICC[2] || 
    mfrc522.uid.uidByte[3] != nuidPICC[3] ) {
    Serial.println(F("A new card has been detected."));

    // Store NUID into nuidPICC array
    for (byte i = 0; i < 4; i++) {
      nuidPICC[i] = mfrc522.uid.uidByte[i];
    }
   
    Serial.println(F("The NUID tag is:"));
    Serial.print(F("In hex: "));
    dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);
    Serial.println();
  }
  else 
  {
    Serial.println(F("Card read previously."));
    Serial.print(F("In hex: "));
    dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);
    Serial.println();
  } 
   
   
*/   
   
   
      Serial.println("Interrupt. ");
      Serial.println();
/*      mfrc522.PICC_ReadCardSerial(); //read the tag data
      // Show some details of the PICC (that is: the tag/card)
      //Serial.print(F("Card UID:"));
      dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);
      Serial.println();
*/    
//======================================================================================================================================
      status = (MFRC522::StatusCode) mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522.uid));
      if (status != MFRC522::STATUS_OK){
          flag_rfid = false;
          return;
      }else flag_rfid = true;

    status = (MFRC522::StatusCode) mfrc522.MIFARE_Read(blockAddr[0], buffer, &size);
      if (status != MFRC522::STATUS_OK) {
          flag_rfid = false;
          return;
      }else flag_rfid = true;
    
    
    
    // Serial.print(F("Data in block ")); Serial.print(blockAddr[0]); Serial.println(F(":"));
    debug_byte_array(buffer, 16); Serial.println();
    Serial.println();

    status = (MFRC522::StatusCode) mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_B, trailerBlock, &key, &(mfrc522.uid));
      if (status != MFRC522::STATUS_OK){
          flag_rfid = false;
          return;
      }else flag_rfid = true;
      
      
    debug_byte_array(DATA_RFID, 16); Serial.println();
  
    status = (MFRC522::StatusCode) mfrc522.MIFARE_Write(blockAddr[0], DATA_RFID, 16);
      if (status != MFRC522::STATUS_OK) {
          flag_rfid = false;
          return;
      }else flag_rfid = true;
      
      
    status = (MFRC522::StatusCode) mfrc522.MIFARE_Read(blockAddr[0], buffer, &size);
    if (status != MFRC522::STATUS_OK) {
        flag_rfid = false;
        return;
    }else flag_rfid = true;
    
    // Serial.print(F("Data in block ")); Serial.print(blockAddr[0]); Serial.println(F(":"));
    debug_byte_array(buffer, 16); Serial.println();
        
    // Check that data in block is what we have written
    // by counting the number of bytes that are equal
    Serial.println(F("Checking result..."));
    byte count = 0;
    for (byte i = 0; i < 16; i++) {
        // Compare buffer (= what we've read) with DATA_RFID (= what we've written)
        if (buffer[i] == DATA_RFID[i])
            count++;
    }
    Serial.print(F("Number of bytes that match = ")); Serial.println(count);
    if (count == 16) {
        Serial.println(F("Success :-)"));
    } else {
        Serial.println(F("Failure, no match :-("));
        Serial.println(F("  perhaps the write didn't work properly..."));
    }
    Serial.println();
        
    // Dump the sector data
    Serial.println(F("Current data in sector:"));
    mfrc522.PICC_DumpMifareClassicSectorToSerial(&(mfrc522.uid), &key, sector[0]);
    Serial.println();
    
    

      
      

//===================================================================================================================    
      

      mfrc522.PICC_HaltA();
      mfrc522.PCD_StopCrypto1();
      
      debug_start();
      debug_data_http();
      debug_data_rfid();
      debug_end();
      
      
      clearInt(mfrc522);
      bNewInt = false;
   }

// The receiving block needs regular retriggering (tell the tag it should transmit??)
// (mfrc522.PCD_WriteRegister(mfrc522.FIFODataReg,mfrc522.PICC_CMD_REQA);)
activateRec(mfrc522);
//   delay(100);
   /*Activate the interrupt*/
} //loop()

/**
 * Helper routine to dump a byte array as hex values to Serial.
 */
void dump_byte_array(byte *buffer, byte bufferSize) {
    for (byte i = 0; i < bufferSize; i++) {
        Serial.print(buffer[i] < 0x10 ? " 0" : " ");
        Serial.print(buffer[i], HEX);
    }
}
/**
 * MFRC522 interrupt serving routine
 */
void readCard(){
   bNewInt = true;
}


/*
 * The function sending to the MFRC522 the needed commands to activate the reception
 */
void activateRec(MFRC522 rfid){
    mfrc522.PCD_WriteRegister(mfrc522.FIFODataReg,mfrc522.PICC_CMD_REQA);
    mfrc522.PCD_WriteRegister(mfrc522.CommandReg,mfrc522.PCD_Transceive);  
    mfrc522.PCD_WriteRegister(mfrc522.BitFramingReg, 0x87);    
}

/*
 * The function to clear the pending interrupt bits after interrupt serving routine
 */
void clearInt(MFRC522 mfrc522){
   mfrc522.PCD_WriteRegister(mfrc522.ComIrqReg,0x7F);
}


/**
 * Helper routine to dump a byte array as hex values to Serial. 
 */
void printHex(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}

//-------------------------------------------------------------------------------//
//------------------------------- Start Clear Rfid ------------------------------//
//-------------------------------------------------------------------------------//
void clear_rfid (){      
    byte CLEAR_DATA [16] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    byte k = 0;
    byte l = 0;
      
    for (byte i = 0; i < 15; i++){
        trailerBlock = trailer[l];
        status = (MFRC522::StatusCode) mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_B, trailerBlock, &key, &(mfrc522.uid));
        if (status != MFRC522::STATUS_OK){
            flag_rfid = false;
        } else flag_rfid = true;
      
        for (byte j = 0; j < 3; j++){
            status = (MFRC522::StatusCode) mfrc522.MIFARE_Write(blockAddr[j+k], CLEAR_DATA, 16);
            if (status != MFRC522::STATUS_OK) {
                flag_rfid = false;
            } else flag_rfid = true;
        }
        k += 3;
        l++;
    }
       
    if (flag_rfid) {
        Serial.println(F("Clear Rfid Complete!"));
    } else Serial.println(F("Clear Rfid Error!"));        
}
//-------------------------------------------------------------------------------//
//------------------------------- End Clear Rfid --------------------------------//
//-------------------------------------------------------------------------------//


//-------------------------------------------------------------------------------//
//---------------------------- Start Create Data Rfid ---------------------------//
//-------------------------------------------------------------------------------//
void create_data_rfid (byte *time){
    byte j = 0;
    for (byte i = 0; i < 2; i++) {
        DATA_RFID[i] = PK_ID[i];
    }
    for (byte i = 2; i < 8; i++) {
        j = i - 2;
        DATA_RFID[i] = time[j];
    }
    for (byte i = 8; i < 16; i++) {
        DATA_RFID[i] = 0;
    }
}
//-------------------------------------------------------------------------------//
//----------------------------- End Create Data Rfid ----------------------------//
//-------------------------------------------------------------------------------//


//-------------------------------------------------------------------------------//
//---------------------------- Start Create Data Http ---------------------------//
//-------------------------------------------------------------------------------//
void create_data_http (byte *rfid_id, byte *time){
    byte j = 0;
    for (byte i = 0; i < 2; i++) {
        j = i;
        DATA_HTTP[i] = PK_ID[j];
    }
    for (byte i = 2; i < 6; i++) {
        j = i - 2;
        DATA_HTTP[i] = rfid_id[j];
    }
    for (byte i = 6; i < 12; i++) {
        j = i - 6;
        DATA_HTTP[i] = time[j];
    }
    for (byte i = 12; i < 16; i++) {
        DATA_HTTP[i] = 0;
    }
}
//-------------------------------------------------------------------------------//
//----------------------------- End Create Data Http ----------------------------//
//-------------------------------------------------------------------------------//


//&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&//
//&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&//
//&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&& End Functions &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&//
//&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&//
//&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&//


//###############################################################################//
//###############################################################################//
//########################### Start "Debug" Functions ###########################//
//###############################################################################//
//###############################################################################//


//-------------------------------------------------------------------------------//
//---------------------------- Start Debug Data Rfid ----------------------------//
//-------------------------------------------------------------------------------//
void debug_data_rfid (){
    Serial.print("Data Rfid:  ");
    for (byte i = 0; i < 2; i++) {
        Serial.print((char)DATA_RFID[i]);  // Print PK ID: A B
        Serial.print(' ');
    }
    for (byte i = 2; i < 16; i++) {
        Serial.print(DATA_RFID[i], HEX);  // Print Card ID: A B C D 
        Serial.print(' ');        
    }
    Serial.println();
}
//-------------------------------------------------------------------------------//
//----------------------------- End Debug Data Rfid -----------------------------//
//-------------------------------------------------------------------------------//


//-------------------------------------------------------------------------------//
//---------------------------- Start Debug Data Http ----------------------------//
//-------------------------------------------------------------------------------//
void debug_data_http (){
    Serial.print("PK ID:      ");
    for (byte i = 0; i < 2; i++) {
        Serial.print((char)DATA_HTTP[i]);  // Print PK ID: A B
        Serial.print(' ');
    }
    Serial.println();
    
    Serial.print("Rfid ID:    ");
    for (byte i = 2; i < 6; i++) {
        Serial.print(DATA_HTTP[i], HEX);  // Print Card ID: A B C D 
        Serial.print(' ');        
    }
    Serial.println();
    
    Serial.print("Time:       ");
    for (byte i = 6; i < 9; i++) {
        Serial.print(DATA_HTTP[i], HEX);  // Print Date: YY MM DD
          if ((i == 6)|(i == 7)) Serial.print('/');
    }
    Serial.print(' ');
    for (byte i = 9; i < 12; i++) {
        Serial.print(DATA_HTTP[i], HEX);  // Print Time: HH MM SS
          if ((i == 9)|(i == 10)) Serial.print(':');
    }
    Serial.println();

    Serial.print("Data Http:  ");
    for (byte i = 0; i < 2; i++) {
        Serial.print((char)DATA_HTTP[i]);  // Print PK ID: A B
        Serial.print(' ');
    }
    
    for (byte i = 2; i < 16; i++) {
        Serial.print(DATA_HTTP[i], HEX);  // Print Card Rest 
        Serial.print(' ');        
    }
    Serial.println();
}
//-------------------------------------------------------------------------------//
//----------------------------- End Debug Data Http -----------------------------//
//-------------------------------------------------------------------------------//


//-------------------------------------------------------------------------------//
//---------------------------- Start Debug Byte Array ---------------------------//
//-------------------------------------------------------------------------------//
void debug_byte_array(byte *buffer, byte bufferSize) { 
    for (byte i = 0; i < bufferSize; i++) {
        Serial.print(buffer[i] < 0x10 ? " 0" : " ");
        Serial.print(buffer[i], HEX);
    }
    Serial.println();
}
//-------------------------------------------------------------------------------//
//---------------------------- End Debug Byte Array -----------------------------//
//-------------------------------------------------------------------------------//


//-------------------------------------------------------------------------------//
//------------------------------ Start Debug Start ------------------------------//
//-------------------------------------------------------------------------------//
void debug_start (){
    Serial.println();
    Serial.println("************************ Start Debug ************************");
    Serial.println();
}  
//-------------------------------------------------------------------------------//
//------------------------------- End Debug Start -------------------------------//
//-------------------------------------------------------------------------------//


//-------------------------------------------------------------------------------//
//------------------------------- Start Debug End -------------------------------//
//-------------------------------------------------------------------------------//
void debug_end (){
    Serial.println();
    Serial.println("************************* End Debug *************************");
    Serial.println();
}
//-------------------------------------------------------------------------------//
//-------------------------------- End Debug End --------------------------------//
//-------------------------------------------------------------------------------//


//###############################################################################//
//###############################################################################//
//############################ End "Debug" Functions ############################//
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
