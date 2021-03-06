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

#ifndef LINCHAT_UI_H
#define LINCHAT_UI_H

#include "colorpairs.h"

void InitUi();
void FinishUi();
void RefreshAll();
void RefreshUsersWindow();
void ClearUsers();
void AddUser(char statuschar, const char *name);
void ProcessKeyboardEvent();
void ShowMessage(const char *title, const char *msg, COLOR_TYPE title_color=STD_FG_COLOR, COLOR_TYPE msg_color=STD_FG_COLOR);
bool CheckUiResize();

#endif  // LINCHAT_UI_H
