CC = gcc
#CC = g++

CFLAGS = -Wall -O6 -fomit-frame-pointer -ffast-math -funroll-loops -mpentiumpro -mcpu=pentiumpro -march=pentiumpro 
#CFLAGS = -g -Wall -O6 -fomit-frame-pointer -ffast-math -funroll-loops -mpentiumpro -mcpu=pentiumpro -march=pentiumpro 
CFLAGS = -g -Wall -ansi -pedantic
#CFLAGS = -Os -g

OBJS 	 = Data.o Doors.o EnMove.o EnThink.o Intro.o Level.o \
	   Missiles.o Sounds.o PlMove.o PlStuff.o PlThink.o PushWall.o \
	   RefBsp.o Refresh.o Refresh2.o Sight.o Main.o StateDef.o \
	   WolfMain.o WolfIO.o InterMis.o SaveLoad.o Burger.o stub.o res.o
SOFTOBJS = RefSprite.o SoftDraw.o SoftDraw2.o
OGLOBJS	 = GLDraw.o
SOBJS	 = $(OBJS) $(SOFTOBJS) vi_svga.o
XOBJS	 = $(OBJS) $(SOFTOBJS) vi_xlib.o
XMOBJS   = $(OBJS) $(SOFTOBJS) vi_xm.o
GOBJS	 = $(OBJS) $(SOFTOBJS) vi_gtk.o
GLOBJS	 = $(OBJS) $(OGLOBJS)  vi_glx.o
GLXMOBJS = $(OBJS) $(OGLOBJS)  vi_glxm.o

LFLAGS = -lm 
#LFLAGS = -lm /home/relnev/ElectricFence-2.1/libefence.a
#LFLAGS = -lm /home/relnev/ElectricFence-2.2.2/libefence.a -lpthread

SLFLAGS		= $(LFLAGS) -lvga
#XLFLAGS 	= $(LFLAGS) -L/usr/X11R6/lib -lX11 -lXext -lXxf86vm -lXxf86dga
XLFLAGS		= $(LFLAGS) -L/usr/X11R6/lib -lX11
XMLFLAGS	= $(LFLAGS) -L/usr/X11R6/lib -lX11 -L/usr/local/motif/lib
GLFLAGS		= $(LFLAGS) `gtk-config --libs` # -L/usr/X11R6/lib -lX11 -lXi -lXext -lgdk -lgtk 
GLLFLAGS	= $(LFLAGS) -L/usr/lib -L/usr/X11R6/lib -lX11 -lXext -lGL
GLXMLFLAGS	= $(LFLAGS) -L/usr/lib -L/usr/X11R6/lib -L/usr/local/motif/lib -lX11 -lXext -lGL

NASM = nasm

.SUFFIXES: .asm

all:	swolf3d xwolf3d xmwolf3d gwolf3d glwolf3d glxmwolf3d

$(SOBJS):  Sounds.h  Sprites.h  States.h  Wolf.h  burger.h  wolfdef.h
$(XOBJS):  Sounds.h  Sprites.h  States.h  Wolf.h  burger.h  wolfdef.h
$(XMOBJS): Sounds.h  Sprites.h  States.h  Wolf.h  burger.h  wolfdef.h
$(GOBJS):  Sounds.h  Sprites.h  States.h  Wolf.h  burger.h  wolfdef.h
$(GLOBJS): Sounds.h  Sprites.h  States.h  Wolf.h  burger.h  wolfdef.h
$(GLXMOBJS): Sounds.h  Sprites.h  States.h  Wolf.h  burger.h  wolfdef.h

.asm.o:
	$(NASM) -f elf -o $@ $<

vi_xm.o: vi_xm.c
	gcc -c vi_xm.c $(CFLAGS) -I/usr/local/motif/include

vi_gtk.o: vi_gtk.c
	gcc -c vi_gtk.c $(CFLAGS) `gtk-config --cflags`

vi_glxm.o: vi_glxm.c
	gcc -c vi_glxm.c $(CFLAGS)  -I/usr/local/motif/include

swolf3d: $(SOBJS)
	gcc -o swolf3d $(SOBJS) $(SLFLAGS) 

xwolf3d: $(XOBJS)
	gcc -o xwolf3d $(XOBJS) $(XLFLAGS)

xmwolf3d: $(XMOBJS)
	gcc -o xmwolf3d $(XMOBJS) $(XMLFLAGS)

gwolf3d: $(GOBJS)
	gcc -o gwolf3d $(GOBJS) $(GLFLAGS)

glwolf3d: $(GLOBJS)
	gcc -o glwolf3d $(GLOBJS) $(GLLFLAGS)

glxmwolf3d: $(GLXMOBJS)
	gcc -o glxmwolf3d $(GLXMOBJS) $(GLXMLFLAGS)

clean:
	rm -rf swolf3d xwolf3d gwolf3d glwolf3d xmwolf3d glxmwolf3d *.o

distclean: clean
	rm -rf core *~ DEADJOE

forced: distclean all
	rm -rf *.o
