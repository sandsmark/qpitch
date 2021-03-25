##
# CMakeLists.txt for QPitch - Program for tuning instruments
#
# Copyright (c) 2008-2009 Nico Schlömer <nico.schloemer@gmx.net>
#                         William Spinelli <wylliam@tiscali.it>
##

# set minimum cmake version
cmake_minimum_required( VERSION 2.4.5 )


# set path for additional cmake modules
set( CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake_modules )


# trigger the actual compilation of QPitch
add_subdirectory( src )
