#include "wl_def.h"

#include "SDL.h"
#include "SDL_mixer.h"

#include "fmopl.h"

/* not yet finished */

/*
WAV Header for prepending onto raw sounds
Configured for 11025Hz, Mono, 8-bit, Unsigned

00: ID 'RIFF'
04: DWORD chunkSize (num samples + 44 - 8)
08: ID 'WAVE'
0c: ID 'fmt '
10: DWORD chunkSize
14: WORD wFormatTag
16: WORD wChannels
18: DWORD dwSamplesPerSec
1c: DWORD dwAvgBytesPerSec
20: WORD wBlockAlign
22: WORD wBitsPerSample
24: ID 'data'
28: DWORD chunkSize (num samples)
2c: data...
*/
static const char wavheader[44] = {
0x52, 0x49, 0x46, 0x46, 0xff, 0xff, 0xff, 0x0ff,
0x57, 0x41, 0x56, 0x45, 0x66, 0x6d, 0x74, 0x20,
0x10, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00,
0x11, 0x2b, 0x00, 0x00, 0x11, 0x2b, 0x00, 0x00,
0x01, 0x00, 0x08, 0x00, 0x64, 0x61, 0x74, 0x61,
0xff, 0xff, 0xff, 0xff };

/* old 7000Hz-based header */
/*
static const char wavheader[44] = {
0x52, 0x49, 0x46, 0x46, 0xff, 0xff, 0xff, 0xff,
0x57, 0x41, 0x56, 0x45, 0x66, 0x6d, 0x74, 0x20,
0x10, 0x00, 0x00, 0x00, 0x01, 0x00, 0x01, 0x00,
0x58, 0x1b, 0x00, 0x00, 0x58, 0x1b, 0x00, 0x00,
0x01, 0x00, 0x08, 0x00, 0x64, 0x61, 0x74, 0x61,
0xff, 0xff, 0xff, 0xff };
*/

/* variables that the callback uses */
typedef struct SoundVars {
	/* read-only for the callback */
	SDL_AudioSpec *spec;
	boolean active;				/* should mixer update? */
	
	word *DigiList;
	
	boolean mix_adlib_music;
	boolean mix_adlib_sound;
	boolean mix_digi_sound;

	int adlib_music_id;
	int adlib_sound_id;
	int digi_sound_id;
	
	byte *adlib_music_data;
	byte *adlib_sound_data;
	
	int digi_sound_left;
	int digi_sound_right;
	int digi_sound_positioned;
	
	int adlib_sound_priority;
	int digi_sound_priority;

	boolean adlib_music_paused;
	
	/* read-write for the callback */
	FM_OPL *OPL;
	boolean adlib_music_reset;
	boolean adlib_sound_reset;
	boolean digi_sound_reset;
	
	boolean adlib_music_playing;
	boolean adlib_sound_playing;
	boolean digi_sound_playing;
	
	int adlib_music_length;
	int adlib_sound_length;
	int digi_sound_length;
		
	int adlib_music_pos;
	int adlib_sound_pos;
	int digi_sound_pos;
	
	int adlib_music_counter;
	int adlib_sound_counter;
} SoundVars;

/* global variables */
boolean SD_Started;
boolean AdLibPresent, SoundBlasterPresent;

SDMode SoundMode;
SMMode MusicMode;
SDSMode DigiMode;

/* local variables */
static SoundVars sound;			/* callback variables */

static fixed globalsoundx, globalsoundy;
static int leftchannel, rightchannel;

static boolean wantpositioned;

static int NumDigi;

#if 0
#define PACKED __attribute__((packed))

typedef	struct {
	longword length;
	word priority;
} PACKED SoundCommon;

typedef	struct {
	SoundCommon common;
	byte data[1];
} PACKED PCSound;

typedef	struct {
	byte mChar, cChar, mScale, cScale, mAttack, cAttack, mSus, cSus,
		mWave, cWave, nConn, voice, mode, unused[3];
} PACKED Instrument;

typedef	struct {
	SoundCommon common;
	Instrument inst;
	byte block, data[1];
} PACKED AdLibSound;

