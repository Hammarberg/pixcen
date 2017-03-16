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

MCBitmap::MCBitmap(int x, int y, int backbuffers)
{
	mode = MC_BITMAP;

	if(x%4!=0)throw _T("X must be divisible by 4");
	if(y%8!=0)throw _T("Y must be divisible by 8");

	xsize=x;
	ysize=y;

	xcell=4;
	ycell=8;

	Create(y * (x/4), (y/8) * (x/4), (y/8) * (x/4), backbuffers);

	infouse[0].use = InfoUse::INFO_VALUE;
	infouse[0].pp = &background;

	infouse[1].use = InfoUse::INFO_INDEX_HIGH;
	infouse[1].pp = &screen;

	infouse[2].use = InfoUse::INFO_INDEX_LOW;
	infouse[2].pp = &screen;

	infouse[3].use = InfoUse::INFO_INDEX;
	infouse[3].pp = &color;

	lock[0]=1;

	DuplicateGlobalsToBackBuffers();

}

MCBitmap::~MCBitmap()
{
}


void MCBitmap::Import(CImage &img)
{
	ClearBackBuffer();

	{
		ImportHelper help(this, img, true);

		BYTE top[16];
		int num = help.CountTopColorsPerCell(top);

		*background = top[0];

		help.ReduceColors(4, top, num < 1 ? num : 1);
	}

	*border = GuessBorderColor();
}

void MCBitmap::GetSaveFormats(narray<autoptr<SaveFormat>,int> &fmt)
{
	__super::GetSaveFormats(fmt);
	//fmt.add(new SaveFormat(_T("Koala"),_T("koa")));

	if(xsize==160 && ysize == 200)
	{
		fmt.add(new SaveFormat(_T("Koala"),_T("kla;koa"),true));
		fmt.add(new SaveFormat(_T("Koala compressed"),_T("gg"),true));
		fmt.add(new SaveFormat(_T("Advanced Art Studio 2"),_T("ocp"),true));
		fmt.add(new SaveFormat(_T("Cenimate"),_T("cen"),true));
		if(crippled[3])fmt.add(new SaveFormat(_T("Paint Magic"),_T("pmg"),true));
		fmt.add(new SaveFormat(_T("C64 Exe"),_T("prg"),false));

	}
}

void MCBitmap::GetLoadFormats(narray<autoptr<SaveFormat>,int> &fmt)
{
	fmt.add(new SaveFormat(_T("Koala"),_T("kla;koa;gg"),true,160,200,MC_BITMAP));
	fmt.add(new SaveFormat(_T("Cenimate"),_T("cen"),true,160,200,MC_BITMAP));
	fmt.add(new SaveFormat(_T("Advanced Art Studio"),_T("ocp"),true,160,200,MC_BITMAP));
	fmt.add(new SaveFormat(_T("Paint Magic"),_T("pmg"),true,160,200,MC_BITMAP));
}


int MCBitmap::DecompressKoalaStream(const BYTE *stream, int stream_size, BYTE *buffer, int buffer_size)
{
	//Plenty of sanity checks
	int r=0,w=0;
	BYTE b;
	int l;

	while(r < stream_size)
	{
		if(w>=buffer_size)
			return -1;
		b=stream[r];
		if(b==0xfe)
		{
			if(r+3>stream_size)
				return -1;
			b=stream[r+1];
			l=stream[r+2];
			if(l==0)l=256;

			for(;l>0;l--)
			{
				if(w>=buffer_size)
					return -1;
				buffer[w]=b;
				w++;
			}
			r+=3;
		}
		else
		{
			buffer[w]=b;
			w++;
			r++;
		}

	}

	return w;
}

int MCBitmap::CompressKoalaStream(const BYTE *stream, int stream_size, BYTE *buffer, int buffer_size)
{
	int r=0,w=0;
	BYTE b;
	int l;

	while(r < stream_size)
	{
		if(w>=buffer_size)return -1;
		b=stream[r];
		l=1;
		while(r+l < stream_size && l < 256 && b==stream[r+l])l++;

		if(l > 3 || b==0xfe)
		{
			if(w+3>buffer_size)return -1;
			buffer[w]=0xfe;
			buffer[w+1]=b;
			buffer[w+2]=l&255;
			w+=3;
			r+=l;
		}
		else
		{
			buffer[w]=b;
			w++;
			r++;
		}
	}

	return w;
}

