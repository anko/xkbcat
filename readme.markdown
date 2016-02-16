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

## Usage

Flags you can pass (all optional):

 - `-display <display>`: set target X display (default `:0`)
 - `-delay <nanosec>`: delay between polls to the keyboard (default `10000000`;
   that's 100ms)
 - `-up`: also prepend key-ups (default: don't)
 - `-help`: print usage hints and exit

Then just type as you would usually. Interrupt signal (`C-c`) to end.

## Related programs

If you like spying on people (ethically, of course), [`xspy`][1] or
[`logkeys`][2] might be better for you. They use the modifier keys to infer
what was actually typed, so it's easier to read what's happening.

If you want to add timestamps to each line for logging purposes, I recommend
piping to the [moreutils package][3]'s `ts`.  [These answers][4] feature
various other tools good for the purpose.

## Versioning

The git-tagged version numbers follow [semver][5].

## License

[ISC][6].


[1]: http://www.freshports.org/security/xspy/
[2]: http://code.google.com/p/logkeys/
[3]: http://joeyh.name/code/moreutils/
[4]: http://stackoverflow.com/questions/21564/is-there-a-unix-utility-to-prepend-timestamps-to-lines-of-text
[5]: http://semver.org/
[6]: http://opensource.org/licenses/ISC
