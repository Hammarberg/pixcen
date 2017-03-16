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
#include <cstdint>
#include <algorithm>

#define PALETTECOUNT (sizeof(s_Pallete)/sizeof(COLORREF[16]))

static const COLORREF s_Pallete[][16]={

	//default
	{
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
	},
	//Pepto
	{
		RGB2REF(0x000000),
		RGB2REF(0xFFFFFF),
		RGB2REF(0x68372B),
		RGB2REF(0x70A4B2),
		RGB2REF(0x6F3D86),
		RGB2REF(0x588D43),
		RGB2REF(0x352879),
		RGB2REF(0xB8C76F),
		RGB2REF(0x6F4F25),
		RGB2REF(0x433900),
		RGB2REF(0x9A6759),
		RGB2REF(0x444444),
		RGB2REF(0x6C6C6C),
		RGB2REF(0x9AD284),
		RGB2REF(0x6C5EB5),
		RGB2REF(0x959595)
	},
	//c64hq
	{
		RGB2REF(0x0A0A0A),
		RGB2REF(0xFFF8FF),
		RGB2REF(0x851F02),
		RGB2REF(0x65CDA8),
		RGB2REF(0xA73B9F),
		RGB2REF(0x4DAB19),
		RGB2REF(0x1A0C92),
		RGB2REF(0xEBE353),
		RGB2REF(0xA94B02),
		RGB2REF(0x441E00),
		RGB2REF(0xD28074),
		RGB2REF(0x464646),
		RGB2REF(0x8B8B8B),
		RGB2REF(0x8EF68E),
		RGB2REF(0x4D91D1),
		RGB2REF(0xBABABA)
	},
	//c64s
	{
		RGB2REF(0x000000),
		RGB2REF(0xFCFCFC),
		RGB2REF(0xA80000),
		RGB2REF(0x54FCFC),
		RGB2REF(0xA800A8),
		RGB2REF(0x00A800),
		RGB2REF(0x0000A8),
		RGB2REF(0xFCFC00),
		RGB2REF(0xA85400),
		RGB2REF(0x802C00),
		RGB2REF(0xFC5454),
		RGB2REF(0x545454),
		RGB2REF(0x808080),
		RGB2REF(0x54FC54),
		RGB2REF(0x5454FC),
		RGB2REF(0xA8A8A8)
	},
	//ccs64
	{
		RGB2REF(0x101010),
		RGB2REF(0xFFFFFF),
		RGB2REF(0xE04040),
		RGB2REF(0x60FFFF),
		RGB2REF(0xE060E0),
		RGB2REF(0x40E040),
		RGB2REF(0x4040E0),
		RGB2REF(0xFFFF40),
		RGB2REF(0xE0A040),
		RGB2REF(0x9C7448),
		RGB2REF(0xFFA0A0),
		RGB2REF(0x545454),
		RGB2REF(0x888888),
		RGB2REF(0xA0FFA0),
		RGB2REF(0xA0A0FF),
		RGB2REF(0xC0C0C0)
	},
	//frodo
	{
		RGB2REF(0x000000),
		RGB2REF(0xFFFFFF),
		RGB2REF(0xCC0000),
		RGB2REF(0x00FFCC),
		RGB2REF(0xFF00FF),
		RGB2REF(0x00CC00),
		RGB2REF(0x0000CC),
		RGB2REF(0xFFFF00),
		RGB2REF(0xFF8800),
		RGB2REF(0x884400),
		RGB2REF(0xFF8888),
		RGB2REF(0x444444),
		RGB2REF(0x888888),
		RGB2REF(0x88FF88),
		RGB2REF(0x8888FF),
		RGB2REF(0xCCCCCC)
	},
	//godot
	{
		RGB2REF(0x000000),
		RGB2REF(0xFFFFFF),
		RGB2REF(0x880000),
		RGB2REF(0xAAFFEE),
		RGB2REF(0xCC44CC),
		RGB2REF(0x00CC55),
		RGB2REF(0x0000AA),
		RGB2REF(0xEEEE77),
		RGB2REF(0xDD8855),
		RGB2REF(0x664400),
		RGB2REF(0xFE7777),
		RGB2REF(0x333333),
		RGB2REF(0x777777),
		RGB2REF(0xAAFF66),
		RGB2REF(0x0088FF),
		RGB2REF(0xBBBBBB)
	},
	//pc64
	{
		RGB2REF(0x212121),
		RGB2REF(0xFFFFFF),
		RGB2REF(0xB52121),
		RGB2REF(0x73FFFF),
		RGB2REF(0xB521B5),
		RGB2REF(0x21B521),
		RGB2REF(0x2121B5),
		RGB2REF(0xFFFF21),
		RGB2REF(0xB57321),
		RGB2REF(0x944221),
		RGB2REF(0xFF7373),
		RGB2REF(0x737373),
		RGB2REF(0x949494),
		RGB2REF(0x73FF73),
		RGB2REF(0x7373FF),
		RGB2REF(0xB5B5B5),
	},
};

static const TCHAR s_PaletteName[][16]={
	_T("default"),
	_T("pepto"),
	_T("c64hq"),
	_T("c64s"),
	_T("ccs64"),
	_T("frodo"),
	_T("godot"),
	_T("pc64"),
	_T("")
};

static const TCHAR s_VICColourName[][16] = {
	_T("Black"),
	_T("White"),
	_T("Red"),
	_T("Cyan"),
	_T("Purple"),
	_T("Green"),
	_T("Blue"),
	_T("Yellow"),
	_T("Orange"),
	_T("Brown"),
	_T("Pink"),
	_T("D.Grey"),
	_T("M.Grey"),
	_T("L.Green"),
	_T("L.Blue"),
	_T("L.Grey")
};

const COLORREF *g_Vic2 = &s_Pallete[0][0];
//const TCHAR* g_VicColourNames = s_VICColourName[0];

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
	int best = INT_MAX;
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

int GetPalette(void)
{
	return int((uintptr_t(g_Vic2)-uintptr_t(&s_Pallete[0][0]))/sizeof(COLORREF[16]));
}

int SetPalette(int n)
{
	int old = GetPalette();
	g_Vic2 = &s_Pallete[n][0];
	return old;
}

LPCTSTR GetPaletteName(int n)
{
	return s_PaletteName[n];
}

int ClosestPalette(const COLORREF *list, int num)
{
	int old = GetPalette();

	int palhit[PALETTECOUNT]={0};

	for(int t=0;t<num;t++)
	{
		for(int r=0;r<PALETTECOUNT;r++)
		{
			int best = INT_MAX;
			SetPalette(r);

			for(int u=0;u<16;u++)
			{
				int dist = ColourDistance(list[t], g_Vic2[u]);

				if(dist<best)
				{
					best = dist;
				}
			}

			palhit[r]+=best;
		}
	}

	int best = INT_MAX;
	int index = 0;

	for(int r=0;r<PALETTECOUNT;r++)
	{
		if(best>palhit[r])
		{
			best = palhit[r];
			index = r;
		}
	}


	SetPalette(old);

	return index;
}
