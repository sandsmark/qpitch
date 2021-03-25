##
# CMakeLists.txt for QPitch - Program for tuning instruments
#
# Copyright (c) 2008-2009 Nico Schlömer <nico.schloemer@gmx.net>
#                         William Spinelli <wylliam@tiscali.it>
##

# set project's name
project( QPITCH )


# list of all source files
set( QPITCH_SRCS
	main.cpp
	qaboutdlg.cpp
	qlogview.cpp
	qosziview.cpp
	qpitch.cpp
	qpitchcore.cpp
	qsettingsdlg.cpp
)


# list of all header files that should be treated with moc
set( QPITCH_MOC_HDRS
	qaboutdlg.h
	qlogview.h
	qosziview.h
	qpitchcore.h
	qpitch.h
	qsettingsdlg.h
)


# list of .ui files
set( QPITCH_UIS
	ui/qaboutdlg.ui
	ui/qpitch.ui
	ui/qsettingsdlg.ui
)


# list of resource file
set( QPITCH_RCS
	ui/qpitch.qrc
)


# set compiler options
add_definitions( -O2 -w -DQT_NO_DEBUG )


# enable additional modules (by default only QtCore and QtGui are enabled)
# set( QT_USE_QTXML TRUE )


# find the required libraries and sets the corresponding environment variables
find_package( Qt4 REQUIRED )
find_package( Portaudio REQUIRED )
find_package( FFTW3 REQUIRED )


# QT_USE_FILE is a variable defined by find_package( Qt4 ) that contains a
# path to CMake
include( ${QT_USE_FILE} )


# run rcc on all resources files
qt4_add_resources( QPITCH_RC_SRCS ${QPITCH_RCS} )


# run uic on all forms files
qt4_wrap_ui( QPITCH_UI_HDRS ${QPITCH_UIS} )


# run moc on all intermediate files
qt4_wrap_cpp( QPITCH_MOC_SRCS ${QPITCH_MOC_HDRS} )


# include headers produced by uic in our code
include_directories(
	${QPITCH_BINARY_DIR}
	${QPITCH_SOURCE_DIR}
)


# place the binaries to the root of the build directory
set( EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR} )


# set object files dependencies for the executable
add_executable( qpitch
	${QPITCH_SRCS}
	${QPITCH_MOC_SRCS}
	${QPITCH_RC_SRCS}
	${QPITCH_UI_HDRS}
)


# add library dependencies needed by the executable (variables are filled
# by FIND_PACKAGE)
target_link_libraries( qpitch
	${QT_LIBRARIES}
	${PORTAUDIO_LIBRARIES}
	${FFTW3_LIBRARIES}
)


# where to install files
install(
	TARGETS qpitch
	RUNTIME DESTINATION /usr/bin
)

install(
	FILES ${CMAKE_CURRENT_SOURCE_DIR}/ui/icons/qpitch.xpm
	DESTINATION /usr/share/pixmaps/
)

install(
	FILES ${CMAKE_CURRENT_SOURCE_DIR}/data/qpitch.desktop
	DESTINATION /usr/share/applications/
)
