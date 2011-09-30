#
# Makefile for UltraRogue 1.02
#
# UltraRogue
# Copyright (C) 1985 Herb Chong
# All rights reserved.
#
# Based on "Advanced Rogue"
# Copyright (C) 1984, 1985 Michael Morgan, Ken Dalka and AT&T
# All rights reserved.
#
# Based on "Rogue: Exploring the Dungeons of Doom"
# Copyright (C) 1980, 1981 Michael Toy, Ken Arnold and Glenn Wichman
# All rights reserved.
#
# See the file LICENSE.TXT for full copyright and licensing information.
#

DISTNAME=urogue1.02.1b
PROGRAM=urogue102

O=o

HDRS  = rogue.h mach_dep.h mdport.h tunable.h
OBJS1 = armor.$(O) artifact.$(O) chase.$(O) command.$(O) daemon.$(O) \
        daemons.$(O) encumb.$(O) fight.$(O) get_play.$(O) init.$(O) \
        io.$(O) list.$(O) main.$(O) maze.$(O) mdport.$(O)\
        misc.$(O) monsdata.$(O) monsters.$(O) move.$(O) new_level.$(O)
OBJS2 = options.$(O) pack.$(O) passages.$(O) player.$(O) potions.$(O) \
        rings.$(O) rip.$(O) rogue.$(O) rooms.$(O) save.$(O) scrolls.$(O) \
        state.$(O) sticks.$(O) things.$(O) trader.$(O) tunable.$(O) \
        vers.$(O) weapons.$(O) wizard.$(O) xcrypt.$(O)
OBJS  = $(OBJS1) $(OBJS2)
CFILES= \
      vers.c armor.c artifact.c chase.c command.c daemon.c \
      daemons.c encumb.c fight.c get_play.c init.c \
      io.c list.c main.c maze.c mdport.c \
      misc.c monsdata.c monsters.c move.c new_level.c \
      options.c pack.c passages.c player.c potions.c \
      rings.c rip.c rogue.c rooms.c save.c scrolls.c \
      state.c sticks.c things.c trader.c tunable.c \
      weapons.c wizard.c xcrypt.c
OTHER=control.c new.things README BUGS.TXT lav.c lex.c namefinder.c 
MISC=	Makefile LICENSE.TXT urogue102.sln urogue102.vcproj
DOCS=

CC    = gcc
CFLAGS= -g
CRLIB = -lcurses
RM    = rm -f
TAR   = tar
MAKEFILE = -f Makefile

.SUFFIXES: .obj

.c.obj:
	$(CC) $(CFLAGS) /c $*.c

$(PROGRAM): $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJS) $(CRLIB) -o $@

tags: $(HDRS) $(CFILES)
	ctags -u $?
	ed - tags < :ctfix
	sort tags -o tags

lint:
	lint -hxbc $(CFILES) $(CRLIB) > linterrs

clean:
	$(RM) $(OBJS1)
	$(RM) $(OBJS2)
	$(RM) core a.exe a.out a.exe.stackdump $(PROGRAM) $(PROGRAM).exe $(PROGRAM).tar $(PROGRAM).tar.gz $(PROGRAM).zip

count:
	wc -l $(HDRS) $(CFILES)

realcount:
	cc -E $(CFILES) | ssp - | wc -l

update:
	ar uv .SAVE $(CFILES) $(HDRS) $(MISC)

dist:
	@mkdir dist
	cp $(CFILES) $(HDRS) $(MISC) dist

dist.src:
	$(MAKE) $(MAKEFILE) clean
	tar cf $(DISTNAME)-src.tar $(CFILES) $(HDRS) $(MISC) $(DOCS) $(OTHER)
	gzip -f $(DISTNAME)-src.tar

dist.irix:
	$(MAKE) $(MAKEFILE) clean
	$(MAKE) $(MAKEFILE) CC=cc CFLAGS="-woff 1116 -O3" $(PROGRAM)
	tar cf $(DISTNAME)-irix.tar $(PROGRAM) LICENSE.TXT $(DOCS)
	gzip -f $(DISTNAME)-irix.tar

dist.aix:
	$(MAKE) $(MAKEFILE) clean
	$(MAKE) $(MAKEFILE) CC=xlc CFLAGS="-qmaxmem=16768 -O3 -qstrict" $(PROGRAM)
	tar cf $(DISTNAME)-aix.tar $(PROGRAM) LICENSE.TXT $(DOCS)
	gzip -f $(DISTNAME)-aix.tar

debug.linux:
	$(MAKE) $(MAKEFILE) clean
	$(MAKE) $(MAKEFILE) CFLAGS="-g -DWIZARD" $(PROGRAM)

dist.linux:
	$(MAKE) $(MAKEFILE) clean
	$(MAKE) $(MAKEFILE) CFLAGS="-g -pedantic -Wall"  $(PROGRAM)
	tar cf $(DISTNAME)-linux.tar $(PROGRAM) LICENSE.TXT $(DOCS)
	gzip -f $(DISTNAME)-linux.tar
	
debug.interix: 
	$(MAKE) $(MAKEFILE) clean
	$(MAKE) $(MAKEFILE) CFLAGS="-g3 -DWIZARD" $(PROGRAM)
	
