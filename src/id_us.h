#ifndef	__ID_US_H__
#define	__ID_US_H__

#define	MaxX	320
#define	MaxY	200

#define	MaxHighName	57
#define	MaxScores	7
typedef	struct
		{
			char	name[MaxHighName + 1];
			long	score;
			word	completed,episode;
		} HighScore;

#define	MaxString	128	// Maximum input string size

typedef	struct
		{
			int	x,y,
				w,h,
				px,py;
		} WindowRec;	// Record used to save & restore screen windows

extern	boolean		tedlevel; /* TODO: rename or remove */
extern	int			tedlevelnum;

extern	boolean		ingame,		// Set by game code if a game is in progress
					abortgame,	// Set if a game load failed
					loadedgame,	// Set if the current game was loaded
					NoWait;
extern	word		PrintX,PrintY;	// Current printing location in the window
extern	word		WindowX,WindowY,// Current location of window
					WindowW,WindowH;// Current size of window

extern	int			CursorX,CursorY;

extern	void		(*USL_MeasureString)(char *,word *,word *),
					(*USL_DrawString)(char *);

extern	HighScore	Scores[];

void US_Startup(void),
				US_Setup(void),
				US_Shutdown(void),
				US_InitRndT(boolean randomize),
				US_DrawWindow(word x,word y,word w,word h),
				US_ClearWindow(void),
				US_PrintCentered(char *s),
				US_CPrint(char *s),
				US_CPrintLine(char *s),
				US_Print(char *s),
				US_PrintUnsigned(longword n),
				US_PrintSigned(long n),
				US_CheckHighScore(long score,word other);
boolean	US_LineInput(int x,int y,char *buf,char *def,boolean escok,
				int maxchars,int maxwidth);
int		US_CheckParm(char *parm,char **strings),
				US_RndT(void);

		void	USL_PrintInCenter(char *s,Rect r);

#elif
#error "fix me: TODO"
#endif
