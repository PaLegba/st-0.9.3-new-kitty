# st version
VERSION = 0.9.3

# Customize below to fit your system

# paths
PREFIX = /usr/local
MANPREFIX = $(PREFIX)/share/man

# X11 headers and libraries. On OpenBSD these are part of the base system.
# pkg-config is used for fontconfig, freetype2 and zlib on both Linux and
# OpenBSD.
X11INC = /usr/X11R6/include
X11LIB = /usr/X11R6/lib

PKG_CONFIG = pkg-config

# includes and libs
INCS = -I$(X11INC) \
       `$(PKG_CONFIG) --cflags fontconfig freetype2`
LIBS = -L$(X11LIB) -lm -lX11 -lutil -lXft -lXrender \
       `$(PKG_CONFIG) --libs zlib fontconfig freetype2`

# flags
STCPPFLAGS = -DVERSION=\"$(VERSION)\" -D_XOPEN_SOURCE=600
STCFLAGS = $(INCS) $(STCPPFLAGS) $(CPPFLAGS) $(CFLAGS)
STLDFLAGS = $(LIBS) $(LDFLAGS)

# compiler and linker
CC = cc