dist.interix: 
	$(MAKE) $(MAKEFILE) clean
	$(MAKE) $(MAKEFILE) $(PROGRAM)
	tar cf $(DISTNAME)-interix.tar $(PROGRAM) LICENSE.TXT $(DOCS)
	gzip -f $(DISTNAME)-interix.tar
	
debug.cygwin:
	$(MAKE) $(MAKEFILE) clean
	$(MAKE) $(MAKEFILE) CFLAGS="-g3 -DWIZARD" $(PROGRAM)

dist.cygwin:
	$(MAKE) $(MAKEFILE) clean
	$(MAKE) $(MAKEFILE) CRLIB="-static -lcurses" $(PROGRAM)
	tar cf $(DISTNAME)-cygwin.tar $(PROGRAM).exe LICENSE.TXT $(DOCS)
	gzip -f $(DISTNAME)-cygwin.tar

#
# Use MINGW32-MAKE to build this target
#
dist.mingw32:
	@$(MAKE) --no-print-directory RM="cmd /c del" clean
	@$(MAKE) $(MAKEFILE) --no-print-directory CPPFLAGS="-I../pdcurses" CRLIB="../pdcurses/pdcurses.a" $(PROGRAM)
	#@$(MAKE) --no-print-directory CRLIB="-lpdcurses" $(PROGRAM)
	cmd /c del $(DISTNAME)-mingw32.zip
	zip $(DISTNAME)-mingw32.zip $(PROGRAM).exe LICENSE.TXT $(DOCS)
	
dist.msys:
	@$(MAKE) $(MAKEFILE) --no-print-directory clean
	@$(MAKE) $(MAKEFILE) --no-print-directory CRLIB="-lcurses" $(PROGRAM)
	tar cf $(DISTNAME)-msys.tar $(PROGRAM).exe LICENSE.TXT $(DOCS)
	gzip -f $(DISTNAME)-msys.tar
	
debug.djgpp: 
	$(MAKE) $(MAKEFILE) clean
	$(MAKE) $(MAKEFILE) CFGLAGS="-g3 -DWIZARD" LDFLAGS="-L$(DJDIR)/LIB" CRLIB="-lpdcurses" $(PROGRAM)

dist.djgpp: 
	@$(MAKE) $(MAKEFILE) --no-print-directory clean
	@$(MAKE) $(MAKEFILE) --no-print-directory LDFLAGS="-L$(DJDIR)/LIB" CRLIB="-lpdcurses" $(PROGRAM)
	rm -f $(DISTNAME)-djgpp.zip
	zip $(DISTNAME)-djgpp.zip $(PROGRAM).exe LICENSE.TXT $(DOCS)

#
# Use NMAKE to build this target
#

debug.win32:
	n$(MAKE) $(MAKEFILE) O="obj" RM="-del" clean
	n$(MAKE) $(MAKEFILE) O="obj" CC="CL" CRLIB="..\pdcurses\pdcurses.lib shfolder.lib user32.lib Advapi32.lib" CFLAGS="-DWIZARD -nologo -I..\pdcurses -Ox -wd4033 -wd4716" $(PROGRAM)

dist.win32:
	$(MAKE) $(MAKEFILE) O="obj" RM="-del" clean
	$(MAKE) $(MAKEFILE) O="obj" CC="CL" CRLIB="..\pdcurses\pdcurses.lib shfolder.lib user32.lib Advapi32.lib" CFLAGS="-nologo -I..\pdcurses -Ox -wd4033 -wd4716" $(PROGRAM)
	-del $(DISTNAME)-win32.zip
	zip $(DISTNAME)-win32.zip $(PROGRAM).exe LICENSE.TXT $(DOCS)

vers.$(O): vers.c rogue.h
armor.$(O): armor.c rogue.h
artifact.$(O): artifact.c rogue.h
chase.$(O): chase.c rogue.h
command.$(O): command.c rogue.h
daemon.$(O): daemon.c rogue.h
daemons.$(O): daemons.c rogue.h
encumb.$(O): encumb.c rogue.h
fight.$(O): fight.c rogue.h
get_play.$(O): get_play.c rogue.h
init.$(O): init.c rogue.h
io.$(O): io.c rogue.h
list.$(O): list.c rogue.h
main.$(O): main.c rogue.h
maze.$(O): maze.c rogue.h
mdport.$(O): mdport.c rogue.h
misc.$(O): misc.c rogue.h
monsdata.$(O): monsdata.c rogue.h
monsters.$(O): monsters.c rogue.h
move.$(O): move.c rogue.h
new_level.$(O): new_level.c rogue.h
options.$(O): options.c rogue.h
pack.$(O): pack.c rogue.h
passages.$(O): passages.c rogue.h
player.$(O): player.c rogue.h
potions.$(O): potions.c rogue.h
rings.$(O): rings.c rogue.h
rip.$(O): rip.c rogue.h
rogue.$(O): rogue.c rogue.h
rooms.$(O): rooms.c rogue.h
save.$(O): save.c rogue.h
scrolls.$(O): scrolls.c rogue.h
state.$(O): state.c rogue.h
sticks.$(O): sticks.c rogue.h
things.$(O): things.c rogue.h
trader.$(O): trader.c rogue.h
tunable.$(O): tunable.c rogue.h
weapons.$(O): weapons.c rogue.h
wizard.$(O): wizard.c rogue.h
xcrypt.$(O): xcrypt.c
