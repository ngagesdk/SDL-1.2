cmake_minimum_required(VERSION 3.00)
set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_SOURCE_DIR}/../cmake/symbian-6.1-series60-toolchain.cmake)

project(ESDL CXX)

set(UID1 0x10000079) # KDynamicLibraryUidValue, e32uid.h
set(UID2 0x100039ce) # KAppUidValue16, apadef.h

set(GCC_COMN_DEFS -D__SYMBIAN32__ -D__GCC32__ -D__EPOC32__ -D__MARM__ -D__MARM_ARMI__)
set(GCC_MODE_DEFS -DNDEBUG -D_UNICODE)
set(GCC_USER_DEFS -DUID1=${UID1} -DUID2=${UID2})
set(GCC_DEFS      ${GCC_COMN_DEFS} ${GCC_MODE_DEFS})

set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(SDL_DIR "${SRC_DIR}/SDL")

set(ESDL_libs)

set(ESDL_sources
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

add_library(${PROJECT_NAME} STATIC ${ESDL_sources})

target_compile_definitions(
    ${PROJECT_NAME}
    PUBLIC
    SYMBIAN_SERIES60
    NO_SIGNAL_H
    ENABLE_EPOC
    DISABLE_JOYSTICK
    DISABLE_CDROM
    ${GCC_DEFS})

target_include_directories(
    ${PROJECT_NAME}
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
    ${SRC_DIR}/EAudioLib/inc)
