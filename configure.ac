dnl Process this file with -*-autoconf-*- to produce a configure script.

AC_PREREQ(2.57)
AC_INIT(libGiGi, 0.5.0.0)

dnl YG_REMOVE(A,B) is equivalent to ${A/$B/} on bash
AC_DEFUN([YG_REMOVE],[`echo "$$1"|sed -e \"s@$$2@@\"`])

dnl From the libtool-infopage:
dnl   Update the version information only immediately before a public
dnl   release of your software.  More frequent updates are unnecessary,
dnl   and only guarantee that the current interface number gets larger
dnl   faster.
dnl  - If the library source code has changed at all since the last
dnl    update, then increment REVISION (`C:R:A' becomes `C:r+1:A').
dnl  - If any interfaces have been added, removed, or changed since the
dnl    last update, increment CURRENT, and set REVISION to 0.
dnl  - If any interfaces have been added since the last public release,
dnl    then increment AGE.
dnl  - If any interfaces have been removed since the last public release,
dnl    then set AGE to 0.

GGAPI_CURRENT=0
GGAPI_REVISION=0
GGAPI_AGE=0
AC_SUBST(GGAPI_CURRENT)
AC_SUBST(GGAPI_REVISION)
AC_SUBST(GGAPI_AGE)

dnl for cross-compilation
dnl BUILD is the system that is compiling
dnl HOST  is the system that the programs will run under
dnl TARGET is needed only if compiling a compiler, but AM_PATH_SDL needs it 
AC_CANONICAL_BUILD
AC_CANONICAL_HOST
AC_CANONICAL_TARGET

case "$host" in 
     *-*-cygwin* | *-*-mingw*)
	WIN32=yes
	AC_MSG_NOTICE([building for Windows])
	;;
     *)
	WIN32=no
	AC_MSG_NOTICE([building for Unix])
	;;
esac
AM_CONDITIONAL([WIN32],[test x$WIN32 = xyes])

AC_CONFIG_SRCDIR([src/GGApp.cpp])
AC_CONFIG_HEADERS([config.h])

