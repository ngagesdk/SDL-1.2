cmake_minimum_required(VERSION 3.00)
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/../../cmake/ngage-toolchain.cmake)

project(xflame C CXX)

set(MMC_DRIVE_LETTER "E")

# Use CMake or Visual Studio to enable these settings.
option(INSTALL_APP "Install app on MMC" OFF)

set(UID1_GW 0x1000007a) # KExecutableImageUidValue, e32uid.h
set(UID1_ER 0x1000008d) # KSharedLibraryUidValue, e32uid.h
set(UID2    0x100039ce) # KAppUidValue16, apadef.h
set(UID3_GW 0x10005b95) # xflame UID
set(UID3_ER 0x101f3d6b) # EpocRuntime UID

set(GCC_COMN_DEFS     -D__SYMBIAN32__ -D__GCC32__ -D__EPOC32__ -D__MARM__ -D__MARM_ARMI__)
set(GCC_MODE_DEFS     -DNDEBUG -D_UNICODE)
set(GCC_DEFS          ${GCC_COMN_DEFS} ${GCC_MODE_DEFS})

set(SRC_DIR       "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(EAUDIOLIB_DIR "${SRC_DIR}/EAudioLib")
set(SDL_DIR       "${SRC_DIR}/SDL")

set(xflame_libs
    ${CMAKE_CURRENT_BINARY_DIR}/libSDL.a
    ${CMAKE_CURRENT_BINARY_DIR}/libSDLmain.a
    ${CMAKE_CURRENT_BINARY_DIR}/libEAudioLib.a
    ${CMAKE_CURRENT_BINARY_DIR}/libEpocRuntime.a
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

set(EAudioLib_sources
    "${EAUDIOLIB_DIR}/src/audiolib.cpp"
    "${EAUDIOLIB_DIR}/src/audioplayer.cpp"
    "${EAUDIOLIB_DIR}/src/tinysdl.cpp"
    "${EAUDIOLIB_DIR}/src/audioconvert.cpp"
    # SDL
    "${EAUDIOLIB_DIR}/Scumm/backends/sdl/sdl.cpp"
    "${EAUDIOLIB_DIR}/Scumm/backends/sdl/sdl-common.cpp"
    "${EAUDIOLIB_DIR}/Scumm/backends/sdl/sdl-symbian.cpp"
    # AUDIO
    "${EAUDIOLIB_DIR}/Scumm/sound/audiostream.cpp"
    "${EAUDIOLIB_DIR}/Scumm/sound/fmopl.cpp"
    "${EAUDIOLIB_DIR}/Scumm/sound/mididrv.cpp"
    "${EAUDIOLIB_DIR}/Scumm/sound/midiparser.cpp"
    "${EAUDIOLIB_DIR}/Scumm/sound/midiparser_smf.cpp"
    "${EAUDIOLIB_DIR}/Scumm/sound/midiparser_xmidi.cpp"
    "${EAUDIOLIB_DIR}/Scumm/sound/mixer.cpp"
    #"${EAUDIOLIB_DIR}/Scumm/sound/mpu401.cpp"
    "${EAUDIOLIB_DIR}/Scumm/sound/rate.cpp"
    #"${EAUDIOLIB_DIR}/Scumm/sound/resample.cpp"
    #"${EAUDIOLIB_DIR}/Scumm/sound/voc.cpp"
    "${EAUDIOLIB_DIR}/Scumm/sound/midiparser_ro.cpp"
    "${EAUDIOLIB_DIR}/Scumm/sound/midiparser_eup.cpp"
    # ADLIB
    "${EAUDIOLIB_DIR}/Scumm/backends/midi/adlib.cpp"
    # UTILS
    "${EAUDIOLIB_DIR}/Scumm/common/util.cpp"
    "${EAUDIOLIB_DIR}/Scumm/common/timer.cpp"
    "${EAUDIOLIB_DIR}/Scumm/common/str.cpp"
    "${EAUDIOLIB_DIR}/Scumm/common/file.cpp"
    "${EAUDIOLIB_DIR}/Scumm/common/system.cpp")

set(EpocRuntime_libs
    ${EPOC_LIB}/euser.lib
    ${EPOC_LIB}/hal.lib)

set(EpocRuntime_sources
    "${SRC_DIR}/SDL_epocruntime/src/SDL_epocruntime.cpp")

set(SDLmain_sources
    "${SDL_DIR}/src/main/epoc/SDL_main.cpp")

set(SDL_sources
    # General sources
    "${SDL_DIR}/src/SDL.c"
    "${SDL_DIR}/src/SDL_error.c"
    "${SDL_DIR}/src/SDL_fatal.c"
    # Video sources
    "${SDL_DIR}/src/video/SDL_blit.c"
    "${SDL_DIR}/src/video/SDL_blit_0.c"
    "${SDL_DIR}/src/video/SDL_blit_1.c"
    "${SDL_DIR}/src/video/SDL_blit_A.c"
    "${SDL_DIR}/src/video/SDL_blit_N.c"
    "${SDL_DIR}/src/video/SDL_bmp.c"
    "${SDL_DIR}/src/video/SDL_cursor.c"
    "${SDL_DIR}/src/video/SDL_gamma.c"
    "${SDL_DIR}/src/video/SDL_pixels.c"
    "${SDL_DIR}/src/video/SDL_RLEaccel.c"
    "${SDL_DIR}/src/video/SDL_stretch.c"
    "${SDL_DIR}/src/video/SDL_surface.c"
    "${SDL_DIR}/src/video/SDL_video.c"
    "${SDL_DIR}/src/video/SDL_yuv.c"
    "${SDL_DIR}/src/video/SDL_yuv_mmx.c"
    "${SDL_DIR}/src/video/SDL_yuv_sw.c"
    "${SDL_DIR}/src/video/Epoc/SDL_epocvideo.cpp"
    "${SDL_DIR}/src/video/Epoc/SDL_epocevents.cpp"
    # Audio sources
    "${SDL_DIR}/src/audio/SDL_audio.c"
    "${SDL_DIR}/src/audio/SDL_audiocvt.c"
    "${SDL_DIR}/src/audio/SDL_audiodev.c"
    "${SDL_DIR}/src/audio/SDL_audiomem.c"
    "${SDL_DIR}/src/audio/SDL_mixer.c"
    "${SDL_DIR}/src/audio/SDL_wave.c"
    "${SDL_DIR}/src/audio/epoc/SDL_epocaudio.cpp"
    #"${SDL_DIR}/src/audio/epoc/SDL_epocaudiosync.cpp"
    # Endian sources
    "${SDL_DIR}/src/endian/SDL_endian.c"
    # Thread sources
    "${SDL_DIR}/src/thread/SDL_thread.c"
    "${SDL_DIR}/src/thread/generic/SDL_syscond.c"
    "${SDL_DIR}/src/thread/epoc/SDL_sysmutex.cpp"
    "${SDL_DIR}/src/thread/epoc/SDL_syssem.cpp"
    "${SDL_DIR}/src/thread/epoc/SDL_systhread.cpp"
    # Events sources
    "${SDL_DIR}/src/events/SDL_active.c"
    "${SDL_DIR}/src/events/SDL_events.c"
    "${SDL_DIR}/src/events/SDL_keyboard.c"
    "${SDL_DIR}/src/events/SDL_mouse.c"
    "${SDL_DIR}/src/events/SDL_quit.c"
    "${SDL_DIR}/src/events/SDL_resize.c"
    # Timer sources
    "${SDL_DIR}/src/timer/SDL_timer.c"
    "${SDL_DIR}/src/timer/epoc/SDL_systimer.cpp"
    # File sources
    "${SDL_DIR}/src/file/SDL_rwops.c")

add_library(EAudioLib   STATIC ${EAudioLib_sources})
add_library(EpocRuntime STATIC ${EpocRuntime_sources})
add_library(SDLmain     STATIC ${SDLmain_sources})
add_library(SDL         STATIC ${SDL_sources})
add_library(xflame     STATIC ${xflame_sources})

build_exe(${PROJECT_NAME} exe ${UID1_GW} ${UID2} ${UID3_GW} "${xflame_libs}")
#build_dll(arm_interwork EpocRuntime dll ${UID1_ER} ${UID2} ${UID3_ER} "${EpocRuntime_libs}")

if(INSTALL_APP)
    install_file(${PROJECT_NAME} ${CMAKE_CURRENT_BINARY_DIR} xflame.exe      ${MMC_DRIVE_LETTER})
#    install_file(${PROJECT_NAME} ${CMAKE_CURRENT_BINARY_DIR} EpocRuntime.dll ${MMC_DRIVE_LETTER})
endif()

add_dependencies(
    xflame
    EAudioLib
    EpocRuntime
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
    UID1=${UID1_GW}
    UID2=${UID2}
    UID3=${UID3_GW})

target_include_directories(
    xflame
    PUBLIC
    ${SDL_DIR}/include)

target_link_libraries(
    xflame
    ${xflame_libs})

target_compile_definitions(
    EAudioLib
    PUBLIC
    ${GCC_DEFS})

target_include_directories(
    EAudioLib
    PUBLIC
    ${EAUDIOLIB_DIR}/inc
    ${EAUDIOLIB_DIR}/Scumm
    ${EAUDIOLIB_DIR}/Scumm/common)

target_compile_definitions(
    EpocRuntime
    PUBLIC
    ${GCC_DEFS})

target_include_directories(
    EpocRuntime
    PUBLIC
    ${SRC_DIR}/SDL_epocruntime/inc)

target_compile_definitions(
    SDLmain
    PUBLIC
    ${GCC_DEFS})

target_include_directories(
    SDLmain
    PUBLIC
    ${SDL_DIR}/include)

target_compile_definitions(
    SDL
    PUBLIC
    SYMBIAN_SERIES60
    NO_SIGNAL_H
    ENABLE_EPOC
    DISABLE_JOYSTICK
    DISABLE_CDROM
    ${GCC_DEFS})

target_include_directories(
    SDL
    PUBLIC
    ${SDL_DIR}/include
    ${SDL_DIR}/src/
    ${SDL_DIR}/src/video
    ${SDL_DIR}/src/video/epoc
    ${SDL_DIR}/src/events
    ${SDL_DIR}/src/audio
    ${SDL_DIR}/src/audio/epoc
    ${SDL_DIR}/src/main/epoc
    ${SDL_DIR}/src/thread
    ${SDL_DIR}/src/thread/generic
    ${SDL_DIR}/src/thread/epoc
    ${SDL_DIR}/src/timer
    ${SDL_DIR}/src/joystick
    ${SRC_DIR}/SDL_epocruntime/inc
    ${SRC_DIR}/include
    ${EAUDIOLIB_DIR}/inc)
