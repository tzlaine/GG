EXTRA_DIST = m4 lgpl.txt
ACLOCAL_AMFLAGS = -I m4
SUBDIRS = include src

pkgconfigdir = $(libdir)/pkgconfig

pkgconfig_DATA = GiGi.pc GiGiSDL.pc
if !DISABLE_NET
  pkgconfig_DATA += GiGiNet.pc
endif

#already handled by automake
#GiGi.pc : GiGi.pc.in config.status
#	./config.status GiGi.pc
