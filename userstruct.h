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

extern const char * ScoreboardFilename;
const int ScoreboardSize = 30;
const int MaxUsername = 14;
const int IdleSeconds = 300; // Set to 5 or 10 for debug.

typedef enum {
	Connect,
	Disconnect,
	Say,
	Emote,
	StatusUpdate
} ChatCommand;

typedef enum {
	UserNormal,
	UserIdle,
	UserTyping
} UserStatus;
extern UserStatus myStatus;

typedef struct {
	bool connected;
	UserStatus status;
	int pid; // So we can check etc
	char name[MaxUsername];
} UserData;

const int MaxMessageSize = 900;

typedef struct {
	int slotnum; // Sender
	ChatCommand cmd;
	char message[MaxMessageSize];
} ChatPacket;
