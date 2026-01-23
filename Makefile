# Part of Astonia Server 3.5 (c) Daniel Brockhaus. Please read license.txt.

all: server35 create_character create_account \
runtime/generic/base.dll \
runtime/generic/lostcon.dll \
runtime/generic/merchant.dll \
runtime/generic/simple_baddy.dll \
runtime/generic/bank.dll \
runtime/generic/clubmaster.dll \
runtime/generic/book.dll \
runtime/generic/transport.dll \
runtime/generic/clanmaster.dll \
runtime/generic/pents.dll  \
runtime/generic/alchemy.dll \
runtime/generic/mine.dll \
runtime/generic/professor.dll \
runtime/generic/sidestory.dll \
runtime/1/cameron.dll \
runtime/1/shrike.dll \
runtime/2/below_aston.dll \
runtime/3/aston.dll \
runtime/3/gatekeeper.dll \
runtime/3/military.dll \
runtime/3/arena.dll \
runtime/5/sewers.dll \
runtime/6/edemon.dll  \
runtime/8/fdemon.dll \
runtime/10/ice.dll \
runtime/11/palace.dll \
runtime/14/random.dll \
runtime/15/swamp.dll \
runtime/16/forest.dll \
runtime/17/exkordon.dll \
runtime/18/bones.dll \
runtime/19/nomad.dll \
runtime/19/saltmine.dll \
runtime/22/lab1.dll \
runtime/22/lab2.dll \
runtime/22/lab3.dll \
runtime/22/lab4.dll \
runtime/22/lab5.dll \
runtime/23/strategy.dll \
runtime/25/warped.dll \
runtime/26/below_aston2.dll \
runtime/29/brannington.dll \
runtime/28/bran_forest.dll \
runtime/33/tunnel.dll \
runtime/36/caligar.dll \
runtime/37/arkhata.dll 

CC=gcc
DEBUG=-g
CFLAGS=-Wall -Wshadow -Werror -Wno-pointer-sign -O3 $(DEBUG) -fno-strict-aliasing -m32 -Isrc # for normal object files
LDFLAGS=-O $(DEBUG) -L/usr/lib/mysql -m32 # building tools
LDRFLAGS=-O $(DEBUG) -rdynamic -L/usr/lib/mysql -m32 # building server
DDFLAGS=-O $(DEBUG) -fPIC -shared -m32 # building DLLs
DFLAGS=$(CFLAGS) -fPIC -m32 -Isrc # for DLL object files
DEPFLAGS = -MMD -MP

OBJS=.obj/server.o .obj/io.o .obj/libload.o .obj/tool.o .obj/sleep.o \
.obj/log.o .obj/create.o .obj/notify.o .obj/skill.o .obj/do.o \
.obj/act.o .obj/player.o .obj/rdtsc.o .obj/los.o .obj/light.o \
.obj/map.o .obj/path.o .obj/error.o .obj/talk.o .obj/drdata.o \
.obj/death.o .obj/database.o .obj/see.o .obj/drvlib.o .obj/timer.o \
.obj/expire.o .obj/effect.o .obj/command.o .obj/date.o \
.obj/container.o .obj/store.o .obj/mem.o .obj/sector.o .obj/chat.o \
.obj/statistics.o .obj/mail.o .obj/player_driver.o .obj/clan.o \
.obj/lookup.o .obj/area.o .obj/task.o .obj/depot.o \
.obj/prof.o .obj/motd.o .obj/ignore.o .obj/tell.o .obj/clanlog.o \
.obj/respawn.o .obj/poison.o .obj/swear.o .obj/lab.o \
.obj/consistency.o .obj/btrace.o .obj/club.o .obj/balance.o \
.obj/questlog.o .obj/badip.o .obj/escape.o .obj/argon.o \
.obj/config.o

