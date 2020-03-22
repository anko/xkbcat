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
    int xiOpcode, queryEvent, queryError;
    if (! XQueryExtension(disp, "XInputExtension", &xiOpcode,
                &queryEvent, &queryError)) {
        fprintf(stderr, "X Input extension not available\n");
        exit(2);
    }
    { // Request XInput 2.0, guarding against changes in future versions
        int major = 2, minor = 0;
        int queryResult = XIQueryVersion(disp, &major, &minor);
        if (queryResult == BadRequest) {
            fprintf(stderr, "Need XI 2.0 support (got %d.%d)\n", major, minor);
            exit(3);
        } else if (queryResult != Success) {
            fprintf(stderr, "Internal error\n");
            exit(4);
        }
    }

    // Register events
    Window root = DefaultRootWindow(disp);
    XIEventMask m;
    m.deviceid = XIAllMasterDevices;
    m.mask_len = XIMaskLen(XI_LASTEVENT);
    m.mask = calloc(m.mask_len, sizeof(char));
    XISetMask(m.mask, XI_RawKeyPress);
    if (printKeyUps) XISetMask(m.mask, XI_RawKeyRelease);
    XISelectEvents(disp, root, &m, 1);
    XSync(disp, false);
    free(m.mask);

    while ("forever") {
        XEvent event;
        XGenericEventCookie *cookie = (XGenericEventCookie*)&event.xcookie;
        XNextEvent(disp, &event);

        if (XGetEventData(disp, cookie) &&
                cookie->type == GenericEvent &&
                cookie->extension == xiOpcode) {
            switch (cookie->evtype) {
                case XI_RawKeyRelease:
                case XI_RawKeyPress: {
                    XIRawEvent *ev = cookie->data;

                    // Ask X what it calls that key
                    KeySym s = XkbKeycodeToKeysym(disp, ev->detail, 0, 0);
                    if (NoSymbol == s) continue;
                    char *str = XKeysymToString(s);
                    if (NULL == str) continue;


                    if (printKeyUps) printf("%s",
                            cookie->evtype == XI_RawKeyPress ? "+" : "-");
                    printf("%s\n", str);
                    fflush(stdout);
                    break;
                                     }
            }
        }
    }
}
