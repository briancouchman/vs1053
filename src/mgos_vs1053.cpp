#include "mgos_vs1053.h"
// #include "mongoose/mongoose.h"

VS1053 *mgos_vs1053_create(){
  return new VS1053();
}

int mgos_vs1053_begin(VS1053 *player){
  if (player == nullptr) return 0;
  return player->begin();
}

void mgos_vs1053_sineTest(VS1053 *player, int n, int ms){
  if (player == nullptr) return;
  player->sineTest((uint8_t) n, (uint8_t) ms);
}

void mgos_vs1053_playFile(VS1053 *player, char *fileName, mgos_vs1053_callback_t cb, char* userdata){
  if (player == nullptr) return;
  player->playFile(fileName, cb, userdata);
}

void mgos_vs1053_closeFile(VS1053 *player){
  if (player == nullptr) return;
  player->closeFile(true);
}

void mgos_vs1053_stopPlaying(VS1053 *player){
  if (player != nullptr){
    player->stopPlaying();
  }
}

void mgos_vs1053_pausePlaying(VS1053 *player){
  if (player != nullptr){
    player->pausePlaying();
  }
}

bool mgos_vs1053_playing(VS1053 *player){
  if (player == nullptr) return 0;
  return player->playing();
}

bool mgos_vs1053_paused(VS1053 *player){
  if (player == nullptr) return 0;
  return player->paused();
}

bool mgos_vs1053_stopped(VS1053 *player){
  if (player == nullptr) return 0;
  return player->stopped();
}

// void mgos_vs1053_currentTrack(Adafruit_VS1053_FilePlayer *player){
//   if (player == nullptr) return "";
//   return player->currentTrack();
// }

void mgos_vs1053_setVolume(VS1053 *player, int left, int right){
  if (player == nullptr) return;
  return player->setVolume(left, right);
}

// int mgos_vs1053_playingMusic(Adafruit_VS1053_FilePlayer *player){
//   if (player == nullptr) return 0;
//   return player->playingMusic();
// }

void mgos_vs1053_close(VS1053 *player) {
    if (player != nullptr) {
        delete player;
        player = nullptr;
    }
}
