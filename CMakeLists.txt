cmake_minimum_required(VERSION 3.10)

# PlayStation Vita build of the Team Forever RSDKv4 engine.
# Builds two VPKs from the same engine source, one per game:
#   Sonic1Forever.vpk   -> data in ux0:data/Sonic1Forever/
#   Sonic2Absolute.vpk  -> data in ux0:data/Sonic2Absolute/
#
#   export VITASDK=/usr/local/vitasdk
#   cmake -S . -B build-vita && cmake --build build-vita -j$(nproc)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    if(DEFINED ENV{VITASDK})
        set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
    else()
        message(FATAL_ERROR "Please define VITASDK to point to your SDK path!")
    endif()
endif()

project(RSDKv4-Vita C CXX)
include("$ENV{VITASDK}/share/vita.cmake" REQUIRED)

set(VITA_MKSFOEX_FLAGS "${VITA_MKSFOEX_FLAGS} -d PARENTAL_LEVEL=1 -d ATTRIBUTE2=12")

# newlib heap in bytes (static buffers come on top of this)
set(VITA_HEAP_SIZE "167772160" CACHE STRING "newlib heap size for the vita build")

set(RSDK_DIR ${CMAKE_CURRENT_SOURCE_DIR}/RSDKv4)
set(DEPS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/all)

set(RSDK_SOURCES
    ${RSDK_DIR}/Animation.cpp
    ${RSDK_DIR}/Audio.cpp
    ${RSDK_DIR}/Collision.cpp
    ${RSDK_DIR}/Debug.cpp
    ${RSDK_DIR}/Drawing.cpp
    ${RSDK_DIR}/Ini.cpp
    ${RSDK_DIR}/Input.cpp
    ${RSDK_DIR}/Math.cpp
    ${RSDK_DIR}/ModAPI.cpp
    ${RSDK_DIR}/Networking.cpp
    ${RSDK_DIR}/Object.cpp
    ${RSDK_DIR}/Palette.cpp
    ${RSDK_DIR}/Reader.cpp
    ${RSDK_DIR}/Renderer.cpp
    ${RSDK_DIR}/RetroEngine.cpp
    ${RSDK_DIR}/Scene.cpp
    ${RSDK_DIR}/Scene3D.cpp
    ${RSDK_DIR}/Script.cpp
    ${RSDK_DIR}/Sprite.cpp
    ${RSDK_DIR}/String.cpp
    ${RSDK_DIR}/Text.cpp
    ${RSDK_DIR}/Userdata.cpp
    ${RSDK_DIR}/Video.cpp
    ${RSDK_DIR}/main.cpp
    ${RSDK_DIR}/NativeObjects/All.cpp
    ${DEPS_DIR}/tinyxml2/tinyxml2.cpp
    ${DEPS_DIR}/theoraplay/theoraplay.c
)

# the engine sources are identical for both games, only the data path & package info change
function(add_vita_game TARGET_NAME DATA_DIR TITLE_NAME TITLE_ID TITLE_VER LIVEAREA_DIR GAME_DEFINE)
    add_executable(${TARGET_NAME} ${RSDK_SOURCES})

    target_include_directories(${TARGET_NAME} PRIVATE
        ${RSDK_DIR}
        ${RSDK_DIR}/NativeObjects
        ${DEPS_DIR}/stb-image
        ${DEPS_DIR}/tinyxml2
        ${DEPS_DIR}/theoraplay
        $ENV{VITASDK}/arm-vita-eabi/include/SDL2
    )

    target_compile_definitions(${TARGET_NAME} PRIVATE
        BASE_PATH="ux0:data/${DATA_DIR}/"
        VITA_HEAP_SIZE=${VITA_HEAP_SIZE}
        ${GAME_DEFINE}=1
    )

    # -fsigned-char: ARM chars are unsigned by default, the engine relies on them being signed (LoadStageCollisions hangs otherwise)
    target_compile_options(${TARGET_NAME} PRIVATE
        -O3 -fsigned-char -mcpu=cortex-a9 -mfpu=neon -mfloat-abi=hard -fno-strict-aliasing
        -Wno-write-strings -Wno-narrowing -Wno-format-overflow -Wno-format-truncation -Wno-unused-result
    )
    target_compile_options(${TARGET_NAME} PRIVATE $<$<COMPILE_LANGUAGE:CXX>:-std=gnu++17 -fpermissive>)

    target_link_libraries(${TARGET_NAME}
        SDL2
        vorbisfile vorbis theoradec ogg
        SceGxm_stub SceDisplay_stub SceCtrl_stub SceAppMgr_stub SceAppUtil_stub
        SceAudio_stub SceAudioIn_stub SceSysmodule_stub SceIofilemgr_stub SceCommonDialog_stub
        SceTouch_stub SceHid_stub SceMotion_stub ScePower_stub SceProcessmgr_stub
        SceNet_stub SceNetCtl_stub
        pthread m
    )

    target_link_options(${TARGET_NAME} PRIVATE -Wl,-T,${CMAKE_CURRENT_SOURCE_DIR}/vita/segment_gap.ld)

    vita_create_self(${TARGET_NAME}.self ${TARGET_NAME} UNSAFE)

    vita_create_vpk(${TARGET_NAME}.vpk ${TITLE_ID} ${TARGET_NAME}.self
        NAME "${TITLE_NAME}"
        VERSION "${TITLE_VER}"
        FILE ${LIVEAREA_DIR}/sce_sys sce_sys
    )
endfunction()

add_vita_game(Sonic1Forever  Sonic1Forever  "Sonic 1 Forever"  "SNCF00001" "01.50" ${CMAKE_CURRENT_SOURCE_DIR}/vita/sonic1forever  VITA_GAME_S1F)
add_vita_game(Sonic2Absolute Sonic2Absolute "Sonic 2 Absolute" "SNCA00002" "01.20" ${CMAKE_CURRENT_SOURCE_DIR}/vita/sonic2absolute VITA_GAME_S2A)
