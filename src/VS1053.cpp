#include "common/cs_dbg.h"
#include "mgos.h"
#include "mgos_SD.h"
#include "mgos_spi.h"
#include "mgos_timers.h"
#include "mgos_gpio.h"
#include "mgos_system.h"

#include "VS1053.h"


#define SPI_CTRL_TXN 0
#define SPI_DATA_TXN 1

#define SCI_FREQ 2000000    // 2MHz
#define SPI_FREQ 10000000   // 10MHz
#define SPI_MODE 1 //use SPI mode 1 for VS1053, 0 does not seem to work
#define MAX_VS1053_SPI_TX_SIZE 18

static VS1053 *myself;

struct mgos_spi *spi;
FILE* currentTrack;
const char* currentTrackname;
SD* sd;
mgos_vs1053_callback_t onEndCallback;
char* onEndUserdata;


static void enableInterrupts() {
    mgos_gpio_enable_int(VS1053_INTERRUPT_PIN);
}

static void disableInterupts() {
    mgos_gpio_disable_int(VS1053_INTERRUPT_PIN);
}


static void interrupt_cb(int pin, void *args) {
//  printf("Interrupt triggered on pin %d\n", VS1053_INTERRUPT_PIN);
  myself->feedBuffer();
}


VS1053::VS1053(){

}

int VS1053::begin(){
    printf("VS1053::begin\n");


    initSDCard();
    initSPI();

    setupVS1053();

    setVolume(40, 40);

    myself = this;

    playingMusic = false;

    return 1;
}

void VS1053::playFile(const char *trackname, mgos_vs1053_callback_t cb, char* userdata){
    if(playing()){
        // Is a file already playing ?
        closeFile(false);
    }

    currentTrack = mgos_sd_openFile(sd, trackname, "r");
    printf("file opened\n");

    currentTrackname = trackname;
    onEndCallback = cb;
    onEndUserdata = userdata;

    playingMusic = true;

    //start immediately
    enableInterrupts();
    feedBuffer();
}

void VS1053::closeFile(bool executeCallback){
    printf("Close file %s\n", currentTrackname);
    playingMusic = false;
    mgos_sd_closeFile(sd, currentTrack);

    if(executeCallback){
        printf("Calling callback\n");
        onEndCallback(onEndUserdata, currentTrackname);
    }
}

void VS1053::stopPlaying(){
    playingMusic = false;
    currentTrack = NULL;
};
void VS1053::pausePlaying(){
    if(playingMusic){
        playingMusic = false;
    }else{
        playingMusic = true;
        feedBuffer();
    }
};


bool VS1053::stopped(){
  return !playingMusic && currentTrack == NULL;
};
bool VS1053::paused(){
  return !playingMusic && currentTrack != NULL;
};
bool VS1053::playing(){
  printf("playingMusic %d, currentTrack %d", playingMusic, currentTrack != NULL);
  return playingMusic && currentTrack != NULL;
};

void VS1053::setVolume(uint8_t left, uint8_t right){
    uint16_t v;
    v = left;
    v <<= 8;
    v |= right;

    uint8_t dataWrite[2];
    dataWrite[0] = (v & 0xFF00)>>8;
    dataWrite[1] =(v & 0x00FF);
    sciWrite(VS1053_REG_VOLUME, dataWrite, 2);
}

void VS1053::feedBuffer(){
    uint8_t buf[32];
    int bytesRead;
    while(playingMusic && readyForData()) {
        bytesRead = mgos_sd_read(sd, currentTrack, buf, 32);

        if (bytesRead == 0) {
             // must be at the end of the file, wrap it up!
             closeFile(true);
             break;
        }

        spiWrite(buf, bytesRead);
    }
}

bool VS1053::readyForData(){
    return mgos_gpio_read(VS1053_INTERRUPT_PIN);
}

int VS1053::initSDCard(){
    printf("VS1053::initSDCard\n");
    sd = mgos_sd_create();
    return mgos_sd_begin (sd, SDCARD_MISO_PIN, SDCARD_MOSI_PIN, SDCARD_CLCK_PIN, SDCARD_CS_PIN);
}

int VS1053::initSPI(){
    struct mgos_config_spi bus_cfg;
     //Initialize SPI
     bus_cfg.enable = true,
     bus_cfg.debug = false,
     bus_cfg.unit_no = VS1053_SPI_HOST, //SDCard calimed HSPI already.
     bus_cfg.miso_gpio = VS1053_MISO_PIN,
     bus_cfg.mosi_gpio = VS1053_MOSI_PIN,
     bus_cfg.sclk_gpio = VS1053_CLCK_PIN,
     bus_cfg.cs0_gpio = VS1053_CTL_CS_PIN,
     bus_cfg.cs1_gpio = VS1053_DATA_CS_PIN,
     bus_cfg.cs2_gpio = -1,

     spi = mgos_spi_create(&bus_cfg);
     if (spi == NULL) {
     LOG(LL_ERROR, ("Bus init failed"));
        return MGOS_APP_INIT_ERROR;
     }

     mgos_gpio_set_mode(VS1053_INTERRUPT_PIN, MGOS_GPIO_MODE_INPUT);
     mgos_gpio_set_int_handler(VS1053_INTERRUPT_PIN, MGOS_GPIO_INT_EDGE_ANY, interrupt_cb, NULL);

     return true;
}

