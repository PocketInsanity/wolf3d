CC = gcc

#CFLAGS = -Wall -O6 -fomit-frame-pointer -ffast-math -funroll-loops -mpentiumpro -mcpu=pentiumpro -march=pentiumpro 
#CFLAGS = -g -Wall
CFLAGS = -g
#CFLAGS = -Os
#CFLAGS = -g -Wall -I/home/relnev/cvs/oal/include

OBJS =  Data.o Doors.o EnMove.o EnThink.o InterMis.o Intro.o Level.o \
	Missiles.o Music.o PlMove.o PlStuff.o PlThink.o PushWall.o \
	RefBsp.o RefSprite.o Refresh.o Refresh2.o Sight.o Main.o \
	StateDef.o WolfMain.o WolfIO.o Burger.o 
SOBJS = $(OBJS) 
XOBJS = $(OBJS)
GOBJS = $(OBJS)

LFLAGS = -lm 
#LFLAGS = -lm -L/home/relnev/cvs/oal/linux/src -lopenal -lpthread -ldl

SLFLAGS = $(LFLAGS) -lvga
XLFLAGS = $(LFLAGS) -L/usr/X11R6/lib -lX11 -lXext -lXxf86vm -lXxf86dga
GLFLAGS = $(LFLAGS) -L/usr/X11R6/lib -lX11 -lXext -lGL

NASM = nasm

.SUFFIXES: .asm

all:	swolf3d xwolf3d gwolf3d

$(SOBJS):
$(XOBJS):
$(GOBJS):

.asm.o:
	$(NASM) -f elf -o $@ $<

swolf3d: $(SOBJS)
	gcc -o swolf3d $(SOBJS) $(SLFLAGS) 

xwolf3d: $(XOBJS)
	gcc -o xwolf3d $(XOBJS) $(XLFLAGS)

gwolf3d: $(GOBJS)
	gcc -o gwolf3d $(GOBJS) $(GLFLAGS)

clean:
	rm -rf swolf3d xwolf3d gwolf3d *.o

distclean: clean
	rm -rf *~ DEADJOE

forced: distclean all
	rm -rf *.o
