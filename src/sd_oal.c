/* id_sd.c */

#include "id_heads.h"

#include <AL/al.h>
#include <AL/alc.h>

//	Global variables
	boolean		SoundSourcePresent,
				AdLibPresent,
				SoundBlasterPresent,SBProPresent,
				NeedsDigitized,NeedsMusic,
				SoundPositioned;
	SDMode		SoundMode;
	SMMode		MusicMode;
	SDSMode		DigiMode;
	word		*SoundTable;	
	word		ssPort = 2;
	int			DigiMap[LASTSOUND];

//	Internal variables
static	boolean			SD_Started;
		boolean			nextsoundpos;

		soundnames		SoundNumber,DigiNumber;
		word			SoundPriority,DigiPriority;
		int				LeftPosition,RightPosition;

		word			NumDigi,DigiLeft,DigiPage;
		word			*DigiList;
		word			DigiLastStart,DigiLastEnd;
		boolean			DigiPlaying;
static	boolean			DigiMissed,DigiLastSegment;
static	memptr			DigiNextAddr;
static	word			DigiNextLen;

//	SoundBlaster variables
static	boolean					sbNoCheck,sbNoProCheck;
static	byte					sbOldIntMask = -1;
static	byte			*sbNextSegPtr;
static	longword		sbNextSegLen;

//	SoundSource variables
		boolean				ssNoCheck;
		boolean				ssActive;
		word				ssControl,ssStatus,ssData;
		byte				ssOn,ssOff;
		byte		*ssSample;
		longword	ssLengthLeft;

//	PC Sound variables
		volatile byte	pcLastSample, *pcSound;
		longword		pcLengthLeft;
		word			pcSoundLookup[255];

//	AdLib variables
		boolean			alNoCheck;
		byte			*alSound;
		word			alBlock;
		longword		alLengthLeft;
		Instrument		alZeroInst;

//	Sequencer variables
		boolean			sqActive;
static	word			alFXReg;
static	ActiveTrack		*tracks[sqMaxTracks],
						mytracks[sqMaxTracks];
static	word			sqMode,sqFadeStep;

/* ------------------------------------------------------------------------ */

ALuint *sources;
ALuint *buffers;
void *cc;

void SD_Poll(void)
{
}

void SD_SetDigiDevice(SDSMode mode)
{
}

///////////////////////////////////////////////////////////////////////////
//
//	SD_SetSoundMode() - Sets which sound hardware to use for sound effects
//
///////////////////////////////////////////////////////////////////////////
boolean SD_SetSoundMode(SDMode mode)
{
	return false;
}

///////////////////////////////////////////////////////////////////////////
//
//	SD_SetMusicMode() - sets the device to use for background music
//
///////////////////////////////////////////////////////////////////////////
boolean SD_SetMusicMode(SMMode mode)
{
	return false;
}

///////////////////////////////////////////////////////////////////////////
//
//	SD_Startup() - starts up the Sound Mgr
//
///////////////////////////////////////////////////////////////////////////
void SD_Startup(void)
{
	int i;
	
	if (SD_Started)
		return;

	for (i = 0; i < LASTSOUND; i++)
		DigiMap[i] = -1;
		
	cc = alcCreateContext(NULL);
	
	if (cc == NULL)
		printf("alcCreateContext failed..\n");
	else {
		word *SoundList = PM_GetPage(ChunksInFile - 1);
		PageListStruct *page = &PMPages[ChunksInFile - 1];
		int p = page->length;
		int x = 0, w, y, z;
		for (i = 0; i < p / 2; i += 2) {
			w = *(SoundList + i);
			y = *(SoundList + i+1);
			
			page = &PMPages[w + PMSoundStart];
			
			if (page->length == 0) {
				x++; // count it?
				continue;
			}
				
			for (z = 0; z < y; w++, z++) {
				page = &PMPages[w + PMSoundStart];
				z += page->length;
			}
			
			x++;
		}
			
		buffers = (ALuint *)malloc(sizeof(ALuint) * x);
		if (alGenBuffers(x, buffers) != x) 
			printf("OpenAL buffer allocation problem\n");
				
			
		x = 0;
		for (i = 0; i < p / 2; i += 2) {
			byte *dat;
			w = *(SoundList + i);
			y = *(SoundList + i+1);
			
			page = &PMPages[w + PMSoundStart];
			
			if (page->length == 0) {
				x++; // count it?
				continue;
			}
			
			if (y == 0){
				printf("wtf?\n");
				continue;
			}
			
			dat = (byte *)malloc(y);
				
			for (z = 0; z < y; w++) {
				page = &PMPages[w + PMSoundStart];
				memcpy(dat+z, PM_GetPage(w + PMSoundStart), page->length);
				z += page->length;
			}
			/* TODO: openal bug! */
			//alBufferData(buffers[x], AL_FORMAT_MONO8, dat, y, 6896);
			alBufferData(buffers[x], AL_FORMAT_MONO8, dat, y, 22050/4);
			
			if(alGetError() != AL_NO_ERROR) {
				printf("AL error\n");
			}
			
			free(dat);
			x++;
		}
		
		sources = (ALuint *)malloc(sizeof(ALuint) * 4);
		alGenSources(4, sources);
			
	}	
	SD_Started = true;
}

