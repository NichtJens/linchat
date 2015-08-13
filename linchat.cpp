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

#include <iostream>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/poll.h>
#include <sys/types.h>
#include <signal.h>
#include <time.h>

#include "common.h"
#include "userstruct.h"
#include "ui.h"

using std::cerr;
using std::endl;

const int MaxPacket = 1000;
const char * SocketDir = "/tmp/linchatdir";
const char * ScoreboardFilename = "/tmp/linchatdir/scoreboard";
const char * ProgramName = "Linchat 1.0 by Mark Robson";
const char * ShortHelp =
"/q by itself quits. Anything else not beginning with / sends a message";

// Data
static int clientsock = -1;

static struct sockaddr_un bindaddr;
static int scoreboardfd;
static UserData * scoreboard; // MMaped
static int scoreboardslot; // which slot we occupy.
bool quitPending = false;
static bool connected = false;
UserStatus myStatus = UserNormal;
static UserStatus oldStatus = UserNormal;

static bool needCleanup = true;
static time_t lastActiveTime;

static char myusername[MaxUsername];

static void InitSocket()
{
	clientsock = socket(PF_UNIX, SOCK_DGRAM, 0);
	if (clientsock == -1)
		throw MyException("Error creating socket.");
	bindaddr.sun_family = AF_UNIX;
	sprintf(bindaddr.sun_path, "%s/%d", SocketDir, getpid());
	// Delete the stale file (if any)
	unlink(bindaddr.sun_path);
	if (bind(clientsock, (struct sockaddr *) & bindaddr, sizeof(bindaddr)) == -1)
		throw MyException("Bind");
}

static void CleanSocket()
{
	close(clientsock);
	sprintf(bindaddr.sun_path, "%s/%d", SocketDir, getpid());
	unlink(bindaddr.sun_path);
}

static void LockScoreboard(bool dolock)
{
	struct flock lock;
	lock.l_type = dolock ? F_WRLCK : F_UNLCK;
	lock.l_whence = SEEK_SET;
	lock.l_start = 0;
	lock.l_len = 0;
	if (fcntl(scoreboardfd, F_SETLKW, &lock) == -1)
		throw MyException("Lock scoreboard");
}

static void InitScoreboard()
{
	// Make the directory.
	// Try to create it
	scoreboardfd = open(ScoreboardFilename, O_RDWR | O_CREAT | O_EXCL, 0666);
	// If that fails, let's open the existing one.
	if (scoreboardfd == -1) {
		scoreboardfd = open(ScoreboardFilename, O_RDWR);
		if (scoreboardfd == -1)
			// Ah, problem.
			throw MyException("Scoreboard file");
		LockScoreboard(true);
	} else {
		// We just created a new scoreboard, hooray!
		// Initialise it.
		LockScoreboard(true);
		UserData blankuser;
		// Wipe it out.
		memset(& blankuser, 0, sizeof(blankuser));
		blankuser.connected = false;
		for (int i=0; i<ScoreboardSize; i++) {
			ssize_t  nwritten = write(scoreboardfd, &blankuser, sizeof(blankuser));
			if (nwritten != sizeof(blankuser))
				throw MyException("Scoreboard write");
		}
	}
	// Now let's mmap it.
	int sblen = sizeof(UserData) * ScoreboardSize;
	void * ptr = mmap(0, // pointer
			sblen, // length
			PROT_READ | PROT_WRITE, // protection
			MAP_SHARED, // flags
			scoreboardfd, // fd
			0); // offset
	if (ptr == MAP_FAILED)
		throw MyException("mmap");
	scoreboard = (UserData *) ptr;
	// Ok, so far so good. Now add ourselves in.
	int i = 0;
	do {
		if (scoreboard[i].connected == false)
			break;
		i++;
		if (i>ScoreboardSize)
		{
			LockScoreboard(false);
			throw MyException("No free slots");
		}
	} while (true);
	scoreboardslot = i;
	scoreboard[i].connected = true;
	scoreboard[i].status = UserNormal;
	strncpy(myusername, getenv("LOGNAME"), MaxUsername);
	// Truncate it totally.
	myusername[MaxUsername - 1] = '\0';
	strcpy(scoreboard[i].name, myusername);
	scoreboard[i].pid = getpid();
	LockScoreboard(false);
	lastActiveTime = time(0);
}

