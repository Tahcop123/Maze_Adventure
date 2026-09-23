#include "inc.h"
#include "audio.h"

// Small polyphonic mixer: previously a single global voice state was reused
// for every sound, so two-tone effects (key, coin, win/lose jingles, ...)
// had their first tone immediately overwritten by the second.

#define MAX_VOICES 8

typedef struct {
    int active;
    int type;       // 0=sine, 1=square, 2=noise
    float freq;
    float t;
    float duration;
    float volume;
} Voice;

static AudioStream audioStream;
static Voice voices[MAX_VOICES];
static float sampleRate = 44100.0f;

// BGM state
static int bgmEnabled = 1;
static int bgmNoteIdx = 0;
static float bgmNoteTime = 0;

// The audio callback runs on its own thread: it must not call rand()
// (thread-safety, and it would desync the game's seeded map RNG). A private
// xorshift32 PRNG generates the noise timbre instead.
static unsigned int noiseState = 0x9E3779B9u;

static float NoiseSample(void)
{
    noiseState ^= noiseState << 13;
    noiseState ^= noiseState >> 17;
    noiseState ^= noiseState << 5;
    return ((noiseState & 0xFFFF) / 32768.0f) - 1.0f;
}

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

    for (unsigned int i = 0; i < frames; i++) {
        float sample = 0;

        // Mix every active SFX voice
        for (int v = 0; v < MAX_VOICES; v++) {
            Voice *vo = &voices[v];
            if (!vo->active) continue;
            float s;
            if (vo->type == 0) s = sinf(2 * PI * vo->freq * vo->t);
            else if (vo->type == 1) s = (sinf(2 * PI * vo->freq * vo->t) > 0) ? 1.0f : -1.0f;
            else s = NoiseSample();
            float env = 1.0f - (vo->t / vo->duration);
            if (env < 0) env = 0;
            sample += s * env * vo->volume;
            vo->t += 1.0f / sampleRate;
            if (vo->t >= vo->duration) vo->active = 0;
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
    Voice *pick = NULL;
    for (int v = 0; v < MAX_VOICES; v++) {
        if (!voices[v].active) { pick = &voices[v]; break; }
    }
    if (!pick) {
        // All voices busy: steal the one closest to finishing
        pick = &voices[0];
        for (int v = 1; v < MAX_VOICES; v++)
            if (voices[v].t > pick->t) pick = &voices[v];
    }
    pick->active = 1;
    pick->type = type;
    pick->freq = freq;
    pick->t = 0;
    pick->duration = duration;
    pick->volume = vol;
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

void ToggleBGM(void) { bgmEnabled = !bgmEnabled; }
int IsBGMOn(void) { return bgmEnabled; }

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
