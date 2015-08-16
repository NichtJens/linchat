# linchat
A simple local Linux / Unix chat program

---

### Linchat 1.0 by Mark Robson

Originally hosted at:

http://vectrex.org.uk/linchat

http://freecode.com/projects/linchat

For the original README click [here](README.orig)

For the LICENSE (GPLv2) click [here](LICENSE)

---

This is a very handy little program.
However, the original website is not reachable anymore.

The version hosted here contains several fixes for deprecated syntax, etc.
It was tested to compile and run fine with gcc (up to) 4.8.2.
It seems, the original was written for gcc 3.0 ...

---

## New feature: COLORS!

The user names are now colored according to their respective slot number.

However, ncurses knows only 8 colors (one of which is equal to the background color), hence after 7 users the colors will repeat.

