# requirement :  'https://github.com/kooBH/QtBin' as submodule 'bin'

# CMAKE version of Ubuntu 16.04 is 3.5.
cmake_minimum_required(VERSION 3.5)
project(IITP_2021)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(LINKLIBS)
set(COMPILE_OPTION)
set(SRC)
set(SRC_Qt_MOC)
set(SRC_Qt)
set(INCL)

# lib

include(lib/RtAudioWrapper/RtAudio.cmake)
include(lib/RtAudioWrapper/Libsamplerate.cmake)
include(lib/KJsonConfig/KJsonConfig.cmake)
include(lib/KWidgetASR/KWidgetASR.cmake)
include(lib/KSpectrogram/KSpectrogram.cmake)
include(lib/WAV/WAV.cmake)
include(lib/STFT/cpp/STFT_cpp.cmake)

# algorithm
include(algorithm/multi_source_CDR_MLDR/C++/CDR_MLDR.cmake)
include(algorithm/multi_source_CDR_IVA_MLDR/C++/CDR_IVA_MLDR.cmake)

macro(display_elements
	DISPLAY_MESSAGE
	LIST_TO_DISPLAY)
    message("[ ${DISPLAY_MESSAGE} ]")
    foreach(_DISPLAY_ELEMENT ${LIST_TO_DISPLAY})
		message(STATUS ${_DISPLAY_ELEMENT})
    endforeach()
endmacro()

file(MAKE_DIRECTORY ${PROJECT_SOURCE_DIR}/tmp)

## MOC(Meta Object Compiler) of QtFramework
display_elements(MOC_TARGET ${SRC_Qt_MOC})
	## sources to be processed by MOC
	list(APPEND SRC_Qt_MOC
		${PROJECT_SOURCE_DIR}/src/app.h
		${PROJECT_SOURCE_DIR}/src/processor.h
		${PROJECT_SOURCE_DIR}/src/WidgetConfig.h
	)	
	
	## sources without MetaObject 
	list(APPEND SRC_Qt
		${PROJECT_SOURCE_DIR}/src/app.cpp
		${PROJECT_SOURCE_DIR}/src/processor.cpp
		${PROJECT_SOURCE_DIR}/src/WidgetConfig.cpp
		)
	
	foreach(i  ${SRC_Qt_MOC})
		string(REPLACE "/" ";" parse_i_1 ${i})
		LIST(GET parse_i_1 -1 parse_i_2)
		string(REPLACE "." ";" parse_i_3 ${parse_i_2})
		LIST(GET parse_i_3 0 fisrt_element)
		
		if(MSVC)
			execute_process(
			COMMAND moc ${i} -o ${PROJECT_SOURCE_DIR}/tmp/moc_${fisrt_element}.cpp
			WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/bin  
			 )
		endif()
		
		list(APPEND SRC_Qt
			tmp/moc_${fisrt_element}.cpp
		)
	endforeach()

################

if(NOT MSVC)
	list(APPEND LINKLIBS
		m
		-fopenmp
	)
	list(APPEND COMPILE_OPTION
		-fopenmp
		-O3
	)
else()
	list(APPEND COMPILE_OPTION
		/openmp
		# /MP - compiled in mult-process : faster build time
		/MP
	)
endif()

list(APPEND SRC
	src/main.cpp
)

list(APPEND SRC ${SRC_Qt} ${SRC_Qt_MOC})
	
list(APPEND INCL
	src
	tmp
	# QtFramework
	bin/include
	bin/include/QtCore
	bin/include/QtCore5Compat
	bin/include/QtGui
	bin/include/QtWidgets
	bin/include/QtOpenGL
	bin/include/QtOpenGLWidgets
)

## QtFramework ##
if(UNIX AND NOT APPLE)
  list(APPEND LINKLIBS
	)
  list(APPEND COMPILE_OPTION
      -fPIC
    )
elseif(MSVC)
	list(APPEND LINKLIBS
		debug ${PROJECT_SOURCE_DIR}/bin/lib/Qt6Cored.lib optimized ${PROJECT_SOURCE_DIR}/bin/lib/Qt6Core.lib
		debug ${PROJECT_SOURCE_DIR}/bin/lib/Qt6Guid.lib optimized ${PROJECT_SOURCE_DIR}/bin/lib/Qt6Gui.lib
		debug ${PROJECT_SOURCE_DIR}/bin/lib/Qt6Widgetsd.lib optimized ${PROJECT_SOURCE_DIR}/bin/lib/Qt6Widgets.lib	
		debug ${PROJECT_SOURCE_DIR}/bin/lib/Qt6OpenGLWidgetsd.lib optimized ${PROJECT_SOURCE_DIR}/bin/lib/Qt6OpenGLWidgets.lib	
	)
endif()

display_elements(INCL ${INCL})
message(${INCL})

#display_elements(SRC "${SRC}")
#display_elements(COMPILE_OPTION ${COMPILE_OPTION})

add_executable(${PROJECT_NAME} ${SRC})
target_include_directories(${PROJECT_NAME}	PRIVATE	${INCL}  )
target_link_libraries(${PROJECT_NAME} ${LINKLIBS})
target_compile_options(${PROJECT_NAME} PRIVATE ${COMPILE_OPTION} )