-include $(wildcard .obj/*.d)
-include $(wildcard .dllobj/*.d)

# ------- Server -----

server35:	$(OBJS) src/version.c
	$(CC) $(LDRFLAGS) -o server35 $(OBJS) src/version.c -lmysqlclient -lm -lz -ldl -lpthread -largon2
	
.obj/rdtsc.o:
	$(CC) $(CFLAGS) $(DEPFLAGS) -o .obj/rdtsc.o -c src/rdtsc.S

.obj/mem.o:
	$(CC) $(CFLAGS) $(DEPFLAGS) -DDEBUG -o $@ -c src/mem.c

.obj/%.o: src/%.c
	$(CC) $(CFLAGS) $(DEPFLAGS) -o $@ -c $<

# ------- DLLs -------

.dllobj/%.o: src/%.c
	$(CC) $(DFLAGS) $(DEPFLAGS) -o $@ -c $<


runtime/generic/base.dll:	.dllobj/base.o
	@mkdir -p runtime/generic
	$(CC) $(DDFLAGS) -o base.tmp .dllobj/base.o
	@mv base.tmp runtime/generic/base.dll

runtime/generic/sidestory.dll:	.dllobj/sidestory.o
	@mkdir -p runtime/generic
	$(CC) $(DDFLAGS) -o sidestory.tmp .dllobj/sidestory.o
	@mv sidestory.tmp runtime/generic/sidestory.dll

runtime/generic/pents.dll:	.dllobj/pents.o
	@mkdir -p runtime/generic
	$(CC) $(DDFLAGS) -o pents.tmp .dllobj/pents.o
	@mv pents.tmp runtime/generic/pents.dll

runtime/generic/professor.dll:	.dllobj/professor.o
	@mkdir -p runtime/generic
	$(CC) $(DDFLAGS) -o professor.tmp .dllobj/professor.o
	@mv professor.tmp runtime/generic/professor.dll

runtime/generic/bank.dll:	.dllobj/bank.o
	@mkdir -p runtime/generic
	$(CC) $(DDFLAGS) -o bank.tmp .dllobj/bank.o
	@mv bank.tmp runtime/generic/bank.dll

runtime/generic/alchemy.dll:	.dllobj/alchemy.o
	@mkdir -p runtime/generic
	$(CC) $(DDFLAGS) -o alchemy.tmp .dllobj/alchemy.o
	@mv alchemy.tmp runtime/generic/alchemy.dll

runtime/generic/book.dll:	.dllobj/book.o
	@mkdir -p runtime/generic
	$(CC) $(DDFLAGS) -o book.tmp .dllobj/book.o
	@mv book.tmp runtime/generic/book.dll

runtime/generic/transport.dll:	.dllobj/transport.o
	@mkdir -p runtime/generic
	$(CC) $(DDFLAGS) -o transport.tmp .dllobj/transport.o
	@mv transport.tmp runtime/generic/transport.dll

runtime/generic/clanmaster.dll:	.dllobj/clanmaster.o
	@mkdir -p runtime/generic
	$(CC) $(DDFLAGS) -o clanmaster.tmp .dllobj/clanmaster.o
	@mv clanmaster.tmp runtime/generic/clanmaster.dll

runtime/generic/clubmaster.dll:	.dllobj/clubmaster.o
	@mkdir -p runtime/generic
	$(CC) $(DDFLAGS) -o clubmaster.tmp .dllobj/clubmaster.o
	@mv clubmaster.tmp runtime/generic/clubmaster.dll

runtime/generic/lostcon.dll:	.dllobj/lostcon.o
	@mkdir -p runtime/generic
	$(CC) $(DDFLAGS) -o lostcon.tmp .dllobj/lostcon.o
	@mv lostcon.tmp runtime/generic/lostcon.dll

runtime/generic/merchant.dll:	.dllobj/merchant.o
	@mkdir -p runtime/generic
	$(CC) $(DDFLAGS) -o merchant.tmp .dllobj/merchant.o
	@mv merchant.tmp runtime/generic/merchant.dll

runtime/generic/simple_baddy.dll:	.dllobj/simple_baddy.o
	@mkdir -p runtime/generic
	$(CC) $(DDFLAGS) -o simple_baddy.tmp .dllobj/simple_baddy.o
	@mv simple_baddy.tmp runtime/generic/simple_baddy.dll

runtime/generic/mine.dll:	.dllobj/mine.o
	@mkdir -p runtime/generic
	$(CC) $(DDFLAGS) -o mine.tmp .dllobj/mine.o
	@mv mine.tmp runtime/generic/mine.dll

runtime/1/cameron.dll:	.dllobj/cameron.o
	@mkdir -p runtime/1
	$(CC) $(DDFLAGS) -o cameron.tmp .dllobj/cameron.o
	@mv cameron.tmp runtime/1/cameron.dll

runtime/1/shrike.dll:	.dllobj/shrike.o
	@mkdir -p runtime/1
	$(CC) $(DDFLAGS) -o shrike.tmp .dllobj/shrike.o
	@mv shrike.tmp runtime/1/shrike.dll

runtime/2/below_aston.dll:	.dllobj/below_aston.o
	@mkdir -p runtime/2
	$(CC) $(DDFLAGS) -o below_aston.tmp .dllobj/below_aston.o
	@mv below_aston.tmp runtime/2/below_aston.dll

runtime/3/aston.dll:	.dllobj/aston.o
	@mkdir -p runtime/3
	$(CC) $(DDFLAGS) -o aston.tmp .dllobj/aston.o
	@mv aston.tmp runtime/3/aston.dll


runtime/3/arena.dll:	.dllobj/arena.o
	@mkdir -p runtime/3
	$(CC) $(DDFLAGS) -o arena.tmp .dllobj/arena.o
	@mv arena.tmp runtime/3/arena.dll

runtime/3/gatekeeper.dll:	.dllobj/gatekeeper.o
	@mkdir -p runtime/3
	$(CC) $(DDFLAGS) -o gatekeeper.tmp .dllobj/gatekeeper.o
	@mv gatekeeper.tmp runtime/3/gatekeeper.dll

runtime/3/military.dll:	.dllobj/military.o
	@mkdir -p runtime/3
	@mkdir -p runtime/29
	$(CC) $(DDFLAGS) -o military.tmp .dllobj/military.o
	@cp military.tmp military.tmp2
	@mv military.tmp runtime/3/military.dll
	@mv military.tmp2 runtime/29/military.dll

runtime/5/sewers.dll:	.dllobj/sewers.o
	@mkdir -p runtime/5
	$(CC) $(DDFLAGS) -o sewers.tmp .dllobj/sewers.o
	@mv sewers.tmp runtime/5/sewers.dll

runtime/6/edemon.dll:	.dllobj/edemon.o
	@mkdir -p runtime/6
	$(CC) $(DDFLAGS) -o edemon.tmp .dllobj/edemon.o
	@mv edemon.tmp runtime/6/edemon.dll

runtime/8/fdemon.dll:	.dllobj/fdemon.o
	@mkdir -p runtime/8
	$(CC) $(DDFLAGS) -o fdemon.tmp .dllobj/fdemon.o
	@mv fdemon.tmp runtime/8/fdemon.dll

runtime/10/ice.dll:	.dllobj/ice.o
	@mkdir -p runtime/10
	$(CC) $(DDFLAGS) -o ice.tmp .dllobj/ice.o
	@mv ice.tmp runtime/10/ice.dll

runtime/11/palace.dll:	.dllobj/palace.o
	@mkdir -p runtime/11
	$(CC) $(DDFLAGS) -o palace.tmp .dllobj/palace.o
	@mv palace.tmp runtime/11/palace.dll

runtime/14/random.dll:	.dllobj/random.o
	@mkdir -p runtime/14
	$(CC) $(DDFLAGS) -o random.tmp .dllobj/random.o
	@mv random.tmp runtime/14/random.dll

runtime/15/swamp.dll:	.dllobj/swamp.o
	@mkdir -p runtime/15
	$(CC) $(DDFLAGS) -o swamp.tmp .dllobj/swamp.o
	@mv swamp.tmp runtime/15/swamp.dll

runtime/16/forest.dll:	.dllobj/forest.o
	@mkdir -p runtime/16
	$(CC) $(DDFLAGS) -o forest.tmp .dllobj/forest.o
	@mv forest.tmp runtime/16/forest.dll

runtime/17/exkordon.dll:	.dllobj/exkordon.o
	@mkdir -p runtime/17
	$(CC) $(DDFLAGS) -o exkordon.tmp .dllobj/exkordon.o
	@mv exkordon.tmp runtime/17/exkordon.dll

runtime/18/bones.dll:	.dllobj/bones.o
	@mkdir -p runtime/18
	$(CC) $(DDFLAGS) -o bones.tmp .dllobj/bones.o
	@mv bones.tmp runtime/18/bones.dll

runtime/19/nomad.dll:	.dllobj/nomad.o
	@mkdir -p runtime/19
	$(CC) $(DDFLAGS) -o nomad.tmp .dllobj/nomad.o
	@mv nomad.tmp runtime/19/nomad.dll

runtime/19/saltmine.dll:	.dllobj/saltmine.o
	@mkdir -p runtime/19
	$(CC) $(DDFLAGS) -o saltmine.tmp .dllobj/saltmine.o
	@mv saltmine.tmp runtime/19/saltmine.dll

runtime/22/lab2.dll:	.dllobj/lab2.o
	@mkdir -p runtime/22
	$(CC) $(DDFLAGS) -o lab2.tmp .dllobj/lab2.o
	@mv lab2.tmp runtime/22/lab2.dll

runtime/22/lab3.dll:	.dllobj/lab3.o
	@mkdir -p runtime/22
	$(CC) $(DDFLAGS) -o lab3.tmp .dllobj/lab3.o
	@mv lab3.tmp runtime/22/lab3.dll

runtime/22/lab4.dll:	.dllobj/lab4.o
	@mkdir -p runtime/22
	$(CC) $(DDFLAGS) -o lab4.tmp .dllobj/lab4.o
	@mv lab4.tmp runtime/22/lab4.dll

runtime/22/lab5.dll:	.dllobj/lab5.o
	@mkdir -p runtime/22
	$(CC) $(DDFLAGS) -o lab5.tmp .dllobj/lab5.o
	@mv lab5.tmp runtime/22/lab5.dll

runtime/22/lab1.dll:		.dllobj/lab1.o
	@mkdir -p runtime/22
	$(CC) $(DDFLAGS) -o lab1.tmp .dllobj/lab1.o
	@mv lab1.tmp runtime/22/lab1.dll

runtime/23/strategy.dll: 	.dllobj/strategy.o
	@mkdir -p runtime/23
	@mkdir -p runtime/24
	$(CC) $(DDFLAGS) -o strategy.tmp .dllobj/strategy.o
	@cp strategy.tmp strategy2.tmp
	@mv strategy.tmp runtime/23/strategy.dll
	@mv strategy2.tmp runtime/24/strategy.dll

runtime/25/warped.dll:	.dllobj/warped.o
	@mkdir -p runtime/25
	$(CC) $(DDFLAGS) -o warped.tmp .dllobj/warped.o
	@mv warped.tmp runtime/25/warped.dll

runtime/26/below_aston2.dll:	.dllobj/below_aston2.o
	@mkdir -p runtime/26
	$(CC) $(DDFLAGS) -o below_aston2.tmp .dllobj/below_aston2.o
	@mv below_aston2.tmp runtime/26/below_aston2.dll

runtime/28/bran_forest.dll:	.dllobj/bran_forest.o
	@mkdir -p runtime/28
	$(CC) $(DDFLAGS) -o bran_forest.tmp .dllobj/bran_forest.o
	@mv bran_forest.tmp runtime/28/bran_forest.dll

runtime/29/brannington.dll:	.dllobj/brannington.o
	@mkdir -p runtime/29
	$(CC) $(DDFLAGS) -o brannington.tmp .dllobj/brannington.o
	@mv brannington.tmp runtime/29/brannington.dll

runtime/33/tunnel.dll: 	.dllobj/tunnel.o
	@mkdir -p runtime/33
	$(CC) $(DDFLAGS) -o tunnel.tmp .dllobj/tunnel.o
	@mv tunnel.tmp runtime/33/tunnel.dll

runtime/36/caligar.dll: 	.dllobj/caligar.o
	@mkdir -p runtime/36
	$(CC) $(DDFLAGS) -o caligar.tmp .dllobj/caligar.o
	@mv caligar.tmp runtime/36/caligar.dll

runtime/37/arkhata.dll: 	.dllobj/arkhata.o
	@mkdir -p runtime/37
	$(CC) $(DDFLAGS) -o arkhata.tmp .dllobj/arkhata.o
	@mv arkhata.tmp runtime/37/arkhata.dll

# ------- Tools -----

chatserver:		.obj/chatserver.o
	$(CC) $(LDFLAGS) -o chatserver .obj/chatserver.o

.obj/chatserver.o:	src/chatserver.c
	$(CC) $(CFLAGS) -o .obj/chatserver.o -c src/chatserver.c

create_character:	src/create_character.c src/config.h src/server.h src/drdata.h .obj/config.o
	$(CC) $(CFLAGS) -o create_character src/create_character.c .obj/config.o -L/usr/lib/mysql -m32 -lmysqlclient

create_account:		src/create_account.c .obj/argon.o src/argon.h src/config.h .obj/config.o
	$(CC) $(CFLAGS) -o create_account src/create_account.c .obj/argon.o .obj/config.o -L/usr/lib/mysql -m32 -lmysqlclient -largon2

# ------- Helper -----

clean:
	@rm -f server35 .obj/*.o .obj/*.d .dllobj/*.o .dllobj/*.d runtime/*/*
	
pretty:
	@ls src/*.c src/*.h | xargs -r clang-format -i

pretty-check:
	@ls src/*.c src/*.h | xargs -r clang-format --dry-run -Werror

