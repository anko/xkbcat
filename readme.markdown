# xkbcat

X11 keylogger with a simple output format: One key event per line on `stdout`.

Great as a data source for keyboard heatmaps, layout comparisons or
screencasting applications.

<100 lines of modern C. Doesn't need `sudo`.

## Examples

Given no flags, `xkbcat` prints only keypresses, one per line. Here's the
output when I type "Hi":

    Shift_L
    h
    i

With key-ups enabled (`xkbcat -up`), the format changes to show them:

    + Shift_L
    + h
    - h
    - Shift_L
    + i
    - i

Lines starting `+` are key-downs; `-` are key-ups. 

Left- and right-positioned modifier keys are recognised separately. The F-keys
work too, as do most media keys like `XF86AudioLowerVolume`.

## Compilation

Just `make`.

Don't have `clang`? Edit the `makefile` to use `gcc`.

Don't have `X11/extensions/XInput2.h`?  Install your distro's `libxi-devel`
package.

## Usage

Flags you can pass (all optional):

 - `-display <display>`: set target X display (default `:0`)
   that's 100ms)
 - `-up`: also prepend key-ups (default: don't)
 - `-help`: print usage hints and exit

Then just type as you would usually. Interrupt signal (`C-c`) to end.

## Related programs

If you need to log keys across a whole Linux system (also in the
framebuffer—not just in X11), try [keysniffer][1].  It works via a kernel
module, and needs `sudo`.

If you want to see what characters the user actually typed (with modifier keys,
backspace, etc resolved into text), [`xspy`][2] or [`logkeys`][3] might be
better for you.

If you want to add timestamps to each line for logging purposes, I recommend
piping to the [moreutils package][4]'s `ts`.  [These answers][5] feature
various other tools good for the purpose.

## Versioning

The git-tagged version numbers follow [semver][6].

## License

[ISC][7].


[1]: https://github.com/jarun/keysniffer
[2]: http://www.freshports.org/security/xspy/
[3]: http://code.google.com/p/logkeys/
[4]: http://joeyh.name/code/moreutils/
[5]: http://stackoverflow.com/questions/21564/is-there-a-unix-utility-to-prepend-timestamps-to-lines-of-text
[6]: http://semver.org/
[7]: http://opensource.org/licenses/ISC
