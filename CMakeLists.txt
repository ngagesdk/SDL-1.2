cmake_minimum_required(VERSION 3.00)
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/ngage-toolchain.cmake)

project(xflame C CXX)

include("${CMAKE_CURRENT_SOURCE_DIR}/../../modules/SDL.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/../../modules/SDLmain.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/../../modules/SDL_epocruntime.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/../../modules/EAudioLib.cmake")

set(MMC_DRIVE_LETTER "E")

# Use CMake or Visual Studio to enable these settings.
option(INSTALL_APP "Install app on MMC" OFF)

set(UID1 0x1000007a) # KExecutableImageUidValue, e32uid.h
set(UID2 0x100039ce) # KAppUidValue16, apadef.h
set(UID3 0x10005b95) # xflame UID

set(GCC_COMN_DEFS -D__SYMBIAN32__ -D__GCC32__ -D__EPOC32__ -D__MARM__ -D__MARM_ARMI__)
set(GCC_MODE_DEFS -DNDEBUG -D_UNICODE)
set(GCC_DEFS      ${GCC_COMN_DEFS} ${GCC_MODE_DEFS})

set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")

set(xflame_libs
    ${CMAKE_CURRENT_BINARY_DIR}/libSDL.a
    ${CMAKE_CURRENT_BINARY_DIR}/libSDLmain.a
    ${CMAKE_CURRENT_BINARY_DIR}/libEAudioLib.a
    ${CMAKE_CURRENT_BINARY_DIR}/libSDL_epocruntime.a
    ${EPOC_LIB}/egcc.lib
    ${EPOC_LIB}/euser.lib
    ${EPOC_LIB}/estlib.lib
    ${EPOC_LIB}/ws32.lib
    ${EPOC_LIB}/hal.lib
    ${EPOC_LIB}/gdi.lib
    ${EPOC_LIB}/mediaclientaudiostream.lib
    ${EPOC_LIB}/bafl.lib
    ${EPOC_LIB}/efsrv.lib
    ${EPOC_LIB}/flogger.lib
    ${EPOC_LIB}/scdv.lib)

set(xflame_sources
    "${SRC_DIR}/xflame.c")

add_library(xflame     STATIC ${xflame_sources})

build_exe(${PROJECT_NAME} exe ${UID1} ${UID2} ${UID3} "${xflame_libs}")

if(INSTALL_APP)
    install_file(${PROJECT_NAME} ${CMAKE_CURRENT_BINARY_DIR} xflame.exe      ${MMC_DRIVE_LETTER})
endif()

add_dependencies(
    xflame
    EAudioLib
    SDL_epocruntime
    SDLmain
    SDL)

add_dependencies(
    xflame.exe
    xflame)

target_compile_definitions(
    xflame
    PUBLIC
    __EXE__
    ${GCC_DEFS}
    UID1=${UID1}
    UID2=${UID2}
    UID3=${UID3})

target_include_directories(
    xflame
    PUBLIC
    ${SDL_DIR}/include)

target_link_libraries(
    xflame
    ${xflame_libs})