typedef	struct {
	word length, values[1];
} PACKED MusicGroup;
#endif

#define alChar		0x20
#define alScale		0x40
#define alAttack	0x60
#define alSus		0x80
#define alFreqL		0xA0
#define alFreqH		0xB0
#define alEffects	0xBD
#define alFeedCon	0xC0
#define alWave		0xE0

#if 0
static void SoundCallback(void *userdata, Uint8 *stream, int len)
{
	int i;
	
	memset(stream, 0, len);
	
	if (!sound.active) {
		return;
	}
	
	if (sound.adlib_music_reset) {
		OPLWrite(sound.OPL, alEffects, 0);
		for (i = 0; i < 10; i++) {
			OPLWrite(sound.OPL, alFreqH + i + 1, 0);
		}
		
		if (!sound.adlib_music_paused) {
		}
		
		sound.adlib_music_reset = false;
	}
	
	if (sound.mix_adlib_music) {
	}
	
	if (sound.adlib_sound_reset) {
		OPLWrite(sound.OPL, alEffects, 0);
		
		OPLWrite(sound.OPL, 0 + alChar, 0);
		OPLWrite(sound.OPL, 0 + alScale, 0);
		OPLWrite(sound.OPL, 0 + alAttack, 0);
		OPLWrite(sound.OPL, 0 + alSus, 0);
		OPLWrite(sound.OPL, 0 + alWave, 0);
		OPLWrite(sound.OPL, 3 + alChar, 0);
		OPLWrite(sound.OPL, 3 + alScale, 0);
		OPLWrite(sound.OPL, 3 + alAttack, 0);
		OPLWrite(sound.OPL, 3 + alSus, 0);
		OPLWrite(sound.OPL, 3 + alWave, 0);
		OPLWrite(sound.OPL, alFreqL, 0);
		OPLWrite(sound.OPL, alFreqH, 0);

		if (sound.adlib_sound_id != -1) {
			byte mChar, cChar, mScale, cScale, mAttack, cAttack;
			byte mSus, cSus, mWave, cWave, nConn;
			
			mChar = sound.adlib_sound_data[0];
			cChar = sound.adlib_sound_data[1];
			mScale = sound.adlib_sound_data[2];
			cScale = sound.adlib_sound_data[3];
			mAttack = sound.adlib_sound_data[4];
			cAttack = sound.adlib_sound_data[5];
			mSus = sound.adlib_sound_data[6];
			cSus = sound.adlib_sound_data[7];
			mWave = sound.adlib_sound_data[8];
			cWave = sound.adlib_sound_data[9];
			nConn = sound.adlib_sound_data[10];
			
			OPLWrite(sound.OPL, 0 + alChar, mChar);
			OPLWrite(sound.OPL, 0 + alScale, mScale);
			OPLWrite(sound.OPL, 0 + alAttack, mAttack);
			OPLWrite(sound.OPL, 0 + alSus, mSus);
			OPLWrite(sound.OPL, 0 + alWave, mWave);
			OPLWrite(sound.OPL, 3 + alChar, cChar);
			OPLWrite(sound.OPL, 3 + alScale, cScale);
			OPLWrite(sound.OPL, 3 + alAttack, cAttack);
			OPLWrite(sound.OPL, 3 + alSus, cSus);
			OPLWrite(sound.OPL, 3 + alWave, cWave);

			/* OPLWrite(sound.OPL, alFeedCon, nConn); */
			OPLWrite(sound.OPL, alFeedCon, 0);
		}
		
		sound.adlib_sound_reset = false;
	}
	
	if (sound.mix_adlib_sound) {
	}
	
	if (sound.digi_sound_reset) {
		sound.digi_sound_reset = false;
	}
	
	if (sound.mix_digi_sound) {
	}
#if 0
	int i, snd;
	short int samp;
	int MusicLength;
	int MusicCount;
	word *MusicData;
	word dat;
	
	AdLibSound *AdlibSnd;
	byte AdlibBlock;
	byte *AdlibData;
	int AdlibLength;
	Instrument *inst;
	
	MusicLength = 0;
	MusicCount = 0;
	MusicData = NULL;
	AdlibBlock = 0;
	AdlibData = NULL;
	AdlibLength = -1;

	

/* Yeah, one day I'll rewrite this... */
	
	while (SD_Started) {
		if (audiofd != -1) {
			if (NewAdlib != -1) {
				AdlibPlaying = NewAdlib;
				AdlibSnd = (AdLibSound *)audiosegs[STARTADLIBSOUNDS+AdlibPlaying];
				inst = (Instrument *)&AdlibSnd->inst;


				OPLWrite(OPL, 0 + alChar, 0);
				OPLWrite(OPL, 0 + alScale, 0);
				OPLWrite(OPL, 0 + alAttack, 0);
				OPLWrite(OPL, 0 + alSus, 0);
				OPLWrite(OPL, 0 + alWave, 0);
				OPLWrite(OPL, 3 + alChar, 0);
				OPLWrite(OPL, 3 + alScale, 0);
				OPLWrite(OPL, 3 + alAttack, 0);
				OPLWrite(OPL, 3 + alSus, 0);
				OPLWrite(OPL, 3 + alWave, 0);
				OPLWrite(OPL, 0xA0, 0);
				OPLWrite(OPL, 0xB0, 0);
				
				OPLWrite(OPL, 0 + alChar, inst->mChar);
				OPLWrite(OPL, 0 + alScale, inst->mScale);
				OPLWrite(OPL, 0 + alAttack, inst->mAttack);
				OPLWrite(OPL, 0 + alSus, inst->mSus);
				OPLWrite(OPL, 0 + alWave, inst->mWave);
				OPLWrite(OPL, 3 + alChar, inst->cChar);
				OPLWrite(OPL, 3 + alScale, inst->cScale);
				OPLWrite(OPL, 3 + alAttack, inst->cAttack);
				OPLWrite(OPL, 3 + alSus, inst->cSus);
				OPLWrite(OPL, 3 + alWave, inst->cWave);

				//OPLWrite(OPL, alFeedCon, inst->nConn);
				OPLWrite(OPL, alFeedCon, 0);
				
				AdlibBlock = ((AdlibSnd->block & 7) << 2) | 0x20;
				AdlibData = (byte *)&AdlibSnd->data;
				AdlibLength = AdlibSnd->common.length*5;
				//OPLWrite(OPL, 0xB0, AdlibBlock);
				NewAdlib = -1;
			}
			
			if (NewMusic != -1) {
				NewMusic = -1;
				MusicLength = Music->length;
				MusicData = Music->values;
				MusicCount = 0;
			}
			for (i = 0; i < 4; i++) {
				if (sqActive) {
					while (MusicCount <= 0) {
						dat = *MusicData++;
						MusicCount = *MusicData++;
						MusicLength -= 4;
						OPLWrite(OPL, dat & 0xFF, dat >> 8);
					}
					if (MusicLength <= 0) {
						NewMusic = 1;
					}
					MusicCount--;
				}

				if (AdlibPlaying != -1) {
					if (AdlibLength == 0) {
						//OPLWrite(OPL, 0xB0, AdlibBlock);
					} else if (AdlibLength == -1) {
						OPLWrite(OPL, 0xA0, 00);
						OPLWrite(OPL, 0xB0, AdlibBlock);
						AdlibPlaying = -1;
					} else if ((AdlibLength % 5) == 0) {
						OPLWrite(OPL, 0xA0, *AdlibData);
						OPLWrite(OPL, 0xB0, AdlibBlock & ~2);
						AdlibData++;
					}
					AdlibLength--;
				}

				YM3812UpdateOne(OPL, &musbuf[i*64], 64);
			} 
			if (NextSound != -1) {
				SoundPlaying = NextSound;
				SoundPage = DigiList[(SoundPlaying * 2) + 0];
				SoundData = PM_GetSoundPage(SoundPage);
				SoundLen = DigiList[(SoundPlaying * 2) + 1];
				SoundPlayLen = (SoundLen < 4096) ? SoundLen : 4096;
				SoundPlayPos = 0;
				NextSound = -1;
			}
			for (i = 0; i < (sizeof(sndbuf)/sizeof(sndbuf[0])); i += 2) {
				if (SoundPlaying != -1) {
					if (SoundPositioned) {
						samp = (SoundData[(SoundPlayPos >> 16)] << 8)^0x8000;
						snd = samp*(16-L)/32+musbuf[i/2];
						//snd = (((signed short)((SoundData[(SoundPlayPos >> 16)] << 8)^0x8000))*(16-L)>>5)+musbuf[i/2];
						if (snd > 32767)
							snd = 32767;
						if (snd < -32768)
							snd = -32768;
						sndbuf[i+0] = snd;
						samp = (SoundData[(SoundPlayPos >> 16)] << 8)^0x8000;
						snd = samp*(16-R)/32+musbuf[i/2];
						//snd = (((signed short)((SoundData[(SoundPlayPos >> 16)] << 8)^0x8000))*(16-R)>>5)+musbuf[i/2];
						if (snd > 32767)
							snd = 32767;
						if (snd < -32768)
							snd = -32768;
						sndbuf[i+1] = snd;
					} else {
						snd = (((signed short)((SoundData[(SoundPlayPos >> 16)] << 8)^0x8000))>>2)+musbuf[i/2];
						if (snd > 32767)
							snd = 32767;
						if (snd < -32768)
							snd = -32768;
						sndbuf[i+0] = snd;
						snd = (((signed short)((SoundData[(SoundPlayPos >> 16)] << 8)^0x8000))>>2)+musbuf[i/2];
						if (snd > 32767)
							snd = 32767;
						if (snd < -32768)
							snd = -32768;
						sndbuf[i+1] = snd;
					}
					SoundPlayPos += 10402; /* 7000 / 44100 * 65536 */
					if ((SoundPlayPos >> 16) >= SoundPlayLen) {
						//SoundPlayPos = 0;
						SoundPlayPos -= (SoundPlayLen << 16);
						SoundLen -= 4096;
						SoundPlayLen = (SoundLen < 4096) ? SoundLen : 4096;
						if (SoundLen <= 0) {
							SoundPlaying = -1;
							SoundPositioned = false;
						} else {
							SoundPage++;
							SoundData = PM_GetSoundPage(SoundPage);
						}
					}
				} else {
					sndbuf[i+0] = musbuf[i/2];
					sndbuf[i+1] = musbuf[i/2];
				}
			}
			write(audiofd, sndbuf, sizeof(sndbuf));
		}		
	}
	return NULL;
#endif	
}
#endif

