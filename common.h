//  Copyright (C) 2003 Mark Robson
//
//  This file is part of Linchat
//
//  Linchat is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307,
//  USA.
//

#ifndef LINCHAT_COMMON_H
#define LINCHAT_COMMON_H

extern const int MaxPacket;
extern const char * SocketDir;
extern bool quitPending;
extern const char * ProgramName;

struct MyException
{
	MyException(const char *s) {
		excuse = s;
		errnum = errno;
	}
	const char *excuse;
	int errnum;
};

void DoSay(const char *msg);
void UnIdle();

#endif  // LINCHAT_COMMON_H
