# st version
VERSION = 0.9.3

# Customize below to fit your system

# paths
PREFIX = /usr/local
MANPREFIX = $(PREFIX)/share/man

# The BSDs install X11 in different prefixes.  Linux uses the system prefix.
OS != uname -s
X11BASE != sh -c 'case "$(OS)" in \
	OpenBSD) echo /usr/X11R6 ;; \
	NetBSD) echo /usr/X11R7 ;; \
	FreeBSD|DragonFly) echo /usr/local ;; \
	*) echo /usr ;; \
esac'

X11INC = $(X11BASE)/include
X11LIB = $(X11BASE)/lib

# shm_open(3) is in libc on OpenBSD and FreeBSD; NetBSD and Linux need librt.
SHM_LIBS != sh -c 'case "$(OS)" in \
	OpenBSD|FreeBSD) ;; \
	*) echo -lrt ;; \
esac'

# includes and libs
INCS = -I$(X11INC) -I$(X11INC)/freetype2
LIBS = -L$(X11LIB) -lm -lz -lX11 -lutil -lXft -lXrender \
       -lfontconfig -lfreetype $(SHM_LIBS)

# flags
STCPPFLAGS = -DVERSION=\"$(VERSION)\" -D_XOPEN_SOURCE=600
STCFLAGS = $(INCS) $(STCPPFLAGS) $(CPPFLAGS) $(CFLAGS)
STLDFLAGS = $(LIBS) $(LDFLAGS)

# compiler and linker
CC = cc
