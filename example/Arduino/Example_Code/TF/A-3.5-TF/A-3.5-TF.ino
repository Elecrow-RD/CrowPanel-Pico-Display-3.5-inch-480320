#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <FS.h>

const int _MISO = 12;  // AKA SPI RX
const int _MOSI = 11;  // AKA SPI TX
const int _CS = 22;
const int _SCK = 10;

File root;

void setup() {
  // put your setup code here, to run once:
  Serial.begin( 9600 ); /*初始化串口*/

}

void loop() {
  Serial1.println("\nInitializing SD card...");
  Serial.println("\nInitializing SD card...");
  bool sdInitialized = false;
  // Ensure the SPI pinout the SD card is connected to is configured properly
  // Select the correct SPI based on _MISO pin for the RP2040
  if (_MISO == 0 || _MISO == 4 || _MISO == 16) {
    SPI.setRX(_MISO);
    SPI.setTX(_MOSI);
    SPI.setSCK(_SCK);
    sdInitialized = SD.begin(_CS);
  } else if (_MISO == 8 || _MISO == 12) {
    SPI1.setRX(_MISO);
    SPI1.setTX(_MOSI);
    SPI1.setSCK(_SCK);
    sdInitialized = SD.begin(_CS, SPI1);
  } else {
    Serial.println(F("ERROR: Unknown SPI Configuration"));
    Serial1.println(F("ERROR: Unknown SPI Configuration"));
    return;
  }

  if (!sdInitialized) {
    Serial1.println("initialization failed. Things to check:");
    Serial1.println("* is a card inserted?");
    Serial1.println("* is your wiring correct?");
    Serial1.println("* did you change the chipSelect pin to match your shield or module?");
    
    Serial.println("initialization failed. Things to check:");
    Serial.println("* is a card inserted?");
    Serial.println("* is your wiring correct?");
    Serial.println("* did you change the chipSelect pin to match your shield or module?");
    return;
  } else {
    Serial1.println("Wiring is correct and a card is present.");
    Serial.println("Wiring is correct and a card is present.");
  }
  // 0 - SD V1, 1 - SD V2, or 3 - SDHC/SDXC
  // print the type of card
  Serial1.println();
  Serial1.print("Card type:         ");
  Serial.println();
  Serial.print("Card type:         ");
  switch (SD.type()) {
    case 0:
      Serial1.println("SD1");
      Serial.println("SD1");
      break;
    case 1:
      Serial1.println("SD2");
      Serial.println("SD2");
      break;
    case 3:
      Serial1.println("SDHC/SDXC");
      Serial.println("SDHC/SDXC");
      break;
    default:
      Serial1.println("Unknown");
      Serial.println("Unknown");
  }

  Serial1.print("Cluster size:          ");
  Serial1.println(SD.clusterSize());
  Serial1.print("Blocks x Cluster:  ");
  Serial1.println(SD.blocksPerCluster());
  Serial1.print("Blocks size:  ");
  Serial1.println(SD.blockSize());

  Serial.print("Cluster size:          ");
  Serial.println(SD.clusterSize());
  Serial.print("Blocks x Cluster:  ");
  Serial.println(SD.blocksPerCluster());
  Serial.print("Blocks size:  ");
  Serial.println(SD.blockSize());
  
  Serial1.print("Total Blocks:      ");
  Serial1.println(SD.totalBlocks());
  Serial1.println();
    
  Serial.print("Total Blocks:      ");
  Serial.println(SD.totalBlocks());
  Serial.println();

  Serial1.print("Total Cluster:      ");
  Serial1.println(SD.totalClusters());
  Serial1.println();
  
  Serial.print("Total Cluster:      ");
  Serial.println(SD.totalClusters());
  Serial.println();

  // print the type and size of the first FAT-type volume
  uint32_t volumesize;
  Serial1.print("Volume type is:    FAT");
  Serial1.println(SD.fatType(), DEC);

  Serial.print("Volume type is:    FAT");
  Serial.println(SD.fatType(), DEC);
  
  volumesize = SD.totalClusters();
  volumesize *= SD.clusterSize();
  volumesize /= 1000;
  Serial1.print("Volume size (Kb):  ");
  Serial1.println(volumesize);
  Serial1.print("Volume size (Mb):  ");

  Serial.print("Volume size (Kb):  ");
  Serial.println(volumesize);
  Serial.print("Volume size (Mb):  ");
  volumesize /= 1024;
  Serial1.println(volumesize);
  Serial1.print("Volume size (Gb):  ");
  Serial1.println((float)volumesize / 1024.0);

  Serial.println(volumesize);
  Serial.print("Volume size (Gb):  ");
  Serial.println((float)volumesize / 1024.0);
  
  Serial1.print("Card size:  ");
  Serial1.println((float)SD.size() / 1000);

  Serial.print("Card size:  ");
  Serial.println((float)SD.size() / 1000);
  FSInfo fs_info;
  SDFS.info(fs_info);

  Serial1.print("Total bytes: ");
  Serial1.println(fs_info.totalBytes);

  Serial1.print("Used bytes: ");
  Serial1.println(fs_info.usedBytes);

  Serial.print("Total bytes: ");
  Serial.println(fs_info.totalBytes);

  Serial.print("Used bytes: ");
  Serial.println(fs_info.usedBytes);
  root = SD.open("/");
  printDirectory(root, 0);
  
}
void printDirectory(File dir, int numTabs) {
  while (true) {

    File entry = dir.openNextFile();
    if (!entry) {
      // no more files
      break;
    }
    for (uint8_t i = 0; i < numTabs; i++) {
      Serial1.print('\t');
      Serial.print('\t');
    }
    Serial1.print(entry.name());
    Serial.print(entry.name());
    if (entry.isDirectory()) {
      Serial1.println("/");
      Serial.println("/");
      printDirectory(entry, numTabs + 1);
    } else {
      // files have sizes, directories do not
      Serial1.print("\t\t");
      Serial1.print(entry.size(), DEC);
      Serial.print("\t\t");
      Serial.print(entry.size(), DEC);
      time_t cr = entry.getCreationTime();
      time_t lw = entry.getLastWrite();
      struct tm* tmstruct = localtime(&cr);
      Serial1.printf("\tCREATION: %d-%02d-%02d %02d:%02d:%02d", (tmstruct->tm_year) + 1900, (tmstruct->tm_mon) + 1, tmstruct->tm_mday, tmstruct->tm_hour, tmstruct->tm_min, tmstruct->tm_sec);
      Serial.printf("\tCREATION: %d-%02d-%02d %02d:%02d:%02d", (tmstruct->tm_year) + 1900, (tmstruct->tm_mon) + 1, tmstruct->tm_mday, tmstruct->tm_hour, tmstruct->tm_min, tmstruct->tm_sec);
      tmstruct = localtime(&lw);
      Serial.printf("\tLAST WRITE: %d-%02d-%02d %02d:%02d:%02d\n", (tmstruct->tm_year) + 1900, (tmstruct->tm_mon) + 1, tmstruct->tm_mday, tmstruct->tm_hour, tmstruct->tm_min, tmstruct->tm_sec);
      Serial1.printf("\tLAST WRITE: %d-%02d-%02d %02d:%02d:%02d\n", (tmstruct->tm_year) + 1900, (tmstruct->tm_mon) + 1, tmstruct->tm_mday, tmstruct->tm_hour, tmstruct->tm_min, tmstruct->tm_sec);
    }
    entry.close();
  }

}

