/**************************************************************************/
/*! 
    @file     readMifare_shield
    @author   originally by Adafruit Industries, modified by eskimwier
	@license  BSD (see license.txt)

    This example will wait for any ISO14443A card or tag, 
    and will attempt to read from it.
   
    If the card has a 4-byte or 7-byte UID :
   
    - Authenticate each block using the default KEYA of 
      0XFF 0XFF 0XFF 0XFF 0XFF 0XFF
    - If authentication succeeds, we then read the 4 blocks in that
      sector 

*/
/**************************************************************************/
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_PN532.h>

// If using the breakout or shield with I2C, define just the pins connected
// to the IRQ and reset lines.  Use the values below (2, 3) for the shield!
#define PN532_IRQ   (2)
#define PN532_RESET (3)  // Not connected by default on the NFC Shield

Adafruit_PN532 nfc(PN532_IRQ, PN532_RESET);

#if defined(ARDUINO_ARCH_SAMD)
// for Zero, output on USB Serial console, remove line below if using programming port to program the Zero!
// also change #define in Adafruit_PN532.cpp library file
   #define Serial SerialUSB
#endif

void setup(void) {

    Serial.begin(115200);
    Serial.println("--------------------------------");
    Serial.println(" Welcome to the NFC Card Reader ");
    Serial.println("--------------------------------");

    nfc.begin();

    uint32_t versiondata = nfc.getFirmwareVersion();
    if (! versiondata) {
        Serial.print("Didn't find PN53x board. Make sure the daughter board is properly attached.\n Halting.");
        while (1); // halt
    }

    Serial.print("Found chip PN5"); Serial.println((versiondata>>24) & 0xFF, HEX); 
    Serial.print("Firmware ver. "); Serial.print((versiondata>>16) & 0xFF, DEC); 
    Serial.print('.'); Serial.println((versiondata>>8) & 0xFF, DEC);

    nfc.SAMConfig(); // configure board to read RFID tags

    Serial.println("Waiting for an ISO14443A Card ...");

}


void loop(void) {

    uint8_t success;
    uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 }; 
    uint8_t uidLength;   // 4 or 7 bytes depending on ISO14443A card type
    
    success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);
  
    if (success) {

        Serial.println("-----------------------------------------------------------------------------");
        Serial.print("ISO14443A card, UID: ");
        nfc.PrintHex(uid, uidLength);
        Serial.println("-----------------------------------------------------------------------------");
        

        if (uidLength == 4)
        {
            //uint8_t keya[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
            uint8_t keya[6] = { 'E', '+', 'M', ' ', 'f', 'o' };
            uint8_t data[16];

            for (int sector = 0; sector < 16; sector++)
            {
                success = nfc.mifareclassic_AuthenticateBlock(uid, uidLength, sector * 4, 0, keya);

                if (success) {

                    Serial.println("Sector " + String(sector) + " (Blocks " + String(sector * 4) + ".." + String((sector * 4) + 3) + " ) authenticated");

                    ReadSector(sector);

                } else {

                    Serial.println("---READ COMPLETE---");
                    break;

                }
            }

            delay(3000); // Wait a bit before reading the card again
        }

        if (uidLength == 7)
        {
  
            char d[2] = { 'E', 'M' };
            //WriteData();
            //Write(d);// "E&M forever");

            for (int page = 0; page < 64; page++)
            {
                ReadPage(page);
            }
            delay(3000); // Wait a bit before reading the card again
        }
    }
}

void Write(char str[] )
{
    int i = 0, j = 0, k = 0;
    int len = strlen(str);
    int blocks = len / 4;
    int remain = len % 4;
    for (i = 0; i < strlen(str); i++) {
        nfc.mifareclassic_WriteDataBlock (i+1, (uint8_t*)str[i*4]);
    }

    /*
    if (remain > 0) {
        uint8_t b[4];
        i++;
        for (j = 0; j < 4; j++) {
            memcpy(b, (const uint8_t*)str[len-remain], remain);
            for (k = 0; k < remain; k++) {
                str[len - k] = 0xFE;
            }
            nfc.mifareclassic_WriteDataBlock (i, (uint8_t*)str[i*4]);
        }
    }
    */

    /*
    char clear[4] = { '.', '.', '.', '.' };
    for (i++; i < 64; i++) {
        nfc.mifareclassic_WriteDataBlock (i, (uint8_t*)clear);
    }
    */

}

void ReadPage(uint8_t page)
{
    uint8_t data[32];
    uint8_t success;

    success = nfc.mifareultralight_ReadPage (page, data);
    if (success)
    {
        nfc.PrintHexChar(data, 4);
        Serial.println("");

    } else {
        Serial.println("Unable to read the requested page");
    }

}


void ReadSector(uint8_t sector)
{
    uint8_t data[16];
    uint8_t success;
    String blockString;

    for (int block = 0; block < 4; block++)
    {
        blockString = String((sector * 4) + block);
        success = nfc.mifareclassic_ReadDataBlock((sector*4)+block, data);

        if (success)
        {
            Serial.print("\tBlock " + blockString + ": ");
            nfc.PrintHexChar(data, 16);
        }
        else
        {
            Serial.println("Unable to read block " + blockString);
        }
    }
}



