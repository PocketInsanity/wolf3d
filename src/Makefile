CC = gcc

#CFLAGS = -Wall -O6 -fomit-frame-pointer -ffast-math -funroll-loops -mpentiumpro -mcpu=pentiumpro -march=pentiumpro
#CFLAGS = -g -Wall
CFLAGS = -g -Wall -pedantic
#CFLAGS = -Os -Wall
#CFLAGS = -Os -Wall -fomit-frame-pointer -ffast-math -mpentiumpro -mcpu=pentiumpro -march=pentiumpro

OBJS = objs.o misc.o id_ca.o id_vh.o id_us.o \
	wl_act1.o wl_act2.o wl_act3.o wl_agent.o wl_game.o \
	wl_inter.o wl_menu.o wl_play.o wl_state.o wl_text.o wl_main.o \
	wl_debug.o gfxsave.o
ROBJS = wl_draw.o
SOBJS = $(OBJS) $(ROBJS) vi_svga.o
XOBJS = $(OBJS) $(ROBJS) vi_xlib.o
DOBJS = $(OBJS) $(ROBJS) vi_sdl.o

LFLAGS = -lm

OBJS += sd_null.o
#OBJS += sd_oss.o fmopl.o
#CFLAGS += -D_REENTRANT
#LFLAGS += -lpthread


CFLAGS += `sdl-config --cflags`

SLFLAGS = $(LFLAGS) -lvga
XLFLAGS = $(LFLAGS) -L/usr/X11R6/lib -lX11 -lXext -lXxf86vm -lXxf86dga
DLFLAGS = $(LFLAGS) `sdl-config --libs` -L/usr/X11R6/lib -lX11 -lXext

NASM = nasm

.SUFFIXES: .asm

all:	swolf3d xwolf3d sdlwolf3d

$(SOBJS): version.h id_heads.h wl_def.h
$(XOBJS): version.h id_heads.h wl_def.h
$(DOBJS): version.h id_heads.h wl_def.h

.asm.o:
	$(NASM) -f elf -o $@ $<

swolf3d: $(SOBJS)
	gcc -o swolf3d $(SOBJS) $(SLFLAGS) 

xwolf3d: $(XOBJS)
	gcc -o xwolf3d $(XOBJS) $(XLFLAGS)

sdlwolf3d: $(DOBJS)
	gcc -o sdlwolf3d $(DOBJS) $(DLFLAGS)

clean:
	rm -rf swolf3d xwolf3d sdlwolf3d *.o

distclean: clean
	rm -rf *~ DEADJOE

forced: distclean all
	rm -rf *.o
