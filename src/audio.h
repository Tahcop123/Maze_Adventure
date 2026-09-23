#ifndef _AUDIO_H
#define _AUDIO_H

void InitAudio(void);
void ToggleBGM(void);
int IsBGMOn(void);
void PlayMoveSound(void);
void PlayKeySound(void);
void PlayCoinSound(void);
void PlayHurtSound(void);
void PlayBombSound(void);
void PlayWinSound(void);
void PlayLoseSound(void);
void PlayDoorSound(void);
void PlayItemSound(void);
void CloseAudioSys(void);

#endif
