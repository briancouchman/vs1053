#ifndef VS1053_H
#define VS1053_H

#include <stdint.h>
#include "mgos_features.h"
#include "mgos_spi.h"




#define SDCARD_MISO_PIN 19
#define SDCARD_MOSI_PIN 18
#define SDCARD_CLCK_PIN 5
#define SDCARD_CS_PIN 14

#define VS1053_MISO_PIN 27
#define VS1053_MOSI_PIN 12
#define VS1053_CLCK_PIN 13
#define VS1053_CTL_CS_PIN 32
#define VS1053_DATA_CS_PIN 33
#define VS1053_INTERRUPT_PIN 15
#define VS1053_SPI_HOST 3 //VSPI


#define VS1053_FILEPLAYER_TIMER0_INT 255 // allows useInterrupt to accept pins 0 to 254
#define VS1053_FILEPLAYER_PIN_INT 5

#define VS1053_SCI_READ 0x03
#define VS1053_SCI_WRITE 0x02

#define VS1053_REG_MODE  0x00
#define VS1053_REG_STATUS 0x01
#define VS1053_REG_BASS 0x02
#define VS1053_REG_CLOCKF 0x03
#define VS1053_REG_DECODETIME 0x04
#define VS1053_REG_AUDATA 0x05
#define VS1053_REG_WRAM 0x06
#define VS1053_REG_WRAMADDR 0x07
#define VS1053_REG_HDAT0 0x08
#define VS1053_REG_HDAT1 0x09
#define VS1053_REG_VOLUME 0x0B

#define VS1053_GPIO_DDR 0xC017
#define VS1053_GPIO_IDATA 0xC018
#define VS1053_GPIO_ODATA 0xC019

#define VS1053_INT_ENABLE  0xC01A

#define VS1053_MODE_SM_DIFF 0x0001
#define VS1053_MODE_SM_LAYER12 0x0002
#define VS1053_MODE_SM_RESET 0x0004
#define VS1053_MODE_SM_CANCEL 0x0008
#define VS1053_MODE_SM_EARSPKLO 0x0010
#define VS1053_MODE_SM_TESTS 0x0020
#define VS1053_MODE_SM_STREAM 0x0040
#define VS1053_MODE_SM_SDINEW 0x0800
#define VS1053_MODE_SM_ADPCM 0x1000
#define VS1053_MODE_SM_LINE1 0x4000
#define VS1053_MODE_SM_CLKRANGE 0x8000


#define VS1053_SCI_AIADDR 0x0A
#define VS1053_SCI_AICTRL0 0x0C
#define VS1053_SCI_AICTRL1 0x0D
#define VS1053_SCI_AICTRL2 0x0E
#define VS1053_SCI_AICTRL3 0x0F

#define VS1053_DATABUFFERLEN 32


typedef void (*mgos_vs1053_callback_t)(void *user_data, const char* trackname);

class VS1053 {
  public:
    VS1053 ();
    int begin();
    void playFile(const char *trackname, mgos_vs1053_callback_t cb, char* userdata);
    void closeFile(bool executeCallback);
    void sineTest(uint8_t n, uint16_t ms);
    void stopPlaying();
    void pausePlaying();
    bool stopped();
    bool paused();
    bool playing();
    void setVolume(uint8_t left, uint8_t right);

    void feedBuffer();

  private:
    int initSDCard();
    int initSPI();
    void setupVS1053();

    bool sciRead(uint8_t addr, uint16_t *resp);
    bool sciWrite(uint8_t addr, uint8_t* dataToSend, uint16_t size);

    // void spiRead(int spi_cs_txn, uint8_t addr, uint8_t *rx);
    bool spiWrite(uint8_t *c, uint8_t num);

    bool readyForData();

    void reset();
    void softReset();
    void delay(int ms);

    volatile bool playingMusic;
};

#endif
