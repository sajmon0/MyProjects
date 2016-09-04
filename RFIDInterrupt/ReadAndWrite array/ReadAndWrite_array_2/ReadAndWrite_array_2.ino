/**
 * ----------------------------------------------------------------------------
 * This is a MFRC522 library example; see https://github.com/miguelbalboa/rfid
 * for further details and other examples.
 * 
 * NOTE: The library file MFRC522.h has a lot of useful info. Please read it.
 * 
 * Released into the public domain.
 * ----------------------------------------------------------------------------
 * This sample shows how to read and write data blocks on a MIFARE Classic PICC
 * (= card/tag).
 * 
 * BEWARE: Data will be written to the PICC, in sector #1 (blocks #4 to #7).
 * 
 * 
 * Typical pin layout used:
 * -----------------------------------------------------------------------------------------
 *             MFRC522      Arduino       Arduino   Arduino    Arduino          Arduino
 *             Reader/PCD   Uno           Mega      Nano v3    Leonardo/Micro   Pro Micro
 * Signal      Pin          Pin           Pin       Pin        Pin              Pin
 * -----------------------------------------------------------------------------------------
 * RST/Reset   RST          9             5         D9         RESET/ICSP-5     RST
 * SPI SS      SDA(SS)      10            53        D10        10               10
 * SPI MOSI    MOSI         11 / ICSP-4   51        D11        ICSP-4           16
 * SPI MISO    MISO         12 / ICSP-1   50        D12        ICSP-1           14
 * SPI SCK     SCK          13 / ICSP-3   52        D13        ICSP-3           15
 * 
 */



/*
writeBlock(blockNumber,arrayAddress);

void writeBlock(int blockNumber, byte arrayAddress[]) 
{
  int largestModulo4Number=blockNumber/4*4;
  int trailerBlock=largestModulo4Number+3;
  mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522.uid));
  mfrc522.MIFARE_Write(blockNumber, arrayAddress, 16);
  //mfrc522.MIFARE_Write(9, value1Block, 16);
}



if (rfid.isCard()) {
        if (rfid.readCardSerial()) {
            if (rfid.serNum[0] != serNum0
                || rfid.serNum[1] != serNum1
                || rfid.serNum[2] != serNum2
                || rfid.serNum[3] != serNum3
                || rfid.serNum[4] != serNum4
            ) {
                // With a new cardnumber, show it.
                Serial.println(" ");
                Serial.println("Card found");
                serNum0 = rfid.serNum[0];
                serNum1 = rfid.serNum[1];
                serNum2 = rfid.serNum[2];
                serNum3 = rfid.serNum[3];
                serNum4 = rfid.serNum[4];
               
                //Serial.println(" ");
                Serial.println("Cardnumber:");
                Serial.print("Dec: ");
		Serial.print(rfid.serNum[0],DEC);
                Serial.print(", ");
		Serial.print(rfid.serNum[1],DEC);
                Serial.print(", ");
		Serial.print(rfid.serNum[2],DEC);
                Serial.print(", ");
		Serial.print(rfid.serNum[3],DEC);
                Serial.print(", ");
		Serial.print(rfid.serNum[4],DEC);
                Serial.println(" ");



byte status = mfrc522.MIFARE_Write(blockAddr, valueBlock, 16);
if (status != MFRC522::STATUS_OK) {
	Serial.print("MIFARE_Write() failed: ");
	Serial.println(mfrc522.GetStatusCodeName(status));
}




*/



