TARGET = freeboxremote
OBJS = template.o		

#To build for custom firmware:
BUILD_PRX = 1
PSP_FW_VERSION=371

INCDIR = 
CFLAGS = -O2 -g -G0 -Wall
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LIBDIR =
LDFLAGS =
STDLIBS= -losl -lpspgu -lpspgum -lpng -lSDLmain -lSDL_mixer -lcurl -lz -lvorbisidec -lvorbis -logg -lSDL -lGL -lpspvfpu -ljpeg -lm \
		-lpspsdk -lpspctrl -lmad -lpspumd -lpsprtc -lpsppower -lpspaudiolib -lpspaudio -lpspmpeg -lpspaudiocodec -lpsphprm -lpsphttp -lpspssl -lpspwlan -lm \
		-lpspnet_adhocmatching -lpspnet_adhoc -lpspnet_adhocctl
LIBS=$(STDLIBS)$(YOURLIBS)


EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = Freebox Remote
PSP_EBOOT_ICON = ICON0.png
PSP_EBOOT_PIC1 = PIC1.png
PSP_EBOOT_SND0 = 

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak
