#ifndef	__SD_COMM_H__
#define	__SD_COMM_H__

#define	TickBase	70	// 70Hz per tick

typedef	enum	{
					sdm_Off,
					sdm_PC,sdm_AdLib
				}	SDMode;
typedef	enum	{
					smm_Off,smm_AdLib
				}	SMMode;
typedef	enum	{
					sds_Off,sds_PC,sds_SoundSource,sds_SoundBlaster
				}	SDSMode;
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
} AdLibSound;

typedef	struct {
	word length, values[1];
} PACKED MusicGroup;

// Global variables
extern	boolean		AdLibPresent,
					SoundSourcePresent,
					SoundBlasterPresent,
					SoundPositioned;
extern	SDMode		SoundMode;
extern	SDSMode		DigiMode;
extern	SMMode		MusicMode;
extern	boolean		DigiPlaying;
extern	int		DigiMap[];

// Function prototypes
extern	void	SD_Startup(void), SD_Shutdown(void);

extern	void	SD_PlaySound(soundnames sound);
extern	void	SD_StopSound(void),
				SD_WaitSoundDone(void),
				SD_StartMusic(MusicGroup *music),
				SD_MusicOn(void),
				SD_MusicOff(void),
				SD_FadeOutMusic(void);

extern	boolean	SD_MusicPlaying(void),
				SD_SetSoundMode(SDMode mode),
				SD_SetMusicMode(SMMode mode);
extern	word	SD_SoundPlaying(void);

extern void SD_SetDigiDevice(SDSMode);
extern void SD_Poll();

void PlaySoundLocGlobal(word s, fixed gx, fixed gy);
void UpdateSoundLoc(fixed x, fixed y, int angle);

#endif
