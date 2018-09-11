/*
 * 2017, 2018 Tarpeeksi Hyvae Soft /
 * RallySportED frame timer
 *
 * A simple timer to count the rough number of milliseconds elapsed between frames.
 *
 * Call Update() once per frame, then Elapsed() will approximate the time it took to
 * compute the stuff between calls.
 *
 */

#include <cstddef>
#include "../../core/frame_timer.h"
#include "../../core/types.h"

#ifdef RSED_ON_QT
    #include <QElapsedTimer>
#elif RSED_ON_WIN32
    #include <windows.h>
    static const uint TIMER_RESOLUTION_MS = 1;  // To what resolution (ms) to set the Windows timer.
#elif RSED_ON_DOS16
    #include <time.h>
    #include <dos.h>
    static void interrupt DOSTimer_HandleInterrupt();
    void DOSTimer_SetInterruptHandler();
    void DOSTimer_RestoreInterruptHandler();
#else
    #error "Unknown platform."
#endif

static long TIMER;

#ifdef RSED_ON_QT
    QElapsedTimer _TIMER;
#else
    long _TIMER;
#endif

void kftimer_initialize_frame_timer(void)
{
    TIMER = 0;

    #ifdef RSED_ON_QT
        _TIMER.start();
    #elif RSED_ON_WIN32
        timeBeginPeriod(TIMER_RESOLUTION_MS);         // Increase precision on timeGetTime().
        _TIMER = timeGetTime();
    #elif RSED_ON_DOS16
        DOSTimer_SetInterruptHandler();
    #else
        #error "Unknown platform."
    #endif

    return;
}

void kftimer_release_frame_timer(void)
{
    #ifdef RSED_ON_WIN32
        timeEndPeriod(TIMER_RESOLUTION_MS);
    #elif RSED_ON_DOS16
        DOSTimer_RestoreInterruptHandler();
    #elif RSED_ON_QT
        /// Nothing.
    #else
        #error "Unknown platform."
    #endif

    return;
}

void kftimer_update(void)
{
    #ifdef RSED_ON_QT
        TIMER = _TIMER.restart();
        _TIMER.restart();
    #elif RSED_ON_WIN32
        TIMER = timeGetTime() - _TIMER;
        _TIMER = timeGetTime();
    #elif RSED_ON_DOS16
        TIMER = DOSTimer_Restart();
    #else
        #error "Unknown platform."
    #endif

    return;
}

long kftimer_elapsed(void)
{
    return TIMER;
}

// DOS version of the timer.
#ifdef RSED_ON_DOS16
    const unsigned char TIMER_RESOLUTION    = 5;    // Milliseconds.
    const unsigned char TIMER_TICKS_PER_SEC = 200;  // How many times per second the timer interrupt is fired.

    static void interrupt (*DOSTimer_OldInterruptHandler)(void);

    static unsigned char timerSeconds = 0;
    static unsigned char timerTicks = 0;
    static unsigned char timerKeepup = 0;
    static short frameTime = 0;

    static short timerIntAddr = 0;
    static short timerIntSegm = 0;

    long FrameTimer::DOSTimer_Restart()
    {
        _asm{ cli }

        long time = frameTime;
        frameTime = 0;

        _asm{ sti }

        return time;
    }

    static void interrupt DOSTimer_HandleInterrupt()
    {
        _asm{ cli }

        frameTime += TIMER_RESOLUTION;

        timerTicks++;
        if (timerTicks >= TIMER_TICKS_PER_SEC)
        {
            timerSeconds++;
            timerTicks = 0;
        }

        // See if we need to call DOS's own timer interrupt to keep the system clock
        // running at the correct rate.
        timerKeepup += TIMER_RESOLUTION;
        if (timerKeepup >= 55)
        {
            timerKeepup = 0;
            DOSTimer_OldInterruptHandler();
        }
        else
        {
            // Clear the interrupt.
            outp(0x20, 0x20);
        }

        _asm{ sti }

        return;
    }

    void DOSTimer_SetInterruptHandler()
    {
        _asm{ cli }

        // Stored away DOS's original timer interrupt, so we can restored it on exit.
        DOSTimer_OldInterruptHandler = getvect(0x08);

        // Replace DOS's timer interrupt handler with ours.
        setvect(0x08, DOSTimer_HandleInterrupt);

        // Set the timer interrupt rate.
        outp(0x43, 0x36);
        outp(0x40, 0xce);
        outp(0x40, 0x13);

        _asm{ sti }

        return;
    }

    void DOSTimer_RestoreInterruptHandler()
    {
        _asm{ cli }

        // Restore DOS's own timer interrupt handler.
        setvect(0x08, DOSTimer_OldInterruptHandler);

        // Restore the clock to its original refresh rate.
        outp(0x43, 0x36);
        outp(0x40, 0x00);
        outp(0x40, 0x00);

        _asm{ sti }

        return;
    }
#endif
