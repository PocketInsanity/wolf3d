CC = gcc
#CC = gcc-3.0
#CC=/opt/intel/compiler60/ia32/bin/icc

CFLAGS = -g -Wall -pedantic
#CFLAGS = -g -O2 -Wall -pedantic 
#CFLAGS = -Wall -O6 -fomit-frame-pointer -ffast-math -funroll-loops -mpentiumpro -mcpu=pentiumpro -march=pentiumpro
#CFLAGS = -g -Wall -pedantic -std=gnu99
#CFLAGS = -Os -Wall -pedantic
#CFLAGS = -Os -Wall -fomit-frame-pointer -ffast-math -mpentiumpro -mcpu=pentiumpro -march=pentiumpro
#CFLAGS=-O3 -xiMK -tpp6 -c99 -wp_ipo -g
OBJS = objs.o misc.o id_ca.o id_vh.o id_us.o \
	wl_act1.o wl_act2.o wl_act3.o wl_agent.o wl_game.o \
	wl_inter.o wl_menu.o wl_play.o wl_state.o wl_text.o wl_main.o \
	wl_debug.o vi_comm.o sd_comm.o
ROBJS = wl_draw.o
SOBJS = $(OBJS) $(ROBJS) vi_svga.o
XOBJS = $(OBJS) $(ROBJS) vi_xlib.o
DOBJS = $(OBJS) $(ROBJS) vi_sdl.o

#LFLAGS = -lm -wp_ipo
LFLAGS = -lm

OBJS += sd_null.o
#OBJS += sd_oss.o fmopl.o
#CFLAGS += -D_REENTRANT
#LFLAGS += -lpthread

CFLAGS += $(shell sdl-config --cflags)

SLFLAGS = $(LFLAGS) -lvga
XLFLAGS = $(LFLAGS) -L/usr/X11R6/lib -lX11 -lXext
DLFLAGS = $(LFLAGS) $(shell sdl-config --libs)

NASM = nasm

.SUFFIXES: .asm

#all:	swolf3d xwolf3d sdlwolf3d
all:	sdlwolf3d

$(SOBJS): version.h id_heads.h wl_def.h
$(XOBJS): version.h id_heads.h wl_def.h
$(DOBJS): version.h id_heads.h wl_def.h

.asm.o:
	$(NASM) -f elf -o $@ $<

swolf3d: $(SOBJS)
	$(CC) -o swolf3d $(SOBJS) $(SLFLAGS) 

xwolf3d: $(XOBJS)
	$(CC) -o xwolf3d $(XOBJS) $(XLFLAGS)

sdlwolf3d: $(DOBJS)
	$(CC) -o sdlwolf3d $(DOBJS) $(DLFLAGS)

clean:
	rm -rf swolf3d xwolf3d sdlwolf3d *.o *.il

distclean: clean
	rm -rf *~ DEADJOE

forced: distclean all
	rm -rf *.o
