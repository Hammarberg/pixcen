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

Sprite::Sprite(int x, int y, int backbuffers)
{
	mode = SPRITE;

	if(x%24!=0)throw _T("X must be divisible by 24");
	if(y%21!=0)throw _T("Y must be divisible by 21");

	xsize=x;
	ysize=y;

	xcell=24;
	ycell=21;

	sizecell = 21 * 3;
	offsetcell = 64;

	int cx = x/24;
	int cy = y/21;

	Create(cx * cy * 64, cx*cy, 0, backbuffers);

	infouse[0].use = InfoUse::INFO_VALUE;
	infouse[0].pp = &background;

	infouse[1].use = InfoUse::INFO_INDEX;
	infouse[1].pp = &color;

	lock[0] = 1;

	DuplicateGlobalsToBackBuffers();

}

Sprite::~Sprite()
{
}

BYTE Sprite::GetPixel(int x, int y)
{
	ASSERT(x>=0 && x<xsize && y>=0 && y<ysize);

	int cx = x/24;
	int cy = y/21;

	int ci = cy * GetCellCountX() + cx;

	int d=(map[64 * (cy * (xsize/24) + cx) + y%21 * 3 + (x/8)%3] >> (1*(7-(x%8)))) & 1;

	BYTE b;
	switch(d)
	{
	case 0:
		b = (*background) & 0x0f;
		break;
	case 1:
		b = color[ci] & 0x0f;
		break;
	}

	return b;
}

void Sprite::SetPixel(int x, int y, BYTE col)
{
	ASSERT(x>=0 && x<xsize && y>=0 && y<ysize);

	int cx = x/24;
	int cy = y/21;

	int ci = cy * GetCellCountX() + cx;

	int mask = ResolveMask1(ci, col, GetMask(x,y));
	if(mask == -1)
		return;

	SetColor(ci, mask, col);

	int si = 64 * (cy * (xsize/24) + cx) + y%21 * 3 + (x/8)%3;

	int filtermask = ~(1 << (1*(7-(x%8))));
	mask = mask << (1*(7-(x%8)));
	map[si] = (map[si] & filtermask) | mask;
}

int Sprite::GetMask(int x, int y)
{
	int cx = x/22;
	int cy = y/21;

	return (map[64 * (cy * (xsize/24) + cx) + y%21 * 3 + (x/8)%3] >> (1*(7-(x%8)))) & 1;
}

void Sprite::GetLoadFormats(narray<autoptr<SaveFormat>,int> &fmt)
{
}

void Sprite::GetSaveFormats(narray<autoptr<SaveFormat>,int> &fmt)
{
	__super::GetSaveFormats(fmt);

	//fmt.add(new SaveFormat(_T("Raw sprites"),_T("bin"),false));
}

void Sprite::Load(nmemfile &file, LPCTSTR type, int version)
{
	__super::Load(file,type,version);

	if(lstrcmpi(_T("bin"),type)==0)
	{
		size_t len = file.len();

		if(len % 64 != 0)
		{
			//Throw PRG header
			unsigned short tmp;
			file >> tmp;
			len -= 2;
		}

		int num = (((len+63)/64)+7)&~7;

		xsize = 24*8;
		ysize = 21 * (num/8);

		Destroy();
		Create(num * 64, 3, 0);

		*background = 0;
		color[0] = 11;
		color[1] = 12;
		color[2] = 1;

		file.read(map, len);
	}

}


void Sprite::Save(nmemfile &file, LPCTSTR type)
{
	__super::Save(file,type);

	if(lstrcmpi(_T("bin"),type)==0)
	{
		file.write(map, rmapsize);
	}
}

void Sprite::Import(CImage &img)
{
	ClearBackBuffer();

	{
		ImportHelper help(this, img, false);

		BYTE top[16];
		int num = help.CountTopColorsPerCell(top);

		*background = top[0];

		help.ReduceColors(2, top, num < 1 ? num : 1);
	}

	*border = GuessBorderColor();
}

//---------------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------

