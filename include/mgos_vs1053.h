#ifdef __cplusplus
#include "VS1053.h"
#else
typedef struct VS1053tag VS1053;
// typedef struct Adafruit_VS1053tag Adafruit_VS1053;
// typedef struct Adafruit_VS1053_FilePlayertag Adafruit_VS1053_FilePlayer;
#endif

#ifdef __cplusplus
extern "C"
{
#endif


VS1053 *mgos_vs1053_create();

int mgos_vs1053_begin(VS1053 *player);

void mgos_vs1053_sineTest(VS1053 *player, int n, int ms);

void mgos_vs1053_playFile(VS1053 *player, char *fileName, mgos_vs1053_callback_t cb, char* userdata);

void mgos_vs1053_closeFile(VS1053 *player);

void mgos_vs1053_stopPlaying(VS1053 *player);

void mgos_vs1053_pausePlaying(VS1053 *player);

bool mgos_vs1053_playing(VS1053 *player);

bool mgos_vs1053_paused(VS1053 *player);

bool mgos_vs1053_stopped(VS1053 *player);

// void /*File*/ mgos_adafruit_VS1053_currentTrack(Adafruit_VS1053_FilePlayer *player);

// bool mgos_adafruit_VS1053_playingMusic(Adafruit_VS1053_FilePlayer *player);

void mgos_vs1053_close(VS1053 *player);

void mgos_vs1053_setVolume(VS1053 *player, int left, int right);

#ifdef __cplusplus
}
#endif
