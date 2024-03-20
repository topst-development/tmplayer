AUTOMAKE_OPTIONS = foreign subdir-objects

CC = @CC@
CPP = @CPP@
AM_CFLAGS = $(TMP_CFLAGS) -I$(top_srcdir)/include -I$(top_srcdir)/tagparser/include
AM_CPPFLAGS = $(AM_CFLAGS)
LIBS = @LIBS@ $(TMP_LIBS) -lTcDBGen -lTcConnect
DEFS = @DEFS@ $(AUDIOMANAGER)
bin_PROGRAMS = tmplayer
tmplayer_SOURCES = tagparser/src/TCTagParser.c \
					 tagparser/src/ID3Parser.c \
					 tagparser/src/nls720.c \
					 tagparser/src/nls_app.c \
					 tagparser/src/nls_cp949.c \
					 src/MetaParser.c \
					 src/DBusMsgDefNames.c \
					 src/UI_META_browsing.c \
					 src/TMPlayerControl.c \
					 src/TMPlayerDBusModeManager.c \
					 src/TMPlayerDBusMediaPlayback.c \
					 src/TMPlayerDBus.c \
					 src/TMPlayerConnect.c \
					 src/TMPlayerDB.cpp \
					 src/TMPlayerAux.c \
					 src/TMPlayer.cpp \
					 src/main.c

if IAP2

AM_CFLAGS += -I$(top_srcdir)/iap2/include -DHAVE_IAP2
AM_CPPFLAGS += -I$(top_srcdir)/iap2/include -DHAVE_IAP2
LIBS += -lstdc++ -lpthread -liap2util
tmplayer_SOURCES += src/TMPlayeriAP2.c \
					 src/TMPlayerDBusiAP2Manager.c \
					 iap2/src/TCiAP2Manager.cpp \
					 iap2/src/TCiAP2FeatureHid.cpp \
					 iap2/src/iAP2BrwsLibrary.cpp \
					 iap2/src/Iap2BrwsCategory.cpp \
					 iap2/src/Iap2BrwsItem.cpp \
					 iap2/src/Iap2BrwsManager.cpp

endif

if A2DP

AM_CFLAGS += -DHAVE_BT
AM_CPPFLAGS += -I$(top_srcdir)/iap2/include -DHAVE_BT
tmplayer_SOURCES += src/TMPlayerBT.c \
					 src/TMPlayerDBusBTManager.c

endif
