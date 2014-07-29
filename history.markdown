This program is based Jon A. Maxwell "JAM" `jmaxwell@acm.vt.edu` 's discovery
that X11 keyboard state can be logged without superuser permissions.

His program `xspy` is an X11 keylogger which output is squarely aimed at
human-readability, which makes it great for (as the name suggests) spying on
someone and quickly making out what they're doing. It has hence been included
in security-focused Linux distributions (BackTrack and BlackArch, at least).

This is a complete rework of his idea, using a modern C version and with the
aim of producing machine-readable output better suitable for physical key press
statistics, deprioritising human parseability of the output.