nstr MCBitmap::IdentifyFile(nmemfile &file)
{
	nstr ex;
	unsigned short addr;

	file >> addr;

	if(file.len() == 10003 && (addr == 0x6000 || addr == 0x2000))
	{
		ex = _T("kla");
	}
	else if(file.len() == 10051 && addr == 0x5800)
	{
		ex = _T("cen");
	}
	else if(file.len() == 10018 && addr == 0x2000)
	{
		ex = _T("ocp");
	}
	else if(file.len() == 9332 && addr == 0x3f8e)
	{
		ex = _T("pmg");
	}
	else if(addr == 0x6000)
	{
		BYTE buffer[10001];
		if(DecompressKoalaStream(((BYTE *)file)+2, int(file.len()-2), buffer, 10001) == 10001)
		{
			ex = _T("gg");
		}
	}

	return ex;
}

void MCBitmap::RemapCellColour(int cx, int cy, int colour, int indexTo)
{
	static int maskLUT[4] = { 0,1,2,3 };

	CellInfo info;
	GetCellInfo(cx, cy, 1, 1, &info);


	int targetMask = maskLUT[indexTo];

	//does this cell use the colour we want
	int colourUsed = 0;
	for (int c = 0; c < 4; ++c)
	{
		if (info.col[c] == colour)
		{
			++colourUsed;
		}
	}
	if (colourUsed == 1)
	{
		for (int c = 0; c < 4; ++c)
		{
			if (info.col[c] == colour)
			{
				//is it already where we want it to be
				if (c != indexTo)
				{
					//no
					int srcMask = maskLUT[c];
					SwapmaskCell2(cx, cy, srcMask, targetMask);
					//also need to swap the colours
					int oldColour = info.col[indexTo];
					int ci = cy * (xsize / 4) + cx;

					SetColor(ci, targetMask, colour);
					SetColor(ci, srcMask, oldColour);
					break;  //found it so exit
				}
			}
		}
	}
	else if (colourUsed >= 3)
	{
		//they are all the colour
		if (targetMask != 1) RemapCell2(cx, cy, maskLUT[1], targetMask);
		if (targetMask != 2) RemapCell2(cx, cy, maskLUT[2], targetMask);
		if (targetMask != 3) RemapCell2(cx, cy, maskLUT[3], targetMask);
	}
	else if (colourUsed >= 2) //2 are the colour
	{
		//find first colour with it
		int c = 0;
		int firstIndex = 0;
		while (c < 4)
		{
			if (info.col[c] == colour)
			{
				firstIndex = c;
				break;
			}
			++c;
		}
		//find second colour with it
		int secondIndex = 0;
		++c;
		while (c < 4)
		{
			if (info.col[c] == colour)
			{
				secondIndex = c;
				break;
			}
			++c;
		}		
		//if dest = first, map second onto first
		if (firstIndex == indexTo)
		{
			RemapCell2(cx, cy, maskLUT[secondIndex], maskLUT[firstIndex]);
		}
		//else if dest = second, map first onto second
		else if (secondIndex == indexTo)
		{
			RemapCell2(cx, cy, maskLUT[firstIndex], maskLUT[secondIndex]);
		}
		//else map first colour into second, swap with dest
		else
		{
			RemapCell2(cx, cy, maskLUT[firstIndex], maskLUT[secondIndex]);
			SwapmaskCell2(cx, cy, maskLUT[secondIndex], targetMask);

			int oldColour = info.col[indexTo];
			int ci = cy * (xsize / 4) + cx;
			SetColor(ci, targetMask, colour);
			SetColor(ci, maskLUT[secondIndex], oldColour);
		}
	}
}

