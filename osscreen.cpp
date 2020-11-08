/************************************
            Known issues:
 - Persistent black screen when exiting application after transitions
    -Wii U is functional besides screen, just reload RPX and exit normal way

 ***********************************/

// TV: 720p (1280 * 720) or 1080p (1920 * 1080).
// Gamepad: 480p (854 * 480).
// Middle screen: TV 720p(640, 360), Gamepad(427,240).

#include <proc_ui/procui.h>
#include <coreinit/foreground.h>
#include <vpad/input.h>
#include <sysapp/launch.h>
#include <coreinit/screen.h>
#include <malloc.h>
#include <coreinit/cache.h>
#include <string>
#include <cmath>

ProcUIStatus status = PROCUI_STATUS_IN_FOREGROUND;
VPADStatus vstatus;
VPADReadError error;
bool vpad_fatal = false;
bool exitter;
short loopstatus = 0;


class Square {

    static void read(Square &square) {
        if (square.isvpad) {

            VPADRead(VPAD_CHAN_0, &vstatus, 1, &error);

            switch (error) {
                case VPAD_READ_SUCCESS: {

                 if ((vstatus.trigger | vstatus.hold) & VPAD_BUTTON_UP) {
                      if (square.tvy - square.speed - ceil(square.size * 1.5) > 0) {square.tvy -= square.speed;
                        square.drcy = floor(square.tvy / 1.5);}
                 }if ((vstatus.trigger | vstatus.hold) & VPAD_BUTTON_DOWN) {
                       if (square.tvy + square.speed + ceil(square.size * 1.5) < 719) {square.tvy += square.speed;
                        square.drcy = floor(square.tvy / 1.5);}
                 }if ((vstatus.trigger | vstatus.hold) & VPAD_BUTTON_RIGHT) {
                      if (square.tvx + square.speed + ceil(square.size * 1.5) < 1279) {square.tvx += square.speed;
                        square.drcx = floor(square.tvx / 1.5);}
                 }
                 if ((vstatus.trigger | vstatus.hold) & VPAD_BUTTON_LEFT) {
                      if (square.tvx - square.speed - ceil(square.size * 1.5) > 0) {square.tvx -= square.speed;
                        square.drcx = floor(square.tvx / 1.5);}
                     }
                 }
                case VPAD_READ_NO_SAMPLES: break;
                default: {vpad_fatal = true; break;}
            }
        }
        else {
            VPADRead(VPAD_CHAN_0, &vstatus, 1, &error);

            switch (error) {
                case VPAD_READ_SUCCESS: {

                    if ((vstatus.trigger | vstatus.hold) & VPAD_BUTTON_X) {
                        if (square.tvy - square.speed - ceil(square.size * 1.5) > 0) {square.tvy -= square.speed;
                            square.drcy = floor(square.tvy / 1.5);}
                    }if ((vstatus.trigger | vstatus.hold) & VPAD_BUTTON_B) {
                        if (square.tvy + square.speed + ceil(square.size * 1.5) < 719) {square.tvy += square.speed;
                            square.drcy = floor(square.tvy / 1.5);}
                    }if ((vstatus.trigger | vstatus.hold) & VPAD_BUTTON_A) {
                        if (square.tvx + square.speed + ceil(square.size * 1.5) < 1279) {square.tvx += square.speed;
                            square.drcx = floor(square.tvx / 1.5);}
                    }
                    if ((vstatus.trigger | vstatus.hold) & VPAD_BUTTON_Y) {
                        if (square.tvx - square.speed - ceil(square.size * 1.5) > 0) {square.tvx -= square.speed;
                            square.drcx = floor(square.tvx / 1.5);}
                    }
                }
                case VPAD_READ_NO_SAMPLES: break;
                default: {vpad_fatal = true; break;}
            }
        }
    }
    static void build(Square &square) {
        for (short i = square.size; i>=-square.size; i--) {
            for (short j = square.size; j>=-square.size; j--) {
                OSScreenPutPixelEx(SCREEN_DRC, square.drcx + i, square.drcy + j, square.color);
            }
        }
        for (short i = ceil(square.size * 1.5); i>=-ceil(square.size * 1.5); i--) {
            for (short j = ceil(square.size * 1.5); j>=-ceil(square.size * 1.5); j--) {
                OSScreenPutPixelEx(SCREEN_TV, square.tvx + i, square.tvy + j, square.color);
            }
        }
    }

public:
    double tvx, tvy, drcx, drcy, size; unsigned int color; short speed; bool isvpad;
    static void setparams (Square &square, bool isvpad, double size = 10, unsigned int color = 0xFFFFFFFF, short speed = 5, double tvx = 640, double tvy = 360) {
        square.tvx = tvx; square.tvy = tvy; square.size = size; square.color = color; square.speed = speed; square.isvpad = isvpad; square.drcx = floor(square.tvx / 1.5); square.drcy = floor(square.tvy / 1.5);
    }
    static void update(Square &square) {
        Square::read(square);
        Square::build(square);
    }

} square1, square2;