dnl use 'foreign dist-zip' to create a Zip-packed distribution in addition to .tar.gz
dnl dist-bz2 would add .tar.bz2
dnl This is no GNU lib, we do not have the README, INSTALL etc. files (and don't want them right now)
dnl so we use foreign
AM_INIT_AUTOMAKE([foreign])

# Checks for programs.
AC_LIBTOOL_WIN32_DLL
AC_PROG_CXX
AC_PROG_CC
AC_PROG_CPP
AC_PROG_LIBTOOL

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS([arpa/inet.h fcntl.h netdb.h netinet/in.h stdlib.h string.h sys/socket.h sys/time.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_SIZE_T
AC_HEADER_TIME

# Checks for library functions.
AC_FUNC_ERROR_AT_LINE
AC_FUNC_MALLOC
AC_FUNC_SELECT_ARGTYPES
AC_TYPE_SIGNAL
AC_CHECK_FUNCS([floor gethostbyaddr gethostbyname memmove memset])
AC_SEARCH_LIBS(sqrt,[m],[],[AC_MSG_FAILURE([The sqrt-function was not found. This is bad!])])

# Checks for libraries.
AC_CACHE_SAVE

### SDL check

SDL_CONFIG=no
if test x$cross_compiling = xno; then
  AC_PATH_PROG(SDL_CONFIG, sdl-config, no, [$PATH])
fi
if test "$SDL_CONFIG" = "no"; then
   if test $WIN32 = yes; then
    # Windows target
    AC_MSG_NOTICE([Target is WIN32, won't rely on sdl-config])
    YG_GET_PATH(sdl)
    SDL_CFLAGS=$SDL_CPPFLAGS
    AC_SUBST(SDL_CFLAGS)
    SDL_LIBS="-lSDL -lSDLmain"
    AC_SUBST(SDL_LIBS)
    OLD_LIBS=$LIBS
    LIBS="$OLD_LIBS $SDL_LIBS"
    AC_LANG_C
    AC_CHECK_HEADERS([SDL.h])
dnl    AC_TRY_LINK([#include "SDL.h"],[SDL_Init(SDL_INIT_VIDEO);],[],[AC_MSG_ERROR([dnl
    AC_CHECK_LIB([SDL],[SDL_Init],[],[AC_MSG_FAILURE([dnl
SDL was not found, did you specify the correct paths
(--with-sdl-include,--with-sdl-libdir)? See ./configure --help])])
    AC_MSG_CHECKING([if SDL version is at least 1.2.5])
    AC_EGREP_CPP([choke me, incorrect version!],[
#include "SDL_version.h"
#if SDL_VERSION_ATLEAST(1,2,5)
  Okay Dokey!
#else
  choke me, incorrect version!
#endif],[AC_MSG_RESULT(no)
AC_MSG_ERROR([Your SDL version is too old, you need at least 1.2.5])],[AC_MSG_RESULT(yes)])
    LIBS="$OLD_LIBS"
  else
     # Unix target, we require sdl-config here (at least for now, to eliminate strange linking errors)
     AM_PATH_SDL([1.2.5],[],[AC_MSG_FAILURE([SDL is required])])
  fi
else
  AM_PATH_SDL([1.2.5],[],[AC_MSG_FAILURE([SDL is required])])
fi

AC_CACHE_SAVE

### Freetype check

dnl which freetype2 version do we need?
FT_CONFIG=no
if test x$cross_compiling = xno; then
  AC_PATH_PROG(FT_CONFIG, freetype-config, no, [$PATH])
fi
if test "$FT_CONFIG" = "no"; then
  if test $WIN32 = yes; then
    # Windows target
    AC_MSG_NOTICE([Target is WIN32, won't rely on freetype-config])
    YG_GET_PATH(ft)
    FT2_CFLAGS=$FT_CPPFLAGS
    AC_SUBST(FT2_CFLAGS)
    FT2_LIBS="-lfreetype $FT2_LDFLAGS"
    AC_SUBST(FT2_LIBS)
    OLD_LIBS=$LIBS
    LIBS="$OLD_LIBS $FT2_LIBS"
    AC_LANG_C
    AC_CHECK_HEADERS([ft2build.h])
    AC_MSG_CHECKING([whether I can link freetype])
    AC_TRY_LINK([
#include "ft2build.h"
#include FT_FREETYPE_H],[
  FT_Library library;
  FT_Init_FreeType(&library);],[AC_MSG_RESULT(yes)],[AC_MSG_RESULT(no)
AC_MSG_ERROR([Freetype was not found, did you specify the correct paths
(--with-ft-include,--with-ft-libdir)? See ./configure --help])])
    LIBS="$OLD_LIBS"
  else
     # Unix target, we require freetype-config here (at least for now, to eliminate strange linking errors)
     AC_CHECK_FT2(9.0.0,[],[AC_MSG_ERROR([FreeType is required])])
  fi
else
  AC_CHECK_FT2(9.0.0,[],[AC_MSG_ERROR([FreeType is required])])
fi

### SDL_image check

#dnl can't use sdl-image, ${sdl-image_libdir} would be misinterpreted
#YG_GET_PATH(sdl_image)
#AC_CACHE_SAVE
#XXCPPFLAGS="$CPPFLAGS"
#CPPFLAGS="$CPPFLAGS $SDL_CFLAGS"
#AC_CHECK_HEADERS([SDL_image.h],[],[AC_MSG_ERROR([SDL_image.h not found])])
#AC_CHECK_LIB(SDL_image,IMG_Load,[],[AC_MSG_ERROR([SDL_image is needed])])
#CPPFLAGS="$XXCPPFLAGS"

### SDL_net check

YG_GET_PATH(sdlnet)
AC_ARG_WITH([builtin-sdlnet],[AC_HELP_STRING([--with-builtin-sdlnet],[forces/forbids use of built-in version of SDL_net, default is force])],[],[with_builtin_sdlnet=yes])

if test x$with_builtin_sdlnet = xyes; then
  AC_MSG_NOTICE(using built-in SDL_net)
else
  AC_CHECK_LIB(SDL_net,SDLNet_TCP_Open,[LIBS="$LIBS -lSDL_net" with_builtin_sdlnet=no], 
              [if test x$with_builtin_sdlnet = xno; then
                 AC_MSG_ERROR([SDL_net not found, and forced to not use built-in SDL_net])
		 SDLNET_LIB="-lSDL_net"
               else
                 AC_MSG_WARN([SDL_net not found, using built-in version])
                 with_builtin_sdlnet=yes
                 SDLNET_LIB=""
               fi])
fi
AC_SUBST(SDLNET_LIB)

AM_CONDITIONAL([USE_BUILTIN_SDLNET],[test x$with_builtin_sdlnet = xyes])

### OpenGL check

AC_CHECK_HEADERS([GL/gl.h GL/glu.h],,[AC_MSG_ERROR([I can't find the OpenGL headers.
Make sure you have the OpenGL-developer-libraries installed and have given
the correct include path!])])

SAVE_LIBS="$LIBS"
YG_SEARCH_LIBS([OpenGl],[GL MesaGL opengl32],
               [#include <GL/gl.h>],[glEnd();],
               [],[AC_MSG_FAILURE([OpenGL headers were found, but unable to link a simple GL program!])])
YG_SEARCH_LIBS([OpenGL's GLU],[GLU MesaGLU glu32],
               [#include <GL/glu.h>],[gluPerspective(0.,0.,0.,0.);],
               [],[AC_MSG_FAILURE([OpenGL's GLU library was not found!])])

GL_LIBS="YG_REMOVE(LIBS,SAVE_LIBS)"
AC_SUBST(GL_LIBS)
LIBS="$SAVE_LIBS"
FAILED=no

AC_CACHE_SAVE

### Boost check

SAVE_LIBS="$LIBS"
YG_GET_PATH(boost)
YG_CHECK_BOOST([1.30.2],,[FAILED=yes])
#AC_CHECK_LIB(boost_regex,main,[],[FAILED=yes])
AC_CHECK_LIB(boost_filesystem,main,[],[FAILED=yes])
AC_CHECK_LIB(boost_signals,main,[],[FAILED=yes])

BOOST_LIBS="YG_REMOVE(LIBS,SAVE_LIBS)"
AC_SUBST(BOOST_LIBS)
LIBS="$SAVE_LIBS"

if test $FAILED = yes; then 
  AC_MSG_ERROR([boost libs not found.
** You need the Boost C++ libraries, version 1.30.2 or better,
** as well as the following boost-modules:
** - boost.filesystem
** - boost.signals
** download them from http://www.boost.org])
fi

AC_CACHE_SAVE

### DevIL check
YG_GET_PATH(devil)
SAVE_LIBS="$LIBS"
SAVE_CPPFLAGS="$CPPFLAGS"
# do we need to check for libjpeg, libpng etc?
LIBS="$GL_LIBS $SDL_LIBS -lIL -lILU"
CPPFLAGS="$CPPFLAGS $SDL_CFLAGS"
AC_CHECK_HEADERS([IL/il.h IL/ilu.h IL/ilut.h],,[AC_MSG_ERROR([The DevIL headers were not found])])
YG_SEARCH_LIBS([DevIL],[ILUT],[#include <IL/ilut.h>],[ilutInit();],[],
               [AC_MSG_FAILURE([Unable to link DevIL test program, do you have DevIL installed?])])
DEVIL_LIBS="YG_REMOVE(LIBS,SAVE_LIBS)"
AC_SUBST(DEVIL_LIBS)
CPPFLAGS="$SAVE_CPPFLAGS"
LIBS="$SAVE_LIBS"

### expat and log4cpp check
#YG_GET_PATH(expat)
#SAVE_LIBS="$LIBS"
#AC_CHECK_LIB(expat,XML_ParserCreate,[],[AC_MSG_FAILURE([You'll need expat])])
#LIBS="$SAVE_LIBS"

YG_GET_PATH(log4cpp)
AC_LANG_CPLUSPLUS
AC_CHECK_HEADERS([log4cpp/Category.hh],,[AC_MSG_FAILURE(log4cpp/Category.hh not found in include-path)])
SAVE_LIBS="$LIBS"
YG_SEARCH_LIBS([log4cpp],[log4cpp],
               [#include <log4cpp/Category.hh>],[log4cpp::Category::shutdown();],
               [],[AC_MSG_FAILURE(can't link log4cpp in)])
LIBS="$SAVE_LIBS"
dnl AC_CHECK_LIB(z,deflate,[],[AC_MSG_ERROR(Zlib is missing!)])

AC_LANG_C
if test x$with_builtin_sdlnet = xyes -a $WIN32 = yes; then
  # the windows version of sdlnet needs wsock32 linked in
  AC_CHECK_LIB(wsock32,main,[],[AC_MSG_FAILURE([wsock32 not found!])])
  LIBS="$LIBS -lwsock32"
fi
dnl without net switch
AC_ARG_ENABLE([net], [AC_HELP_STRING([--disable-net],[disables net support (default on)])]dnl
                   , [disable_net=yes], [])  
AM_CONDITIONAL(DISABLE_NET,[test x$disable_net = xyes])
if test x$disable_net != xyes; then
  AC_DEFINE(GG_USE_NET,1,[Define to true if you want net support])
  GG_USE_NET=1
  GIGINET_DEP=", GiGiNet" # for GiGiSDL.pc
else
  GG_USE_NET=0
  GIGINET_DEP=""
fi
AC_SUBST(GG_USE_NET)
AC_SUBST(GIGINET_DEP)

dnl debug-mode switch
AC_ARG_ENABLE([debug], [AC_HELP_STRING([--disable-debug],[compile without debugging symbols])] dnl
		     , [], [enable_debug=yes])
AM_CONDITIONAL(DEBUG,[test x$enable_debug = xyes])

AC_CONFIG_FILES([Makefile 
                 src/Makefile 
                 include/Makefile
                 include/GGConfig.h
                 GiGi.pc
                 GiGiNet.pc
                 GiGiSDL.pc
                ])
AC_OUTPUT
