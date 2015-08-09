#  Copyright (C) 2003 Mark Robson
#
#  This file is part of Linchat
#
#  Linchat is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,
#  USA.
#

OBJECTS=linchat.o ui.o

HEADERS=ui.h common.h userstruct.h
# Debug build
# CCPLUS=g++-3.0
# CFLAGS=-g -Wall -Werror
# STRIPFLAGS=
# Production build
CCPLUS=g++
CFLAGS=-O -Wall -Werror -Wno-write-strings -Wno-unused-result
STRIPFLAGS=-Wl,-s
#
LDFLAGS=-lcurses

linchat: $(OBJECTS) $(HEADERS) Makefile
	$(CCPLUS) -o linchat $(OBJECTS) $(LDFLAGS) $(STRIPFLAGS)

.cpp.o: $(HEADERS) Makefile
	$(CCPLUS) $(CFLAGS) -c $<

clean:
	rm -f $(OBJECTS)
	rm -f linchat 