bool VS1053::sciRead(uint8_t addr, uint16_t *resp){
    uint16_t data;

    //Create transaction control structure
    struct mgos_spi_txn txn;
    txn.cs = 0; //Chip select
    txn.mode = SPI_MODE;
    txn.freq = SCI_FREQ;

    //Assemble data to send (read op | addr)
    uint8_t dataOut[2];
    dataOut[0] = (uint8_t)VS1053_SCI_READ;
    dataOut[1] = addr;

    LOG(LL_DEBUG, ("Sending data %02x %02x ...", dataOut[0], dataOut[1]));
    txn.hd.tx_len = 2; //vs1053 expects one byte for read operation and one byte for register address
    txn.hd.tx_data = dataOut;
    txn.hd.dummy_len = 0; //No dummy bytes needed

    //Setup data to reveive
    txn.hd.rx_len = 2; //Two bytes expected in response
    uint8_t dataIn[2] = {0x00, 0x00};
    txn.hd.rx_data = dataIn;

    //TODO : Verify VS1053_INTERRUPT_PIN pin state to know if chip is busy

    #if 1
    //Start transaction
    if (!mgos_spi_run_txn(spi, false /* full_duplex */, &txn))
    {
        LOG(LL_ERROR, ("SPI transaction failed"));
        return false;
    }

    LOG(LL_DEBUG, ("...Received data %02x %02x", dataIn[0], dataIn[1]));
    *resp = (dataIn[0] << 8);
    *resp |= dataIn[1];

    #endif

    return true;
}

bool VS1053::sciWrite(uint8_t addr, uint8_t* dataToSend, uint16_t size){
    if(size == 0 || dataToSend == 0)
      {
          return false;
      }
      //Create transaction control structure
      struct mgos_spi_txn txn;
      txn.cs = 0; //Chip select
      txn.mode = SPI_MODE;
      txn.freq = SCI_FREQ;

      //Assemble data to send (write op | addr | data)
      uint8_t dataOut[MAX_VS1053_SPI_TX_SIZE];
      dataOut[0] = (uint8_t)VS1053_SCI_WRITE;
      dataOut[1] = addr;
      memcpy(&dataOut[2], dataToSend, size);

      LOG(LL_DEBUG, ("Sending data %02x %02x | %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x ", dataOut[0], dataOut[1], dataOut[2], dataOut[3], dataOut[4], dataOut[5], dataOut[6], dataOut[7], dataOut[8], dataOut[9]\
      , dataOut[10], dataOut[11], dataOut[12], dataOut[13], dataOut[14], dataOut[15], dataOut[16], dataOut[17]));
      txn.hd.tx_len = size + 2; //vs1053 expects one byte for read operation and one byte for register address
      txn.hd.tx_data = dataOut;
      txn.hd.dummy_len = 0; //No dummy bytes needed

      //Setup data to reveive
      txn.hd.rx_len = 0; //No byte in response to write operations
      uint8_t dummyIn;
      txn.hd.rx_data = &dummyIn;

      //TODO : Verify VS1053_INTERRUPT_PIN pin state to know if chip is busy

      #if 1
      //Start transaction
      if (!mgos_spi_run_txn(spi, false /* full_duplex */, &txn))
      {
          LOG(LL_ERROR, ("SPI transaction failed"));
          return false;
      }

      while(!readyForData()){
        //wait for the register to write data. VS1053_INTERRUPT_PIN is pulled low while the register is writing.
      }

      #endif

      return true;
}

bool VS1053::spiWrite(uint8_t *buf, uint8_t size) {
    struct mgos_spi_txn txn;
   txn.cs = 1; //Chip select
   txn.mode = SPI_MODE;
   txn.freq = SPI_FREQ;

 //  printf("%02x", buf[0]);

   txn.hd.tx_len = size;
   txn.hd.tx_data = buf;
   txn.hd.dummy_len = 0; //No dummy bytes needed

   //Setup data to reveive
   txn.hd.rx_len = 0; //No byte in response to write operations
   uint8_t dummyIn;
   txn.hd.rx_data = &dummyIn;

   //TODO : Verify VS1053_INTERRUPT_PIN pin state to know if chip is busy

   while (!readyForData());

   #if 1
   //Start transaction
   if (!mgos_spi_run_txn(spi, false /* full_duplex */, &txn))
   {
       LOG(LL_ERROR, ("SPI transaction failed"));
       return false;
   }

   #endif

   return true;
}

