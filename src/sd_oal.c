#include "wl_def.h"

#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alut.h>

/* old stuff */
boolean AdLibPresent, SoundBlasterPresent;
SDMode SoundMode, MusicMode;
SDSMode DigiMode;
static boolean SD_Started;
static boolean sqActive;


/* AdLib Sound:
	longword length;
	word priority;
	byte mChar, cChar, mScale, cScale, mAttack, cAttack, mSus, cSus,
		mWave, cWave, nConn, voice, mode, unused[3];
	byte block, data[];
*/

/* AdLib Music:
	word length, data[]; 
	(data is organized in reg|val (word) and tic count (word) pairs)
*/

/* PC Sound:
	longword length;
	word priority;
	byte data[];
*/


/* new stuff */

static int NumPCM;
struct _PCMSound {
	byte *data;
	int length;
	int used;
	
	ALuint handle;
} static *PCMSound;

struct _SoundData {
	int priority;
	
	/* PCM */
	struct _PCMSound *pcm;
	
	/* AdLib */
	int alength;
	byte 	mChar, cChar, mScale, cScale, 
		mAttack, cAttack, mSus, cSus, 
		mWave, cWave, nConn;
	byte block;
	byte *adata;
} static SoundData[LASTSOUND];

struct _MusicData {
	int length;
	
	word *regval;
	word *count;
} static MusicData[LASTMUSIC];

#define CHANNELS	5	/* channel 0: adlib */
				/* channel 1: local sounds */
				/* channel 2+ positioned sounds */
struct _SoundChan {
	int id;		/* what is making the sound */	
	int sound;	/* what is the sound */
	int priority;	/* priority of this sound */
	int type;	/* adlib or pcm? */
	int pos;	/* internal variable for soundplayer */
	
	ALuint handle;	/* OpenAL source handle */
} static SoundChan[CHANNELS];


void SD_Startup()
{
	byte *p, *s;	
	
	int len, l, i, c;
	
	if (SD_Started)
		return;
	
	alutInit(NULL, 0);
	
	InitDigiMap();
	
	p = PM_GetPage(ChunksInFile - 1);	/* get pcm size table */

	/* find how many full sounds there are */
	c = PMSoundStart;
	for (i = 0; i < 1024; i++) {
		if (c >= (ChunksInFile - 1))
			break;
			
		len = p[i*4+2] | (p[i*4+3] << 8);
		
		/* convert len into amount of pages used */
		c += (len + (PMPageSize - 1)) / PMPageSize;
	}
	NumPCM = i;
		
	MM_GetPtr((memptr)&PCMSound, sizeof(struct _PCMSound) * NumPCM);
	
	for (i = 0; i < NumPCM; i++) {
		PCMSound[i].length = len = p[i*4+2] | (p[i*4+3] << 8);
		PCMSound[i].used = 0;
		
		if (len) {
			MM_GetPtr((memptr)&(PCMSound[i].data), len);
			
			c = p[i*4+0] | (p[i*4+1] << 8);
		
			s = PCMSound[i].data;
			while (len > 0) {
				l = (len >= PMPageSize) ? PMPageSize : len;
					
				memcpy(s, PM_GetSoundPage(c), l);
				PM_FreePage(c);
				
				c++;
				s += PMPageSize;
				len -= PMPageSize;
			}
		} else {
			PCMSound[i].data = NULL;
			PCMSound[i].handle = -1;
		}
	}
	
	for (i = 0; i < LASTSOUND; i++) {
		CA_CacheAudioChunk(STARTADLIBSOUNDS + i);
		
		s = audiosegs[STARTADLIBSOUNDS + i];
	
		if (s) {
			SoundData[i].alength = s[0] | (s[1] << 8) | (s[2] << 16) | (s[3] << 24);
			SoundData[i].priority = s[4] | (s[5] << 8);
		
			/* Rest of AdLib Sound stuff here */
			
			if (DigiMap[i] != -1) {
				struct _PCMSound *pcm = &PCMSound[DigiMap[i]];
			
				if (pcm->used || pcm->data) {
					SoundData[i].pcm = pcm;
					
					if (!pcm->used) {
						pcm->used = 1;
					
						alGenBuffers(1, &(pcm->handle));
						alBufferData(pcm->handle, AL_FORMAT_MONO8, pcm->data, pcm->length, 7000);
						if (alGetError() != AL_NO_ERROR) {
							fprintf(stderr, "AL error: trying to load pcm sound %d (sound %d)\n", DigiMap[i], i);
							
							alDeleteBuffers(1, &(pcm->handle));
							pcm->handle = -1;
						} else {
							MM_FreePtr((memptr)&(pcm->data));
							pcm->data = NULL;
						}
					}
				} else {
					SoundData[i].pcm = NULL;
				}
			} else {
				SoundData[i].pcm = NULL;
			}
		} else {
			SoundData[i].priority = 0;
			SoundData[i].alength = 0;
			SoundData[i].pcm = NULL;
		}
	}
	
	SoundChan[0].id = -1;
	SoundChan[0].priority = -1;
	
	for (i = 1; i < CHANNELS; i++) {
		alGenSources(1, &(SoundChan[i].handle));
		
		SoundChan[i].id = -1;
		SoundChan[i].priority = -1;
		
		alSourcei(SoundChan[i].handle, AL_SOURCE_RELATIVE, AL_FALSE);
		/* alSourcef(SoundChan[i].handle, AL_ROLLOFF_FACTOR, 1.0f); */
	}
	
	alSourcei(SoundChan[1].handle, AL_SOURCE_RELATIVE, AL_TRUE);
	alSourcef(SoundChan[i].handle, AL_ROLLOFF_FACTOR, 0.0f);
	
	MusicData[0].length = 0; /* silence warnings for now */
	
	SD_Started = true;
}

