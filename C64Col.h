/*
   Pixcen - A windows platform low level pixel editor for C64
   Copyright (C) 2013  John Hammarberg (crt@nospam.censordesign.com)
   
    This file is part of Pixcen.

    Pixcen is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Pixcen is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Pixcen.  If not, see <http://www.gnu.org/licenses/>.
*/


#pragma once

#define RGB2REF(x)  (((x&0xff0000) >> 16) | (x & 0x00ff00) | ((x&0x0000ff) << 16))

extern const COLORREF g_Vic2[16];

int ClosestMatch(COLORREF c , const COLORREF *list, int num);
BYTE ToC64Index(COLORREF c);

int ClosestMatch(COLORREF c, const BYTE *list, int num);

static COLORREF blend(COLORREF aa, COLORREF bb)
{
	BYTE a = (((aa)&0xff) + ((bb)&0xff)) / 2;
	BYTE b = (((aa>>8)&0xff) + ((bb>>8)&0xff)) / 2;
	BYTE c = (((aa>>16)&0xff) + ((bb>>16)&0xff)) / 2;

	return COLORREF((c << 16) | (b << 8) | a);
}