void MCBitmap::Load(nmemfile &file, LPCTSTR type, int version)
{
	__super::Load(file,type,version);

	if(lstrcmpi(_T("kla"),type)==0 || lstrcmpi(_T("koa"),type)==0)
	{
		if(file.len() != 10003)
		{
			throw _T("Invalid koala file size");
		}

		unsigned short addr;
		file >> addr;

		file.read(map, 8000);
		file.read(screen, 1000);
		file.read(color, 1000);
		file.read(background, 1);

		//Clean up masks
		*background &= 0x0f;
		for(int r=0;r<1000;r++)
			color[r] &= 0x0f;

		*border = GuessBorderColor();

	}
	else if(lstrcmpi(_T("gg"),type)==0)
	{
		BYTE buffer[10001];	//Decompress will skip PRG header
		if(DecompressKoalaStream(((BYTE *)file)+2, int(file.len()-2), buffer, 10001)!=10001)
		{
			throw _T("Invalid Koala stream");
		}

		memcpy(map, buffer, 8000);
		memcpy(screen, buffer+8000, 1000);
		memcpy(color, buffer+9000, 1000);

		*background = buffer[10000]&0x0f;

		//Clean up masks
		for(int r=0;r<1000;r++)
			color[r] &= 0x0f;

		*border = GuessBorderColor();
	}
	else if(lstrcmpi(_T("cen"),type)==0)
	{
		if(file.len() != 10051)
		{
			throw _T("Invalid cenimate file size");
		}

		unsigned short addr;
		file >> addr;

		file.read(color, 1000);
		file.read(24);
		file.read(screen, 1000);
		file.read(24);
		file.read(map, 8000);
		file.read(background, 1);

		//Clean up masks
		*background &= 0x0f;
		for(int r=0;r<1000;r++)
			color[r] &= 0x0f;

		*border = GuessBorderColor();

	}
	else if(lstrcmpi(_T("ocp"),type)==0)
	{
		if(file.len() != 10018)
		{
			throw _T("Invalid Advanced Art Studio file size");
		}

		unsigned short addr;
		file >> addr;

		file.read(map, 8000);
		file.read(screen, 1000);

		file.read(border, 1);		//Border color
		file.read(background, 1);
		for(int r=0;r<14;r++)
			file.read(1);

		file.read(color, 1000);

		//Clean up masks
		*background &= 0x0f;
		for(int r=0;r<1000;r++)
			color[r] &= 0x0f;
	}
	else if(lstrcmpi(_T("pmg"),type)==0)
	{
		if(file.len() != 9332)
		{
			throw _T("Invalid Advanced Paint Magic file size");
		}

		unsigned short addr;
		file >> addr;
		assert(addr == 0x3f8e);

		//Skip display routine
		file.read(0x4000-0x3f8e);

		file.read(map, 8000);

		file.read(background, 1);

		//Throw 2 bytes
		file.read(2);

		//Read color
		BYTE c;
		file >> c;
		memset(color, c, 1000);

		file.read(border, 1);

		file.read(0x6000-0x5f45);
		file.read(screen, 1000);

		//Clean up masks
		*background &= 0x0f;
		for(int r=0;r<1000;r++)
			color[r] &= 0x0f;

		crippled[3] = 1;

	}
	else if(lstrcmpi(_T("bin"),type)==0)
	{
		size_t len = file.len();

		if(len % 8 != 0)
		{
			//Throw PRG header
			unsigned short tmp;
			file >> tmp;
			len -= 2;
		}

		xsize = 40*4;
		ysize = int(((len+319)/320)*8);

		Destroy();
		Create(ysize * (xsize/4), (xsize/4) * (ysize/8), (xsize/4) * (ysize/8));

		*background = 0;
		memset(screen,0xbc,(xsize/4) * (ysize/8));
		memset(color,0x01,(xsize/4) * (ysize/8));

		file.read(map, len);
	}

}


