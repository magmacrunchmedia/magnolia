#ifndef AUDIO_H
#define AUDIO_H

/* PCM16 playback over libogc's ASND.
 *
 * Assets are raw signed 16-bit little-endian stereo at 48kHz -- the format the
 * conversion recipe in the game docs produces:
 *   ffmpeg -i in.ogg -f s16le -acodec pcm_s16le -ar 48000 -ac 2 out.pcm
 *
 * Clips are held in main RAM, so this suits short effects and a modest music
 * loop. Budget roughly 192KB per second of stereo 48kHz audio against the Wii's
 * 24MB, and prefer trimming or downsampling a long track over streaming it.
 */

#define AUDIO_MAX_SFX 8

int  audio_init(void);
void audio_shutdown(void);
int  audio_available(void);

/* slot is 0..AUDIO_MAX_SFX-1. path is relative to the app's SD directory.
   Returns 1 on success; a failed load leaves the slot silent, not broken. */
int  audio_load_sfx(int slot, const char *path);
void audio_play_sfx(int slot);

/* Variants for PCM already in memory -- typically linked into the binary. The
   buffer is used in place when it is already 32-byte aligned (ASND DMAs from
   it); otherwise it is copied into an aligned one, so a change in how assets
   are emitted costs memory rather than producing silent DMA corruption. */
int  audio_load_sfx_mem(int slot, const void *data, unsigned int len);
int  audio_play_music_mem(const void *data, unsigned int len);

/* Loads and starts a looping track, replacing any current one. */
int  audio_play_music(const char *path);
/* Whether a music track is currently loaded in memory. */
int  audio_music_loaded(void);
void audio_stop_music(void);

/* Muting is remembered across calls, so a track started while muted stays
   silent until unmuted rather than blaring on the next state change. */
void audio_set_muted(int muted);
int  audio_get_muted(void);

/* 0..255, applied to subsequently started sounds. */
void audio_set_music_volume(int vol);
void audio_set_sfx_volume(int vol);

#endif
