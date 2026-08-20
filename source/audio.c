#include <asndlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include "audio.h"
#include "core.h"

#define PCM_RATE     AUDIO_RATE_DEFAULT

/* ffmpeg's s16le output is little-endian; the Wii is big-endian, so the voice
   format has to say so or every sample comes out as noise. */
static u32 voice_format(AudioFormat fmt) {
    return (fmt == AUDIO_MONO_16) ? VOICE_MONO_16BIT_LE : VOICE_STEREO_16BIT_LE;
}

typedef struct {
    void *data;
    u32   size;
    int   owned;      /* 0 when pointing at a linked-in buffer we must not free */
    u32   format;     /* the ASND voice format this clip's samples are in */
    s32   rate;
} Clip;

/* ASND DMAs straight out of these buffers. */
#define AUDIO_ALIGN 32

static Clip sfx[AUDIO_MAX_SFX];
static Clip music;
static int  initialised = 0;
static int  muted = 0;
static int  music_vol = 200;
static int  sfx_vol = 255;
static int  music_voice = -1;

/* Point at the caller's buffer when alignment allows, else take an aligned copy. */
static int adopt_clip(Clip *c, const void *data, u32 len) {
    c->data = NULL;
    c->size = 0;
    c->owned = 0;
    if (!data || !len) return 0;

    if (((u32)data % AUDIO_ALIGN) == 0) {
        c->data = (void *)data;
        c->size = len;
        return 1;
    }

    void *buf = memalign(AUDIO_ALIGN, len);
    if (!buf) return 0;
    memcpy(buf, data, len);
    c->data = buf;
    c->size = len;
    c->owned = 1;
    return 1;
}

static int load_clip(Clip *c, const char *path) {
    c->data = NULL;
    c->size = 0;
    c->owned = 1;

    FILE *f = fopen(magnolia_asset_path(path), "rb");
    if (!f) return 0;

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (size <= 0) { fclose(f); return 0; }

    /* ASND wants 32-byte aligned buffers for DMA. */
    void *buf = memalign(32, (size_t)size);
    if (!buf) { fclose(f); return 0; }

    size_t got = fread(buf, 1, (size_t)size, f);
    fclose(f);
    if (got != (size_t)size) { free(buf); return 0; }

    c->data = buf;
    c->size = (u32)size;
    return 1;
}

static void free_clip(Clip *c) {
    if (c->data && c->owned) free(c->data);
    c->data = NULL;
    c->size = 0;
    c->owned = 0;
}

int audio_init(void) {
    if (initialised) return 1;
    ASND_Init();
    ASND_Pause(0);
    memset(sfx, 0, sizeof(sfx));
    memset(&music, 0, sizeof(music));
    initialised = 1;
    return 1;
}

void audio_shutdown(void) {
    if (!initialised) return;
    audio_stop_music();
    for (int i = 0; i < AUDIO_MAX_SFX; i++) free_clip(&sfx[i]);
    free_clip(&music);
    ASND_Pause(1);
    ASND_End();
    initialised = 0;
}

int audio_available(void) { return initialised; }

int audio_load_sfx_fmt(int slot, const char *path, AudioFormat fmt, int rate) {
    if (!initialised || slot < 0 || slot >= AUDIO_MAX_SFX) return 0;
    free_clip(&sfx[slot]);
    if (!load_clip(&sfx[slot], path)) return 0;
    sfx[slot].format = voice_format(fmt);
    sfx[slot].rate   = rate > 0 ? rate : PCM_RATE;
    return 1;
}

int audio_load_sfx_mem_fmt(int slot, const void *data, unsigned int len,
                           AudioFormat fmt, int rate) {
    if (!initialised || slot < 0 || slot >= AUDIO_MAX_SFX) return 0;
    free_clip(&sfx[slot]);
    if (!adopt_clip(&sfx[slot], data, len)) return 0;
    sfx[slot].format = voice_format(fmt);
    sfx[slot].rate   = rate > 0 ? rate : PCM_RATE;
    return 1;
}

int audio_load_sfx(int slot, const char *path) {
    return audio_load_sfx_fmt(slot, path, AUDIO_STEREO_16, PCM_RATE);
}

int audio_load_sfx_mem(int slot, const void *data, unsigned int len) {
    return audio_load_sfx_mem_fmt(slot, data, len, AUDIO_STEREO_16, PCM_RATE);
}

void audio_play_sfx(int slot) {
    if (!initialised || muted) return;
    if (slot < 0 || slot >= AUDIO_MAX_SFX || !sfx[slot].data) return;

    s32 voice = ASND_GetFirstUnusedVoice();
    /* Voice 0 is reserved for music so a burst of effects cannot evict it. */
    if (voice <= 0 || voice >= MAX_SND_VOICES) return;

    ASND_SetVoice(voice, sfx[slot].format, sfx[slot].rate, 0,
                  sfx[slot].data, (s32)sfx[slot].size,
                  sfx_vol, sfx_vol, NULL);
}

static void start_music_voice(void) {
    music_voice = 0;
    int vol = muted ? 0 : music_vol;
    ASND_SetInfiniteVoice(music_voice, music.format, music.rate, 0,
                          music.data, (s32)music.size, vol, vol);
}

int audio_play_music_mem_fmt(const void *data, unsigned int len,
                             AudioFormat fmt, int rate) {
    if (!initialised) return 0;
    audio_stop_music();
    if (!adopt_clip(&music, data, len)) return 0;
    music.format = voice_format(fmt);
    music.rate   = rate > 0 ? rate : PCM_RATE;
    start_music_voice();
    return 1;
}

int audio_play_music_fmt(const char *path, AudioFormat fmt, int rate) {
    if (!initialised) return 0;
    audio_stop_music();
    if (!load_clip(&music, path)) return 0;
    music.format = voice_format(fmt);
    music.rate   = rate > 0 ? rate : PCM_RATE;
    start_music_voice();
    return 1;
}

int audio_play_music_mem(const void *data, unsigned int len) {
    return audio_play_music_mem_fmt(data, len, AUDIO_STEREO_16, PCM_RATE);
}

int audio_play_music(const char *path) {
    return audio_play_music_fmt(path, AUDIO_STEREO_16, PCM_RATE);
}

int audio_music_loaded(void) { return music.data != NULL; }

void audio_stop_music(void) {
    if (!initialised) return;
    if (music_voice >= 0) {
        ASND_StopVoice(music_voice);
        music_voice = -1;
    }
    free_clip(&music);
}

void audio_set_muted(int m) {
    muted = m ? 1 : 0;
    if (initialised && music_voice >= 0) {
        int vol = muted ? 0 : music_vol;
        ASND_ChangeVolumeVoice(music_voice, vol, vol);
    }
}

int audio_get_muted(void) { return muted; }

void audio_set_music_volume(int vol) {
    if (vol < 0) vol = 0;
    if (vol > 255) vol = 255;
    music_vol = vol;
    if (initialised && music_voice >= 0 && !muted) {
        ASND_ChangeVolumeVoice(music_voice, music_vol, music_vol);
    }
}

void audio_set_sfx_volume(int vol) {
    if (vol < 0) vol = 0;
    if (vol > 255) vol = 255;
    sfx_vol = vol;
}
