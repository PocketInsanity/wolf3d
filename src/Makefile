CC = gcc
#CFLAGS = -Wall -O6 -fomit-frame-pointer -mpentiumpro -mcpu=pentiumpro -march=pentiumpro -DUSEVGA
CFLAGS = -g -Wall
#CFLAGS = -Os -fwritable-strings
#CFLAGS = -g -Wall -DUSEVGA -DDEBUG 
#CFLAGS = -g -DUSEVGA -DDEBUG
#CFLAGS = -g -pg -DDEBUG
OBJS = objs.o misc.o id_ca.o id_in.o id_sd.o id_vl.o id_vh.o id_us.o \
	wl_scale.o wl_draw.o wl_act1.o wl_act2.o wl_agent.o wl_game.o \
	wl_inter.o wl_menu.o wl_play.o wl_state.o wl_text.o wl_main.o \
	wl_debug.o

LFLAGS = -lm -lvga
#LFLAGS = -lm -lvga ElectricFence-2.2.2/libefence.a -lpthread

NASM = nasm

.SUFFIXES: .asm

all:	wolf3d

$(OBJS): version.h id_heads.h

.asm.o:
	$(NASM) -f elf -o $@ $<

wolf3d: $(OBJS)
	gcc -o wolf3d $(OBJS) $(LFLAGS) 

clean:
	-rm -rf wolf3d *.o

distclean: clean
	rm -rf *~ DEADJOE
