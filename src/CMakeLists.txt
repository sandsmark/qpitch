##
# CMakeLists.txt for QPitch - Program for tuning instruments
#
# Copyright (c) 2008-2009 Nico Schlömer <nico.schloemer@gmx.net>
#                         William Spinelli <wylliam@tiscali.it>
##

# place the binaries to the root of the build directory
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})

set(CMAKE_AUTOUIC_SEARCH_PATHS ui)

# set object files dependencies for the executable
add_executable( qpitch
	main.cpp
	qaboutdlg.cpp
	qlogview.cpp
	qosziview.cpp
	qpitch.cpp
	qpitchcore.cpp
	qsettingsdlg.cpp

	qaboutdlg.h
	qlogview.h
	qosziview.h
	qpitchcore.h
	qpitch.h
	qsettingsdlg.h

	ui/qpitch.qrc

	ui/qaboutdlg.ui
	ui/qpitch.ui
	ui/qsettingsdlg.ui
)

set(CMAKE_INCLUDE_CURRENT_DIR ON)


# add library dependencies needed by the executable (variables are filled
# by FIND_PACKAGE)
target_link_libraries( qpitch
    Qt5::Widgets
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