bool reset(size_t &tvsize, size_t &drcsize, void* &tvmem, void* &drcmem) {
    OSScreenClearBufferEx(SCREEN_TV, 0x00000000);
    OSScreenClearBufferEx(SCREEN_DRC, 0x00000000);
    Square::setparams(square1, false, 75,0xFF333300, 5, 1160, 120);
    Square::setparams(square2, true, 25, 0x00FF0000, 10, 40, 680);
    OSScreenPutFontEx(SCREEN_TV, 0, 0, "Press A to start");
    OSScreenPutFontEx(SCREEN_DRC, 0, 0, "Press A to start");
    DCFlushRange(tvmem, tvsize);
    DCFlushRange(drcmem, drcsize);
    OSScreenFlipBuffersEx(SCREEN_TV);
    OSScreenFlipBuffersEx(SCREEN_DRC);

    while (true) {
        VPADRead(VPAD_CHAN_0, &vstatus, 1, &error);
        if ((vstatus.trigger | vstatus.hold) & VPAD_BUTTON_A) return false;
        if (vstatus.hold & VPAD_BUTTON_MINUS) return true;
    }
}

short loop(size_t &tvsize, size_t &drcsize, void* &tvmem, void* &drcmem) {
    OSScreenClearBufferEx(SCREEN_TV, 0x00000000);
    OSScreenClearBufferEx(SCREEN_DRC, 0x00000000);

        Square::update(square1);
        Square::update(square2);

           /* std::string tvxs = std::to_string(square1.tvx); const char* tvxx = tvxs.c_str();
            std::string tvys = std::to_string(square1.tvy); const char* tvyy = tvys.c_str();
            std::string drcxs = std::to_string(square2.tvx); const char* drcxx = drcxs.c_str();
            std::string drcys = std::to_string(square2.tvy); const char* drcyy = drcys.c_str();

            OSScreenPutFontEx(SCREEN_TV, 0, 0, tvxx);
            OSScreenPutFontEx(SCREEN_TV, 1, 1, tvyy);
            OSScreenPutFontEx(SCREEN_TV, 3, 2, drcxx);
            OSScreenPutFontEx(SCREEN_TV, 5, 3, drcyy); */

    DCFlushRange(tvmem, tvsize);
    DCFlushRange(drcmem, drcsize);
    OSScreenFlipBuffersEx(SCREEN_TV);
    OSScreenFlipBuffersEx(SCREEN_DRC);
    if (vpad_fatal) return -2;
    if (vstatus.hold & VPAD_BUTTON_MINUS) return -1;

if (abs(square1.tvx - square2.tvx) < 154 && abs(square1.tvy - square2.tvy) < 154) {
    exitter = reset(tvsize, drcsize, tvmem, drcmem);
    if (exitter) return -1;

    }
    return 0;
}

void reconnect() {
    while (error != VPAD_READ_SUCCESS) {
        VPADRead(VPAD_CHAN_0, &vstatus, 1, &error);
    }
    vpad_fatal = false;
}

void
GameIsRunning(size_t &tvsize, size_t &drcsize, void* &tvmem, void* &drcmem) {
    while (status != PROCUI_STATUS_EXITING) {
        status = ProcUIProcessMessages(true);
        if (status == PROCUI_STATUS_IN_FOREGROUND) {
            loopstatus = loop(tvsize, drcsize, tvmem, drcmem);
            if (loopstatus == -1) status = PROCUI_STATUS_EXITING;
            if (loopstatus == -2) reconnect();
        }
        if (status == PROCUI_STATUS_RELEASE_FOREGROUND) {
            ProcUIDrawDoneRelease();
            status = ProcUIProcessMessages(true);
        }
    }
    SYSRelaunchTitle(0, nullptr);
}

void saveCallback() {
    OSSavesDone_ReadyToRelease();
}

int
main(int argc, char **argv)
{
    ProcUIInit(saveCallback);
    OSScreenInit();
    size_t tvBufferSize = OSScreenGetBufferSizeEx(SCREEN_TV);
    size_t drcBufferSize = OSScreenGetBufferSizeEx(SCREEN_DRC);
    void* tvBuffer = memalign(0x100, tvBufferSize);
    void* drcBuffer = memalign(0x100, drcBufferSize);
    if (!tvBuffer || !drcBuffer) status = PROCUI_STATUS_EXITING;
    else {
        OSScreenSetBufferEx(SCREEN_TV, tvBuffer);
        OSScreenSetBufferEx(SCREEN_DRC, drcBuffer);
        OSScreenEnableEx(SCREEN_TV, true);
        OSScreenEnableEx(SCREEN_DRC, true);
        exitter = reset(tvBufferSize, drcBufferSize, tvBuffer, drcBuffer);
    }

    if (!exitter) GameIsRunning(tvBufferSize, drcBufferSize, tvBuffer, drcBuffer);


    if (tvBuffer) free(tvBuffer);
    if (drcBuffer) free(drcBuffer);
    OSScreenShutdown();
    ProcUIShutdown();
    return 0;
}