void CleanScoreboard()
{
	// remove our entry from the sb.
	scoreboard[scoreboardslot].connected = false;
	int sblen = sizeof(UserData) * ScoreboardSize;
	munmap(scoreboard, sblen);
	close(scoreboardfd);
}

static mode_t oldumask;

static void SaveUmask()
{
	oldumask = umask(0);
}

static void RestoreUmask()
{
	umask(oldumask);
}

static void MakeDirectory()
{
	mkdir(SocketDir, 0777);
}

static void SigHandler(int sig)
{
	quitPending = true;
}

static void InitSignals()
{
	signal(SIGINT, SigHandler);
	signal(SIGHUP, SigHandler);
	signal(SIGTERM, SigHandler);
}

void StartUp()
{
	cerr << ProgramName << endl;
	InitSignals();
	SaveUmask();
	MakeDirectory();
	InitSocket();
	InitScoreboard();
	RestoreUmask();
	InitUi();
}

static void UpdateUsers()
{
	ClearUsers();
	for (int i=0; i<ScoreboardSize; i++) {
		if (scoreboard[i].connected)
		{
			char c = '\0';
			switch (scoreboard[i].status)
			{
				case UserIdle: c = 'I';
					break;
				case UserTyping: c = '*';
					break;
				default:
					break;
			}
			AddUser(c, scoreboard[i].name);
		}
	}
	RefreshUsersWindow();
}

static void DisconnectDeadUser(int slot, char *sockpath)
{
	// Kill their slot.
	scoreboard[slot].connected = false;
	// Remove their socket if poss.
	unlink(sockpath);
	// TODO: Tell the other users that they're gone.
	ShowMessage("*** Error: user cannot be contacted, assumed dead:", scoreboard[slot].name);
	UpdateUsers();
}

static void SendPacket(ChatPacket *p)
{
	p->slotnum = scoreboardslot;
	for (int i=0; i<ScoreboardSize; i++) {
		// Don't send ourselves a message.
		if (i == scoreboardslot)
			continue;
		if (scoreboard[i].connected)
		{
			struct sockaddr_un targetaddr;
			targetaddr.sun_family = AF_UNIX;
			sprintf(targetaddr.sun_path, "%s/%d", SocketDir, scoreboard[i].pid);
			int result =
				sendto(clientsock, p, sizeof(ChatPacket), 0,
						(struct sockaddr *) & targetaddr, sizeof(targetaddr));
			if (result == -1)
			{
				// Failure. Handle it sensibly.
				DisconnectDeadUser(i, targetaddr.sun_path);
			}
		}
	}
}

static void SayHello()
{
	ChatPacket p;
	p.cmd = Connect;
	strcpy(p.message, "Hello");
	SendPacket(&p);
	connected = true;
}

static void SayGoodbye()
{
	connected = false;
	ChatPacket p;
	p.cmd = Disconnect;
	strcpy(p.message, "Goodbye");
	SendPacket(&p);
}

void CleanUp()
{
	if (!needCleanup)
		return;
	// Ensure no recursion in case cleanup gets called inside cleanup.
	needCleanup = false;
	FinishUi();
	cerr << "Cleaning up" << endl;
	if (connected)
	{
		scoreboard[scoreboardslot].connected = false;
		try {
			SayGoodbye();
		} catch (...) {
			// Ignore.
		}
	}

	CleanScoreboard();
	CleanSocket();
}

