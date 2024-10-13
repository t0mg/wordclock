#pragma once

// Define debug to turn on debug logging.
// The flag is normally set by the selected PlatformIO environment.
// #define DEBUG 1

// DLOG and DLOGLN are equivalent of Serial.print and println, but turned off by
// the DEBUG macro absence.
// DCHECK only logs if the first parameter is false.
// setupLogging() must be called from setup() for everything to work.

#if defined(DEBUG)

#include <HardwareSerial.h>

#define DLOG(...) Serial.print(__VA_ARGS__)
#define DLOGLN(...) Serial.println(__VA_ARGS__)
#define DCHECK(condition, ...) \
    if (!(condition))          \
    {                          \
        DLOG("DCHECK Line ");  \
        DLOG(__LINE__);        \
        DLOG(": (");           \
        DLOG(#condition);      \
        DLOG(") ");            \
        DLOGLN(__VA_ARGS__);   \
    }

#define setupLogging()              \
    {                               \
        Serial.begin(115200);       \
        delay(2000);                \
        DLOGLN("Logging started."); \
    }

#else

#define DLOG(...)
#define DLOGLN(...)
#define DCHECK(condition, ...)

#define setupLogging()

#endif // defined(DEBUG)