///////////////////////////////////////////////////////////////////////////
//
//	SD_Shutdown() - shuts down the Sound Mgr
//		Removes sound ISR and turns off whatever sound hardware was active
//
///////////////////////////////////////////////////////////////////////////
void SD_Shutdown(void)
{
	if (!SD_Started)
		return;
	
	if (cc) {
		alcDestroyContext(cc);
		cc = NULL;
	}
	
	SD_MusicOff();
	SD_StopSound();

	SD_Started = false;
}

///////////////////////////////////////////////////////////////////////////
//
//	SD_PlaySound() - plays the specified sound on the appropriate hardware
//
///////////////////////////////////////////////////////////////////////////

ALfloat gval[6], pval[6];

void SD_PlaySound(soundnames sound)
{
	int i;

	//printf("Playing sound %d, digimap %d\n", sound, DigiMap[sound]);
	//fflush(stdout);
	
	if (DigiMap[sound] != -1) {
		/* TODO: openal bug? (need to stop before play) */
		for (i = 0; i < 4; i++) {
			if (alSourceIsPlaying(sources[i]) == AL_FALSE) {
				//alSourceStop(*sources);
				alSourcefv(sources[i], AL_POSITION, gval);
				alSource3f(sources[i], AL_DIRECTION, -pval[0], 0.0f, -pval[2]);
				alSourcei(sources[i], AL_BUFFER, buffers[DigiMap[sound]]);
				alSourcePlay(sources[i]);
				break;
			}
		}
	}
}

/* TODO: velocity ?! */
void UpdateSoundLoc(fixed x, fixed y, int angle)
{		
	pval[0] = gval[0] = x >> 15;
	pval[1] = 0.0f;
	pval[2] = gval[2] = y >> 15;
	alListenerfv(AL_POSITION, pval);
	
	pval[3] = 0.0f;
	pval[4] = 1.0f;
	pval[5] = 0.0f;
	
	pval[0] *= sin(angle * PI / 180.0f);
	pval[2] *= cos(angle * PI / 180.0f);
	alListenerfv(AL_ORIENTATION, pval);
}

void PlaySoundLocGlobal(word sound, fixed x, fixed y)
{
	ALfloat val[3];
	int i;
			
	//printf("Playing sound %d, digimap %d\n", sound, DigiMap[sound]);
	//fflush(stdout);
	
	if (DigiMap[sound] != -1) {
		/* TODO: openal bug? (need to stop before play) */
		for (i = 0; i < 4; i++) {
			if (alSourceIsPlaying(sources[i]) == AL_FALSE) {
				//alSourceStop(*sources);
				val[0] = x >> 15;
				val[1] = 0.0f;
				val[2] = y >> 15;
				alSourcefv(sources[i], AL_POSITION, val);
				alSourcei(sources[i], AL_BUFFER, buffers[DigiMap[sound]]);
				alSourcePlay(sources[i]);
				break;
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////
//
//	SD_SoundPlaying() - returns the sound number that's playing, or 0 if
//		no sound is playing
//
///////////////////////////////////////////////////////////////////////////
/* TODO: SD_IsSoundPlaying or something? */
word SD_SoundPlaying(void)
{
	int i;
	
	/* Watch out for any looped sounds */
	for (i = 0; i < 4; i++) {
		if (alSourceIsPlaying(sources[i]) == AL_TRUE) {
			ALint ret;
			
			alGetSourcei(sources[i], AL_LOOPING, &ret);
			if (ret == AL_FALSE)
				return true;
		}
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////
//
//	SD_StopSound() - if a sound is playing, stops it
//
///////////////////////////////////////////////////////////////////////////
void SD_StopSound()
{
	//int i;
	
	/* TODO: this crashes for some reason... */
	//for (i = 0; i < 4; i++)
	//	if (alSourceIsPlaying(sources[i]) == AL_TRUE)
	//		alSourceStop(sources[i]);
}

///////////////////////////////////////////////////////////////////////////
//
//	SD_WaitSoundDone() - waits until the current sound is done playing
//
///////////////////////////////////////////////////////////////////////////
void SD_WaitSoundDone(void)
{
	while (SD_SoundPlaying());
}

///////////////////////////////////////////////////////////////////////////
//
//	SD_MusicOn() - turns on the sequencer
//
///////////////////////////////////////////////////////////////////////////
void SD_MusicOn(void)
{
	sqActive = true;
}

///////////////////////////////////////////////////////////////////////////
//
//	SD_MusicOff() - turns off the sequencer and any playing notes
//
///////////////////////////////////////////////////////////////////////////
void SD_MusicOff(void)
{
	sqActive = false;
}

///////////////////////////////////////////////////////////////////////////
//
//	SD_StartMusic() - starts playing the music pointed to
//
///////////////////////////////////////////////////////////////////////////
void SD_StartMusic(MusicGroup *music)
{
	SD_MusicOff();
}

///////////////////////////////////////////////////////////////////////////
//
//	SD_FadeOutMusic() - starts fading out the music. Call SD_MusicPlaying()
//		to see if the fadeout is complete
//
///////////////////////////////////////////////////////////////////////////
void SD_FadeOutMusic(void)
{
}

///////////////////////////////////////////////////////////////////////////
//
//	SD_MusicPlaying() - returns true if music is currently playing, false if
//		not
//
///////////////////////////////////////////////////////////////////////////
boolean SD_MusicPlaying(void)
{
	return false;
}