void MCBitmap::Save(nmemfile &file, LPCTSTR type)
{
	__super::Save(file,type);

	if(lstrcmpi(_T("kla"),type)==0 || lstrcmpi(_T("koa"),type)==0)
	{
		if(xsize!=160 || ysize!=200)
			throw _T("Buffers are not in standard multi-color format");

		file << unsigned short(0x6000);

		file.write(map,8000);
		file.write(screen,1000);
		file.write(color,1000);
		file << *background;
	}
	else if(lstrcmpi(_T("gg"),type)==0)
	{
		if(xsize!=160 || ysize!=200)
			throw _T("Buffers are not in standard multi-color format");

		BYTE src[10001];

		memcpy(src,map,8000);
		memcpy(src+8000,screen,1000);
		memcpy(src+9000,color,1000);
		src[10000]=*background;

		BYTE buffer[30003];
		int size=CompressKoalaStream(src, 10001, buffer, 30003);

		if(size == -1)
		{
			throw _T("Failed compressing koala");
		}

		file << unsigned short(0x6000);
		file.write(buffer,size);
	}
	else if(lstrcmpi(_T("cen"),type)==0)
	{
		if(xsize!=160 || ysize!=200)
			throw _T("Buffers are not in standard multi-color format");

		file << unsigned short(0x5800);

		file.write(color, 1000);
		for(int r=0;r<24;r++)file << BYTE(0);
		file.write(screen, 1000);
		for(int r=0;r<24;r++)file << BYTE(0);
		file.write(map, 8000);
		file << *background;
	}
	else if(lstrcmpi(_T("ocp"),type)==0)
	{
		if(xsize!=160 || ysize!=200)
			throw _T("Buffers are not in standard multi-color format");

		file << unsigned short(0x2000);

		file.write(map, 8000);
		file.write(screen, 1000);
		file << *border;
		file << *background;
		for(int r=0;r<14;r++)file << BYTE(0);
		file.write(color, 1000);
	}
	else if(lstrcmpi(_T("pmg"),type)==0)
	{
		if(xsize!=160 || ysize!=200)
			throw _T("Buffers are not in standard multi-color format");

		assert(crippled[3]);

		static unsigned char viewer[]={
		0x0b,0x08,0x0a,0x00,0x9e,0x32,0x30,0x36,0x39,0x00,0x13,0x08,0x14,0x00,
		0x89,0x32,0x30,0x00,0x00,0x00,0xa0,0x00,0x8c,0x11,0xd0,0xa2,0x24,0xb9,0x73,0x08,
		0x99,0x00,0x40,0xc8,0xd0,0xf7,0xee,0x1e,0x08,0xee,0x21,0x08,0xca,0xd0,0xee,0xa9,
		0x00,0x8d,0x11,0xd0,0xad,0x44,0x5f,0x8d,0x20,0xd0,0xad,0x00,0xdd,0x29,0xfc,0x09,
		0x02,0x8d,0x00,0xdd,0xa9,0x80,0x8d,0x18,0xd0,0xa9,0xd8,0x8d,0x16,0xd0,0xad,0x40,
		0x5f,0x8d,0x21,0xd0,0xad,0x43,0x5f,0xa0,0x00,0x99,0x00,0xd8,0x99,0x00,0xd9,0x99,
		0x00,0xda,0x99,0x00,0xdb,0xc8,0xd0,0xf1,0xa9,0x3b,0x8d,0x11,0xd0,0x60,0x48,0x81,
		0x71,0x80,0x71,0x80};

		unsigned short addr = 0x3f8e;

		file << addr;
		file.write(viewer, sizeof(viewer));
		addr+=sizeof(viewer);

		while(addr < 0x4000)
		{
			file << BYTE(0);
			addr++;
		}

		file.write(map, 8000);
		file << *background;

		file << BYTE(0);
		file << BYTE(0);

		file << *color;
		file << *border;

		addr = 0x5f45;
		while(addr < 0x6000)
		{
			file << BYTE(0);
			addr++;
		}

		file.write(screen, 1000);

		addr = 0x63e8;
		while(addr < 0x6400)
		{
			file << BYTE(0);
			addr++;
		}

	}
	else if(lstrcmpi(_T("prg"),type)==0)
	{
		if(xsize!=160 || ysize!=200)
			throw _T("Buffers are not in standard multi-color format");

		static unsigned char viewer[]={
		0x00,0x1f,0x78,0xa9,0x3b,0x8d,0x11,0xd0,0xa9,0x18,0x8d,0x16,0xd0,0xa9,0x18,0x8d,
		0x18,0xd0,0xa2,0x00,0xbd,0x40,0x3f,0x9d,0x00,0x04,0xbd,0x40,0x40,0x9d,0x00,0x05,
		0xbd,0x40,0x41,0x9d,0x00,0x06,0xbd,0x40,0x42,0x9d,0x00,0x07,0xbd,0x28,0x43,0x9d,
		0x00,0xd8,0xbd,0x28,0x44,0x9d,0x00,0xd9,0xbd,0x28,0x45,0x9d,0x00,0xda,0xbd,0x28,
		0x46,0x9d,0x00,0xdb,0xe8,0xd0,0xcd,0xad,0x10,0x47,0x8d,0x21,0xd0,0xad,0x11,0x47,
		0x8d,0x20,0xd0,0x4c,0x51,0x1f};

		file.write(viewer,sizeof(viewer));

		int pad = sizeof(viewer) - 2;	//minus prg header
		while(pad < 256)
		{
			file << BYTE(0);
			pad++;
		}

		file.write(map,8000);
		file.write(screen,1000);
		file.write(color,1000);
		file << *background;
		file << *border;

		ByteBoozer::File bbin, bbout;
		bbin.size = file.len();
		bbin.data = (byte *)file.detach();

		ByteBoozer::crunch(&bbin, &bbout, 0x1f00, ByteBoozer::normalDecr, false, false);
		ByteBoozer::freeFile(&bbin);

		file.attach(bbout.data, bbout.size, true);
	}
}

