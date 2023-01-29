# xkbcat [![](https://img.shields.io/github/actions/workflow/status/anko/xkbcat/ci.yml?branch=master&style=flat-square)](https://github.com/anko/xkbcat/actions) ![](https://img.shields.io/github/languages/code-size/anko/xkbcat?style=flat-square)

Simple X11 keylogger.

 - Simple output format:  One line on `stdout` per key event.
 - Simple to audit:  One short file of modern C.
 - Simple to run:  Does not need `sudo`.

## Examples

### Keypresses only

Given no options, `xkbcat` prints only keypresses, one per line. Here's the
output when I type "Hi":

    Shift_L
    h
    i

### Keypresses and key-ups

With key-ups enabled (`xkbcat -up`), the format changes to show them:

    +Shift_L
    +h
    -h
    -Shift_L
    +i
    -i

Lines starting `+` are key-downs; `-` are key-ups.

## Compilation

Just `make`.

Don't have `X11/extensions/XInput2.h`?  Install your distro's `libxi-devel`
package.

## Usage

Options you can pass (all optional):

 - `-display <display>`: set target X display (default `:0`)
 - `-up`: also prepend key-ups (default: don't)
 - `-help`: print usage hints and exit

Then just use your computer as usual.  Interrupt signal (`C-c`) to quit.

## Related programs

### Other keyloggers

 - If you need to log keys across a whole Linux system (also in the
   framebuffer—not just in X11), try [keysniffer][1].  It works via a kernel
   module, and needs `sudo`.
 - If you want to see what characters the user actually typed (with modifier
   keys, backspace, etc resolved into text), [`xspy`][2] or [`logkeys`][3]
   might be better for you.

### Programs that work well together with `xkbcat`

 - If you want to add timestamps to each line for logging purposes, I recommend
   piping to the [moreutils package][4]'s `ts`.  [These answers][5] feature
   various other tools good for the purpose.
 - If you only want to see key names when you press keys in the same terminal
   where `xkbcat` is running, you can temporarily disable terminal echo with
   `stty -echo && xkbcat`.  (`stty` is in coreutils.)

### Programs for logging other X11 events

 - [xinput][6] invoked as `xinput --test-xi2 --root` logs everything
   input-related; even mouse movements and clicks, and touchpad stuff.  Its
   output is very comprehensive, but harder to parse.

 - If you need to log X11 events more generally, various protocol monitoring
   programs are listed in the [X11 debugging guide][7].

## Versioning

The git-tagged version numbers follow [semver][8].

Error outputs (on stderr) are intended to be read by people.  Changes to their
wording are not considered breaking changes.  Don't parse them
programmatically.

## License

[ISC][9].


[1]: https://github.com/jarun/keysniffer
[2]: http://www.freshports.org/security/xspy/
[3]: http://code.google.com/p/logkeys/
[4]: http://joeyh.name/code/moreutils/
[5]: http://stackoverflow.com/questions/21564/is-there-a-unix-utility-to-prepend-timestamps-to-lines-of-text
[6]: https://www.x.org/archive/current/doc/man/man1/xinput.1.xhtml
[7]: https://www.x.org/wiki/guide/debugging/
[8]: http://semver.org/
[9]: http://opensource.org/licenses/ISC
