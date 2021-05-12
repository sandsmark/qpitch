project(qpitch)
##
# CMakeLists.txt for QPitch - Program for tuning instruments
#
# Copyright (c) 2008-2009 Nico Schlömer <nico.schloemer@gmx.net>
#                         William Spinelli <wylliam@tiscali.it>
##

# set minimum cmake version
cmake_minimum_required( VERSION 3.0 )


# set path for additional cmake modules
set( CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake_modules )
# find the required libraries and sets the corresponding environment variables
find_package( Qt5 REQUIRED COMPONENTS Widgets)
find_package( Portaudio REQUIRED )
find_package( FFTW3 REQUIRED )

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)


# trigger the actual compilation of QPitch
add_subdirectory( src )