BYTE MCBitmap::GetPixelInternal(int x, int y)
{
	ASSERT(x >= 0 && x<xsize && y >= 0 && y<ysize);

	int cx = x / 4;
	int cy = y / 8;

	int d = (map[cy * xsize * 2 + cx * 8 + (y % 8)] >> (2 * (3 - (x % 4)))) & 3;

	BYTE b;
	switch (d)
	{
	case 0:
		b = *background;
		break;
	case 1:
		b = screen[cy * (xsize / 4) + cx] >> 4;
		break;
	case 2:
		b = screen[cy * (xsize / 4) + cx] & 0x0f;
		break;
	case 3:
		b = color[cy * (xsize / 4) + cx];
		break;
	}

	return b;
}

BYTE MCBitmap::GetPixel(int x, int y)
{
	return GetPixelInternal(x,y);
}

BYTE MCBitmap::GetPixelColourMap(int x, int y)
{
	int cx = x / 4;
	int cy = y / 8;
	CellInfo info;
	GetCellInfo(cx, cy, 1, 1, &info);
	int iX = 0;
	if (x % 4 >= 2) iX = 1;
	int iY = 0;
	if (y % 8 >= 4) iY = 2;
	int colourIndex = iX + iY;
	BYTE colour = info.col[colourIndex];
	return colour;
}

void MCBitmap::GetPixelBatch(BYTE *p, int xs, int ys, int w, int h)
{
	for (int y = ys; y < ys + h; y++)
	{
		for (int x = xs; x < xs + w; x++)
		{
			*p = GetPixelInternal(x, y);
			p++;
		}
	}
}

void MCBitmap::SetPixel(int x, int y, BYTE col)
{
	ASSERT(x>=0 && x<xsize && y>=0 && y<ysize);

	int cx = x/4;
	int cy = y/8;
	int ci = cy * (xsize/4) + cx;

	int mask = ResolveMask2(ci, col, GetMask(x,y));
	if(mask == -1)
		return;

	SetColor(ci, mask, col);

	int mi = cy * xsize*2 + cx * 8 + (y%8);

	int filtermask = ~(3 << (2*(3-(x%4))));
	mask = mask << (2*(3-(x%4)));
	map[mi] = (map[mi] & filtermask) | mask;
}

int MCBitmap::GetMask(int x, int y)
{
	int cx = x/4;
	int cy = y/8;

	return (map[cy * xsize*2 + cx * 8 + (y%8)] >> (2*(3-(x%4)))) & 3;
}
