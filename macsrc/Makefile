CC = gcc

#CFLAGS = -Wall -O6 -fomit-frame-pointer -ffast-math -funroll-loops -mpentiumpro -mcpu=pentiumpro -march=pentiumpro 
#CFLAGS = -g -Wall -O6 -fomit-frame-pointer -ffast-math -funroll-loops -mpentiumpro -mcpu=pentiumpro -march=pentiumpro 
CFLAGS = -g -Wall -ansi -pedantic -W -O2
#CFLAGS = -Os -g

CFLAGS += $(shell sdl-config --cflags)

OBJS 	 = Data.o Doors.o EnMove.o EnThink.o Intro.o Level.o \
	   Missiles.o Sounds.o PlMove.o PlStuff.o PlThink.o PushWall.o \
	   RefBsp.o Refresh.o Refresh2.o Sight.o Main.o StateDef.o \
	   WolfMain.o WolfIO.o InterMis.o SaveLoad.o Burger.o stub.o res.o
SOFTOBJS = RefSprite.o SoftDraw.o SoftDraw2.o
OGLOBJS	 = GLDraw.o
SOBJS	 = $(OBJS) $(SOFTOBJS) vi_svga.o
XOBJS	 = $(OBJS) $(SOFTOBJS) vi_xlib.o
GOBJS	 = $(OBJS) $(SOFTOBJS) vi_gtk.o gtkimagedb.o
GLOBJS	 = $(OBJS) $(OGLOBJS)  vi_glx.o
DOBJS	 = $(OBJS) $(SOFTOBJS) vi_sdl.o

LFLAGS = -lm 

SLFLAGS		= $(LFLAGS) -lvga
#XLFLAGS 	= $(LFLAGS) -L/usr/X11R6/lib -lX11 -lXext -lXxf86vm -lXxf86dga
XLFLAGS		= $(LFLAGS) -L/usr/X11R6/lib -lX11
GLFLAGS		= $(LFLAGS) $(shell gtk-config --libs)
GLLFLAGS	= $(LFLAGS) -L/usr/X11R6/lib -lX11 -lXext -lGL
DLFLAGS		= $(LFLAGS) $(shell sdl-config --libs)

NASM = nasm

.SUFFIXES: .asm

#all:	swolf3d xwolf3d gwolf3d glwolf3d sdlwolf3d

all: xwolf3d

$(SOBJS):  Sounds.h  Sprites.h  States.h  Wolf.h  burger.h  wolfdef.h
$(XOBJS):  Sounds.h  Sprites.h  States.h  Wolf.h  burger.h  wolfdef.h
$(GOBJS):  Sounds.h  Sprites.h  States.h  Wolf.h  burger.h  wolfdef.h
$(GLOBJS): Sounds.h  Sprites.h  States.h  Wolf.h  burger.h  wolfdef.h
$(DOBJS):  Sounds.h  Sprites.h  States.h  Wolf.h  burger.h  wolfdef.h

.asm.o:
	$(NASM) -f elf -o $@ $<

vi_gtk.o: vi_gtk.c
	$(CC) -c vi_gtk.c $(CFLAGS) $(shell gtk-config --cflags)

gtkimagedb.o: gtkimagedb.c gtkimagedb.h
	$(CC) -c gtkimagedb.c $(CFLAGS) $(shell gtk-config --cflags)

swolf3d: $(SOBJS)
	$(CC) -o swolf3d $(SOBJS) $(SLFLAGS) 

xwolf3d: $(XOBJS)
	$(CC) -o xwolf3d $(XOBJS) $(XLFLAGS)

gwolf3d: $(GOBJS)
	$(CC) -o gwolf3d $(GOBJS) $(GLFLAGS)

glwolf3d: $(GLOBJS)
	$(CC) -o glwolf3d $(GLOBJS) $(GLLFLAGS)

sdlwolf3d: $(DOBJS)
	$(CC) -o sdlwolf3d $(DOBJS) $(DLFLAGS)

clean:
	rm -rf swolf3d xwolf3d gwolf3d glwolf3d sdlwolf3d *.o

distclean: clean
	rm -rf core *~ DEADJOE

forced: distclean all
	rm -rf *.o