static Mix_Chunk **chunks;

static void SD_LoadDigi(int s)
{
	SDL_RWops *rw;
        byte *ptr, *buf;
        int start;
	int pages;
	int size;
	int len;
	int audiolen;
        int j;
	unsigned int frac;
	
	if (chunks[s] != NULL) {
		return;
	}
	        
        start = sound.DigiList[s*2+0];
        len = sound.DigiList[s*2+1];
        pages = (len + (PMPageSize - 1)) / PMPageSize;
        	
       	if (len == 0) {
       		chunks[s] = NULL;
		return;
       	}

	MM_GetPtr((memptr *) &ptr, len);
	
        for (j = 0; j < pages; j++) {
        	memptr sptr = PM_GetSoundPage(start + j);
        	
        	size = len-(j*PMPageSize);
        	if (size > PMPageSize) {
        		size = PMPageSize;
        	}
        	
        	memcpy(ptr+j*PMPageSize, sptr, size);
        	
		PM_FreeSoundPage(start + j);
        } 

	/* 
	  this is where we'd send it off to SDL_mixer, but SDL 
	  audio conversion can only convert frequences by factors of 2,
	  so since we are wanting to use 44100, we will first convert to 
	  11025, and then give that to SDL_mixer
	 */
	
	audiolen = (len * 11025 + 6999) / 7000;
	
	MM_GetPtr((memptr *) &buf, audiolen+44);
       	memcpy(buf, wavheader, 44);
        	
        /* adjust the size in the wave header */
        size = audiolen + 44 - 8;
        buf[0x04+0] = (size >>  0) & 0xFF;
        buf[0x04+1] = (size >>  8) & 0xFF;
        buf[0x04+2] = (size >> 16) & 0xFF;
        buf[0x04+3] = (size >> 24) & 0xFF;
        	
        size = audiolen;
        buf[0x28+0] = (size >>  0) & 0xFF;
        buf[0x28+1] = (size >>  8) & 0xFF;
        buf[0x28+2] = (size >> 16) & 0xFF;
        buf[0x28+3] = (size >> 24) & 0xFF;
        
        for (j = 0, frac = 0; j < audiolen; j++, frac += (7000*65536/11025)) {
		buf[44+j] = ptr[frac >> 16];
        }
        
        rw = SDL_RWFromMem((void *) buf, audiolen+44);
        chunks[s] = Mix_LoadWAV_RW(rw, 1);
        if (chunks[s] == NULL) {
	        fprintf(stderr, "Decode Error: %s\n", SDL_GetError());
        }
        
       	MM_FreePtr((memptr *) &buf);
       	MM_FreePtr((memptr *) &ptr);
}

