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


#include "StdAfx.h"
#include "C64Col.h"
//#include <cmath>

const COLORREF g_Vic2[16]={
		RGB2REF(0x000000),
		RGB2REF(0xffffff),
		RGB2REF(0x894036),
		RGB2REF(0x7abfc7),
		RGB2REF(0x8a46ae),
		RGB2REF(0x68a941),
		RGB2REF(0x3e31a2),
		RGB2REF(0xd0dc71),
		RGB2REF(0x905f25),
		RGB2REF(0x5c4700),
		RGB2REF(0xbb776d),
		RGB2REF(0x555555),
		RGB2REF(0x808080),
		RGB2REF(0xacea88),
		RGB2REF(0x7c70da),
		RGB2REF(0xababab)
	};

//http://www.compuphase.com/cmetric.htm

static int ColourDistance(COLORREF c1, COLORREF c2)
{
	int rmean = ( (int)REF2R(c1) + (int)REF2R(c2) ) / 2;
	int r = (int)REF2R(c1) - (int)REF2R(c2);
	int g = (int)REF2G(c1) - (int)REF2G(c2);
	int b = (int)REF2B(c1) - (int)REF2B(c2);
	return ((((512+rmean)*r*r)>>8) + 4*g*g + (((767-rmean)*b*b)>>8));
}

int ClosestMatch(COLORREF c , const COLORREF *list, int num)
{
	int best = 1000000;
	int index=0;

	for(int t=0;t<num;t++)
	{
		int dist = ColourDistance(c, list[t]);

		if(dist<best)
		{
			best = dist;
			index = t;
		}
	}

	return BYTE(index);
}

BYTE ToC64Index(COLORREF c)
{
	return ClosestMatch(c, g_Vic2, 16);
}

int ClosestMatch(COLORREF c, const BYTE *list, int num)
{
	COLORREF rgb[16];
	for(int r=0;r<num;r++)
		rgb[r]=g_Vic2[list[r]];

	return ClosestMatch(c,rgb,num);
}
