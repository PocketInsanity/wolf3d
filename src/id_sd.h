#ifndef	__ID_SD_H__
#define	__ID_SD_H__

#define	TickBase	70		// 70Hz per tick - used as a base for timer 0

typedef	enum	{
					sdm_Off,
					sdm_PC,sdm_AdLib,
				}	SDMode;
typedef	enum	{
					smm_Off,smm_AdLib
				}	SMMode;
typedef	enum	{
					sds_Off,sds_PC,sds_SoundSource,sds_SoundBlaster
				}	SDSMode;
typedef	struct
		{
			longword	length;
			word		priority;
		} SoundCommon;

typedef	struct
		{
			SoundCommon	common;
			byte		data[1];
		} PCSound;

typedef	struct
		{
			SoundCommon	common;
			word		hertz;
			byte		bits,
						reference,
						data[1];
		} SampledSound;

typedef	struct
		{
			byte	mChar,cChar,
					mScale,cScale,
					mAttack,cAttack,
					mSus,cSus,
					mWave,cWave,
					nConn,

					// These are only for Muse - these bytes are really unused
					voice,
					mode,
					unused[3];
		} Instrument;

typedef	struct
		{
			SoundCommon	common;
			Instrument	inst;
			byte		block,
						data[1];
		} AdLibSound;

//
//	Sequencing stuff
//
#define	sqMaxTracks	10
#define	sqMaxMoods	1	// DEBUG

#define	sev_Null		0	// Does nothing
#define	sev_NoteOff		1	// Turns a note off
#define	sev_NoteOn		2	// Turns a note on
#define	sev_NotePitch		3	// Sets the pitch of a currently playing note
#define	sev_NewInst		4	// Installs a new instrument
#define	sev_NewPerc		5	// Installs a new percussive instrument
#define	sev_PercOn		6	// Turns a percussive note on
#define	sev_PercOff		7	// Turns a percussive note off
#define	sev_SeqEnd		-1	// Terminates a sequence

// 	Flags for MusicGroup.flags
#define	sf_Melodic		0
#define	sf_Percussive	1

typedef	struct {
	word length, values[1];
} PACKED MusicGroup;

typedef	struct
		{
			/* This part needs to be set up by the user */
			word        mood, *moods[sqMaxMoods];

			/* The rest is set up by the code */
			Instrument	inst;
			boolean		percussive;
			word		*seq;
			longword	nextevent;
		} ActiveTrack;

#define	sqmode_Normal		0
#define	sqmode_FadeIn		1
#define	sqmode_FadeOut		2

#define	sqMaxFade		64	// DEBUG


// Global variables
extern	boolean		AdLibPresent,
					SoundSourcePresent,
					SoundBlasterPresent,
					NeedsMusic,					// For Caching Mgr
					SoundPositioned;
extern	SDMode		SoundMode;
extern	SDSMode		DigiMode;
extern	SMMode		MusicMode;
extern	boolean		DigiPlaying;
extern	int			DigiMap[];

// Function prototypes
extern	void	SD_Startup(void),
				SD_Shutdown(void),
				SD_Default(boolean gotit,SDMode sd,SMMode sm),

				SD_PositionSound(int leftvol,int rightvol);
extern	boolean	SD_PlaySound(soundnames sound);
extern	void	SD_SetPosition(int leftvol,int rightvol),
				SD_StopSound(void),
				SD_WaitSoundDone(void),

				SD_StartMusic(MusicGroup *music),
				SD_MusicOn(void),
				SD_MusicOff(void),
				SD_FadeOutMusic(void),

				SD_SetUserHook(void (*hook)(void));
extern	boolean	SD_MusicPlaying(void),
				SD_SetSoundMode(SDMode mode),
				SD_SetMusicMode(SMMode mode);
extern	word	SD_SoundPlaying(void);

extern	void	SD_SetDigiDevice(SDSMode),
				SD_PlayDigitized(word which,int leftpos,int rightpos),
				SD_StopDigitized(void),
				SD_Poll(void);

#else
#error "fix me TODO"
#endif