static void SD_InitDigiList()
{
	word *list;
        byte *p;
        int pg;
	int i;
	
        MM_GetPtr((memptr *) &list, PMPageSize);
        
        p = PM_GetPage(ChunksInFile - 1);
        memcpy((void *)list,(void *)p,PMPageSize);
        
        /* count the number of sounds available */
        pg = PMSoundStart;
        for (NumDigi = 0; NumDigi < PMPageSize / (sizeof(word) * 2);
        	NumDigi++, p += 4)
        {
                list[0] = (p[0] << 0) | (p[1] << 8);
                list[1] = (p[2] << 0) | (p[3] << 8);
                
                if (pg >= ChunksInFile - 1)
                        break;
                pg += (list[1] + (PMPageSize - 1)) / PMPageSize;
        }
        
        MM_GetPtr((memptr *)&(sound.DigiList), NumDigi * sizeof(word) * 2);
        memcpy((void *)sound.DigiList, (void *)list, 
        	NumDigi * sizeof(word) * 2);
        MM_FreePtr((memptr *) &list);
        
        PM_FreePage(ChunksInFile - 1);
        
        MM_GetPtr((memptr *)&chunks, NumDigi * sizeof(Mix_Chunk *));
        
        for (i = 0; i < NumDigi; i++) {
        	chunks[i] = NULL;
        }
}

