/*
Pixcen - A windows platform low level pixel editor for C64
Copyright (C) 2013  John Hammarberg (crt@nospam.binarybone.com)

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
#include "Romfont.h"

CommonFont::CommonFont(void)
{
	fontdisplay = FONT_1X1;
	charcount=0;
	thread=(HANDLE)_beginthreadex(NULL,0,threadentry,(void *)this,0,NULL);
}

CommonFont::~CommonFont()
{
	quit.SetEvent();
	WaitForSingleObject(thread,INFINITE);
	CloseHandle(thread);
}

unsigned int CommonFont::threadentry( LPVOID pParam )
{
	CommonFont *p=(CommonFont *)pParam;

	while(WaitForSingleObject(p->quit,2000)==WAIT_TIMEOUT)
	{
		narray<BYTE,int> c,s;
		p->charcount=p->CreateCharScreen(c,s);
	}

	return 0;
}

int CommonFont::CreateCharScreen(narray<BYTE,int> &c, narray<BYTE,int> &s, bool space)
{
	while(!map)
	{
		Sleep(1000);
	}

	int spos,smax=rmapsize/8,r;
	int dpos,dmax=0;

	c.count(0);
	s.count(smax);

	for(spos=0;spos<smax;spos++)
	{
		BYTE *dest=c.getarray();
		dmax=c.count()/8;

		for(dpos=0;dpos<dmax;dpos++)
		{
			for(r=7;r>=0;r--)
			{
				if(map[spos*8+r]!=dest[dpos*8+r])
					goto next;
			}

			goto found;

			next:;
		}

		if(dpos==dmax)
		{
			for(r=0;r<8;r++)
			{
				c.add(map[spos*8+r]);
			}
		}

found:	
		s[spos]=dpos;
	}

	if(space && c.count()/8 > 0x20)
	{
		//Find space char
		BYTE *cc=c.getarray();
		for(int r=0;r<c.count()/8;r++)
		{
			int l;
			for(l=0;l<8;l++)
			{
				if(cc[r*8+l]!=0)break;
			}

			if(l==8)
			{
				//Swap witch char at space position
				for(l=0;l<8;l++)
				{
					BYTE t=cc[32*8+l];
					cc[32*8+l]=cc[r*8+l];
					cc[r*8+l]=t;
				}

				for(l=0;l<s.count();l++)
				{
					if(s[l] == 32)
					{
						s[l]=BYTE(r);
					}
					else if(s[l] == BYTE(r))
					{
						s[l]=32;
					}
				}

				break;
			}
		}
	}

	return c.count()/8;
}

void CommonFont::GetSaveFormats(narray<autoptr<SaveFormat>,int> &fmt)
{
	__super::GetSaveFormats(fmt);

	if(charcount<=256)
	{
		fmt.add(new SaveFormat(_T("Index screen"),_T("iscr"),false));
		fmt.add(new SaveFormat(_T("PRG Index screen"),_T("piscr"),false));
		fmt.add(new SaveFormat(_T("Indexed char map"),_T("imap"),false));
		fmt.add(new SaveFormat(_T("PRG Indexed char map"),_T("pimap"),false));
	}

}

void CommonFont::Save(nmemfile &file, LPCTSTR type)
{

	__super::Save(file,type);

	if(lstrcmpi(_T("iscr"),type)==0)
	{
		narray<BYTE,int> c,s;
		if(CreateCharScreen(c,s,true)>256)
			throw _T("Too many characters");

		file.write(s.getarray(), s.count());
	}
	else if(lstrcmpi(_T("piscr"),type)==0)
	{
		file << (unsigned short)GetMetaInt("piscr");

		narray<BYTE,int> c,s;
		if(CreateCharScreen(c,s,true)>256)
			throw _T("Too many characters");

		file.write(s.getarray(), s.count());
	}
	else if(lstrcmpi(_T("imap"),type)==0)
	{
		narray<BYTE,int> c,s;
		if(CreateCharScreen(c,s,true)>256)
			throw _T("Too many characters");

		file.write(c.getarray(), c.count());
	}
	else if(lstrcmpi(_T("pimap"),type)==0)
	{
		file << (unsigned short)GetMetaInt("pimap");

		narray<BYTE,int> c,s;
		if(CreateCharScreen(c,s,true)>256)
			throw _T("Too many characters");

		file.write(c.getarray(), c.count());
	}
}

void CommonFont::Load(nmemfile &file, LPCTSTR type, int version)
{
	__super::Load(file,type,version);

	fontdisplay = (tfontdisplay)GetMetaInt("fontdisplay");
}

void CommonFont::SetFontDisplay(int mode)
{
	SetMetaInt("fontdisplay", fontdisplay = (tfontdisplay)mode);
}

void CommonFont::TranslateFontDisplay(int &cx, int &cy)
{
	switch (fontdisplay)
	{
	case FONT_1X1:
		break;
	case FONT_1X2:
		{
			int c = cx + (cy / 2) * GetCellCountX();
			cx = (c & 63) + (c/64) * 128 + (cy & 1) * 64;
			cy = 0;
		}
	break;
	case FONT_2X1:
		{
			int c = (cx / 2) + cy * GetCellCountX() / 2;
			cx = (c & 63) + (c / 64) * 128 + (cx & 1) * 64;
			cy = 0;
		}
		break;
	case FONT_2X2:
		{
			int c = (cx / 2) + (cy / 2) * GetCellCountX() / 2;
			cx = c  + (cx & 1) * 64 + (cy & 1) * 128;
			cy = 0;
		}
		break;
	default:
		cx = 0;
		cy = 0;
		assert(false);
	}

}

//---------------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------

MCFont::MCFont(int x, int y, int backbuffers)
{
	mode = MC_CHAR;

	if(x%4!=0)throw _T("X must be divisible by 4");
	if(y%8!=0)throw _T("Y must be divisible by 8");

	xsize=x;
	ysize=y;

	xcell=4;
	ycell=8;

	Create(y * (x/4), (y/8) * (x/4), 0, backbuffers);

	infouse[0].use = InfoUse::INFO_VALUE;
	infouse[0].pp = &background;

	infouse[1].use = InfoUse::INFO_VALUE;
	infouse[1].pp = &ext0;

	infouse[2].use = InfoUse::INFO_VALUE;
	infouse[2].pp = &ext1;

	infouse[3].use = InfoUse::INFO_INDEX;
	infouse[3].pp = &color;

	lock[0] = 1;

	DuplicateGlobalsToBackBuffers();

}

MCFont::~MCFont()
{
}

void MCFont::Import(CImage &img)
{
	ClearBackBuffer();

	{
		ImportHelper help(this, img, true);

		BYTE top[16];
		int num = help.CountTopColorsPerCell(top);

		*background = top[0];
		//if(num>=2)ext[0]=top[1];
		//if(num>=3)ext[1]=top[2];

		//PushLocks();
		//lock[0]=lock[1]=lock[2]=1;
		//lock[3]=lock[4]=lock[5]=0;

		help.ReduceColors(4, top, num < 3 ? num : 3);

		//PopLocks();
	}

	*border = GuessBorderColor();
}

void MCFont::GetSaveFormats(narray<autoptr<SaveFormat>,int> &fmt)
{
	__super::GetSaveFormats(fmt);

	if(xsize == 160 && ysize == 200)
		fmt.add(new SaveFormat(_T("C64 Exe"),_T("prg"),false));
}

void MCFont::GetLoadFormats(narray<autoptr<SaveFormat>,int> &fmt)
{
}

void MCFont::Load(nmemfile &file, LPCTSTR type, int version)
{
	__super::Load(file,type,version);

	if(lstrcmpi(_T("bin"),type)==0)
	{
		size_t len = file.len();

		if(len % 8 != 0)
		{
			//Throw PRG header
			unsigned short tmp;
			file >> tmp;
			len -= 2;
		}

		xsize = 32*4;
		ysize = int(((len+255)/256)*8);

		Destroy();
		Create(ysize * (xsize/4), 3, 0);

		*background = 0;
		color[0] = 11;
		color[1] = 12;
		color[2] = 1;

		file.read(map, len);
	}

}

void MCFont::Save(nmemfile &file, LPCTSTR type)
{
	__super::Save(file,type);

	if(lstrcmpi(_T("prg"),type)==0)
	{
		if(xsize!=160 || ysize!=200)
			throw _T("Dimensions must be 160*200");

		narray<BYTE,int> c,s;
		if(CreateCharScreen(c,s,true)>256)
			throw _T("Too many characters");

		ASSERT(s.count() == 1000);

		static unsigned char viewer[]={
			0x00,0x1f,0x78,0xad,0x42,0x1f,0x8d,0x20,0xd0,0xad,0x43,0x1f,0x8d,0x21,0xd0,0xad,
			0x44,0x1f,0x8d,0x22,0xd0,0xad,0x45,0x1f,0x8d,0x23,0xd0,0xa9,0x8a,0x8d,0x18,0xd0,
			0xa9,0x18,0x8d,0x16,0xd0,0xa2,0x00,0xbd,0x00,0x24,0x9d,0x00,0xd8,0xbd,0x00,0x25,
			0x9d,0x00,0xd9,0xbd,0x00,0x26,0x9d,0x00,0xda,0xbd,0x00,0x27,0x9d,0x00,0xdb,0xe8,
			0xd0,0xe5,0xf0,0xfe};

		file.write(viewer,sizeof(viewer));

		file << *border;
		file << *background;
		file << ext[0];
		file << ext[1];

		while(file.len() < (256+2))
			file << BYTE(0);

		file.write(s.getarray(),1000);

		while(file.len() < (256+2+0x0400))
			file << BYTE(0);

		for(int r=0;r<1000;r++)
		{
			assert(color[r]<8);
			file << BYTE((color[r]&7) | 8);
		}

		while(file.len() < (256+2+0x0800))
			file << BYTE(0);

		file.write(c.getarray(),c.count());

		ByteBoozer::File bbin, bbout;
		bbin.size = file.len();
		bbin.data = (byte *)file.detach();

		ByteBoozer::crunch(&bbin, &bbout, 0x1f00, ByteBoozer::normalDecr, false, false);
		ByteBoozer::freeFile(&bbin);

		file.attach(bbout.data, bbout.size, true);
	}
}

BYTE MCFont::GetPixel(int x, int y)
{
	ASSERT(x>=0 && x<xsize && y>=0 && y<ysize);

	int cx = x / 4;
	int cy = y / 8;

	TranslateFontDisplay(cx, cy);

	int d=(map[cy * xsize*2 + cx * 8 + (y%8)] >> (2*(3-(x%4)))) & 3;

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
		b = ext[1];
		break;
	case 3:
		b = color[GetCellCountX() * cy + cx];
		break;
	}

	return b;
}

void MCFont::SetPixel(int x, int y, BYTE col)
{
	ASSERT(x>=0 && x<xsize && y>=0 && y<ysize);

	int cx = x/4;
	int cy = y/8;

	TranslateFontDisplay(cx, cy);

	int ci = cy * (xsize/4) + cx;

	int mask = ResolveMask2(ci, col, GetMask(x,y), true);
	if(mask == -1)
		return;

	SetColor(ci, mask, col);

	int mi = cy * xsize*2 + cx * 8 + (y%8);

	int filtermask = ~(3 << (2*(3-(x%4))));
	mask = mask << (2*(3-(x%4)));
	map[mi] = (map[mi] & filtermask) | mask;
}

int MCFont::GetMask(int x, int y)
{
	int cx = x/4;
	int cy = y/8;

	return (map[cy * xsize*2 + cx * 8 + (y%8)] >> (2*(3-(x%4)))) & 3;
}


//---------------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------------------------


SFont::SFont(int x, int y, int backbuffers)
{
	mode = CHAR;

	if(x%8!=0)throw _T("X must be divisible by 8");
	if(y%8!=0)throw _T("Y must be divisible by 8");

	xsize=x;
	ysize=y;

	xcell=8;
	ycell=8;
	
	Create(y * (x/8), (x/8)*(y/8), 0, backbuffers);

	infouse[0].use = InfoUse::INFO_VALUE;
	infouse[0].pp = &background;

	infouse[1].use = InfoUse::INFO_INDEX;
	infouse[1].pp = &color;

	lock[0] = 1;

	DuplicateGlobalsToBackBuffers();

}

SFont::~SFont()
{
}

void SFont::GetSaveFormats(narray<autoptr<SaveFormat>,int> &fmt)
{
	__super::GetSaveFormats(fmt);

	fmt.add(new SaveFormat(_T("Width list"),_T("width"),false));

	if(xsize == 320 && ysize == 200)
		fmt.add(new SaveFormat(_T("C64 Exe"),_T("prg"),false));
}

void SFont::GetLoadFormats(narray<autoptr<SaveFormat>,int> &fmt)
{
}

void SFont::Load(nmemfile &file, LPCTSTR type, int version)
{
	__super::Load(file,type,version);

	if(lstrcmpi(_T("bin"),type)==0)
	{
		size_t len = file.len();

		if(len % 8 != 0)
		{
			//Throw PRG header
			unsigned short tmp;
			file >> tmp;
			len -= 2;
		}

		xsize = 32*8;
		ysize = int(((len+255)/256)*8);

		Destroy();

		int colsize = (ysize/8) * (xsize/8);

		Create(ysize * (xsize/8), colsize, 0);

		*background = 0;

		memset(color, 1, colsize);

		file.read(map, len);
	}
}

void SFont::Save(nmemfile &file, LPCTSTR type)
{
	__super::Save(file,type);

	if(lstrcmpi(_T("prg"),type)==0)
	{
		if(xsize!=320 || ysize!=200)
			throw _T("Dimensions must be 320*200");

		narray<BYTE,int> c,s;
		if(CreateCharScreen(c,s,true)>256)
			throw _T("Too many characters");

		ASSERT(s.count() == 1000);

		static unsigned char viewer[]={
			0x00,0x1f,0x78,0xad,0x31,0x1f,0x8d,0x20,0xd0,0xad,0x32,0x1f,0x8d,0x21,0xd0,0xa9,
			0x8a,0x8d,0x18,0xd0,0xa2,0x00,0xbd,0x00,0x24,0x9d,0x00,0xd8,0xbd,0x00,0x25,0x9d,
			0x00,0xd9,0xbd,0x00,0x26,0x9d,0x00,0xda,0xbd,0x00,0x27,0x9d,0x00,0xdb,0xe8,0xd0,
			0xe5,0xf0,0xfe};

		file.write(viewer,sizeof(viewer));

		file << *border;
		file << *background;

		while(file.len() < (256+2))
			file << BYTE(0);

		file.write(s.getarray(),1000);

		while(file.len() < (256+2+0x0400))
			file << BYTE(0);

		file.write(color, 1000);

		while(file.len() < (256+2+0x0800))
			file << BYTE(0);

		file.write(c.getarray(),c.count());

		ByteBoozer::File bbin, bbout;
		bbin.size = file.len();
		bbin.data = (byte *)file.detach();

		ByteBoozer::crunch(&bbin, &bbout, 0x1f00, ByteBoozer::normalDecr, false, false);
		ByteBoozer::freeFile(&bbin);

		file.attach(bbout.data, bbout.size, true);
	}
	else if(lstrcmpi(_T("width"),type)==0)
	{
		narray<BYTE,int> space;
		int cx,cy,x,y;
		for(cy=0;cy<ysize/ycell;cy++)
		{
			for(cx=0;cx<xsize/xcell;cx++)
			{
				int maxx=0;
				for(y=0;y<ycell;y++)
				{
					for(x=0;x<xcell;x++)
					{
						if(GetPixel(cx*xcell+x,cy*ycell+y)==*color)
						{
							if(x>maxx)
								maxx=x;
						}
					}
				}

				if(space.count() == 32)
				{
					space.add(5);
				}
				else
				{
					space.add(maxx+2);
				}
			}
		}

		file.write(space.getarray(),space.count());
	}
}

BYTE SFont::GetPixel(int x, int y)
{
	ASSERT(x>=0 && x<xsize && y>=0 && y<ysize);

	int cx = x/8;
	int cy = y/8;

	TranslateFontDisplay(cx, cy);

	int d=(map[cy * xsize + cx * 8 + (y%8)] >> (1*(7-(x%8)))) & 1;

	BYTE b;
	switch(d)
	{
	case 0:
		b = (*background) & 0x0f;
		break;
	case 1:
		b = color[GetCellCountX()*cy + cx] & 0x0f;
		break;
	}

	return b;
}

void SFont::SetPixel(int x, int y, BYTE col)
{
	ASSERT(x>=0 && x<xsize && y>=0 && y<ysize);

	int cx = x/8;
	int cy = y/8;

	TranslateFontDisplay(cx, cy);

	int ci = cy * (xsize/8) + cx;

	int mask = ResolveMask1(ci, col, GetMask(x,y));
	if(mask == -1)
		return;

	SetColor(ci, mask, col);

	int mi = cy * xsize + cx * 8 + (y%8);

	int filtermask = ~(1 << (1*(7-(x%8))));
	mask = mask << (1*(7-(x%8)));
	map[mi] = (map[mi] & filtermask) | mask;
}

void SFont::Import(CImage &img)
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

int SFont::GetMask(int x, int y)
{
	int cx = x/8;
	int cy = y/8;

	return (map[cy * xsize + cx * 8 + (y%8)] >> (1*(7-(x%8)))) & 1;
}

void SFont::CustomCommand(int n)
{
	int r;
	for(r=0;r<2048 && r<rmapsize;r++)
	{
		map[r]=g_ROMFont[2048*n+r];
	}

	for(r=0;r<2048/8 && r<rmapsize/8;r++)
	{
		if(*background == 0x01)
			color[r] = 0x00;
		else
			color[r] = 0x01;
	}
}


