if !DISABLE_NET
  lib_LTLIBRARIES = libGiGi.la libGiGiNet.la libGiGiSDL.la
else DISABLE_NET
  lib_LTLIBRARIES = libGiGi.la libGiGiSDL.la
endif

libGiGi_la_SOURCES = 		                \
GGApp.cpp					\
GGButton.cpp					\
GGClr.cpp					\
GGControl.cpp					\
GGDrawUtil.cpp					\
GGDropDownList.cpp				\
GGDynamicGraphic.cpp				\
GGEdit.cpp					\
GGEventPump.cpp					\
GGFont.cpp					\
GGLayout.cpp					\
GGListBox.cpp					\
GGMenu.cpp					\
GGMultiEdit.cpp					\
GGPluginInterface.cpp				\
GGPtRect.cpp					\
GGScroll.cpp					\
GGSlider.cpp					\
GGStaticGraphic.cpp				\
GGTextControl.cpp				\
GGTexture.cpp					\
GGWnd.cpp					\
GGZList.cpp					\
XMLDoc.cpp					\
dialogs/GGFileDlg.cpp				\
dialogs/GGThreeButtonDlg.cpp

libGiGiNet_la_SOURCES =				\
net/fastevents.c				\
net/net2.c					\
net/trace.c
if USE_BUILTIN_SDLNET
  libGiGiNet_la_SOURCES += net/SDLnet.c net/SDLnetselect.c net/SDLnetTCP.c net/SDLnetUDP.c
endif

libGiGiSDL_la_SOURCES = 			\
SDL/SDLGGApp.cpp

libGiGi_la_LDFLAGS = -no-undefined -version-info @GGAPI_CURRENT@:@GGAPI_REVISION@:@GGAPI_AGE@
libGiGi_la_LIBADD = @FT2_LIBS@ @GL_LIBS@ @BOOST_LIBS@ -llog4cpp @DEVIL_LIBS@ @LTLIBOBJS@ @LIBLTDL@

libGiGiNet_la_LDFLAGS = -no-undefined -version-info @GGAPI_CURRENT@:@GGAPI_REVISION@:@GGAPI_AGE@
libGiGiNet_la_LIBADD  = @SDL_LIBS@ @SDLNET_LIB@

if DISABLE_NET
  NET_CFLAGS =
else !DISABLE_NET
  NET_CFLAGS = -I$(top_srcdir)/include/net
endif

libGiGiSDL_la_LDFLAGS = -no-undefined -version-info @GGAPI_CURRENT@:@GGAPI_REVISION@:@GGAPI_AGE@
libGiGiSDL_la_LIBADD  =@SDL_LIBS@ libGiGi.la
if !DISABLE_NET
  libGiGiSDL_la_LIBADD += libGiGiNet.la
endif

AM_CPPFLAGS = -I$(top_srcdir)/include -I$(top_srcdir)/include/dialogs $(NET_CFLAGS) @SDL_CFLAGS@ @FT2_CFLAGS@ @INCLTDL@ @BOOST_CPPFLAGS@

if DEBUG
  AM_CXXFLAGS = -g3 -Wall
else !DEBUG
  AM_CXXFLAGS = -O2 -DNDEBUG
endif

TESTS = testy
check_PROGRAMS= testy
testy_SOURCES = $(top_srcdir)/test.cpp 
testy_LDADD = libGiGiSDL.la
testy_CPPFLAGS = -I$(top_srcdir)/include -I$(top_srcdir)/include/dialogs -I$(top_srcdir)/include/net -I$(top_srcdir)/include/SDL @SDL_CFLAGS@ @FT2_CFLAGS@ @BOOST_CPPFLAGS@

DISTCLEANFILES = GG.log