static void channelFinishedCallback(int channel)
{
}

void SD_Startup()
{
	if (SD_Started)
		return;

	AdLibPresent = false;
	SoundBlasterPresent = false;
	
	SDL_ClearError();
	if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0) {
		SoundMode = sdm_Off;
		MusicMode = smm_Off;
		DigiMode = sds_Off;
		
		fprintf(stderr, "Unable to initialize SDL audio: %s\n", SDL_GetError());
		
		AdLibPresent = false;
		SoundBlasterPresent = false;

		return;
	}

	if (Mix_OpenAudio(44100, AUDIO_S16, 2, 4096) < 0) {
		SoundMode = sdm_Off;
		MusicMode = smm_Off;
		DigiMode = sds_Off;
		
		fprintf(stderr, "Unable to initialize SDL_mixer audio: %s\n", SDL_GetError());
		
		AdLibPresent = false;
		SoundBlasterPresent = false;

		return;
	}
	
	Mix_AllocateChannels(1);
	Mix_ChannelFinished(channelFinishedCallback);
	
	SD_InitDigiList();	
	InitDigiMap();
	
	sound.OPL = OPLCreate(OPL_TYPE_YM3812, 3579545, 44100);
	OPLWrite(sound.OPL, 0x01, 0x20); /* Set WSE=1 */
	OPLWrite(sound.OPL, 0x08, 0x00); /* Set CSM=0 & SEL=0 */
	
	AdLibPresent = true;
	SoundBlasterPresent = true;
	SD_Started = true;
}

void SD_Shutdown()
{
	if (!SD_Started)
		return;

	SD_MusicOff();
	SD_StopSound();

	Mix_CloseAudio();

	SD_Started = false;
}

