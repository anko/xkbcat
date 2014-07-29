/*
   xkbcat
   Monitors key presses globally across X11.

   A rather simplified version of
   `xspy` by Jon A. Maxwell (JAM) <jmaxwell@acm.vt.edu>
   as modified for easier physical key monitoring by Antti Korpi <an@cyan.io>.
   */

/*
   xspy polls the keyboard to determine the state of all keys on
   the keyboard.  By comparing results it determines which key has
   been pressed.  In this way it echos to the user all physical keys typed.
   */

#include <X11/Xlib.h>
#include <X11/X.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

char *    DEFAULT_DISPLAY = ":0";
const int DEFAULT_DELAY   = 10000000;
static inline int BIT(char *c, int x) { return ( c[x/8]& (1<<(x%8)) ); }
const int KEYSYM_STRLEN = 64;

/* Globals */
Display *disp;
int printKeyUps = false;

char *KeyCodeToStr(int code, int down);

int usage() {
    printf("\
USAGE: xkbcat [-display <display>] [-delay <nanosec>] [-up]\n\
    display  target X display                   (default %s)\n\
    delay    polling frequency; nanoseconds     (default %d)\n\
    up       also print key-ups                 (default %s)\n",
        DEFAULT_DISPLAY, DEFAULT_DELAY, (printKeyUps ? "yes" : "no") );
    exit(0);
}

int main(int argc, char *argv[]) {
    char    *hostname = DEFAULT_DISPLAY,
            *char_ptr,
            buf1[32], buf2[32],
            *keys,
            *saved;
    int delay = DEFAULT_DELAY;

    /* get args */
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-help")) usage();
        else if (!strcmp(argv[i], "-display")) {
            i++;
            hostname = argv[i];
        }
        else if (!strcmp(argv[i], "-delay")) {
            i++;
            delay = atoi(argv[i]);
        }
        else if (!strcmp(argv[i], "-up")) { printKeyUps = true; }
        else usage();
    }

    /* setup Xwindows */
    disp = XOpenDisplay(hostname);
    if (NULL == disp) {
        fprintf(stderr, "Cannot open X display: %s\n", hostname);
        exit(1);
    }
    XSynchronize(disp, true);

    /* setup buffers */
    saved = buf1; keys=buf2;
    XQueryKeymap(disp, saved);

    struct timespec sleepTime = { .tv_nsec = delay };

    while (1) {
        /* find changed keys */
        XQueryKeymap(disp, keys);
        for (int i = 0; i < 32*8; i++) {
            if (BIT(keys, i) != BIT(saved, i)) {
                register char *str;
                str = (char *)KeyCodeToStr(i, BIT(keys, i));
                if (BIT(keys, i) != 0 || printKeyUps) printf("%s\n",str);
                fflush(stdout); /* in case user is writing to a pipe */
            }
        }

        /* swap buffers */
        char_ptr = saved;
        saved = keys;
        keys = char_ptr;

        nanosleep(&sleepTime, NULL);
    }
}

/*
   Have a keycode, Look up keysym for it.
   Convert keysym into its string representation.
   Put it as (+string) or (-string), depending on if it's up or down.
   Print out the string.
   */

char *KeyCodeToStr(int code, int down) {
    static char *str, buf[KEYSYM_STRLEN + 1];
    KeySym keysym = XKeycodeToKeysym(disp, code, 0);
    if (NoSymbol == keysym) return "";

    /* convert keysym to a string, copy it to a local area */
    str = XKeysymToString(keysym);

    if (NULL == str) return "";
    strncpy(buf, str, KEYSYM_STRLEN); buf[KEYSYM_STRLEN] = 0;

    /* still a string, so put it in form (+str) or (-str) */
    if (down) strcpy(buf, "+ ");
    else      strcpy(buf, "- ");
    strcat(buf, str);
    return buf;
}
