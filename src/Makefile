CC = gcc
#CFLAGS = -Wall -O6 -fomit-frame-pointer -mpentiumpro -mcpu=pentiumpro -march=pentiumpro -DUSEVGA
CFLAGS = -g -DUSEVGA
#CFLAGS = -g -Wall -DUSEVGA -DDEBUG 
#CFLAGS = -g -DUSEVGA -DDEBUG
#CFLAGS = -g -pg -DDEBUG
OBJS = objs.o misc.o id_ca.o id_in.o id_sd.o id_vl.o id_vh.o id_us.o \
	wl_scale.o wl_draw.o wl_act1.o wl_act2.o wl_agent.o wl_game.o \
	wl_inter.o wl_menu.o wl_play.o wl_state.o wl_main.o wl_debug.o

#LFLAGS = -lm -lvga -pg
LFLAGS = -lm -lvga 

#LFLAGS = /home/relnev/midas/lib/linux/gcdebug/libmidas.a -lpthread -lm -lvga 
# /home/relnev/ElectricFence-2.0.5/libefence.a -lm -lvga 
NASM = nasm

.SUFFIXES: .asm

all:	wolf3d

.asm.o:
	$(NASM) -f elf -o $@ $<

wolf3d: $(OBJS)
	gcc -o wolf3d $(OBJS) $(LFLAGS) 

clean:
	-rm -rf wolf3d *.o

distclean: clean
	rm -rf *~ DEADJOE
