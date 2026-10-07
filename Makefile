## Haiku Generic Build Makefile ##
# Native build on Haiku: `make`. On a gcc2 hybrid (x86_gcc2) build with the
# modern compiler: `setarch x86 make`. Cross builds: tools/cross-build.sh.
NAME = RWriteRoom
TYPE = APP
APP_MIME_SIG = application/x-vnd.rainygirl-RWriteRoom

SRCS = \
	src/App.cpp \
	src/EditorView.cpp \
	src/Settings.cpp \
	src/Strings.cpp \
	src/WriteWindow.cpp

RDEFS = src/app.rdef
RSRCS =

# be: interface kit; tracker: BFilePanel.
LIBS = be tracker stdc++
LIBPATHS =
SYSTEM_INCLUDE_PATHS =
LOCAL_INCLUDE_PATHS =
OPTIMIZE := FULL
LOCALES =
DEFINES =
WARNINGS = ALL
SYMBOLS =
DEBUGGER =
COMPILER_FLAGS = -std=c++17
LINKER_FLAGS =
APP_VERSION =
DRIVER_PATH =

include /boot/system/develop/etc/makefile-engine