void SD_Shutdown()
{
	if (!SD_Started)
		return;

	SD_Started = false;
}

boolean SD_PlaySound(soundnames sound)
{
	struct _SoundData *s = &SoundData[sound];
	ALint val;
		
	if (s->pcm) {
		if (s->pcm->handle != -1) {
			alGetSourceiv(SoundChan[1].handle, AL_SOURCE_STATE, &val);
			
			if ((val != AL_PLAYING) || (s->priority >= SoundChan[1].priority)) {
				SoundChan[1].sound = sound;
				SoundChan[1].priority = s->priority;
				
				alSourceStop(SoundChan[1].handle);
				alSourcei(SoundChan[1].handle, AL_BUFFER, s->pcm->handle);
				alSourcePlay(SoundChan[1].handle);
				return false;
			}
		}
	}
	
	/* Adlib */
	return false;
}

void PlaySoundLocGlobal(word sound, int id, fixed gx, fixed gy)
{
	struct _SoundData *s = &SoundData[sound];
	ALfloat fval[3];
	ALint val;
	int i;
	
	if (s->pcm) {
		if (s->pcm->handle != -1) {
			for (i = 2; i < CHANNELS; i++) {
				if (id == SoundChan[i].id)
					break;
			}
			
			if (i == CHANNELS) {
				for (i = 2; i < CHANNELS; i++) {	
					alGetSourceiv(SoundChan[i].handle, AL_SOURCE_STATE, &val);
				
					if (val != AL_PLAYING)
						break;
				}
			}
			
			if (i == CHANNELS) {
				for (i = 2; i < CHANNELS; i++) {
					if (s->priority >= SoundChan[i].priority)
						break;
				}
			}
			
			if (i != CHANNELS) {
				SoundChan[i].id = id;
				SoundChan[i].sound = sound;
				SoundChan[i].priority = s->priority;
				
				fval[0] = (ALfloat)gx / 32768.0f;
				fval[1] = 0.0f;
				fval[2] = (ALfloat)gy / 32768.0f;
				
				alSourceStop(SoundChan[i].handle);
				alSourcei(SoundChan[i].handle, AL_BUFFER, s->pcm->handle);
				alSourcefv(SoundChan[i].handle, AL_POSITION, fval);
				alSourcePlay(SoundChan[i].handle);
			}
			return;
		}
	}
	
	/* AdLib */
	SD_PlaySound(sound);
}

void UpdateSoundLoc(fixed x, fixed y, int angle)
{
	ALfloat val[6];
	
	val[0] = (ALfloat)x / 32768.0f;
	val[1] = 0.0f;
	val[2] = (ALfloat)y / 32768.0f;
	alListenerfv(AL_POSITION, val); 
	
	val[0] = cos(angle * PI / 180.0f);
	val[1] = 0.0f;
	val[2] = -sin(angle * PI / 180.0f);
	val[3] = 0.0f;
	val[4] = 1.0f;
	val[5] = 0.0f;
	alListenerfv(AL_ORIENTATION, val);
	
}

void SD_StopSound()
{
	int i;
	
	/* Stop AdLib */
	
	for (i = 1; i < CHANNELS; i++) {
		SoundChan[i].id = -1;
		SoundChan[i].sound = -1;
		SoundChan[i].priority = -1;
		
		alSourceStop(SoundChan[i].handle);
	}
		
}

word SD_SoundPlaying()
{
/* returns 0 or currently playing sound */
/* this is only checked for GETGATLINGSND so to be pedantic */
/* return currently playing adlib else return the first found */
/* playing channel */
	ALint val;
	int i;

	/* Check AdLib sound status */
	
	for (i = 1; i < CHANNELS; i++) {
		alGetSourceiv(SoundChan[i].handle, AL_SOURCE_STATE, &val);
		if (val == AL_PLAYING)
			return SoundChan[i].sound;
	}
	
	return false;
}

void SD_WaitSoundDone()
{
/* TODO: should also "work" when sound is disabled... */
	while (SD_SoundPlaying())
		;
}

void SD_SetDigiDevice(SDSMode mode)
{
}

boolean SD_SetSoundMode(SDMode mode)
{
	return false;
}

boolean SD_SetMusicMode(SMMode mode)
{
	return false;
}

void SD_MusicOn()
{
	sqActive = true;
}

void SD_MusicOff()
{
	sqActive = false;
}

void SD_StartMusic(int music)
{
	SD_MusicOff();
}

void SD_FadeOutMusic()
{
}

boolean SD_MusicPlaying()
{
	return false;
}
