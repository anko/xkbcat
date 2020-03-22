// xkbcat: Logs X11 keypresses, globally.

#include <X11/XKBlib.h>
#include <X11/extensions/XInput2.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

const char * DEFAULT_DISPLAY    = ":0";
const bool   DEFAULT_PRINT_UP   = false;

int printUsage() {
    printf("\
USAGE: xkbcat [-display <display>] [-up]\n\
    display  target X display                   (default %s)\n\
    up       also print key-ups                 (default %s)\n",
        DEFAULT_DISPLAY, (DEFAULT_PRINT_UP ? "yes" : "no") );
    exit(0);
}

int main(int argc, char * argv[]) {

    const char * hostname    = DEFAULT_DISPLAY;
    bool         printKeyUps = DEFAULT_PRINT_UP;

    // Get arguments
    for (int i = 1; i < argc; i++) {
        if      (!strcmp(argv[i], "-help"))     printUsage();
        else if (!strcmp(argv[i], "-display"))  hostname    = argv[++i];
        else if (!strcmp(argv[i], "-up"))       printKeyUps = true;
        else { printf("Unexpected argument `%s`\n", argv[i]); printUsage(); }
    }

    // Set up X
    Display * disp = XOpenDisplay(hostname);
    if (NULL == disp) {
        fprintf(stderr, "Cannot open X display: %s\n", hostname);
        exit(1);
    }

    // Test for XInput 2 extension
    int xiOpcode;
    int queryEvent, queryError;
    if (! XQueryExtension(disp, "XInputExtension", &xiOpcode,
                &queryEvent, &queryError)) {
        // XXX Test version >=2
        fprintf(stderr, "X Input extension not available\n"); return 1;
    }

    // Register events
    Window root = DefaultRootWindow(disp);
    XIEventMask m;
    m.deviceid = XIAllMasterDevices;
    m.mask_len = XIMaskLen(XI_LASTEVENT);
    m.mask = calloc(m.mask_len, sizeof(char));
    XISetMask(m.mask, XI_RawKeyPress);
    XISetMask(m.mask, XI_RawKeyRelease);
    XISelectEvents(disp, root, &m, 1);
    XSync(disp, false);
    free(m.mask);

    while (1) { // Forever
        XEvent event;
        XGenericEventCookie *cookie = (XGenericEventCookie*)&event.xcookie;
        XNextEvent(disp, &event);

        if (XGetEventData(disp, cookie) &&
                cookie->type == GenericEvent &&
                cookie->extension == xiOpcode)
        {
            switch (cookie->evtype)
            {
                case XI_RawKeyRelease: if (!printKeyUps) continue;
                case XI_RawKeyPress: {
                    XIRawEvent *ev = cookie->data;

                    // Ask X what it calls that key
                    KeySym s = XkbKeycodeToKeysym(disp, ev->detail, 0, 0);
                    if (NoSymbol == s) continue;
                    char *str = XKeysymToString(s);
                    if (NULL == str) continue;


                    if (printKeyUps) printf("%s", cookie->evtype == XI_RawKeyPress ? "+" : "-");
                    printf("%s\n", str);
                    break;
                                     }
            }
        }
        fflush(stdout);
    }
}
