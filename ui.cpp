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
#include <string>
#include <list>
#include <curses.h>
#include <errno.h>
#include <signal.h>

using namespace std;

#include "ui.h"
#include "common.h"
#include "userstruct.h"

static WINDOW *mainwin = 0;
static WINDOW *userwin = 0;
static WINDOW *entrywin = 0;
// static WINDOW *headingwin = 0;

static string inputline;
static int mainwinw; // Main window width
static int mainwinh;
static int userwinw;
static bool resizePending;
typedef struct {
	char statuschar;
	string name;
} UserListEntry;

static list<UserListEntry> users;
typedef list<UserListEntry>::const_iterator UsersIterator;

// Type for the messages.
typedef struct {
	string prefix;
	string message;
} MessageLine;

static list<MessageLine> messages;
typedef list<MessageLine>::const_iterator MessagesIterator;
const unsigned int MaxMessages = 50;
static void RefreshMainWindow();

void RefreshAll()
{
	/*
	wrefresh(headingwin);
	wrefresh(mainwin);
	wrefresh(userwin);
	wrefresh(entrywin);
	*/
//	wnoutrefresh(headingwin);
	wnoutrefresh(mainwin);
	wnoutrefresh(userwin);
	wnoutrefresh(entrywin);
	doupdate();
}

void RefreshUsersWindow()
{
	// wclear(userwin);
	// whline(userwin,0,userwinw);
	// mvwvline(userwin,1,0,0,mainwinh);
	int row=1;
	for (UsersIterator i = users.begin(); i != users.end(); i++)
	{
		mvwaddstr(userwin,row++, 2 , (*i).name.c_str());
		wclrtoeol(userwin);
	}
	wclrtobot(userwin);
	box(userwin,0,0);
	mvwaddstr(userwin,0,2 , "User list");
	row=1;
	for (UsersIterator i = users.begin(); i != users.end(); i++)
	{
		char c = (*i).statuschar;
		if (c != '\0')
			mvwaddch(userwin, row, 0, c);
		row++;
	}
	/*endwin();
	cout << "Users: " << users.size() << endl;
	throw MyException("Got here");
	*/
}

void ClearUsers()
{
	users.clear();
}

void AddUser(char statuschar, const char *name)
{
	UserListEntry e;
	e.statuschar = statuschar;
	e.name = name;
	users.push_back(e);
}

static void UpdateInput()
{
	// wclear(entrywin);
	wmove(entrywin,0,0);
	wprintw(entrywin, ">> %s", inputline.c_str());
	wclrtobot(entrywin);
	wrefresh(entrywin);
	bool isEmpty = (inputline == "");
	if (isEmpty && (myStatus == UserTyping))
	{
		myStatus = UserNormal;
	}
	if (! isEmpty && (myStatus == UserNormal))
	{
		myStatus = UserTyping;
	}
}

static void DeleteWindows()
{
	if (mainwin != 0)
		delwin(mainwin);
	if (userwin != 0)
		delwin(userwin);
	if (entrywin != 0)
		delwin(entrywin);
//	if (headingwin != 0)
//		delwin(headingwin);
	mainwin = 0;
	userwin = 0;
	entrywin = 0;
//	headingwin = 0;
}

static void ResizeSignal(int x)
{
	resizePending = true;
}

static void SetupWindows()
{
	int maxx, maxy;
	DeleteWindows();
	getmaxyx(stdscr,maxy,maxx);
	userwinw = MaxUsername + 2;
	mainwinh = maxy-2;
	mainwinw = maxx - userwinw;
	mainwin = newwin(mainwinh, mainwinw, 0,0);
	userwin = newwin(mainwinh, userwinw, 0, (maxx - userwinw));
	entrywin = newwin(2, maxx, maxy-2, 0);
	// headingwin = newwin(1, mainwinw, 0,0);
	// wprintw(headingwin, "%s", ProgramName);
	keypad(entrywin,true);
	scrollok(entrywin,true);
	scrollok(mainwin,true);
	wmove(mainwin,mainwinh-1,0);
	// Set up the users window colours
	init_pair(1,COLOR_WHITE, COLOR_BLUE);
	wcolor_set(userwin, 1, 0);
	wattron(userwin,A_BOLD);
	wbkgdset(userwin, COLOR_PAIR(1));
	wclear(userwin);

	RefreshUsersWindow();
	UpdateInput();
	signal(SIGWINCH, ResizeSignal);
}