#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN         9           // Configurable, see typical pin layout above
#define SS_PIN          10          // Configurable, see typical pin layout above


    // In this sample we use the second sector,
    // that is: sector #1, covering block #4 up to and including block #7
    char PK_ID[2]         = {'A', 'B'};
    byte RFID_ID[4]       = {10, 20, 30, 40};
    byte TIME[6]          = {16, 10, 23, 19, 47, 23};
    //byte TIME[6]          = {16, 10, 29, 21, 14, 47};
    
    // [PK_ID[2], RFID_ID[4],  TIME[6], ]
    byte DATA_HTTP [16];
    byte DATA_RFID [8];
    
    byte sector[15]       = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
    byte blockAddr[45]    = {
        4, 5, 6,   8, 9, 10,   12, 13, 14,   16, 17, 18,   20, 21, 22,   24, 25, 26,   28, 29, 30,   32, 33, 34,
        36, 37, 38,   40, 41, 42,   44, 45, 46,   48, 49, 50,    52, 53, 54,    56, 57, 58,   60, 61, 62    
    };
    byte dataBlock[16];
    byte trailerBlock   = 7;





MFRC522 mfrc522(SS_PIN, RST_PIN);   // Create MFRC522 instance.

MFRC522::MIFARE_Key key;
MFRC522::StatusCode status;

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

    Serial.println(F("Scan a MIFARE Classic PICC to demonstrate read and write."));
    Serial.print(F("Using key (for A and B):"));
//    dump_byte_array(key.keyByte, MFRC522::MF_KEY_SIZE);
    Serial.println();
    
    Serial.println(F("BEWARE: Data will be written to the PICC, in sector #1"));
}

/**
 * Main loop.
 */
void loop() {
    // Look for new cards
    if ( ! mfrc522.PICC_IsNewCardPresent())
        return;
    // Select one of the cards
    if ( ! mfrc522.PICC_ReadCardSerial())
        return;
    
/*    // Show some details of the PICC (that is: the tag/card)
    Serial.print(F("Card UID:"));
    dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);
    Serial.println();
*/    
    // Store NUID into RFID_ID array
    for (byte i = 0; i < 4; i++) {
      RFID_ID[i] = mfrc522.uid.uidByte[i];
    }
   
    Serial.println(F("The RFID_ID tag is:"));
    dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);
    Serial.println();
    
    
/*    Serial.print(F("PICC type: "));
    MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
    Serial.println(mfrc522.PICC_GetTypeName(piccType));

    // Check for compatibility
    if (    piccType != MFRC522::PICC_TYPE_MIFARE_MINI
        &&  piccType != MFRC522::PICC_TYPE_MIFARE_1K
        &&  piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
        Serial.println(F("This sample only works with MIFARE Classic cards."));
        return;
    }
*/
    byte buffer[18];
    byte size = sizeof(buffer);





    create_data_http(RFID_ID, TIME);
    


    // Authenticate using key A
    Serial.println(F("Authenticating using key A..."));
    status = (MFRC522::StatusCode) mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522.uid));
    if (status != MFRC522::STATUS_OK) {
        Serial.print(F("PCD_Authenticate() failed: "));
        Serial.println(mfrc522.GetStatusCodeName(status));
        return;
    }
    else
    {
        Serial.print(F("Authenticating OK"));
        Serial.println();   
    }
