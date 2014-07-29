This program is based Jon A. Maxwell "JAM" `jmaxwell@acm.vt.edu` 's discovery
and proof-of-concept that X11 keyboard state can be logged without superuser
permissions. (I'm not aware of others prior.)

His program `xspy` is an X11 keylogger which output is squarely aimed at
human-readability, which makes it great for (as the name suggests) spying on
someone and quickly making out what they're doing. Close variations of it have
hence featured in security-focused Linux distributions ([in Kali][1] and [in
BlackArch][2], [among others][3]).

This is a complete rework of his idea, using a modern C version and with the
aim of producing machine-readable output better suitable for physical key press
statistics, deprioritising human parseability of the output.


[1]: http://www6.frugalware.org/mirrors/linux/kali/kali/pool/main/x/xspy/
[2]: https://github.com/BlackArch/blackarch/blob/master/packages/xspy/PKGBUILD
[3]: http://www.freshports.org/security/xspy/)
