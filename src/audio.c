#include "inc.h"
#include "audio.h"

static AudioStream audioStream;
static float sfxFreq = 0;
static float sfxDuration = 0;
static float sfxTime = 0;
static int sfxType = 0;
static float sfxVolume = 0.3f;

// BGM state
static int bgmEnabled = 1;
static int bgmNoteIdx = 0;
static float bgmNoteTime = 0;

// Simple melody (frequencies in Hz) - minor key adventure theme
static float bgmMelody[] = {
    220, 261, 293, 329, 293, 261, 220, 196,
    220, 261, 293, 349, 329, 293, 261, 220,
    196, 220, 261, 293, 261, 220, 196, 174,
    220, 261, 293, 329, 392, 349, 293, 261
};
static int bgmMelodyLen = 32;

static void MyAudioCallback(void *buffer, unsigned int frames)
{
    float *d = (float*)buffer;
    float sampleRate = 44100.0f;

    for (unsigned int i = 0; i < frames; i++) {
        float sample = 0;

        // SFX
        if (sfxTime < sfxDuration && sfxFreq > 0) {
            float t = sfxTime;
            if (sfxType == 0) sample += sinf(2 * PI * sfxFreq * t);
            else if (sfxType == 1) sample += (sinf(2 * PI * sfxFreq * t) > 0) ? 1.0f : -1.0f;
            else sample += (float)(rand() % 1000) / 500.0f - 1.0f;
            float env = 1.0f - (sfxTime / sfxDuration);
            sample *= env * sfxVolume;
            sfxTime += 1.0f / sampleRate;
        }

        // BGM
        if (bgmEnabled) {
            bgmNoteTime += 1.0f / sampleRate;
            if (bgmNoteTime > 0.35f) {
                bgmNoteTime = 0;
                bgmNoteIdx = (bgmNoteIdx + 1) % bgmMelodyLen;
            }
            float noteFreq = bgmMelody[bgmNoteIdx];
            float noteT = bgmNoteTime;
            float noteEnv = (noteT < 0.05f) ? noteT / 0.05f :
                            (noteT > 0.3f) ? (0.35f - noteT) / 0.05f : 1.0f;
            if (noteEnv < 0) noteEnv = 0;
            // Triangle wave for softer sound
            float phase = fmodf(noteFreq * noteT, 1.0f);
            float tri = (phase < 0.5f) ? (phase * 4 - 1) : (3 - phase * 4);
            sample += tri * noteEnv * 0.08f;
            // Bass note (octave lower)
            float bassFreq = noteFreq / 2;
            float bassPhase = fmodf(bassFreq * noteT, 1.0f);
            float bassTri = (bassPhase < 0.5f) ? (bassPhase * 4 - 1) : (3 - bassPhase * 4);
            sample += bassTri * noteEnv * 0.05f;
        }

        d[i] = sample;
    }
}

static void PlayTone(float freq, float duration, int type, float vol)
{
    sfxFreq = freq;
    sfxDuration = duration;
    sfxTime = 0;
    sfxType = type;
    sfxVolume = vol;
}

void InitAudio(void)
{
    InitAudioDevice();
    audioStream = LoadAudioStream(44100, 32, 1);
    SetAudioStreamCallback(audioStream, MyAudioCallback);
    PlayAudioStream(audioStream);
}

void CloseAudioSys(void)
{
    StopAudioStream(audioStream);
    UnloadAudioStream(audioStream);
    CloseAudioDevice();
}

void PlayMoveSound(void) { PlayTone(300, 0.05f, 1, 0.15f); }
void PlayKeySound(void) { PlayTone(880, 0.15f, 0, 0.3f); PlayTone(1100, 0.2f, 0, 0.25f); }
void PlayCoinSound(void) { PlayTone(988, 0.08f, 0, 0.25f); PlayTone(1319, 0.12f, 0, 0.2f); }
void PlayHurtSound(void) { PlayTone(150, 0.2f, 2, 0.35f); }
void PlayBombSound(void) { PlayTone(80, 0.4f, 2, 0.4f); PlayTone(120, 0.3f, 1, 0.3f); }
void PlayWinSound(void) {
    PlayTone(523, 0.15f, 0, 0.3f);
    PlayTone(659, 0.15f, 0, 0.3f);
    PlayTone(784, 0.15f, 0, 0.3f);
    PlayTone(1047, 0.3f, 0, 0.35f);
}
void PlayLoseSound(void) {
    PlayTone(392, 0.2f, 0, 0.3f);
    PlayTone(349, 0.2f, 0, 0.3f);
    PlayTone(311, 0.2f, 0, 0.3f);
    PlayTone(261, 0.4f, 0, 0.35f);
}
void PlayDoorSound(void) { PlayTone(200, 0.15f, 1, 0.25f); PlayTone(400, 0.1f, 0, 0.2f); }
void PlayItemSound(void) { PlayTone(660, 0.1f, 0, 0.25f); PlayTone(880, 0.15f, 0, 0.2f); }
