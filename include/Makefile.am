libGiGiincludedir = $(includedir)/GG
nobase_libGiGiinclude_HEADERS = 		\
GGApp.h						\
GGBase.h					\
GGButton.h					\
GGConfig.h					\
GGClr.h						\
GGControl.h					\
GGDrawUtil.h					\
GGDropDownList.h				\
GGDynamicGraphic.h				\
GGEdit.h					\
GGFont.h					\
GGListBox.h					\
GGMenu.h					\
GGMultiEdit.h					\
GGPtRect.h					\
GGScroll.h					\
GGSignalsAndSlots.h				\
GGSlider.h					\
GGSpin.h					\
GGStaticGraphic.h				\
GGTextControl.h					\
GGTexture.h					\
GGWnd.h						\
GGZList.h					\
XMLDoc.h					\
XMLObjectFactory.h				\
dialogs/GGFileDlg.h				\
dialogs/GGThreeButtonDlg.h			\
SDL/SDLGGApp.h					\
net/fastevents.h				\
net/net2.h					\
net/queue.h					\
net/trace.h

#there should be no nobase_ here, since SDL_net is normally installed into the SDL-include directory and therefor
#it is included as <SDL_net.h> instead of <net/SDL_net.h>
if USE_BUILTIN_SDLNET
  libGiGiinclude_HEADERS = net/SDL_net.h net/SDLnetsys.h
else 
  # we need to pack it into the distribution
  EXTRA_DIST = net/SDL_net.h net/SDLnetsys.h
endif
