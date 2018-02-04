#include "mgos_SD.h"
#include "mgos_spi.h"
#include "mgos_timers.h"
#include "mgos_gpio.h"
#include "mgos_system.h"

#include "VS1053.h"


#define SPI_CTRL_TXN 0
#define SPI_DATA_TXN 1

SD* sd;
FILE* currentTrack;
struct mgos_spi *spi;

struct mgos_spi_txn ctl_txn = {
    .cs = 0,
    .mode = 0,
    .freq = 250000,
};

struct mgos_spi_txn data_txn = {
    .cs = 1,
    .mode = 0,
    .freq = 8000000,
};


VS1053::VS1053(){

}

int VS1053::begin(){
  initSDCard();
  initSPI();
  return 1;
}

void VS1053::playFile(const char *trackname){

}

void VS1053::stopPlaying(){};
void VS1053::pausePlaying(){};
bool VS1053::stopped(){
  return false;
};
bool VS1053::paused(){
  return false;
};

void VS1053::setVolume(uint8_t left, uint8_t right){
  uint16_t v;
  v = left;
  v <<= 8;
  v |= right;

  sciWrite(VS1053_REG_VOLUME, v);
}


void VS1053::sineTest(uint8_t n, uint16_t ms){
    uint16_t mode = sciRead(VS1053_REG_MODE);
    printf("mode %d\n", mode);
    mode |= 0x0020;
    sciWrite(VS1053_REG_MODE, mode);
  	 //  delay(10);




    spiWrite(SPI_DATA_TXN, 0x53);
    spiWrite(SPI_DATA_TXN, 0xEF);
    spiWrite(SPI_DATA_TXN, 0x6E);
    spiWrite(SPI_DATA_TXN, n);
    spiWrite(SPI_DATA_TXN, 0x00);
    spiWrite(SPI_DATA_TXN, 0x00);
    spiWrite(SPI_DATA_TXN, 0x00);
    spiWrite(SPI_DATA_TXN, 0x00);

    mgos_usleep(ms * 1000);


    spiWrite(SPI_DATA_TXN, 0x45);
    spiWrite(SPI_DATA_TXN, 0x78);
    spiWrite(SPI_DATA_TXN, 0x69);
    spiWrite(SPI_DATA_TXN, 0x74);
    spiWrite(SPI_DATA_TXN, 0x00);
    spiWrite(SPI_DATA_TXN, 0x00);
    spiWrite(SPI_DATA_TXN, 0x00);
    spiWrite(SPI_DATA_TXN, 0x00);
}







void VS1053::initSDCard(){
  sd = mgos_sd_create();
  mgos_sd_begin (sd, MOSI_PIN, MISO_PIN, CLOCK_PIN, SDCARD_CS_PIN);
}

void VS1053::initSPI(){
  spi = mgos_spi_get_global();
  if (spi == NULL) {
    LOG(LL_ERROR, ("SPI is not configured, make sure spi.enable is true"));
  }
}

uint16_t VS1053::sciRead(uint8_t addr){
  uint16_t data;
  //
  // spiWrite(SPI_CTRL_TXN, VS1053_SCI_READ);
  // spiWrite(SPI_CTRL_TXN, addr);
  // mgos_usleep(10);
  // data = spiRead(SPI_CTRL_TXN);
  // data <<= 8;
  // data |= spiRead(SPI_CTRL_TXN);
  //
  // return data;
  printf("addr %x", addr);

   uint8_t rx_data[3] = {0, 0, 0};
   spiRead(SPI_CTRL_TXN, addr, rx_data);
   data = rx_data[0];
   data <<= 8;
   data |= rx_data[1];
   return data;
}

void VS1053::sciWrite(uint8_t addr, uint16_t data){
  // spiWrite(SPI_CTRL_TXN, VS1053_SCI_WRITE);
  // spiWrite(SPI_CTRL_TXN, addr);
  // spiWrite(SPI_CTRL_TXN, data >> 8);
  // spiWrite(SPI_CTRL_TXN, data & 0xFF);
  uint8_t tx[3];
  tx[0] = addr;
  tx[1] = data >> 8;
  tx[2] = data & 0xFF;
  spiWrite(SPI_CTRL_TXN, tx, 3);
}

void VS1053::spiRead(int spi_cs_txn, uint8_t addr, uint8_t *rx_data){

  mgos_spi_txn txn;
  if(spi_cs_txn == SPI_CTRL_TXN){
    txn = ctl_txn;
  }else if (spi_cs_txn == SPI_DATA_TXN){
    txn = data_txn;
  }

  uint8_t tx_data[1];
  tx_data[0] = addr;
  /* Half-duplex, command/response transaction setup */
  /* Transmit 1 byte from tx_data. */
  txn.hd.tx_len = 1;
  txn.hd.tx_data = tx_data;
  txn.hd.dummy_len = 1;
  txn.hd.rx_len = 3;
  txn.hd.rx_data = rx_data;
  if (!mgos_spi_run_txn(spi, false, &txn)) {
    LOG(LL_ERROR, ("SPI transaction failed"));
    return;
  }
  printf("spiRead @ %x -> %x %x %x\n", tx_data[0],rx_data[0], rx_data[1], rx_data[2]);
}



void VS1053::spiWrite(int spi_cs_txn, uint8_t c) {

  uint8_t x __attribute__ ((aligned (32))) = c;
  spiWrite(spi_cs_txn, &x, 1);
}

void VS1053::spiWrite(int spi_cs_txn, uint8_t *c, uint8_t num) {
  mgos_spi_txn txn;
  if(spi_cs_txn == SPI_CTRL_TXN){
    txn = ctl_txn;
  }else if (spi_cs_txn == SPI_DATA_TXN){
    txn = data_txn;
  }

  uint8_t rx_data[3] = {0, 0, 0};
  /* Half-duplex, command/response transaction setup */
  /* Transmit 1 byte from tx_data. */
  txn.hd.tx_len = num;
  txn.hd.tx_data = c;
  txn.hd.dummy_len = 0;
  txn.hd.rx_len = 3;
  txn.hd.rx_data = rx_data;
  if (!mgos_spi_run_txn(spi, false, &txn)) {
    LOG(LL_ERROR, ("SPI transaction failed"));
    return;
  }

  printf("spiWrite %x -> %x %x %x\n", c[0], rx_data[0], rx_data[1], rx_data[2]);
}


void VS1053::reset() {
  // TODO: http://www.vlsi.fi/player_vs1011_1002_1003/modularplayer/vs10xx_8c.html#a3
  // hardware reset
  mgos_usleep(100 * 1000);
  softReset();
  mgos_usleep(100 * 1000);

  sciWrite(VS1053_REG_CLOCKF, 0x6000);

  setVolume(40, 40);
}

void VS1053::softReset(void) {
  sciWrite(VS1053_REG_MODE, VS1053_MODE_SM_SDINEW | VS1053_MODE_SM_RESET);
  mgos_usleep(100 * 1000);
}
