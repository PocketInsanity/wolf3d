#include "wl_def.h"

#include <unistd.h>
#include <pthread.h>

#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alut.h>

#include "fmopl.h"

/* old stuff */
boolean AdLibPresent, SoundBlasterPresent;
SDMode SoundMode, MusicMode;
SDSMode DigiMode;
static boolean SD_Started;


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
struct _PCMSound
{
	byte *data;
	int length;
	int used;
	
	ALuint handle;
} static *PCMSound;

struct _SoundData
{
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

static ALuint AdBuf[4];

struct _MusicData
{
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


/* how to do streaming buffer:
queue say 4 buffers of a certain size and play
now, run your updating loop: 
	unqueue buffers that have finished
	generate new data and fill these buffers
	queue the buffers with the new data

since "Applications can have multiple threads that share one more or contexts.
In other words, AL and ALC are threadsafe.", the streaming code can be
implemented as a second thread

*/

/* threads:
	main thread handles pcm channels/sounds
	sound thread handles adlib channels/sounds
	
	sound thread sets priority to -1 if a sound is not playing
	
	potential thread contention with both main and sound threads 
	accessing channel 0's information, but:
	1. it's not done in any loops
	2. any data mismatch won't affect the game (at least I think so)
*/

static pthread_t hSoundThread;
static pthread_mutex_t SoundMutex;

#define SOUND_START	0	/* play this adlib sound */
#define SOUND_STARTD	1	/* play this pcm sound */
#define SOUND_STOP	2	/* stop all sounds */
#define MUSIC_START	3	/* play this song */
#define MUSIC_STOP	4	/* stop music */
#define MUSIC_PAUSE	5	/* pause music */
#define MUSIC_UNPAUSE	6	/* unpause music */
#define ST_SHUTDOWN	7	/* shutdown thread */

struct _SoundMessage
{
	int type;	/* what is it? (see above) */
	
	int item;	/* what to play (if anything) */
	
	struct _SoundMessage *next;
};

struct _SoundMessage *SoundMessage;

static void SendSTMessage(int type, int item)
{
	struct _SoundMessage *t;
	
	pthread_mutex_lock(&SoundMutex);
	
	MM_GetPtr((memptr)&t, sizeof(struct _SoundMessage));
	
	t->type = type;
	t->item = item;
	
	t->next = SoundMessage;
	SoundMessage = t;
	
	pthread_mutex_unlock(&SoundMutex);	
}

static boolean ReceiveSTMessage(int *type, int *item)
{
	boolean retr = false;
	
	pthread_mutex_lock(&SoundMutex);
	
	if (SoundMessage) {
		struct _SoundMessage *p, *t;
		
		p = NULL;
		t = SoundMessage;
		while (t->next) {
			p = t;
			t = t->next;
		}
		
		if (p) {
			p->next = NULL;
		} else {
			SoundMessage = NULL;
		}
				
		*type = t->type;
		*item = t->item;
		
		MM_FreePtr((memptr)&t);
		
		retr = true;
	} else {
		retr = false;
	}
	
	pthread_mutex_unlock(&SoundMutex);
	
	return retr;
}

static void *SoundThread(void *data)
{
	FM_OPL *OPL;
	byte *s;
	int type, item;
	int i;
	
	OPL = OPLCreate(OPL_TYPE_YM3812, 3579545, 44100);
	OPLWrite(OPL, 0x01, 0x20); /* Set WSE=1 */
	OPLWrite(OPL, 0x08, 0x00); /* Set CSM=0 & SEL=0 */
	
	MM_GetPtr((memptr)&s, 63*2*5);
	alGenBuffers(4, AdBuf);
	
	for (i = 0; i < 4; i++)
		alBufferData(AdBuf[i], AL_FORMAT_MONO16, s, 63*5, 44100);
	
	alSourceQueueBuffers(SoundChan[0].handle, 4, AdBuf);
	
	for (;;) {
		while (ReceiveSTMessage(&type, &item)) {
			switch (type) {
				case SOUND_START:
					break;
				case SOUND_STOP:
					break;
				case MUSIC_START:
					break;
				case MUSIC_STOP:
					break;
				case MUSIC_PAUSE:
					break;
				case MUSIC_UNPAUSE:
					break;
				case ST_SHUTDOWN:
					/* TODO: free the openal stuff allocated in this thread */
					OPLDestroy(OPL);
					return NULL;
			}
		}
		
		/* Do Stuff */
		usleep(1);
	}
		
	return NULL;
}

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
	
	for (i = 0; i < CHANNELS; i++) {
		alGenSources(1, &(SoundChan[i].handle));
		
		SoundChan[i].id = -1;
		SoundChan[i].priority = -1;
		
		alSourcei(SoundChan[i].handle, AL_SOURCE_RELATIVE, AL_FALSE);
		/* alSourcef(SoundChan[i].handle, AL_ROLLOFF_FACTOR, 1.0f); */
	}
	
	alSourcei(SoundChan[0].handle, AL_SOURCE_RELATIVE, AL_TRUE);
	alSourcef(SoundChan[0].handle, AL_ROLLOFF_FACTOR, 0.0f);
	alSourcei(SoundChan[1].handle, AL_SOURCE_RELATIVE, AL_TRUE);
	alSourcef(SoundChan[i].handle, AL_ROLLOFF_FACTOR, 0.0f);
	
	MusicData[0].length = 0; /* silence warnings for now */
		
	SoundMessage = NULL;
	
	pthread_mutex_init(&SoundMutex, NULL);
	if (pthread_create(&hSoundThread, NULL, SoundThread, NULL) != 0) {
		perror("pthread_create");		
	}
	
	SD_Started = true;
}

void SD_Shutdown()
{
	if (!SD_Started)
		return;
		
	SendSTMessage(ST_SHUTDOWN, 0);
	
/* TODO: deallocate everything here */

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
			}
			return false;
		}
	}
	
	if (s->priority >= SoundChan[0].priority)
		SendSTMessage(SOUND_START, sound);
	
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
	
	/* Just play the AdLib version */
	SD_PlaySound(sound);
}

void UpdateSoundLoc(fixed x, fixed y, int angle)
{
	ALfloat val[6];
	
	val[0] = (ALfloat)x / 32768.0f;
	val[1] = 0.0f;
	val[2] = (ALfloat)y / 32768.0f;
	alListenerfv(AL_POSITION, val); 
	
	val[0] = -cos(angle * PI / 180.0f);
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
	
	SendSTMessage(SOUND_STOP, 0);
		
	for (i = 1; i < CHANNELS; i++) {
		SoundChan[i].id = -1;
		SoundChan[i].sound = -1;
		SoundChan[i].priority = -1;
		
		alSourceStop(SoundChan[i].handle);
	}
		
}

word SD_SoundPlaying()
{
	ALint val;
	int i;

	if (SoundChan[0].priority != -1)
		return SoundChan[0].sound;
	
	for (i = 1; i < CHANNELS; i++) {
		alGetSourceiv(SoundChan[i].handle, AL_SOURCE_STATE, &val);
		if (val == AL_PLAYING)
			return SoundChan[i].sound;
	}
	
	return false;
}

void SD_WaitSoundDone()
{
	while (SD_SoundPlaying())
		;
}

void SD_MusicOn()
{
	SendSTMessage(MUSIC_UNPAUSE, 0);
}

void SD_MusicOff()
{
	SendSTMessage(MUSIC_PAUSE, 0);
}

void SD_StartMusic(int music)
{
	SendSTMessage(MUSIC_START, music);
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