void VS1053::setupVS1053(){
    uint16_t response = 0;
    uint16_t regValue5 = VS1053_MODE_SM_SDINEW | VS1053_MODE_SM_RESET;
//    printf("regvalue 0x%04x\n", regValue5);
    uint8_t dataWrite5[2];
    dataWrite5[0] = (regValue5 & 0xFF00)>>8;
    dataWrite5[1] =(regValue5 & 0x00FF);
//    printf("Writing 0x%02x%02x to VS1053_REG_MODE\n", dataWrite5[0], dataWrite5[1]);
    sciWrite(VS1053_REG_MODE, dataWrite5, 2);
//    printf("After writing to VS1053_REG_MODE\n");
    sciRead(VS1053_REG_MODE, &response);

//    printf("\n\n");
    delay (10);

    //set clock

    uint16_t regValue0 = 0x6000;
    uint8_t dataWrite0[2];
    dataWrite0[0] = (regValue0 & 0xFF00)>>8;
    dataWrite0[1] =(regValue0 & 0x00FF);
//    printf("Writing 0x%02x%02x to VS1053_REG_CLOCKF\n", dataWrite0[0], dataWrite0[1]);
    sciWrite(VS1053_REG_CLOCKF, dataWrite0, 2);
//    printf("After writing to VS1053_REG_MODE\n");
    sciRead(VS1053_REG_CLOCKF, &response);

//    printf("\n\n");
    delay (10);

    uint16_t regValue1 = VS1053_MODE_SM_LINE1 | VS1053_MODE_SM_SDINEW;
    uint8_t dataWrite1[2];
    dataWrite1[0] = (regValue1 & 0xFF00)>>8;
    dataWrite1[1] =(regValue1 & 0x00FF);
//    printf("Writing 0x%02x%02x to VS1053_REG_MODE\n", dataWrite1[0], dataWrite1[1]);
    sciWrite(VS1053_REG_MODE, dataWrite1, 2);
//    printf("After writing to VS1053_REG_MODE\n");
    sciRead(VS1053_REG_MODE, &response);
    // resync

//    printf("\n\n");
    delay (10);

    uint16_t regValue2 = 0x1e29;
    uint8_t dataWrite2[2];
    dataWrite2[0] = (regValue2 & 0xFF00)>>8;
    dataWrite2[1] =(regValue2 & 0x00FF);
//    printf("Writing 0x%02x%02x to VS1053_REG_WRAMADDR\n", dataWrite2[0], dataWrite2[1]);
    sciWrite(VS1053_REG_WRAMADDR, dataWrite2, 2);
//    printf("After writing to VS1053_REG_WRAMADDR\n");
    sciRead(VS1053_REG_WRAMADDR, &response);

//    printf("\n\n");
    delay (10);

    uint8_t dataWrite3[1] = {0x00};
//    printf("Writing 0x%02x to VS1053_REG_WRAM\n", dataWrite3[0]);
    sciWrite(VS1053_REG_WRAM, dataWrite3, 1);
//    printf("After writing to VS1053_REG_WRAM\n");
    sciRead(VS1053_REG_WRAM, &response);

    // As explained in datasheet, set twice 0 in REG_DECODETIME to set time back to 0

    uint8_t dataWrite4[1] = {0x00};
    sciWrite(VS1053_REG_DECODETIME, dataWrite4, 1);
    sciWrite(VS1053_REG_DECODETIME, dataWrite4, 1);

    // printf("REG before Test mode\n");
    // VS1053ReadWord(VS1053_REG_MODE, &response);
    // VS1053EnableTestMode(true);
    // printf("REG After Test mode\n");
    // VS1053ReadWord(VS1053_REG_MODE, &response);

//
//    printf("Stat =");
//    sciRead(VS1053_REG_STATUS, &response);
//    printf("   ClkF = ");
//    sciRead(VS1053_REG_CLOCKF, &response);
//    printf("   Vol. = ");
//    sciRead(VS1053_REG_VOLUME, &response);
//

    delay (100);

    printf("Finished setup VS1053\n");
}


void VS1053::reset() {
   // TODO: http://www.vlsi.fi/player_vs1011_1002_1003/modularplayer/vs10xx_8c.html#a3
   // hardware reset
    delay(100);
    softReset();
    delay(100);

    uint16_t regValue = 0x6000;
    uint8_t dataWrite[2];
    dataWrite[0] = (regValue & 0xFF00)>>8;
    dataWrite[1] =(regValue & 0x00FF);
    sciWrite(VS1053_REG_CLOCKF, dataWrite, 2);

    setVolume(40, 40);
}

void VS1053::softReset(void) {
    uint16_t regValue = VS1053_MODE_SM_SDINEW | VS1053_MODE_SM_RESET;
    uint8_t dataWrite[2];
    dataWrite[0] = (regValue & 0xFF00)>>8;
    dataWrite[1] =(regValue & 0x00FF);
    sciWrite(VS1053_REG_MODE, dataWrite, 2);
    delay(100);
}

void VS1053::delay(int ms){
  mgos_usleep(ms * 1000);
}
