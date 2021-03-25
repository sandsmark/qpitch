# - Try to find FFTW3
# Once done this will define
#
#  FFTW3_FOUND - system has FFTW3
#  FFTW3_INCLUDE_DIRS - the FFTW3 include directory
#  FFTW3_LIBRARIES - Link these to use FFTW3
#  FFTW3_DEFINITIONS - Compiler switches required for using FFTW3
#
#  Copyright (c) 2008 Nico Schlömer <nico.schloemer@gmx.net>
#                     William Spinelli <wylliam@tiscali.it>
#
# Redistribution and use is allowed according to the terms of the New BSD license.
#


if (FFTW3_LIBRARIES AND FFTW3_INCLUDE_DIRS)
	# in cache already
	set(FFTW3_FOUND TRUE)
else (FFTW3_LIBRARIES AND FFTW3_INCLUDE_DIRS)
	find_path(FFTW3_INCLUDE_DIR
		NAMES
			fftw3.h
		PATHS
			/usr/include
			/usr/local/include
			/opt/local/include
			/sw/include
	)

	find_library(FFTW3_LIBRARY
		NAMES
			fftw3
		PATHS
			/usr/lib
			/usr/local/lib
			/opt/local/lib
			/sw/lib
	)

	set(FFTW3_INCLUDE_DIRS
		${FFTW3_INCLUDE_DIR}
	)
	set(FFTW3_LIBRARIES
		${FFTW3_LIBRARY}
	)

	if (FFTW3_INCLUDE_DIRS AND FFTW3_LIBRARIES)
		set(FFTW3_FOUND TRUE)
	endif (FFTW3_INCLUDE_DIRS AND FFTW3_LIBRARIES)

	if (FFTW3_FOUND)
		if (NOT Portaudio_FIND_QUIETLY)
			message(STATUS "Found FFTW3: ${FFTW3_LIBRARIES}")
		endif (NOT Portaudio_FIND_QUIETLY)
	else (FFTW3_FOUND)
		if (Portaudio_FIND_REQUIRED)
			message(FATAL_ERROR "Could not find FFTW3")
		endif (Portaudio_FIND_REQUIRED)
	endif (FFTW3_FOUND)

	# show the FFTW3_INCLUDE_DIRS and FFTW3_LIBRARIES variables only in the advanced view
	mark_as_advanced(FFTW3_INCLUDE_DIRS FFTW3_LIBRARIES)

endif (FFTW3_LIBRARIES AND FFTW3_INCLUDE_DIRS)