/*
    // Show the whole sector as it currently is
    Serial.println(F("Current data in sector:"));
    mfrc522.PICC_DumpMifareClassicSectorToSerial(&(mfrc522.uid), &key, sector[0]);
    Serial.println();
*/
    // Read data from the block
    Serial.print(F("Reading data from block ")); Serial.print(blockAddr[0]);
    Serial.println(F(" ..."));
    status = (MFRC522::StatusCode) mfrc522.MIFARE_Read(blockAddr[0], buffer, &size);
    if (status != MFRC522::STATUS_OK) {
        Serial.print(F("MIFARE_Read() failed: "));
        Serial.println(mfrc522.GetStatusCodeName(status));
    }
    Serial.print(F("Data in block ")); Serial.print(blockAddr[0]); Serial.println(F(":"));
    dump_byte_array(buffer, 16); Serial.println();
    Serial.println();

    // Authenticate using key B
    Serial.println(F("Authenticating again using key B..."));
    status = (MFRC522::StatusCode) mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_B, trailerBlock, &key, &(mfrc522.uid));
    if (status != MFRC522::STATUS_OK) {
        Serial.print(F("PCD_Authenticate() failed: "));
        Serial.println(mfrc522.GetStatusCodeName(status));
        return;
    }
    else
    {
        Serial.print(F("Authenticating OK"));   
    }

    // Write data to the block
    Serial.print(F("Writing data into block ")); Serial.print(blockAddr[0]);
    Serial.println(F(" ..."));
    dump_byte_array(dataBlock, 16); Serial.println();
    status = (MFRC522::StatusCode) mfrc522.MIFARE_Write(blockAddr[0], dataBlock, 16);
   // status = mfrc522.MIFARE_Write(blockAddr[0], dataBlock, 16);
    if (status != MFRC522::STATUS_OK) {
        Serial.print(F("MIFARE_Write() failed: "));
        Serial.println(mfrc522.GetStatusCodeName(status));
    }
    
    Serial.println();

    // Read data from the block (again, should now be what we have written)
    Serial.print(F("Reading data from block ")); Serial.print(blockAddr[0]);
    Serial.println(F(" ..."));
    status = (MFRC522::StatusCode) mfrc522.MIFARE_Read(blockAddr[0], buffer, &size);
    if (status != MFRC522::STATUS_OK) {
        Serial.print(F("MIFARE_Read() failed: "));
        Serial.println(mfrc522.GetStatusCodeName(status));
    }
    Serial.print(F("Data in block ")); Serial.print(blockAddr[0]); Serial.println(F(":"));
    dump_byte_array(buffer, 16); Serial.println();
        
    // Check that data in block is what we have written
    // by counting the number of bytes that are equal
    Serial.println(F("Checking result..."));
    byte count = 0;
    for (byte i = 0; i < 16; i++) {
        // Compare buffer (= what we've read) with dataBlock (= what we've written)
        if (buffer[i] == dataBlock[i])
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




    // Halt PICC
    mfrc522.PICC_HaltA();
    // Stop encryption on PCD
    mfrc522.PCD_StopCrypto1();
    
    
    debug_data_http();
    
    
    
    
    
}

/**
 * Helper routine to dump a byte array as hex values to Serial.
 */
void dump_byte_array(byte *buffer, byte bufferSize) {
    for (byte i = 0; i < bufferSize; i++) {
        Serial.print(buffer[i] < 0x10 ? " 0" : " ");
        Serial.print(buffer[i], HEX);
    }
}

void create_data_rfid (byte *time){
    for (byte i = 0; i < 2; i++) {
        DATA_RFID[i] = PK_ID[i];
    }
    for (byte i = 2; i < 8; i++) {
        DATA_RFID[i] = time[i];
    }
}

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


void debug_data_http (){
    Serial.print("PK ID: ");
    for (byte i = 0; i < 2; i++) {
        Serial.print((char)DATA_HTTP[i]);
        Serial.print(' ');
    }
    Serial.println();
    
    Serial.print("Card ID: ");
    for (byte i = 2; i < 6; i++) {
        Serial.print(DATA_HTTP[i], HEX);
        Serial.print(' ');        
    }
    Serial.println();
    
    Serial.print("Time: ");
    for (byte i = 6; i < 12; i++) {
        Serial.print(DATA_HTTP[i], HEX);
        Serial.print(' ');
    }
    Serial.println();
    
    Serial.print("Rest: ");
    for (byte i = 12; i < 16; i++) {
        Serial.print(DATA_HTTP[i], HEX);
        Serial.print(' ');
    }
    Serial.println(" ");
}

/*
void clear_card (){
  
    Serial.println(F("Clear Card"));
    dump_byte_array(dataBlock, 16); Serial.println();
    
    for(byte i = 0; i < 45; i++) {
      status = (MFRC522::StatusCode) mfrc522.MIFARE_Write(blockAddr[i], dataBlock, 16);
      if (status != MFRC522::STATUS_OK) {
          Serial.print(F("MIFARE_Write() failed: "));
          Serial.println(mfrc522.GetStatusCodeName(status)); 
      }
      Serial.println("Clear Block nr:");
      Serial.print(blockAddr[i]); 
    }  
}*/