static void ProcessNetworkEvent()
{
	ChatPacket p;
	int result = recv(clientsock, &p, sizeof(p), 0);
	if (result <1)
	{
		throw MyException("Network error");
	}
	if (result < (int) sizeof(p)) {
		// Short datagram. ignore?
		return;
	}
	char buf[MaxMessageSize + 100];
	// Bad things check.
	if ((p.slotnum <0) || (p.slotnum >= ScoreboardSize))
		return;
	// Another bad things check - people who aren't logged on should not be saying anything.
	// But they might have disconnected.
	if (!scoreboard[p.slotnum].connected)
		if (p.cmd != Disconnect)
			return;
	char *name = scoreboard[p.slotnum].name;
	switch(p.cmd)
	{
		case Connect:
			snprintf(buf, sizeof(buf), "%s has arrived (%s)", name, p.message);
			ShowMessage("***", buf);
			UpdateUsers();
			break;
		case Disconnect:
			snprintf(buf, sizeof(buf), "%s has gone (%s)", name, p.message);
			ShowMessage("***", buf);
			UpdateUsers();
			break;
		case Say:
			snprintf(buf, sizeof(buf), "<%s>", name);
			ShowMessage(buf, p.message);
			break;
		case Emote:
			snprintf(buf, sizeof(buf), "%s", name);
			ShowMessage(buf, p.message);
			break;
		case StatusUpdate:
			UpdateUsers();
			break;
		default:
			ShowMessage("*** Error: unknown command (version mismatch?) from :", name);
			break;
	}
	RefreshAll();
}

static void CheckStatus()
{
	if (myStatus == oldStatus)
		return;
	// Set it up in the scoreboard.
	scoreboard[scoreboardslot].status = myStatus;
	// Send a packet to tell people
	ChatPacket p;
	p.cmd = StatusUpdate;
	SendPacket(&p);
	// Update our own display.
	UpdateUsers();
	RefreshAll();
	oldStatus = myStatus;
}

static void CheckIdle()
{
	time_t now;
	now = time(0);
	time_t idletime = now - lastActiveTime;
	if (idletime >= IdleSeconds)
	{
		myStatus = UserIdle;
	} else {
		if (myStatus == UserIdle)
			myStatus = UserNormal;
	}
}

void UnIdle()
{
	lastActiveTime = time(0);
}

static void WaitForEvent()
{
	struct pollfd poll_events[2];
	poll_events[0].fd = 0; // STDIN
	poll_events[0].events = POLLIN;
	poll_events[1].fd = clientsock;
	poll_events[1].events = POLLIN;
	int numevents;
	do {
		numevents = poll(poll_events, 2, 5000);
		if (numevents == -1) {
			if (errno != EINTR)
				throw MyException("Poll");
		}
		// Check if we resize
		if (CheckUiResize())
		{
			RefreshAll();
		}
		// Check if we need to quit now.
		if (quitPending)
			// atexit should take care of cleanup.
			exit(0);
		if (poll_events[0].revents && POLLIN)
		{
			ProcessKeyboardEvent();
		}
		if (poll_events[1].revents && POLLIN)
		{
			ProcessNetworkEvent();
		}
		CheckIdle();
		CheckStatus();
	} while (numevents <1);
}

void DoSay(const char *msg)
{
	// Now send it out to the others.
	ChatPacket p;
	p.cmd = Say;
	strncpy(p.message, msg, sizeof(p.message));
	SendPacket(&p);
	// Print what we said locally.
	char buf[MaxMessageSize + 100];
	snprintf(buf, sizeof(buf), "*%s*", myusername);
	ShowMessage(buf, msg);
	RefreshAll();
}

static void MainLoop()
{
	UpdateUsers();
	SayHello();
	RefreshAll();
	while (!quitPending)
	{
		WaitForEvent();
	}
}

int main(int argc, char *argv[])
{
	try {
		StartUp();
	} catch (MyException &ex) {
		cerr << "Exception starting up: " << ex.excuse << endl;
		return 1;
	}
	atexit(CleanUp);
	try {
		MainLoop();
	} catch (MyException &ex) {
		CleanUp();
		cerr << "Exception: " << ex.excuse << ex.errnum << endl;
		return 1;
	}
}
