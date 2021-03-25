# .pro configuration file for the qpitch application
TEMPLATE		=	app


## CONFIGURATIONS ##
win32:CONFIG	+=	qt thread windows resources
unix:CONFIG		+=	qt thread x11

# create a debug version
CONFIG			+=	debug warn_on

# create a release version
#CONFIG			+=	release warn_off


## FILES AND DIRECTORIES ##
HEADERS			+=	\
					qaboutdlg.h \
					qlogview.h \
					qosziview.h \
					qpitch.h \
					qpitchcore.h \
					qsettingsdlg.h

SOURCES			+=	\
					main.cpp \
					qaboutdlg.cpp \
					qlogview.cpp \
					qosziview.cpp \
					qpitch.cpp \
					qpitchcore.cpp \
					qsettingsdlg.cpp

FORMS			+=	\
					ui/qaboutdlg.ui \
					ui/qsettingsdlg.ui \
					ui/qpitch.ui


RESOURCES		=	ui/qpitch.qrc

win32:RC_FILE	=	ui/qpitch.rc

mac:ICON		=	ui/icons/qpitch.icns


## LIBRARIES ##
unix:LIBS		+=	-lportaudio -lfftw3

mac:INCLUDEPATH	+=	/Users/willy/Sviluppo/PortAudio/portaudio/include \
					/sw/include/
					
mac:LIBS		+=	-L/Users/willy/Sviluppo/PortAudio/portaudio/lib \
					-L/sw/lib

mac:LIBS		+=	-framework CoreAudio -framework AudioUnit -framework AudioToolbox

win32:INCLUDEPATH	+=	C:\Development\fftw-3.1.2-dll \
					C:\Development\portaudio\include
					
win32:LIBS		+=	C:\Development\portaudio\build\msvc\Win32\Release\portaudio_x86.lib \
					C:\Development\fftw-3.1.2-dll\libfftw3-3.lib


## TEMPORARY DIRECTORIES ##
UI_DIR			=	ui

MOC_DIR			=	tmp
OBJECTS_DIR		=	tmp
RCC_DIR			=	tmp
UI_DIR			=	tmp


## TARGET NAME ##
TARGET			=	qpitch
