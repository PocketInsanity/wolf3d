#ifndef __MISC_H__
#define __MISC_H__

extern int _argc;
extern char **_argv;

void SavePCX256ToFile(unsigned char *buf, int width, int height, unsigned char *pal, char *name);
void SavePCXRGBToFile(unsigned char *buf, int width, int height, char *name);

void set_TimeCount(unsigned long t);
unsigned long get_TimeCount();

#ifndef DOSISM

long filelength(int handle);

#define stricmp strcasecmp
#define strnicmp strncasecmp
char *strlwr(char *s);

char *itoa(int value, char *string, int radix);
char *ltoa(long value, char *string, int radix);
char *ultoa(unsigned long value, char *string, int radix);

#endif /* DOSISM */

uint16_t SwapInt16L(uint16_t i);
uint32_t SwapInt32L(uint32_t i);

extern int OpenWrite(char *fn);
extern int OpenWriteAppend(char *fn);
extern void CloseWrite(int fp);

extern int WriteSeek(int fp, int offset, int whence);
extern int WritePos(int fp);

extern int WriteInt8(int fp, int8_t d);
extern int WriteInt16(int fp, int16_t d);
extern int WriteInt32(int fp, int32_t d);
extern int WriteBytes(int fp, byte *d, int len);

extern int OpenRead(char *fn);
extern void CloseRead(int fp);

extern int ReadSeek(int fp, int offset, int whence);
extern int ReadLength(int fp);

extern int8_t ReadInt8(int fp);
extern int16_t ReadInt16(int fp);
extern int32_t ReadInt32(int fp);
extern int ReadBytes(int fp, byte *d, int len);

#endif