///////////////////////////////////////////////////////////////////////////
//
//	SD_PlaySound() - plays the specified sound on the appropriate hardware
//
///////////////////////////////////////////////////////////////////////////
boolean SD_PlaySound(soundnames sound_id)
{
	boolean retr = false;
	int priority;
	int adlib_sound_id;
	byte *data;
	
	if (!SD_Started) {
		return false;
	}
	
	adlib_sound_id = STARTADLIBSOUNDS + sound_id;
	data = audiosegs[adlib_sound_id];
	priority = (data[4] << 0) | (data[5] << 8);
	
	if ((DigiMode == sds_SoundBlaster) &&
			(DigiMap[sound_id] != -1) &&
			(!Mix_Playing(0) ||
			(sound.digi_sound_priority <= priority)
			)) {
		
		int digi = DigiMap[sound_id];
		
		/* try to load the sound if not already initialized */
		if (chunks[digi] == NULL) {
			SD_LoadDigi(digi);
		}
		
		if (chunks[digi] != NULL) {
			Mix_PlayChannel(0, chunks[digi], 0);
		}
		
		retr = true;		
	} else if ((SoundMode == sdm_AdLib) &&
			(DigiMap[sound_id] == -1) &&
			((sound.adlib_sound_priority <= priority) ||
			!sound.adlib_sound_playing)) {
		
		retr = false;
	}
	
	return retr;
#if 0

	boolean retr = false;
	int priority;
	int adlib_sound_id;
	byte *data;
	
	if (!SD_Started)
		return false;

	adlib_sound_id = STARTADLIBSOUNDS + sound_id;
	data = audiosegs[adlib_sound_id];
	priority = (data[4] << 0) | (data[5] << 8);
	
	SDL_LockAudio();
	
	if ((DigiMode == sds_SoundBlaster) &&
			(DigiMap[sound_id] != -1) &&
			((sound.digi_sound_priority <= priority) ||
			!sound.digi_sound_playing)) {
			
			sound.digi_sound_id = sound_id;
			
			if (wantpositioned) {
				sound.digi_sound_left = leftchannel;
				sound.digi_sound_right = rightchannel;
				
				sound.digi_sound_positioned = true;
			} else {
				sound.digi_sound_left = 16;
				sound.digi_sound_right = 16;
				
				sound.digi_sound_positioned = false;
			}
			
			sound.mix_digi_sound = true;
			sound.digi_sound_reset = true;
			
			retr = true;
	} else if ((SoundMode == sdm_AdLib) &&
			(DigiMap[sound_id] == -1) &&
			((sound.adlib_sound_priority <= priority) ||
			!sound.adlib_sound_playing)) {
			
			sound.adlib_sound_id = adlib_sound_id;
			sound.adlib_sound_data = data;
			
			sound.mix_adlib_sound = true;
			sound.adlib_sound_reset = true;
			
			retr = true;
	}
	
	wantpositioned = false;
	
	SDL_UnlockAudio();
	
	return retr;
#endif
}

///////////////////////////////////////////////////////////////////////////
//
//	SD_SoundPlaying() - returns the sound number that's playing, or 0 if
//		no sound is playing
//
///////////////////////////////////////////////////////////////////////////
word SD_SoundPlaying()
{
#if 0
	int playing;
	
	if (!SD_Started)
		return 0;

	SDL_LockAudio();
	
	if (sound.adlib_sound_playing) {
		playing = sound.adlib_sound_id;
	} else {
		playing = 0;
	}
	
	SDL_UnlockAudio();

	return playing;
#else
	return 0;
#endif
}

///////////////////////////////////////////////////////////////////////////
//
//	SD_StopSound() - if a sound is playing, stops it
//
///////////////////////////////////////////////////////////////////////////
void SD_StopSound()
{
#if 0
	if (!SD_Started)
		return;
		
	SDL_LockAudio();

	sound.adlib_sound_id = -1;
	sound.adlib_sound_reset = true;
	
	sound.mix_adlib_sound = false;
	
	sound.digi_sound_id = -1;
	sound.digi_sound_reset = true;
	
	sound.mix_digi_sound = false;
	
	SDL_UnlockAudio();
#endif
}

