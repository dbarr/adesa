CC      = cc
PROF    = 
NOCRYPT =

C_FLAGS = -O -g -Wall -ansi
L_FLAGS = -O -g

O_FILES = auction.o dns.o treasury.o act_comm.o act_info.o act_move.o act_obj.o act_wiz.o comm.o const.o \
          db.o fight.o handler.o interp.o magic.o magic2.o mob_commands.o mob_prog.o \
	  save.o special.o update.o board.o areasave.o buildtab.o build.o \
          write.o act_clan.o buildare.o hunt.o clutch.o \
          obj_fun.o macros.o trigger.o magic3.o quest.o lists.o \
          social-edit.o mount.o pdelete.o wizutil.o ssm.o \
	  scheck.o rulers.o spendqp.o enchant.o mccp.o trade.o md5c.o duel.o magic4.o version.o


all: merc

build: all

merc: $(O_FILES)
	rm -f merc
	$(CC) $(L_FLAGS) -o merc $(O_FILES) -lm -lz -lcrypt
	@mv version.c version.c.last

.c.o: merc.h
	$(CC) -c $(C_FLAGS) $<

version.c: version.c.sh
	@/bin/sh ./version.c.sh

clean:
	rm -f $(O_FILES) merc ../area/core*

