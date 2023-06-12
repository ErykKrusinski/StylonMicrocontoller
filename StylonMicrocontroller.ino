// Adding some imports.
#include <SPI.h>       // We amy be betrayed by SPI.
#include <MFRC522.h>   // Reading RFID reader.

// Defining the pins layout to adhere to the above. 

#define RST_PIN         53       // Configurable, see typical pin layout above
#define NR_OF_READERS   1        // Defining number of readers. In this case, it's 1 but this may change in the future.
#define SS_1_PIN        48       // Configurable, take a unused pin, only HIGH/LOW required, must be different to SS 2.
//#define SS_2_PIN        49     // Configurable, take a unused pin, only HIGH/LOW required, must be different to SS 1.
// /\ Commenting this one out because we are only taking readings from 1 RFID reader. This may change in the future. 

// Initialisng variables.
const int mainIndex = 30;         // Maximum number of items = 30.
int rfidArray[mainIndex];          
byte arrayIndex = 0;
int foundFlag = 0;
int runCounter = 0;

// Defining SDA pin(s).
byte ssPins[] = {SS_1_PIN}; // If you want to get more than 1 pin, change to {SS_1_PIN, SS_2_PIN};  

// Create MFRC522 instance.
MFRC522 mfrc522[NR_OF_READERS];   

// Initialize.
void setup() {

  // Build connection. Why like this?? Because arduino says so...
  Serial.begin(9600);  // Initialize serial communications with the PC
  while (!Serial);     // Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)
  SPI.begin();         // Init SPI bus

  // Iterate through all readers. (Only useful if more than 1 reader.)
  for (uint8_t reader = 0; reader < NR_OF_READERS; reader++) {
    mfrc522[reader].PCD_Init(ssPins[reader], RST_PIN); // Init each MFRC522 card
    Serial.print(F("Reader "));                        // Commenting output like an idiot.
    Serial.print(reader);
    Serial.print(F(": "));
    mfrc522[reader].PCD_DumpVersionToSerial();
  }
}

// Main loop.
void loop() {

  // Iterate through RFID Readers. (There is currently 1.)
  for (uint8_t reader = 0; reader < NR_OF_READERS; reader++) {
    
    //// Detect Tag.

    // Look for new Tags.
    if (mfrc522[reader].PICC_IsNewCardPresent() && mfrc522[reader].PICC_ReadCardSerial()) {
      Serial.print(F("Reader "));
      Serial.print(reader);
      // Show some details of the PICC (i.e. the tag)
      Serial.print(F(": Card UID:"));
      dump_byte_array(mfrc522[reader].uid.uidByte, mfrc522[reader].uid.size);
      Serial.println();
      Serial.print(F("PICC type: "));
      MFRC522::PICC_Type piccType = mfrc522[reader].PICC_GetType(mfrc522[reader].uid.sak);
      Serial.println(mfrc522[reader].PICC_GetTypeName(piccType));

      // Halt PICC.
      mfrc522[reader].PICC_HaltA();
      // Stop encryption on PCD.
      mfrc522[reader].PCD_StopCrypto1();

      // Base code curtosey of https://github.com/miguelbalboa/rfid

      //// Counting number of Tags.              ------------------- THIS IS BASICALLY PSEUDO-CODE BEYOND HERE.    -------------------
            
      // Verify if RFID already counted.
      for(uint8_t loopIndex = 0; loopIndex < mainIndex; loopIndex++) {

        // Verify if currently selected RFID exists in currently selected array position. 
        if (mfrc522[reader].PICC_GetTypeName(piccType) == rfidArray[loopIndex]) {

          // Flag as found.
          foundFlag = 1;

        }
            
      } 
      // Please note, this code is stupid but it has to be like this because arrays are not dynamic here. 
      // Therefore, a pre-defined, full iteration is necessary each time.

      // Insert RFID into array, if not previously found.
      if (foundFlag == 0) {

        // Iterate array until next free position is found.
        for(uint8_t loopIndex2 = 0; loopIndex2 < mainIndex; loopIndex2++) {

          // Verify array position is not empty.
          if (strlen(rfidArray[loopIndex2]) == 0) {

            // Add RFID to array.
            rfidArray[loopIndex2] = mfrc522[reader].PICC_GetTypeName(piccType);

          }
      
        // Reset foundFlag.
        foundFlag = 0;

      }
      
    } //if (mfrc522[reader].PICC_IsNewC

    // Check for RFID <mainIndex> no. of times. => 30 scans => 30 seconds seconds.
    String outputMessage = String(runCounter + 1) + " scan of " + String(mainIndex) + " complete.";
    Serial.println(outputMessage);

    runCounter = runCounter + 1;
    if (runCounter == mainIndex) {
      break;
    }
    
    // Introduce delay to reduce number of scans. Less SPAM.
    delay(1000);

  } //for(uint8_t reader

  //-//- HERE WOULD BE CODE THAT UPDATES THE AWS DATABASE, THAT THEN THE APP WOULD API CALL DATA FROM. -//-//

    // Count RFIDs of items in array.
    int rfidCount = 0;
    for(uint8_t loopIndex3 = 0; loopIndex3 < mainIndex; loopIndex3++) {
    
      // Verify position is not empty.
      if (strlen(rfidArray[loopIndex3]) != 0) {

        // Count RFID as present if not empty.
        rfidCount = rfidCount + 1;

      }

    }
  
  }

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
