#include "RetroEngine.hpp"

#if !RETRO_USE_ORIGINAL_CODE

#if RETRO_PLATFORM == RETRO_WIN
#include "Windows.h"
#endif

#if RETRO_PLATFORM == RETRO_VITA
#include <psp2/power.h>
#include <psp2/kernel/processmgr.h>

#ifndef VITA_HEAP_SIZE
#define VITA_HEAP_SIZE (160 * 1024 * 1024)
#endif
// newlib heap (malloc/new). SDL's textures live in separate gpu memory.
int _newlib_heap_size_user = VITA_HEAP_SIZE;
// the script compiler and scene loader keep some big buffers on the stack
unsigned int sceUserMainThreadStackSize = 2 * 1024 * 1024;

static SceUInt64 profStart[VPROF_COUNT];
static SceUInt64 profTotal[VPROF_COUNT];
static SceUInt64 profMax[VPROF_COUNT];
static SceUInt64 profFrameMax[VPROF_COUNT];
static SceUInt64 profWindowStart = 0;
static int profFrames            = 0;

void VitaProfileBegin(int section)
{
    if (engineDebugMode)
        profStart[section] = sceKernelGetProcessTimeWide();
}
void VitaProfileEnd(int section)
{
    if (engineDebugMode && profStart[section]) {
        SceUInt64 t = sceKernelGetProcessTimeWide() - profStart[section];
        profTotal[section] += t;
        profFrameMax[section] += t; // a section can run more than once per frame
        profStart[section] = 0;
    }
}
// Debug helper: if BASE_PATH/dumpframe.txt holds a frame number, the RNG is seeded with a constant and the raw
// framebuffer of that frame is written to BASE_PATH/frame.raw (used to check optimisations don't change the output)
static int dumpFrameID    = -2;
static int dumpFrameCount = 0;
static void VitaDumpFrame()
{
    if (dumpFrameID == -2) {
        dumpFrameID  = -1;
        FileIO *file = fOpen(BASE_PATH "dumpframe.txt", "rb");
        if (file) {
            char text[0x20] = {};
            fRead(text, 1, sizeof(text) - 1, file);
            fClose(file);
            dumpFrameID = atoi(text);
        }
    }
    if (dumpFrameID < 0)
        return;

    if (dumpFrameCount++ == dumpFrameID) {
        FileIO *file = fOpen(BASE_PATH "frame.raw", "wb");
        if (file) {
            for (int y = 0; y < SCREEN_YSIZE; ++y) fWrite(&Engine.frameBuffer[y * GFX_LINESIZE], sizeof(ushort), SCREEN_XSIZE, file);
            fClose(file);
        }
    }
}
bool VitaDumpFrameEnabled()
{
    FileIO *file = fOpen(BASE_PATH "dumpframe.txt", "rb");
    if (file)
        fClose(file);
    return file != NULL;
}

void VitaProfileFrame()
{
    VitaDumpFrame();
    if (!engineDebugMode)
        return;

    for (int s = 0; s < VPROF_COUNT; ++s) {
        if (profFrameMax[s] > profMax[s])
            profMax[s] = profFrameMax[s];
        profFrameMax[s] = 0;
    }

    SceUInt64 now = sceKernelGetProcessTimeWide();
    if (!profWindowStart)
        profWindowStart = now;
    ++profFrames;
    if (now - profWindowStart >= 2000000) {
        const char *names[] = { "native", "objects", "draw", "sort3d", "draw3d", "flip" };
        char buffer[0x200];
        // avg/max ms per frame; keep it short, PrintLog formats into a 256 byte buffer
        int pos = sprintf(buffer, "[prof] %.1ffps", profFrames * 1000000.0 / (now - profWindowStart));
        for (int s = 0; s < VPROF_COUNT; ++s)
            pos += sprintf(buffer + pos, " %s %.1f/%.1f", names[s], profTotal[s] / 1000.0 / profFrames, profMax[s] / 1000.0);
        PrintLog("%s", buffer);

        memset(profTotal, 0, sizeof(profTotal));
        memset(profMax, 0, sizeof(profMax));
        profFrames      = 0;
        profWindowStart = now;
    }
}
#endif

void parseArguments(int argc, char *argv[])
{
    for (int a = 0; a < argc; ++a) {
        const char *find = "";

        find = strstr(argv[a], "stage=");
        if (find) {
            int b = 0;
            int c = 6;
            while (find[c] && find[c] != ';') Engine.startSceneFolder[b++] = find[c++];
            Engine.startSceneFolder[b] = 0;
        }

        find = strstr(argv[a], "scene=");
        if (find) {
            int b = 0;
            int c = 6;
            while (find[c] && find[c] != ';') Engine.startSceneID[b++] = find[c++];
            Engine.startSceneID[b] = 0;
        }

        find = strstr(argv[a], "console=true");
        if (find) {
            engineDebugMode       = true;
            Engine.devMenu        = true;
            Engine.consoleEnabled = true;
#if RETRO_PLATFORM == RETRO_WIN
            AllocConsole();
            freopen_s((FILE **)stdin, "CONIN$", "w", stdin);
            freopen_s((FILE **)stdout, "CONOUT$", "w", stdout);
            freopen_s((FILE **)stderr, "CONOUT$", "w", stderr);
#endif
        }

        find = strstr(argv[a], "usingCWD=true");
        if (find) {
            usingCWD = true;
        }
    }
}
#endif

#ifdef NXLINK
#include <switch.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/errno.h>
#include <arpa/inet.h>
#include <unistd.h>

static int s_nxlinkSock = -1;

static void initNxLink()
{
    if (R_FAILED(socketInitializeDefault()))
        return;

    s_nxlinkSock = nxlinkStdio();
    if (s_nxlinkSock >= 0)
        printf("printf output now goes to nxlink server\n");
    else
        socketExit();
}
#endif

int main(int argc, char *argv[])
{
#ifdef NXLINK
    initNxLink();
#endif

#if RETRO_PLATFORM == RETRO_VITA
    // the whole frame is drawn in software, so run at full speed
    scePowerSetArmClockFrequency(444);
    scePowerSetBusClockFrequency(222);
    scePowerSetGpuClockFrequency(222);
    scePowerSetGpuXbarClockFrequency(166);
#endif

#if !RETRO_USE_ORIGINAL_CODE
    parseArguments(argc, argv);
#endif

    SDL_SetHint(SDL_HINT_WINRT_HANDLE_BACK_BUTTON, "1");
    Engine.Init();
#if RETRO_PLATFORM == RETRO_VITA
    if (VitaDumpFrameEnabled())
        srand(1); // deterministic run for frame comparisons
#endif
    Engine.Run();

#if RETRO_PLATFORM == RETRO_VITA
    sceKernelExitProcess(0);
#endif

#if !RETRO_USE_ORIGINAL_CODE
    if (Engine.consoleEnabled) {
#if RETRO_PLATFORM == RETRO_WIN
        FreeConsole();
#endif //! RETRO_PLATFORM == RETRO_WIN
    }
#endif //! !RETRO_USE_ORIGINAL_CODE

#ifdef NXLINK
    socketExit();
#endif //! NXLINK

    return 0;
}

#if RETRO_PLATFORM == RETRO_UWP
int __stdcall wWinMain(HINSTANCE, HINSTANCE, PWSTR, int) { return SDL_WinRTRunApp(main, NULL); }
#endif