static void InitCurses()
{
	initscr();
	cbreak();
	noecho();
	start_color();
	refresh();
}

bool CheckUiResize()
{
	if (resizePending)
	{
		DeleteWindows();
		endwin();
		InitCurses();
		SetupWindows();
		RefreshUsersWindow();
		RefreshMainWindow();
		resizePending = false;
		return true; // our caller can RefreshAll
	}
	return false;
}

void InitUi()
{
	InitCurses();
	SetupWindows();
	inputline = "";
	ShowMessage("*** ", ProgramName);
}

void FinishUi()
{
	DeleteWindows();
	endwin();
}

static void ProcessLine()
{
	if (inputline[0] == '/') {
		// Command
		if (inputline[1] == 'q')
			quitPending = true;
		return;
	}
	// Ignore blanks.
	if (inputline.length() == 0)
		return;
	// Otherwise process a say command
	if (((int) inputline.length()) > MaxMessageSize)
	{
		inputline = inputline.substr(0, MaxMessageSize);
	}
	DoSay(inputline.c_str());
}

void ProcessKeyboardEvent()
{
	int k = wgetch(entrywin);
	if ((k >= ' ') && (k < '~'))
	{
		UnIdle();
		// Ascii printable.
		inputline += (char) k;
		UpdateInput();
		return;
	}
	if ((k == KEY_BACKSPACE) || (k == 0x7f))
	{
		UnIdle();
		if (inputline.length() >0)
		{
			inputline = inputline.substr(0,inputline.length()-1);
		}
		UpdateInput();
		return;
	}
	if ((k == KEY_ENTER) || (k == '\n'))
	{
		UnIdle();
		ProcessLine();
		inputline = "";
		UpdateInput();
		return;
	}
	if (k == 21) {
		UnIdle();
		// erase line, ^U
		inputline = "";
		UpdateInput();
		return;
	}
	/*
	if (k == KEY_RESIZE)
	{
		resizeterm(LINES,COLS);
		SetupWindows();
		// wprintw(mainwin,"Resize\nLINES:%d COLS:%d\n", LINES, COLS);
		// wrefresh(mainwin);
		return;
	}
	wprintw(mainwin, "Unknown key: %x\n", k);
	wrefresh(mainwin);
	*/
}

const int MinSplitPos = 5;

static void WrapString(string &str, unsigned int width, unsigned int startAt = 0)
{
	if ((str.length()-startAt) < width) return;
	int splitpos = (startAt + width - 1);
	do {
		if (str[splitpos] == ' ') {
			str[splitpos] = '\n';
			WrapString(str, width, splitpos + 1);
			return;
		}
		splitpos--;
	} while (splitpos > MinSplitPos);
}

static void ShowMessageLine(MessageLine &msgline)
{
	// TODO: make them different colours, etc
	string str(msgline.prefix);
	str += msgline.message; // stick them together to wrap.
	WrapString(str, mainwinw);
	str = str.substr(msgline.prefix.length()); // cut the prefix back off
	// (assuming it hasn't been wraped)
	waddstr(mainwin,"\n"); // Newline
	wattron(mainwin,A_BOLD); // Put the prefix in bold
	waddstr(mainwin,msgline.prefix.c_str());
	wattroff(mainwin,A_BOLD); // put the line in normal
	waddstr(mainwin,str.c_str());
	//wprintw(mainwin, "\n%s", str.c_str());
}

static void RefreshMainWindow()
{
	wclear(mainwin);
	wmove(mainwin,mainwinh-1,0);
	for (MessagesIterator i = messages.begin(); i != messages.end(); i++)
	{
		MessageLine ml = *i;
		ShowMessageLine(ml);
	}
}

void ShowMessage(const char *title, const char *msg)
{
	MessageLine msgline;
	msgline.prefix = title;
	msgline.message = msg;
	ShowMessageLine( msgline);
	// Add it to our buffer.
	messages.push_back(msgline);
	if (messages.size() > MaxMessages)
	{
		messages.pop_front();
	}
}

