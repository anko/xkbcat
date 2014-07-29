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

static inline int BIT(char *c, int x) { return ( c[x/8]& (1<<(x%8)) ); }
const int KEYSYM_STRLEN = 64;

char *keyPressToString(Display *disp, int code, bool down);

int printUsage() {
    printf("\
USAGE: xkbcat [-display <display>] [-delay <nanosec>] [-up]\n\
    display  target X display                   (default %s)\n\
    delay    polling frequency; nanoseconds     (default %d)\n\
    up       also print key-ups                 (default %s)\n",
        DEFAULT_DISPLAY, DEFAULT_DELAY, (DEFAULT_PRINT_UP ? "yes" : "no") );
    exit(0);
}

int main(int argc, char *argv[]) {

    char *  hostname    = DEFAULT_DISPLAY;
    int     delay       = DEFAULT_DELAY;
    bool    printKeyUps = DEFAULT_PRINT_UP;

    // Get args
    for (int i = 1; i < argc; i++) {
        if      (!strcmp(argv[i], "-help"))     printUsage();
        else if (!strcmp(argv[i], "-display"))  hostname = argv[++i];
        else if (!strcmp(argv[i], "-delay"))    delay    = atoi(argv[++i]);
        else if (!strcmp(argv[i], "-up"))       printKeyUps = true;
        else printUsage();
    }

    // Setup Xwindows
    Display *disp = XOpenDisplay(hostname);
    if (NULL == disp) {
        fprintf(stderr, "Cannot open X display: %s\n", hostname);
        exit(1);
    }
    XSynchronize(disp, true);

    // Setup buffers
    char keyBuffer1[32], keyBuffer2[32];
    char *saved = keyBuffer1,
         *keys  = keyBuffer2;
    XQueryKeymap(disp, saved);

    struct timespec sleepTime = { .tv_nsec = delay };

    while (1) {
        // find changed keys
        XQueryKeymap(disp, keys);
        for (int i = 0; i < 32*8; i++) {
            if (BIT(keys, i) != BIT(saved, i)) {
                register char *str = keyPressToString(disp, i, BIT(keys, i));
                if (BIT(keys, i) != 0 || printKeyUps) printf("%s\n",str);
                fflush(stdout); // In case user is writing to a pipe
            }
        }

        { // Swap buffers
            char * temp = saved;
            saved = keys;
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

char *keyPressToString(Display *disp, int code, bool down) {
    static char *str, buf[KEYSYM_STRLEN + 1];
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
