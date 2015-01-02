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
#include "C64Interface.h"

Unrestricted::Unrestricted(int x, int y, bool wide, int backbuffers)
{
	if(wide)
	{
		mode = W_UNRESTRICTED;
		xcell=4;
	}
	else
	{
		mode = UNRESTRICTED;
		xcell=8;
	}

	xsize=x;
	ysize=y;

	ycell=8;

	Create(x*y, 0, 0, backbuffers);

}

Unrestricted::~Unrestricted()
{
}

void Unrestricted::GetSaveFormats(narray<autoptr<SaveFormat>,int> &fmt)
{
	__super::GetSaveFormats(fmt);

	//fmt.add(new SaveFormat(_T("Assembler"),_T("s")));
	//fmt.add(new SaveFormat(_T("Binary"),_T("bin"),false));
}

void Unrestricted::GetLoadFormats(narray<autoptr<SaveFormat>,int> &fmt)
{
}

void Unrestricted::Save(nmemfile &file, LPCTSTR type)
{
	if(lstrcmpi(_T("map"),type)==0)
	{
		BYTE b;
		int x,y;
		for(y=0;y<ysize;y++)
		{
			for(x=0;x<xsize;x++)
			{
				b=GetPixel(x,y)&0x0f;
				b = b | (b<<4);
				file << b;
			}
		}
	}
	else __super::Save(file,type);
}

BYTE Unrestricted::GetPixel(int x, int y)
{
	ASSERT(x>=0 && x<xsize && y>=0 && y<ysize);
	return map[y*xsize+x];
}

void Unrestricted::SetPixel(int x, int y, BYTE col)
{
	ASSERT(x>=0 && x<xsize && y>=0 && y<ysize);
	map[y*xsize+x] = (col&0x0f);
}

void Unrestricted::GetCellInfo(int cx, int cy, int w, int h, CellInfo *info)
{
	info->h = 8;
	info->w = 8 / GetPixelWidth();
	info->col[0]=info->col[1]=info->col[2]=info->col[3]=info->col[4]=0xff;

	C64Interface::GetCellInfo(cx, cy, w, h, info);
}

void Unrestricted::Import(CImage &img, bool foreceLocks)
{
	ClearBackBuffer(foreceLocks);

	ImportHelper help(img, xsize, ysize, xcell, ycell, mode == W_UNRESTRICTED ? true : false);

	int x,y;

	for(y=0;y<ysize;y++)
	{
		for(x=0;x<xsize;x++)
		{
			SetPixel(x, y, help.GetPixel(x,y));
		}
	}

	if(!foreceLocks)
		*border = GuessBorderColor();
}
