// xkbcat: Logs X11 keypresses, globally.

#include <X11/XKBlib.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

const char * DEFAULT_DISPLAY  = ":0";
const int    DEFAULT_DELAY    = 10000000;
const bool   DEFAULT_PRINT_UP = false;

typedef char KbBuffer[32];
static inline bool keyState(KbBuffer b, int key) {
    return b[key/8] & (1<<(key%8));
}

int printUsage() {
    printf("\
USAGE: xkbcat [-display <display>] [-delay <nanosec>] [-up]\n\
    display  target X display                   (default %s)\n\
    delay    polling frequency; nanoseconds     (default %d)\n\
    up       also print key-ups                 (default %s)\n",
        DEFAULT_DISPLAY, DEFAULT_DELAY, (DEFAULT_PRINT_UP ? "yes" : "no") );
    exit(0);
}

void printKeyPress(Display * disp, int code, bool down, bool printKeyUps);

int main(int argc, char * argv[]) {

    const char * hostname    = DEFAULT_DISPLAY;
    int          delay       = DEFAULT_DELAY;
    bool         printKeyUps = DEFAULT_PRINT_UP;

    // Get arguments
    for (int i = 1; i < argc; i++) {
        if      (!strcmp(argv[i], "-help"))     printUsage();
        else if (!strcmp(argv[i], "-display"))  hostname    = argv[++i];
        else if (!strcmp(argv[i], "-delay"))    delay       = atoi(argv[++i]);
        else if (!strcmp(argv[i], "-up"))       printKeyUps = true;
        else { printf("Unexpected argument `%s`\n", argv[i]); printUsage(); }
    }

    // Set up X
    Display * disp = XOpenDisplay(hostname);
    if (NULL == disp) {
        fprintf(stderr, "Cannot open X display: %s\n", hostname);
        exit(1);
    }
    XSynchronize(disp, true);

    // Set up buffers
    KbBuffer keyBuffer1, keyBuffer2;
    KbBuffer * oldKeys = &keyBuffer1,
             * keys    = &keyBuffer2;
    XQueryKeymap(disp, *oldKeys); // Initial fetch

    // Timespec for time to sleep for
    struct timespec sleepTime = { .tv_nsec = delay };

    while (1) { // Forever
        XQueryKeymap(disp, *keys); // Fetch changed keys

        for (int i = 0; i < sizeof(KbBuffer) * 8; i++) {
            bool stateBefore = keyState(*oldKeys, i),
                 stateNow    = keyState(*keys, i);
            if ( stateNow != stateBefore        // Changed?
                 && (stateNow || printKeyUps) ) // Should print?
                printKeyPress(disp, i, stateNow, printKeyUps);
        }

        { // Swap buffers
            KbBuffer * temp = oldKeys;
            oldKeys = keys;
            keys = temp;
        }

        nanosleep(&sleepTime, NULL);
    }
}

// Since `XKeysymToString` returns a string of unknown length that shouldn't be
// modified, so it makes more sense to just `printf` it in-place.
void printKeyPress(Display * disp, int code, bool down, bool printKeyUps) {

    KeySym s = XkbKeycodeToKeysym(disp, code, 0, 0); if (NoSymbol == s) return;
    char * str = XKeysymToString(s);                 if (NULL == str)   return;

    if (printKeyUps)
        printf("%s %s\n", (down ? "+" : "-"), str);
    else
        puts(str);
}