///////////////////////////////////////////////////////////////////////////
//
//	SD_WaitSoundDone() - waits until the current sound is done playing
//
///////////////////////////////////////////////////////////////////////////
void SD_WaitSoundDone()
{
	if (!SD_Started)
		return;

	while (SD_SoundPlaying()) ;
}

/*
==========================
=
= SetSoundLoc - Given the location of an object (in terms of global
=	coordinates, held in globalsoundx and globalsoundy), munges the values
=	for an approximate distance from the left and right ear, and puts
=	those values into leftchannel and rightchannel.
=
= JAB
=
==========================
*/

#define ATABLEMAX 15
static const byte righttable[ATABLEMAX][ATABLEMAX * 2] = {
{ 8, 8, 8, 8, 8, 8, 8, 7, 7, 7, 7, 7, 7, 6, 0, 0, 0, 0, 0, 1, 3, 5, 8, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 7, 7, 7, 7, 7, 6, 4, 0, 0, 0, 0, 0, 2, 4, 6, 8, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 7, 7, 7, 7, 6, 6, 4, 1, 0, 0, 0, 1, 2, 4, 6, 8, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 7, 7, 7, 7, 6, 5, 4, 2, 1, 0, 1, 2, 3, 5, 7, 8, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 7, 7, 7, 6, 5, 4, 3, 2, 2, 3, 3, 5, 6, 8, 8, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 7, 7, 7, 6, 6, 5, 4, 4, 4, 4, 5, 6, 7, 8, 8, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 8, 7, 7, 7, 6, 6, 5, 5, 5, 6, 6, 7, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 7, 7, 7, 6, 6, 7, 7, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8}
};
static const byte lefttable[ATABLEMAX][ATABLEMAX * 2] = {
{ 8, 8, 8, 8, 8, 8, 8, 8, 5, 3, 1, 0, 0, 0, 0, 0, 6, 7, 7, 7, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 6, 4, 2, 0, 0, 0, 0, 0, 4, 6, 7, 7, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 6, 4, 2, 1, 0, 0, 0, 1, 4, 6, 6, 7, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 7, 5, 3, 2, 1, 0, 1, 2, 4, 5, 6, 7, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 8, 6, 5, 3, 3, 2, 2, 3, 4, 5, 6, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 8, 7, 6, 5, 4, 4, 4, 4, 5, 6, 6, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 7, 6, 6, 5, 5, 5, 6, 6, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 7, 7, 6, 6, 7, 7, 7, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8},
{ 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8}
};

static void SetSoundLoc(fixed gx, fixed gy)
{
	fixed xt, yt;
	int x, y;

//
// translate point to view centered coordinates
//
	gx -= viewx;
	gy -= viewy;

//
// calculate newx
//
	xt = FixedByFrac(gx,viewcos);
	yt = FixedByFrac(gy,viewsin);
	x = (xt - yt) >> TILESHIFT;

//
// calculate newy
//
	xt = FixedByFrac(gx,viewsin);
	yt = FixedByFrac(gy,viewcos);
	y = (yt + xt) >> TILESHIFT;

//
// clip values
//
	if (y >= ATABLEMAX)
		y = ATABLEMAX - 1;
	else if (y <= -ATABLEMAX)
		y = -ATABLEMAX;
	if (x < 0)
		x = -x;
	if (x >= ATABLEMAX)
		x = ATABLEMAX - 1;

	leftchannel  = 16 - lefttable[x][y + ATABLEMAX];
	rightchannel = 16 - righttable[x][y + ATABLEMAX];
}

/*
==========================
=
= SetSoundLocGlobal - Sets up globalsoundx & globalsoundy and then calls
=	UpdateSoundLoc() to transform that into relative channel volumes. Those
=	values are then passed to the Sound Manager so that they'll be used for
=	the next sound played (if possible).
=
==========================
*/

void PlaySoundLocGlobal(word s, int id, fixed gx, fixed gy)
{
	if (!SD_Started)
		return;

	SetSoundLoc(gx, gy);

	wantpositioned = true;

	if (SD_PlaySound(s)) {
		globalsoundx = gx;
		globalsoundy = gy;
	}
}

