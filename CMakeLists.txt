add_executable(midieditor)
add_definitions(-DMY_PATH="${CMAKE_CURRENT_LIST_DIR}")
option(SDL "use sdl as gui backend" ON)
#QMAKE_CC = tcc
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-pointer-sign -Werror-implicit-function-declaration")
include(${CMAKE_CURRENT_SOURCE_DIR}/../../library/stb/CMakeLists.txt)
#include(../../library/gui-SDL/gui.pri)
include(${CMAKE_CURRENT_SOURCE_DIR}/../../library/gui/CMakeLists.txt)
include(${CMAKE_CURRENT_SOURCE_DIR}/../../library/extmath/CMakeLists.txt)
list(APPEND SOURCES main.c
    melody.c
	midi.c
    playback.c 
    roll.c 
    save.c 
    sortediterable.c)
target_link_libraries(midieditor 
	SDL2
	SDL2_ttf
	smf
	rtmidi)


find_package(PkgConfig REQUIRED) 
pkg_check_modules(GLIB2  REQUIRED glib-2.0)

target_link_libraries(midieditor ${GLIB2_LIBRARIES})
target_include_directories(midieditor PUBLIC ${GLIB2_INCLUDE_DIRS})
target_compile_options(midieditor PUBLIC ${GLIB2_CFLAGS_OTHER})