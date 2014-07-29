/*
   xkbcat
   ------

   Monitors key presses globally across X11.
   */

#include <X11/Xlib.h>
#include <X11/X.h>
#include <X11/XKBlib.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

char *     DEFAULT_DISPLAY  = ":0";
const int  DEFAULT_DELAY    = 10000000;
const bool DEFAULT_PRINT_UP = false;

typedef char KbBuffer[32];
static inline bool keyState(KbBuffer c, int key) {
    return ( c[key/8] & (1<<(key%8)) );
}
const int KEYSYM_STRLEN = 64;

char * keyPressToString(Display * disp, int code, bool down);

int printUsage() {
    printf("\
USAGE: xkbcat [-display <display>] [-delay <nanosec>] [-up]\n\
    display  target X display                   (default %s)\n\
    delay    polling frequency; nanoseconds     (default %d)\n\
    up       also print key-ups                 (default %s)\n",
        DEFAULT_DISPLAY, DEFAULT_DELAY, (DEFAULT_PRINT_UP ? "yes" : "no") );
    exit(0);
}

int main(int argc, char * argv[]) {

    char *  hostname    = DEFAULT_DISPLAY;
    int     delay       = DEFAULT_DELAY;
    bool    printKeyUps = DEFAULT_PRINT_UP;

    // Get args
    for (int i = 1; i < argc; i++) {
        if      (!strcmp(argv[i], "-help"))     printUsage();
        else if (!strcmp(argv[i], "-display"))  hostname    = argv[++i];
        else if (!strcmp(argv[i], "-delay"))    delay       = atoi(argv[++i]);
        else if (!strcmp(argv[i], "-up"))       printKeyUps = true;
        else printUsage();
    }

    // Setup Xwindows
    Display * disp = XOpenDisplay(hostname);
    if (NULL == disp) {
        fprintf(stderr, "Cannot open X display: %s\n", hostname);
        exit(1);
    }
    XSynchronize(disp, true);

    // Setup buffers
    KbBuffer keyBuffer1, keyBuffer2;
    KbBuffer * oldKeys = &keyBuffer1,
             * keys    = &keyBuffer2;
    XQueryKeymap(disp, * oldKeys); // Initial get

    struct timespec sleepTime = { .tv_nsec = delay };

    while (1) { // Forever
        // Get changed keys
        XQueryKeymap(disp, * keys);

        for (int i = 0; i < sizeof(KbBuffer) * 8; i++) {
            bool stateBefore = keyState(*oldKeys, i),
                 stateNow    = keyState(*keys, i);
            if ( stateNow != stateBefore        // Changed?
                 && (stateNow || printKeyUps) ) // Should print?
            {
                printf("%s\n", keyPressToString(disp, i, stateNow));
                fflush(stdout); // Ensure pipe is updated
            }
        }

        { // Swap buffers
            KbBuffer * temp = oldKeys;
            oldKeys = keys;
            keys = temp;
        }

        nanosleep(&sleepTime, NULL);
    }
}

/*
   Have a keycode, Look up keysym for it.
   Convert keysym into its string representation.
   Put it as (+string) or (-string), depending on if it's up or down.
   Print out the string.
   */

char * keyPressToString(Display * disp, int code, bool down) {
    static char * str, buf[KEYSYM_STRLEN + 1];
    KeySym keysym = XkbKeycodeToKeysym(disp, code, 0, 0);
    if (NoSymbol == keysym) return "";

    // Convert keysym to a string, copy it to a local area
    str = XKeysymToString(keysym);

    if (NULL == str) return "";
    strncpy(buf, str, KEYSYM_STRLEN); buf[KEYSYM_STRLEN] = 0;

    // Still a string, so put it in form (+str) or (-str)
    if (down) strcpy(buf, "+ ");
    else      strcpy(buf, "- ");
    strcat(buf, str);
    return buf;
}