MCSprite::MCSprite(int x, int y, int backbuffers)
{
	mode = MC_SPRITE;

	if(x%12!=0)throw _T("X must be divisible by 24");
	if(y%21!=0)throw _T("Y must be divisible by 21");

	xsize=x;
	ysize=y;

	xcell=12;
	ycell=21;

	sizecell = 21 * 3;
	offsetcell = 64;

	int cx = x/12;
	int cy = y/21;

	Create(cx * cy * 64, cx*cy, 0, backbuffers);

	infouse[0].use = InfoUse::INFO_VALUE;
	infouse[0].pp = &background;

	infouse[1].use = InfoUse::INFO_VALUE;
	infouse[1].pp = &ext0;

	infouse[2].use = InfoUse::INFO_INDEX;
	infouse[2].pp = &color;

	infouse[3].use = InfoUse::INFO_VALUE;
	infouse[3].pp = &ext1;

	lock[0] = 1;

	DuplicateGlobalsToBackBuffers();

}

MCSprite::~MCSprite()
{
}

BYTE MCSprite::GetPixel(int x, int y)
{
	ASSERT(x>=0 && x<xsize && y>=0 && y<ysize);

	int cx = x/12;
	int cy = y/21;

	int ci = cy * GetCellCountX() + cx;

	int si = 64 * (cy * (xsize/12) + cx) + y%21 * 3 + (x/4)%3;

	int d=(map[si] >> (2*(3-(x%4)))) & 3;

	BYTE b;
	switch(d)
	{
	case 0:
		b = *background;
		break;
	case 1:
		b = ext[0];
		break;
	case 2:
		b = color[ci];
		break;
	case 3:
		b = ext[1];
		break;
	}

	return b;

}

void MCSprite::SetPixel(int x, int y, BYTE col)
{
	ASSERT(x>=0 && x<xsize && y>=0 && y<ysize);

	int cx = x/12;
	int cy = y/21;

	int ci = cy * GetCellCountX() + cx;

	int mask = ResolveMask2(ci, col, GetMask(x,y));
	if(mask == -1)
		return;

	SetColor(ci, mask, col);

	int mi = 64 * (cy * (xsize/12) + cx) + y%21 * 3 + (x/4)%3;

	int filtermask = ~(3 << (2*(3-(x%4))));
	mask = mask << (2*(3-(x%4)));
	map[mi] = (map[mi] & filtermask) | mask;
}

int MCSprite::GetMask(int x, int y)
{
	int cx = x/12;
	int cy = y/21;

	return (map[64 * (cy * (xsize/12) + cx) + y%21 * 3 + (x/4)%3] >> (2*(3-(x%4)))) & 3;
}


void MCSprite::GetLoadFormats(narray<autoptr<SaveFormat>,int> &fmt)
{
}

void MCSprite::GetSaveFormats(narray<autoptr<SaveFormat>,int> &fmt)
{
	__super::GetSaveFormats(fmt);

	//fmt.add(new SaveFormat(_T("Raw sprites"),_T("bin"),false));
}

void MCSprite::Load(nmemfile &file, LPCTSTR type, int version)
{
	__super::Load(file,type,version);

	if(lstrcmpi(_T("bin"),type)==0)
	{
		size_t len = file.len();

		if(len % 64 != 0)
		{
			//Throw PRG header
			unsigned short tmp;
			file >> tmp;
			len -= 2;
		}

		int num = (((len+63)/64)+7)&~7;

		xsize = 12*8;
		ysize = 21 * (num/8);

		Destroy();
		Create(num * 64, 3, 0);

		*background = 0;
		color[0] = 11;
		color[1] = 12;
		color[2] = 1;

		file.read(map, len);
	}

}


void MCSprite::Save(nmemfile &file, LPCTSTR type)
{
	__super::Save(file,type);

	if(lstrcmpi(_T("bin"),type)==0)
	{
		file.write(map, rmapsize);
	}
}

void MCSprite::Import(CImage &img)
{
	ClearBackBuffer();

	{
		ImportHelper help(this, img, true);

		BYTE top[16];
		int num = help.CountTopColorsPerCell(top);

		*background = top[0];
		if (num >= 2)ext[0] = top[1];
		if (num >= 3)ext[1] = top[2];

		PushLocks();
		lock[0] = lock[1] = lock[3] = 1;
		lock[3] = lock[4] = lock[5] = 0;

		help.ReduceColors(4, top, num < 3 ? num : 3);

	}
	PopLocks();

	*border = GuessBorderColor();
}