void UpdateSoundLoc(fixed x, fixed y, int angle)
{
#if 0
	if (!SD_Started)
		return;	

	SDL_LockAudio();
	
	if (sound.digi_sound_positioned) {
		SetSoundLoc(globalsoundx, globalsoundy);
		sound.digi_sound_left = leftchannel;
		sound.digi_sound_right = rightchannel;
	}
	
	SDL_UnlockAudio();
#endif
}

///////////////////////////////////////////////////////////////////////////
//
//	SD_MusicOn() - turns on the sequencer
//
///////////////////////////////////////////////////////////////////////////
void SD_MusicOn()
{
#if 0
	if (!SD_Started)
		return;

	SDL_LockAudio();
	
	sound.mix_adlib_music = (MusicMode == smm_AdLib) ? true : false;
	sound.adlib_music_paused = false;
	
	SDL_UnlockAudio();
#endif
}

///////////////////////////////////////////////////////////////////////////
//
//	SD_MusicOff() - turns off the sequencer and any playing notes
//
///////////////////////////////////////////////////////////////////////////
void SD_MusicOff()
{
#if 0
	if (!SD_Started)
		return;

	SDL_LockAudio();

	sound.mix_adlib_music = false;
	sound.adlib_music_reset = true;
	sound.adlib_music_paused = true;
	
	SDL_UnlockAudio();
#endif
}

///////////////////////////////////////////////////////////////////////////
//
//	SD_StartMusic() - starts playing the music pointed to
//
///////////////////////////////////////////////////////////////////////////
void SD_StartMusic(int music)
{
#if 0
	if (!SD_Started)
		return;

	SDL_LockAudio();
	
	music += STARTMUSIC;
	
	if (sound.adlib_music_id != -1) {
		CA_UnCacheAudioChunk(sound.adlib_music_id);
	}
	
	CA_CacheAudioChunk(music);
	
	sound.adlib_music_id = music;
	sound.adlib_music_data = audiosegs[music];
	sound.adlib_music_reset = true;
	sound.adlib_music_paused = false;
		
	SDL_UnlockAudio();
#endif
}

void SD_SetDigiDevice(SDSMode mode)
{
	if (!SD_Started)
		return;

	if (mode == DigiMode) {
		return;
	}
	
	if (mode == sds_PC) {
		return;
	}
	
	if (!SoundBlasterPresent && (mode != sds_Off)) {
		return;
	}
	
	DigiMode = mode;
	if (DigiMode == sds_SoundBlaster) {
	} else {
		Mix_HaltChannel(-1);
	}
}

///////////////////////////////////////////////////////////////////////////
//
//	SD_SetSoundMode() - Sets which sound hardware to use for sound effects
//
///////////////////////////////////////////////////////////////////////////
boolean SD_SetSoundMode(SDMode mode)
{
	if (!SD_Started)
		return false;

	if (mode == SoundMode)
		return true;
		
	if (mode == sdm_PC)
		return false;
	
	if (!AdLibPresent && (mode != sdm_Off)) {
		return false;
	}
		
	SoundMode = mode;
	
	return true;
}
///////////////////////////////////////////////////////////////////////////
//
//	SD_SetMusicMode() - sets the device to use for background music
//
///////////////////////////////////////////////////////////////////////////
boolean SD_SetMusicMode(SMMode mode)
{
#if 0
	if (!SD_Started)
		return false;

	if (mode == MusicMode)
		return true;
		
	if (!AdLibPresent && (mode != smm_Off)) {
		return false;
	}
	
	SDL_LockAudio();
	
	MusicMode = mode;
	if ((MusicMode == sdm_AdLib) || 
			(SoundMode == smm_AdLib) || 
			(DigiMode == sds_SoundBlaster)) {
		SDL_PauseAudio(0);
	} else {
		SDL_PauseAudio(1);
	}
	
	sound.mix_adlib_music = (MusicMode == smm_AdLib) ? true : false;
	
	SDL_UnlockAudio();
	
	return true;
#else
	return false;
#endif
}
