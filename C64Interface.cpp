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
#include <malloc.h>
#include <unordered_set>

#define MINIZ_NO_ARCHIVE_APIS
#define MINIZ_NO_STDIO
#include "miniz/miniz.c"

const double C64Interface::parvalue[3]={1.0,0.9365,0.75};

C64Interface::C64Interface()
{
	buffer = tbuffer = NULL;

	overflow = REPLACE;

	offsetcell = sizecell = 8;
	rbackbufnum = 0;
	par=0;

	infouse[5].use = InfoUse::INFO_VALUE;
	infouse[5].pp = &border;

	SetMetaInt("pmap",0x4000);
	SetMetaInt("pscr",0x6000);
	SetMetaInt("pcol",0x6400);

	history_previous = history_next = NULL;
}

C64Interface::~C64Interface()
{
	ResetHistory();
	Destroy();

	DeleteHistoryPrevious();
	DeleteHistoryNext();
}

void C64Interface::DeleteHistoryPrevious(void)
{
	if(history_previous)
	{
		history_previous->history_next = NULL;
		delete history_previous;
		history_previous = NULL;
	}
}

void C64Interface::DeleteHistoryNext(void)
{
	if(history_next)
	{
		history_next->history_previous = NULL;
		delete history_next;
		history_next = NULL;
	}
}

void C64Interface::DeleteHistory(void)
{
	//Delete all history
	DeleteHistoryPrevious();
	DeleteHistoryNext();
	history_pos = 0;
	history_undo.count(0);
	history_redo.count(0);
}

void C64Interface::BeginHistory(void)
{
	memcpy(tbuffer, buffer, buffersize);

	if(history_pos > 100)
	{
		while(history_pos > 100)
		{
			delete [] history_undo[0];
			delete [] history_redo[0];
			history_undo.remove(0);
			history_redo.remove(0);
			history_pos--;
		}

		DeleteHistoryPrevious();
	}

	if(history_pos < history_redo.count())
	{
		DeleteHistoryNext();

		//Kill any redo from this point
		for(int r = history_pos; r < history_redo.count() ; r++)
		{
			delete [] history_undo[r];
			delete [] history_redo[r];
		}
	}

	history_undo.count(history_pos);
	history_redo.count(history_pos);
}

void C64Interface::EndHistory(void)
{
	//TODO SSE optimize the search
	unsigned short skipcount;
	unsigned short changedcount;

	narray<BYTE,int> undo;
	narray<BYTE,int> redo;

	undo.count(4);	//Reserve for size
	redo.count(4);

	int local_buffersize = buffersize; //Copy to local for optimizer

	for(int r=0,t;;)
	{
		skipcount = 0;

		for(;r<local_buffersize&&skipcount<0xffff;r++,skipcount++)
		{
			if(buffer[r]!=tbuffer[r])break;
		}

		undo.add(skipcount & 0xff);
		undo.add(skipcount >> 8);
		redo.add(skipcount & 0xff);
		redo.add(skipcount >> 8);

		if(!skipcount)break;

		changedcount = 0;

		for(t=r;t<local_buffersize&&changedcount<0xffff;t++,changedcount++)
		{
			if(buffer[t]==tbuffer[t])break;
		}

		undo.add(changedcount & 0xff);
		undo.add(changedcount >> 8);
		redo.add(changedcount & 0xff);
		redo.add(changedcount >> 8);

		for(;r<t;r++)
		{
			undo.add(tbuffer[r]);
			redo.add(buffer[r]);
		}
	}
	*((int32_t *)undo.getarray()) = undo.count();
	*((int32_t *)redo.getarray()) = redo.count();

	history_undo.add(undo.detach());
	history_redo.add(redo.detach());

	history_pos++;

	dirty = true;
}

void C64Interface::InheritHistory(C64Interface *old)
{
	old->history_next = this;
	history_previous = old;
}

C64Interface *C64Interface::Undo(void)
{
	if(!history_pos && history_previous)
	{
		history_previous->file_name.empty();
		return history_previous;
	}

	history_pos--;PlayHistory(history_undo[history_pos]+4);
	return this;
}

C64Interface *C64Interface::Redo(void)
{
	if(history_pos == history_redo.count() && history_next)
	{
		history_next->file_name.empty();
		return history_next;
	}

	PlayHistory(history_redo[history_pos]+4);history_pos++;
	return this;
}


void C64Interface::GetCellInfo(int cx, int cy, int w, int h, CellInfo *info)
{
	info->h = xcell;
	info->w = ycell;

	for(int r=0;r<6;r++)
		GetMaskInfo(cx, cy, w, h, r, *info);

}

void C64Interface::GetMaskInfo(int cx, int cy, int w, int h, int infoindex, CellInfo &info)
{

	int y,ci;
	BYTE b,*p;

	InfoUse &use = infouse[infoindex];

	BYTE &col=info.col[infoindex];

	int &crip=info.crippled[infoindex];

	switch(use.use)
	{
	case InfoUse::INFO_NO:
		col = 0xff;
		crip = 0;
		break;
	case InfoUse::INFO_VALUE:
		col = **use.pp;
		crip = 1;
		break;
	case InfoUse::INFO_INDEX:
	case InfoUse::INFO_INDEX_LOW:
		p = *use.pp;

		if(crippled[infoindex])
		{
			col = p[0]&0x0f;
			break;
		}

		crip = 0;

		if(cx == -1 || cy == -1)
		{
			col = 0xff;
			break;
		}
		ASSERT(cx>=0 && cy>=0 && cx+w <= GetCellCountX() && cy+h <= GetCellCountY());
		b = p[cy*GetCellCountX() + cx] & 0x0f;
		for(y=0;y<h;y++)
		{
			ci = (y+cy)*GetCellCountX() + cx + 0;
			if(!mymemspn_low(&p[ci], b, w))
			{
				col = 0xfe;
				break;
			}
		}
		col = b;
		break;
	case InfoUse::INFO_INDEX_HIGH:
		p = *use.pp;

		if(crippled[infoindex])
		{
			col = p[0] >> 4;
			break;
		}

		crip = 0;

		if(cx == -1 || cy == -1)
		{
			col = 0xff;
			break;
		}
		ASSERT(cx>=0 && cy>=0 && cx+w <= GetCellCountX() && cy+h <= GetCellCountY());
		b = p[cy*GetCellCountX() + cx] & 0xf0;
		for(y=0;y<h;y++)
		{
			ci = (y+cy)*GetCellCountX() + cx + 0;
			if(!mymemspn_high(&p[ci], b, w))
			{
				col = 0xfe;
				goto skip;
			}
		}
		col = b >> 4;
		skip:
		break;
	default:
		ASSERT(false);
		break;
	};

	info.lock[infoindex]=lock[infoindex];

}


void C64Interface::SetCellInfo(int cx, int cy, int w, int h, CellInfo *info)
{
	for(int r=0;r<6;r++)
		SetMaskInfo(cx, cy, w, h, r, *info);
}

void C64Interface::SetMaskInfo(int cx, int cy, int w, int h, int infoindex, CellInfo &info)
{
	InfoUse &use = infouse[infoindex];

	if(info.lock[infoindex]!=-1)
		lock[infoindex]=1-lock[infoindex];

	if(info.crippled[infoindex]!=-1)
	{		
		if(crippled[infoindex]==0 && use.use!=InfoUse::INFO_NO && use.use!=InfoUse::INFO_VALUE)
		{
			BYTE *p = *use.pp;
			bool high = use.use == InfoUse::INFO_INDEX_HIGH ? true : false;

			int size = GetCellCountX()*GetCellCountY();

			BYTE col=GetCommonColorFromMask(infoindex, p, size, high);

			if(use.use == InfoUse::INFO_INDEX)
			{
				memset(p, col, size);
			}
			else if(use.use == InfoUse::INFO_INDEX_LOW)
			{
				for(int r=0;r<size;r++)
				{
					p[r]=(p[r]&0xf0)|col;
				}
			}
			else //(use.use == InfoUse::INFO_INDEX_LOW)
			{
				for(int r=0;r<size;r++)
				{
					p[r]=(p[r]&0x0f)|(col<<4);
				}
			}

			crippled[infoindex]=1;
		}
		else
		{
			crippled[infoindex]=0;
		}
	}

	BYTE col = info.col[infoindex];

	if(col == 0xff || col == 0xfe)
		return;

	int y,x,ci;
	BYTE b,*p;

	switch(use.use)
	{
	case InfoUse::INFO_NO:
		break;
	case InfoUse::INFO_VALUE:
		**use.pp = col;
		break;
	case InfoUse::INFO_INDEX:

		//Special for MC Char
		if(infoindex == 3 && mode == MC_CHAR)
			col&=7;

		p = *use.pp;
		if(crippled[infoindex])
		{
			int size = GetCellCountX()*GetCellCountY();

			if(use.use == InfoUse::INFO_INDEX)
			{
				memset(p, col, size);
			}
			else if(use.use == InfoUse::INFO_INDEX_LOW)
			{
				for(int r=0;r<size;r++)
				{
					p[r]=(p[r]&0xf0)|col;
				}
			}
			else //(use.use == InfoUse::INFO_INDEX_LOW)
			{
				for(int r=0;r<size;r++)
				{
					p[r]=(p[r]&0x0f)|(col<<4);
				}
			}
			break;
		}

		ASSERT(cx>=0 && cy>=0 && cx+w <= GetCellCountX() && cy+h <= GetCellCountY());
		b = col&0x0f;
		for(y=0;y<h;y++)
		{
			ci = (y+cy)*GetCellCountX() + cx + 0;
			memset(&p[ci], b, w);
		}
		break;
	case InfoUse::INFO_INDEX_LOW:
		p = *use.pp;
		if(crippled[infoindex])
		{
			p[0] = (p[0]&0xf0)|col;
			break;
		}
		ASSERT(cx>=0 && cy>=0 && cx+w <= GetCellCountX() && cy+h <= GetCellCountY());
		p = *use.pp;
		b = col&0x0f;
		for(y=0;y<h;y++)
		{
			ci = (y+cy)*GetCellCountX() + cx + 0;
			for(x=0;x<w;x++)
				p[ci+x] = (p[ci+x]&0xf0)|b;
		}
		break;
	case InfoUse::INFO_INDEX_HIGH:
		p = *use.pp;
		if(crippled[infoindex])
		{
			p[0] = (p[0]&0x0f)|(col<<4);
			break;
		}
		ASSERT(cx>=0 && cy>=0 && cx+w <= GetCellCountX() && cy+h <= GetCellCountY());
		p = *use.pp;
		b = (col&0x0f) << 4;
		for(y=0;y<h;y++)
		{
			ci = (y+cy)*GetCellCountX() + cx + 0;
			for(x=0;x<w;x++)
				p[ci+x] = (p[ci+x]&0x0f)|b;
		}
		break;
	default:
		ASSERT(false);
		break;
	};
}


void C64Interface::Save(LPCTSTR pszFileName, LPCTSTR type)
{
	nstr stype;

	if(!pszFileName)
	{
		ASSERT(file_name.isnotempty());
		pszFileName = file_name;
	}

	if(!type)
	{
		stype=GetFileExt();
		type=stype;
	}

	if(lstrcmpi(_T("bmp"),type)==0 || lstrcmpi(_T("png"),type)==0 || lstrcmpi(_T("jpg"),type)==0 || lstrcmpi(_T("gif"),type)==0)
	{
		CImage img;
		RenderImage(img);
		if(img.Save(pszFileName) != 0)
			throw _T("Error saving file");

		return;
	}

	nmemfile file;
	Save(file, type);

	FILE *fp=_tfopen(pszFileName,_T("wb"));
	if(!fp)
		throw _T("Unable to open file for writing");
	file.save(fp);
	fclose(fp);

}

void C64Interface::GetSaveFormats(narray<autoptr<SaveFormat>,int> &fmt)
{
	fmt.add(new SaveFormat(_T("Pixcen"),_T("gpx"),true));
	fmt.add(new SaveFormat(_T("BMP"),_T("bmp"),false));
	fmt.add(new SaveFormat(_T("PNG"),_T("png"),false));
	fmt.add(new SaveFormat(_T("JPG"),_T("jpg"),false));
	fmt.add(new SaveFormat(_T("GIF"),_T("gif"),false));
	if(rscreensize)
	{
		fmt.add(new SaveFormat(_T("Screen RAM"),_T("scr"),false));
		fmt.add(new SaveFormat(_T("PRG Screen RAM"),_T("pscr"),false));
	}
	if(rcolorsize)
	{
		fmt.add(new SaveFormat(_T("Color RAM"),_T("col"),false));
		fmt.add(new SaveFormat(_T("PRG Color RAM"),_T("pcol"),false));
	}
	fmt.add(new SaveFormat(_T("Bitmap RAM"),_T("map"),false));
	fmt.add(new SaveFormat(_T("PRG Bitmap RAM"),_T("pmap"),false));
}

void C64Interface::GetLoadFormats(narray<autoptr<SaveFormat>,int> &fmt)
{
	fmt.add(new SaveFormat(_T("Pixcen"),_T("gpx"),true));

	MCBitmap::GetLoadFormats(fmt);
	Bitmap::GetLoadFormats(fmt);
	Unrestricted::GetLoadFormats(fmt);
	MCFont::GetLoadFormats(fmt);
	SFont::GetLoadFormats(fmt);
	Sprite::GetLoadFormats(fmt);
	MCSprite::GetLoadFormats(fmt);

	fmt.add(new SaveFormat(_T("BMP"),_T("bmp"),false));
	fmt.add(new SaveFormat(_T("PNG"),_T("png"),false));
	fmt.add(new SaveFormat(_T("JPG"),_T("jpg"),false));
	fmt.add(new SaveFormat(_T("GIF"),_T("gif"),false));
}


void C64Interface::Optimize(void)
{
	CImage img;
	RenderImage(img);
	ZeroMemory(buffer,buffersize);
	Import(img);
}

void C64Interface::RenderImage(CImage &inimg, int startx, int starty, int width, int height)
{
	int pw = GetPixelWidth();
	int xmax=GetSizeX(),ymax=GetSizeY();

	if(width != -1)
		xmax = xmax < startx+width ? xmax : startx+width;

	if(height != -1)
		ymax = ymax < starty+height ? ymax : starty+height;

	inimg.Create((xmax-startx)*pw,ymax-starty,24);

	CImageFast &img = static_cast<CImageFast &>(inimg);

	paralell_for(ymax - starty, [starty, startx, xmax, &img, pw, this](int py)
	{
		int y = py + starty;
		int px, x;
		for (px = 0, x = startx; x<xmax; x++, px++)
		{
			COLORREF c = GetPixelCR(x, y);
			if (pw != 2)
			{
				img.SetPixel(px, py, c);
			}
			else
			{
				img.SetPixel(px * 2, py, c);
				img.SetPixel(px * 2 + 1, py, c);
			}
		}
	});
}

void C64Interface::RenderColourUsageImage(CImage &inimg, int startx, int starty, int width, int height)
{
	int pw = GetPixelWidth();
	int xmax = GetSizeX(), ymax = GetSizeY();

	if (width != -1)
		xmax = xmax < startx + width ? xmax : startx + width;

	if (height != -1)
		ymax = ymax < starty + height ? ymax : starty + height;

	inimg.Create((xmax - startx)*pw, ymax - starty, 24);

	CImageFast &img = static_cast<CImageFast &>(inimg);

	startx /= GetCellSizeX();
	starty /= GetCellSizeY();
	xmax /= GetCellSizeX();
	ymax /= GetCellSizeY();

	paralell_for(ymax - starty, [starty, startx, xmax, &img, pw, this](int py)
	{
		const int colourBlockHeight = GetCellSizeY() / 2;
		const int colourBlockWidth = GetCellSizeX() / 2 * GetPixelWidth();
		
		int pdy = (starty+py)*GetCellSizeY();
		int y = py + starty;
		for (int px = startx; px < xmax; ++px)
		{
			CellInfo info;
			GetCellInfo(px, y, 1, 1, &info);
			int pdx = px*(GetCellSizeX()* GetPixelWidth());
			for (int dy = 0; dy < colourBlockHeight; ++dy)
			{
				for (int dx = 0; dx < colourBlockWidth; ++dx)
				{
					img.SetPixel(pdx + dx, pdy + dy, g_Vic2[info.col[0]]);
					img.SetPixel(pdx + dx + colourBlockWidth, pdy + dy, g_Vic2[info.col[1]]);
					img.SetPixel(pdx + dx, pdy + dy + colourBlockHeight, g_Vic2[info.col[2]]);
					img.SetPixel(pdx + dx + colourBlockWidth, pdy + dy + colourBlockHeight, g_Vic2[info.col[3]]);
				}
			}
		}
 	});
}

int C64Interface::GuessBorderColor(void)
{
	/*
	int colcount[16]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	int high = -1;
	int best = 0;
	int x,y;

	for(x=0;x<xsize;x++)
	{
		colcount[GetPixel(x,0)]++;
		colcount[GetPixel(x,ysize-1)]++;
	}

	for(y=0;y<ysize;y++)
	{
		colcount[GetPixel(0,y)]++;
		colcount[GetPixel(xsize-1,y)]++;
	}

	return ImportHelper::HighIndex(colcount, 16);

	*/

	return 0x0e;
}

void C64Interface::Save(nmemfile &file, LPCTSTR type)
{
	if(!lstrcmpi(_T("gpx"),type))
	{
		file << int32_t(CURRENT_GPX_FILE_VERSION);	//Version
		file << int32_t(mode);

		SetMetaInt("xsize",xsize);
		SetMetaInt("ysize",ysize);
		SetMetaInt("mapsize",rmapsize);
		SetMetaInt("colorsize",rcolorsize);
		SetMetaInt("screensize",rscreensize);
		SetMetaInt("backbuffers",rbackbuffers);
		SetMetaInt("backbufsel",rbackbufnum);
		SetMetaInt("par",par);
		SetMetaInt("overflow",overflow);

		int32_t metacount=0;

		for(std::map<nstrc,nstrw>::iterator i=meta.begin();i!=meta.end();i++)
			metacount++;

		file << metacount;

		for(std::map<nstrc,nstrw>::iterator i=meta.begin();i!=meta.end();i++)
		{
			file << i->first << i->second;
		}

		file.write(buffer, buffersize);

		file << int32_t(history_pos);

		file << history_undo.count();
		for(int r=0;r<history_undo.count();r++)
			file.write(history_undo[r], *((int32_t *)history_undo[r]));

		file << history_redo.count();
		for(int r=0;r<history_redo.count();r++)
			file.write(history_redo[r], *((int32_t *)history_redo[r]));

		uLongf destsize = uLongf(file.len()+16);
		BYTE *p = (BYTE *)malloc(destsize);
		if(compress2(p,&destsize,file.buf(),(uLong)file.len(),Z_BEST_COMPRESSION) != Z_OK)
		{
			free(p);
			throw _T("Error compressing gpx-file");
		}

		free(file.detach());
		file.attach(p,destsize,true);
	}
	else if(!lstrcmpi(_T("pscr"),type))
	{
		file << (unsigned short)GetMetaInt("pscr");
		file.write(screen, rscreensize);
	}
	else if(!lstrcmpi(_T("scr"),type))
	{
		file.write(screen, rscreensize);
	}
	else if(!lstrcmpi(_T("pcol"),type))
	{
		file << (unsigned short)GetMetaInt("pcol");
		if(mode == MC_CHAR)
		{
			for(int r=0;r<1000;r++)
			{
				assert(color[r]<8);
				file << BYTE((color[r]&7) | 8);
			}
		}
		else
		{
			file.write(color, rcolorsize);
		}
	}
	else if(!lstrcmpi(_T("col"),type))
	{
		if(mode == MC_CHAR)
		{
			for(int r=0;r<1000;r++)
			{
				assert(color[r]<8);
				file << BYTE((color[r]&7) | 8);
			}
		}
		else
		{
			file.write(color, rcolorsize);
		}
	}
	else if(!lstrcmpi(_T("pmap"),type))
	{
		file << (unsigned short)GetMetaInt("pmap");
		file.write(map, rmapsize);
	}
	else if(!lstrcmpi(_T("map"),type))
	{
		file.write(map, rmapsize);
	}
}

void C64Interface::Load(nmemfile &file, LPCTSTR type, int version)
{
	if(!lstrcmpi(_T("gpx"),type))
	{
		int32_t tmp;

		if(version >= 4)
		{
			//meta.clear();

			int32_t metacount;
			nstrc first;
			nstrw second;

			file >> metacount;

			for(;metacount>0;metacount--)
			{
				file >> first;
				file >> second;
				meta[first]=second;
			}

			par=GetMetaInt("par");
			xsize=GetMetaInt("xsize");
			ysize=GetMetaInt("ysize");
			rmapsize=GetMetaInt("mapsize");
			rcolorsize=GetMetaInt("colorsize");
			rscreensize=GetMetaInt("screensize");
			rbackbuffers=GetMetaInt("backbuffers");
			rbackbufnum=GetMetaInt("backbufsel");
			overflow=(toverflow)GetMetaInt("overflow");

			Destroy();
			Create(rmapsize, rcolorsize, rscreensize, rbackbuffers);

			file.read(buffer, buffersize);

			file >> history_pos;

			file >> tmp;
			history_undo.count(tmp);
			for(int r=0;r<history_undo.count();r++)
			{
				file >> tmp;
				BYTE *p=new BYTE[tmp];
				*((int32_t *)p) = tmp;
				file.read(p+4,tmp-4);
				history_undo[r] = p;
			}

			file >> tmp;
			history_redo.count(tmp);
			for(int r=0;r<history_redo.count();r++)
			{
				file >> tmp;
				BYTE *p=new BYTE[tmp];
				*((int32_t *)p) = tmp;
				file.read(p+4,tmp-4);
				history_redo[r] = p;
			}
		}
		else
		{
			//Legacy GPX load
			int32_t tmplock;

			file >> tmp; overflow = toverflow(tmp);

			if(version >= 2)
			{
				file >> tmplock; 
			}
			else 
			{
				tmplock=0;
			}

			file >> tmp; xsize = tmp;
			file >> tmp; ysize = tmp;
			file >> tmp; xcell = tmp;
			file >> tmp; ycell = tmp;

			file >> tmp; rmapsize = tmp;
			file >> tmp; rcolorsize = tmp;
			file >> tmp; rscreensize = tmp;

			if(version >= 3)
			{
				file >> tmp; rbackbuffers = tmp;
			}
			else
				rbackbuffers = 1;

			//The old size calculation
			int temp_buffersize = 1 + (5 + rmapsize + rscreensize + rcolorsize) * 1;
			BYTE *temp_buffer = (BYTE *)_aligned_malloc(temp_buffersize, 16);

			//Expand from crippled to full
			if(mode == CHAR || mode == MC_CHAR)
				rcolorsize = rmapsize/8;

			Destroy();
			Create(rmapsize, rcolorsize, rscreensize, rbackbuffers);

			file.read(temp_buffer, temp_buffersize);

			//Copy over lock
			for(int r=0;r<6;r++)
				lock[r]=BYTE(tmplock);

			//Copy over background
			*background = temp_buffer[1];

			//Copy over map
			memcpy(map, temp_buffer+1+4, rmapsize);

			//Copy over color
			if(mode == MC_CHAR)
			{
				memset(color, temp_buffer[1+4+rmapsize + 2], rcolorsize);
				ext[1] = temp_buffer[1+4+rmapsize + 1];
				ext[0] = temp_buffer[1+4+rmapsize + 0];

				//Copy over screen
				//memcpy(screen, temp_buffer+1+4+rmapsize+3, rscreensize);
			}
			else if(mode == CHAR)
			{
				memset(color, temp_buffer[1+4+rmapsize + 0], rcolorsize);

				//Copy over screen
				//memcpy(screen, temp_buffer+1+4+rmapsize+1, rscreensize);
			}
			else
			{
				memcpy(color, temp_buffer+1+4+rmapsize, rcolorsize);
				//Copy over screen
				memcpy(screen, temp_buffer+1+4+rmapsize+rcolorsize, rscreensize);
			}


			_aligned_free(temp_buffer);
		}

		SetPalette(GetMetaInt("palette"));
	}
}

void C64Interface::SetBackBuffer(int n)
{
	BYTE *p = buffer + 64 + (64 + rmapsize + rscreensize + rcolorsize) * n;

	p+=47;	//Extra bytes

	crippled = p;
	p+=6;

	lock = p;
	p+=6;

	border = p;
	p++;

	background = p;
	p++;

	ext = p;
	ext0 = p;
	p++;
	ext1 = p;
	p++;
	ext2 = p;
	p++;


	map = p;
	p += rmapsize;

	color = p;
	p += rcolorsize;

	screen = p;

	rbackbufnum = n;
}


void C64Interface::ClearBackBuffer(void)
{
	BYTE *p = buffer + 64 + (64 + rmapsize + rscreensize + rcolorsize) * rbackbufnum;
	ZeroMemory(p+64, (rmapsize + rscreensize + rcolorsize));
}


void C64Interface::Create(int mapsize, int colorsize, int screensize, int backbuffers)
{
	ASSERT(backbuffers);

	rmapsize = mapsize;
	rcolorsize = colorsize;
	rscreensize = screensize;
	rbackbuffers = backbuffers;

	buffersize = 64 + (64 + mapsize + screensize + colorsize) * backbuffers;

	buffer = (BYTE *)_aligned_malloc(buffersize, 16);
	tbuffer = (BYTE *)_aligned_malloc(buffersize, 16);
	
	ZeroMemory(buffer,buffersize);
	ZeroMemory(tbuffer,buffersize);

	for(int r=backbuffers-1;r>=0;r--)
	{
		SetBackBuffer(r);
		*border = 14;
	}

	ResetHistory();

	dirty = false;
}

void C64Interface::DuplicateGlobalsToBackBuffers(void)
{
	BYTE *src = map-64;
	int restore=GetBackBuffer();

	for(int r=0;r<rbackbuffers;r++)
	{
		SetBackBuffer(r);
		memcpy(map-64, src, 64);
	}

	SetBackBuffer(restore);
}

void C64Interface::Destroy()
{
	_aligned_free(buffer);
	_aligned_free(tbuffer);
	buffer = tbuffer = NULL;
}

void C64Interface::RemapCell1(int cx, int cy, int maskfrom, int maskto)
{
	assert(maskto == 0 || maskto == 1);
	int mi = GetMapIndexFromCell(cx, cy), value;

	value = maskto ? 0xff : 0x00;

	memset(&map[mi], value, sizecell);
}


void C64Interface::RemapCell2(int cx, int cy, int maskfrom, int maskto)
{
	int mi = GetMapIndexFromCell(cx, cy), t;
	for(t=0;t<sizecell;t++)
		map[mi+t]=RemapByte2(map[mi+t], maskfrom, maskto);
}

void C64Interface::SwapmaskCell2(int cx, int cy, int maskfrom, int maskto)
{
	int mi = GetMapIndexFromCell(cx, cy), t;
	for(t=0;t<sizecell;t++)
		map[mi+t]=SwapmaskByte2(map[mi+t], maskfrom, maskto);
}

void C64Interface::RemapCell2(int cx, int cy, int w, int h, int maskfrom, int maskto)
{
	int x,y;
	for(y=0;y<h;y++)
	{
		for(x=0;x<w;x++)
		{
			RemapCell2(cx+x,cy+y,maskfrom,maskto);
		}
	}
}

void C64Interface::SwapmaskCell2(int cx, int cy, int w, int h, int maskfrom, int maskto)
{
	int x,y;
	for(y=0;y<h;y++)
	{
		for(x=0;x<w;x++)
		{
			SwapmaskCell2(cx+x,cy+y,maskfrom,maskto);
		}
	}
}

BYTE C64Interface::RemapByte2(BYTE in, int maskfrom, int maskto)
{
	if((in&3)==maskfrom)in=(in&(~3))|maskto;
	if(((in>>2)&3)==maskfrom)in=(in&(~12))|(maskto<<2);
	if(((in>>4)&3)==maskfrom)in=(in&(~48))|(maskto<<4);
	if(((in>>6)&3)==maskfrom)in=(in&(~192))|(maskto<<6);
	return in;
}

BYTE C64Interface::SwapmaskByte2(BYTE in, int maskfrom, int maskto)
{
	if((in&3)==maskfrom)in=(in&(~3))|maskto;
	else if((in&3)==maskto)in=(in&(~3))|maskfrom;
	if(((in>>2)&3)==maskfrom)in=(in&(~12))|(maskto<<2);
	else if(((in>>2)&3)==maskto)in=(in&(~12))|(maskfrom<<2);
	if(((in>>4)&3)==maskfrom)in=(in&(~48))|(maskto<<4);
	else if(((in>>4)&3)==maskto)in=(in&(~48))|(maskfrom<<4);
	if(((in>>6)&3)==maskfrom)in=(in&(~192))|(maskto<<6);
	else if(((in>>6)&3)==maskto)in=(in&(~192))|(maskfrom<<6);
	return in;
}

void C64Interface::CountCell2(int cx, int cy, int *c)
{
	int mi = GetMapIndexFromCell(cx, cy), t;
	//c[0]=c[1]=c[2]=c[3]=0;
	for(t=0;t<sizecell;t++)
	{
		byte d=map[mi+t];
		c[d&3]++;
		c[(d>>2)&3]++;
		c[(d>>4)&3]++;
		c[d>>6]++;
	}
}

void C64Interface::CountCell2(int cx, int cy, int w, int h, int *c)
{
	int x,y;
	for(y=0;y<h;y++)
	{
		for(x=0;x<w;x++)
		{
			CountCell2(cx+x,cy+y,c);
		}
	}
}

void C64Interface::CountCell1(int cx, int cy, int *c)
{
	int mi = GetMapIndexFromCell(cx, cy), t, r;
	//c[0]=c[1]=0;
	for(t=0;t<sizecell;t++)
	{
		byte d=map[mi+t];

		for(r=0;r<8;r++)
		{
			c[d&1]++;
			d>>=1;
		}
	}
}

void C64Interface::CountCell1(int cx, int cy, int w, int h, int *c)
{
	int x,y;
	for(y=0;y<h;y++)
	{
		for(x=0;x<w;x++)
		{
			CountCell1(cx+x,cy+y,c);
		}
	}
}

BYTE C64Interface::GetCommonColorFromMask(int mask, BYTE *index, int size, bool high)
{
	int col[16];
	ZeroMemory(col, sizeof(col));
	int x,y;
	int h=GetCellCountY();
	int w=GetCellCountX();

	if(GetPixelWidth()==2)
	{
		assert(mask<4);

		for(y=0;y<h;y++)
		{
			for(x=0;x<w;x++)
			{
				int c[4]={0,0,0,0};
				CountCell2(x,y,c);
				if(c[mask])
				{
					if(high)
						col[index[y*w+x]>>4]++;
					else
						col[index[y*w+x]&0x0f]++;
				}
			}
		}
	}
	else
	{
		assert(mask<2);
		for(y=0;y<h;y++)
		{
			for(x=0;x<w;x++)
			{
				int c[2]={0,0};
				CountCell1(x,y,c);
				if(c[mask])
				{
					if(high)
						col[index[y*w+x]>>4]++;
					else
						col[index[y*w+x]&0x0f]++;
				}
			}
		}
	}

	return BYTE(ImportHelper::HighIndex(col, 16));
}


BYTE C64Interface::GetColor(int ci, int infoindex)
{
	InfoUse &use = infouse[infoindex];
	BYTE col = 0;
	BYTE *p = *use.pp;

	switch(use.use)
	{
	case InfoUse::INFO_VALUE:
		col = *p;
		break;
	case InfoUse::INFO_INDEX:
	case InfoUse::INFO_INDEX_LOW:
		col = p[ci]&0x0f;
		break;
	case InfoUse::INFO_INDEX_HIGH:
		col = (p[ci]>>4);
		break;
	default:
		assert(false);
		break;
	};

	return col;

}


void C64Interface::SetColor(int ci, int infoindex, BYTE col)
{
	InfoUse &use = infouse[infoindex];
	BYTE *p = *use.pp;

	if(!crippled[infoindex])
	{
		switch(use.use)
		{
		case InfoUse::INFO_VALUE:
			*p = col;
			break;
		case InfoUse::INFO_INDEX:
			p[ci]=col&0x0f;
			break;
		case InfoUse::INFO_INDEX_LOW:
			p[ci]=(p[ci]&0xf0)|(col&0x0f);
			break;
		case InfoUse::INFO_INDEX_HIGH:
			p[ci]=(p[ci]&0x0f)|(col<<4);
			break;
		default:
			assert(false);
			break;
		};
	}
	else
	{
		int size = GetCellCountX() * GetCellCountY();
		switch(use.use)
		{
		case InfoUse::INFO_VALUE:
			*p = col;
			break;
		case InfoUse::INFO_INDEX:
			if(*p!=col)
				memset(p, col, size);
			break;
		case InfoUse::INFO_INDEX_LOW:
			if(((*p)&0x0f)!=col)
			{
				for(int r=0;r<size;r++)
				{
					p[r]=(p[r]&0xf0)|(col&0x0f);
				}
			}
			break;
		case InfoUse::INFO_INDEX_HIGH:
			if(((*p)>>4)!=col)
			{
				for(int r=0;r<size;r++)
				{
					p[r]=(p[r]&0x0f)|(col<<4);
				}
			}
			break;
		default:
			assert(false);
			break;
		};
	}

}


int C64Interface::ResolveMask1(int ci, BYTE &col, int pointmask)
{
	int r,t;
	BYTE c[2];

	bool uglobal = false;	//Flag if unlocked globals exist

	//Collect colors available
	for(r=0;r<2;r++)
	{
		assert(infouse[r].use != InfoUse::INFO_NO);

		if(IsMaskGlobal(r) && lock[r]==0)
			uglobal=true;

		c[r]=GetColor(ci, r);
	}

	//Direct color match
	for(r=0;r<2;r++)
	{
		if(c[r] == col)
			return r;
	}

	//See if any indexed color is unused for the cell
	int num[2]={0,0};
	CountCell1(ci, 0, num);

	for(r=1;r>=0;r--)
	{
		if(IsMaskLocal(r))
		{
			if(num[r]==0 && lock[r]==0)
			{
				return r;
			}
		}
	}

	//Expensive, see if any global (value) color is unused, all bitmap check
	if(uglobal)
	{
		ZeroMemory(num, sizeof(num));
		CountCell1(0, 0, GetCellCountX(), GetCellCountY(), num);

		for(r=1;r>=0;r--)
		{
			if(IsMaskGlobal(r))
			{
				if(num[r]==0 && lock[r]==0)
						return r;
			}
		}
	}

	//See if any color can be remapped, ie same colors on two different masks
	for(t=0;t<2;t++)
	{
		for(r=1;r>=0;r--)
		{
			if(t==r)continue;
			if(IsMaskLocal(r) && c[t]==c[r] && lock[r]==0)
			{
				//Same color
				RemapCell1(ci, 0, r, t);
				return r;
			}
		}
	}

	//Replace
	if(overflow == REPLACE && lock[pointmask]==0)
	{
		return pointmask;
	}


	//Closest color
	if(overflow == CLOSEST)
	{
		int match = ClosestMatch(g_Vic2[col], c, 2);

		col = c[match];

		return match;
	}

	return -1;
}


int C64Interface::ResolveMask2(int ci, BYTE &col, int pointmask, bool charmode)
{
	int r,t;
	BYTE c[4];

	bool hicol = charmode && col>=8;	//True if char mode and the color is 8 and above
	bool uglobal = false;	//Flag if unlocked globals exist

	//Collect colors available
	for(r=0;r<4;r++)
	{
		assert(infouse[r].use != InfoUse::INFO_NO);

		if(IsMaskGlobal(r) && lock[r]==0)
			uglobal=true;

		c[r]=GetColor(ci, r);
	}

	//Direct color match
	for(r=0;r<4;r++)
	{
		if(c[r] == col)
			return r;
	}

	//See if any indexed color is unused for the cell
	int num[4]={0,0,0,0};
	CountCell2(ci, 0, num);

	if(hicol == false)
	{
		for(r=3;r>=0;r--)
		{
			if(IsMaskLocal(r))
			{
				if(num[r]==0 && lock[r]==0)
				{
					return r;
				}
			}
		}
	}

	//Expensive, see if any global (value) color is unused, all bitmap check
	if(uglobal)
	{
		ZeroMemory(num, sizeof(num));
		CountCell2(0, 0, GetCellCountX(), GetCellCountY(), num);

		for(r=3;r>=0;r--)
		{
			if(IsMaskGlobal(r))
			{
				if(num[r]==0 && lock[r]==0)
						return r;
			}
		}
	}

	//See if any color can be remapped, ie same colors on two different masks
	for(t=0;t<4;t++)
	{
		for(r=3;r>=0;r--)
		{
			if(hicol==false || r!=3)
			{
				if(t==r)continue;
				if(IsMaskLocal(r) && c[t]==c[r] && lock[r]==0)
				{
					//Same color
					RemapCell2(ci, 0, r, t);
					return r;
				}
			}
		}
	}

	if(hicol && uglobal)
	{
		//Special case for MC Char mode. Search for possible bitmap wide remap
		r=ResolveRemask2(c);
		if(r!=-1)
			return r;
	}

	//Replace
	if(overflow == REPLACE && lock[pointmask]==0)
	{
		if(hicol==false || pointmask!=3)
		{
			return pointmask;
		}
		else
		{
			r=ResolveReplaceRemask2(c);
			if(r!=-1)
				return r;
		}
	}


	//Closest color
	if(overflow == CLOSEST)
	{
		int match = ClosestMatch(g_Vic2[col], c, 4);

		col = c[match];

		return match;
	}

	return -1;
}

int C64Interface::ResolveRemask2(BYTE *c)
{
	int swap = -1, r;

	//Look for a global candidate
	if(lock[0]==0 && background[0]<8)swap=0;
	else if(lock[1]==0 && ext[0]<8)swap=1;
	else if(lock[2]==0 && ext[1]<8)swap=2;

	if(swap!=-1 && lock[3]==0)
	{
		//Check if the indexed color is a candidate
		narray<int,int> changelist;
		for(r=0;r<rcolorsize;r++)
		{
			int num[4]={0,0,0,0};
			CountCell2(r, 0, num);
			if(num[3])
			{
				return -1;
			}
			else if(num[swap])
			{
				changelist.add(r);
			}
		}

		SwapmaskCell2(0, 0, GetCellCountX(), GetCellCountY(), swap, 3);

		if(IsMaskGlobal(3))
		{
			memset(color, c[swap], rcolorsize);
		}
		else
		{
			for(r=0;r<changelist.count();r++)
			{
				color[changelist[r]]=c[swap];
			}
		}

		return swap;
	}

	return -1;
}

int C64Interface::ResolveReplaceRemask2(BYTE *c)
{
	int swap = -1, r;

	//Look for a global candidate
	if(lock[0]==0 && background[0]<8)swap=0;
	else if(lock[1]==0 && ext[0]<8)swap=1;
	else if(lock[2]==0 && ext[1]<8)swap=2;

	if(swap!=-1 && lock[3]==0)
	{
		//Check if the indexed color is a candidate
		BYTE cc=0xff;
		for(r=0;r<rcolorsize;r++)
		{
			int num[4]={0,0,0,0};
			CountCell2(r, 0, num);
			if(num[3])
			{
				if(cc!=0xff)
				{
					if(cc!=color[r])
						return -1;
				}
				else
				{
					cc=color[r];
				}
			}
		}

		SwapmaskCell2(0, 0, GetCellCountX(), GetCellCountY(), swap, 3);

		**infouse[swap].pp = c[3];

		return swap;
	}

	return -1;
}

void C64Interface::ResetHistory(void)
{
	for(int t=history_undo.count()-1;t>=0;t--)
	{
		delete [] history_undo[t];
		delete [] history_redo[t];
	}

	history_undo.count(0);
	history_redo.count(0);

	history_pos = 0;
}

void C64Interface::PlayHistory(BYTE *p)
{
	int r,t;
	unsigned short skipcount;
	unsigned short changedcount;

	for(r=0;;)
	{
		skipcount = p[0] | (p[1] << 8);
		if(!skipcount)break;
		p+=2;

		changedcount = p[0] | (p[1] << 8);
		//if(!changedcount)break;
		p+=2;

		r+=skipcount;
		t=r+changedcount;

		for(;r<t;r++,p++)
		{
			buffer[r] = *p;
		}
	}
}


C64Interface *C64Interface::CreateFromImage(CImage *img, int count, tmode type)
{
	C64Interface *i;

	switch(type)
	{
		case BITMAP:
			i = new Bitmap( ((img->GetWidth()+319)/320) * 320,  ((img->GetHeight()+199)/200) * 200, count );
			break;
		case MC_BITMAP:
			i = new MCBitmap( ((img->GetWidth()+319)/320) * 160,  ((img->GetHeight()+199)/200) * 200, count );
			break;
		case CHAR:
			i = new SFont( ((img->GetWidth()+7)/8) * 8,  ((img->GetHeight()+7)/8) * 8, count );
			break;
		case MC_CHAR:
			i = new MCFont( ((img->GetWidth()+7)/8) * 4,  ((img->GetHeight()+7)/8) * 8, count );
			break;
		case UNRESTRICTED:
			i = new Unrestricted(img->GetWidth(),  img->GetHeight(), false, count);
			break;
		case W_UNRESTRICTED:
			i = new Unrestricted((img->GetWidth()+1)/2,  img->GetHeight(), true, count);
			break;
		case SPRITE:
			i = new Sprite( ((img->GetWidth()+23)/24) * 24,  ((img->GetHeight()+20)/21) * 21, count );
			break;
		case MC_SPRITE:
			i = new MCSprite( ((img->GetWidth()+23)/24) * 12,  ((img->GetHeight()+20)/21) * 21, count );
			break;
		default:
			throw _T("Mode conversion not implemented");
			break;
	};

	for(int r=0; r<count; r++)
	{
		i->SetBackBuffer(r);
		i->Import(img[r]);
	}

	return i;
}

nstr C64Interface::IdentifyFile(LPCTSTR pszFileName)
{
	nmemfile file;
	FILE *fp=_tfopen(pszFileName,_T("rb"));
	if(!fp)
		throw _T("Unable to open file for reading");
	file.load(fp);
	fclose(fp);

	static nstr (*identify[])(nmemfile &)=
	{
		MCBitmap::IdentifyFile,
		Bitmap::IdentifyFile,
		Unrestricted::IdentifyFile,
		MCFont::IdentifyFile,
		SFont::IdentifyFile,
		Sprite::IdentifyFile,
		MCSprite::IdentifyFile,
		NULL
	};

	nstr ex;
	for(int r=0;;r++)
	{
		if(!identify[r])break;
		file.setpos(0,nstream::pos_begin);
		ex=identify[r](file);
		if(ex.isnotempty())break;
	}

	return ex;
}


C64Interface *C64Interface::Load(LPCTSTR pszFileName, LPCTSTR type, tmode mode, int width, int height)
{
	C64Interface *i;

	int version = 0;

	if(lstrcmpi(_T("bmp"),type)==0 || lstrcmpi(_T("png"),type)==0 || lstrcmpi(_T("jpg"),type)==0 || lstrcmpi(_T("gif"),type)==0)
	{
		CImage img, img24;
		if(img.Load(pszFileName) != 0)
			throw _T("Error opening file");

		//Make sure the image is 24 bit
		img24.Create(img.GetWidth(), img.GetHeight(), 24);
		img.BitBlt(img24.GetDC(), 0, 0, img.GetWidth(), img.GetHeight(), 0, 0);
		img24.ReleaseDC();

		i = CreateFromImage(&img24, 1, mode);

		i->file_name = pszFileName;

		return i;
	}

	nmemfile file;
	FILE *fp=_tfopen(pszFileName,_T("rb"));
	if(!fp)
		throw _T("Unable to open file for reading");
	file.load(fp);
	fclose(fp);

	if(!lstrcmpi(_T("gpx"),type))
	{
		BYTE *p = NULL;
		uLongf destsize = 0;

		for(;;)
		{
			free(p);
			destsize+=0x10000;
			p = (BYTE *)malloc(destsize);

			int n=uncompress(p, &destsize, file.buf(), (uLong)file.len());
			if(n == Z_BUF_ERROR) continue;
			if(n == Z_OK)break;

			delete [] p;
			throw _T("Error decompressing gpx-file, corrupt?");
		}

		free(file.detach());
		file.attach(p,destsize,true);

		int tmp;

		file >> version;
		file >> tmp;

		if(version > CURRENT_GPX_FILE_VERSION)
			throw _T("This GPX file was created with newer version of Pixcen!");

		if(version == 0)
			tmp = MC_BITMAP;

		mode = tmode(tmp);
	}

	i=Allocate(mode,width,height);

	try
	{
		i->Load(file, type, version);
	}
	catch(...)
	{
		delete i;
		throw;
	}

	return i;
}


C64Interface *C64Interface::Allocate(tmode mode, int w, int h, int buffers)
{
	C64Interface *i;

	switch(mode)
	{
	case MC_BITMAP:
		i = new class MCBitmap(w,h,buffers);
		break;
	case BITMAP:
		i = new class Bitmap(w,h,buffers);
		break;
	case MC_CHAR:
		i = new class MCFont(w,h,buffers);
		break;
	case CHAR:
		i = new class SFont(w,h,buffers);
		break;
	case SPRITE:
		i = new class Sprite(w,h,buffers);
		break;
	case MC_SPRITE:
		i = new class MCSprite(w,h,buffers);
		break;
	case UNRESTRICTED:
		i = new class Unrestricted(w, h, false, buffers);
		break;
	case W_UNRESTRICTED:
		i = new class Unrestricted(w, h, true, buffers);
		break;
	default:
		throw _T("Wooh, unknown format.");
	};

	return i;
}

C64Interface *C64Interface::CreateFromSelection(int x, int y, int w, int h)
{
	C64Interface *i = NULL;

	try
	{
		if(x%xcell)throw -1;
		if(y%ycell)throw -1;
		if(w%xcell)throw -1;
		if(h%ycell)throw -1;

		i = Allocate(mode, w, h, 1);

		int cx,cy,cw=w/xcell,ch=h/ycell;

		int sx=x/xcell;
		int sy=y/ycell;

		for(cy=0;cy<ch;cy++)
		{
			for(cx=0;cx<cw;cx++)
			{
				//Copy map
				memcpy(&i->map[i->GetMapIndexFromCell(cx,cy)], &map[GetMapIndexFromCell(sx+cx,sy+cy)], offsetcell);

				int di = cy * i->GetCellCountX() + cx;
				int si = (sy+cy) * GetCellCountX() + (sx+cx);

				//Copy color
				if(rcolorsize)
				{
					i->color[di]=color[si];
				}

				//Copy screen
				if(rscreensize)
				{
					i->screen[di]=screen[si];
				}
			}
		}

		//Other data, global colors, locks, etc, 64 bytes before map pointer
		memcpy(i->map-64, map-64, 64);
	}
	catch(...)
	{
		if(i)delete i;

		return NULL;
	}

	return i;
}

//Default slow implementation. All formats should override this
void C64Interface::GetPixelBatch(BYTE *p, int xs, int ys, int w, int h)
{
	for (int y = ys; y < ys + h; y++)
	{
		for (int x = xs; x < xs + w; x++)
		{
			*p = GetPixel(x, y);
			p++;
		}
	}
}


C64Interface::ImportHelper::ImportHelper(C64Interface *pp, CImage &inimg, bool inwide)
	:img(static_cast<CImageFast &>(inimg))
	,parent(pp)
{
	xsize=parent->GetSizeX();
	ysize=parent->GetSizeY();
	xcs=parent->GetCellSizeX();
	ycs=parent->GetCellSizeY();
	wide=inwide;

	p=new BYTE[xsize*ysize];
	ZeroMemory(p, xsize*ysize);

	//Identify what palette have the best match
	std::unordered_set<COLORREF> unique;
	COLORREF *prgbmap = new COLORREF[xsize*ysize];

	bool many = false;

	int x, y;
	for(y=0;y<ysize;y++)
	{
		for(x=0;x<xsize;x++)
		{
			COLORREF c = GetImagePixel(x,y);
			prgbmap[y * xsize + x] = c;

			if(!many)
			{
				many = unique.size() >= 32;
				unique.insert(c);
			}
		}
	}

	if(!many)
	{
		COLORREF *unmap = new COLORREF[unique.size()];

		int r=0;
		for(std::unordered_set<COLORREF>::iterator i=unique.begin();i!=unique.end();i++,r++)
		{
			unmap[r]=*i;
		}

		palette = ClosestPalette(unmap, r);

		delete unmap;
	}
	else
	{
		//Just using pepto for anything truly RGB
		palette = 1;
	}

	//Set the palette and use the regular color reduction
	
	int old = SetPalette(palette);

	paralell_for(ysize, [this, prgbmap](int y)
	{
		for (int x = 0; x<xsize; x++)
		{
			p[y * xsize + x] = ToC64Index(prgbmap[y * xsize + x]);
		}
	});

	delete [] prgbmap;

	//Restore palette
	SetPalette(old);
	
}

C64Interface::ImportHelper::~ImportHelper()
{
	parent->paralell_for_ycell(0, ysize, [this](int y)
	{
		for (int x = 0; x<xsize; x++)
		{
			parent->SetPixel(x, y, GetPixel(x, y));
		}
	});

	delete [] p;
}

COLORREF C64Interface::ImportHelper::GetImagePixel(int x, int y)
{
	if(wide)
	{
		x*=2;

		if(x >= img.GetWidth() || y >= img.GetHeight()) return 0;
		if(x+1>=img.GetWidth()) return img.GetPixel(x,y);
		else return blend(img.GetPixel(x+0,y),img.GetPixel(x+1,y));
	}

	if(x >= img.GetWidth() || y >= img.GetHeight()) return 0;

	return img.GetPixel(x,y);
}


int C64Interface::ImportHelper::CountColors(int cx, int cy, int w, int h, int *colcount)
{
	ZeroMemory(colcount, 16*sizeof(int));

	int x,y,num=0;
	BYTE b;

	for(y=cy*ycs;y<(cy+h)*ycs;y++)
	{
		for(x=cx*xcs;x<(cx+w)*xcs;x++)
		{
			b=GetPixel(x,y);
			if(!colcount[b])num++;
			colcount[b]++;
		}
	}

	return num;
}


int C64Interface::ImportHelper::CountTopColors(int cx, int cy, int w, int h, BYTE *col)
{
	int colcount2[16], count=0;

	CountColors(cx,cy,w,h,colcount2);

	for(;;count++)
	{
		int i = highindex(colcount2, 16);
		if(i == -1)break;

		colcount2[i] = -1;
		col[count] = BYTE(i);
	}

	return count;
}


int C64Interface::ImportHelper::CountTopColorsPerCell(BYTE *col, int ceil)
{
	int colcount[16], count=0;
	int colcount2[16];
	ZeroMemory(colcount2, 16*sizeof(int));

	int cx,cy;

	for(cy=0;cy<ysize/ycs;cy++)
	{
		for(cx=0;cx<xsize/xcs;cx++)
		{
			if(CountColors(cx,cy,1,1,colcount)>=ceil)
			{
				for(int r=0;r<16;r++)
				{
					if(colcount[r])
						colcount2[r]++;
				}
			}
		}
	}

	for(;;count++)
	{
		int i = highindex(colcount2, 16);
		if(i == -1)break;

		colcount2[i] = -1;
		col[count] = BYTE(i);
	}

	if(!count)
	{
		return CountTopColorsPerCell(col, 1);
	}

	return count;
}

void C64Interface::ImportHelper::ReduceColors(int maxcolor, BYTE *force, int forcecount)
{
	int old = SetPalette(palette);

	paralell_for(ysize / ycs, [this, maxcolor, force, forcecount](int cy)
	{
		for (int cx = 0; cx<xsize / xcs; cx++)
		{
			BYTE col[16];
			BYTE use[16];
			ZeroMemory(use, 16);

			int num = CountTopColors(cx, cy, 1, 1, col);

			memcpy(use, force, forcecount);
			int fc = forcecount;

			for (int r = 0; r<num && fc != maxcolor; r++)
			{
				if (!find(col[r], use, fc))
				{
					use[fc] = col[r];
					fc++;
				}
			}

			for (int y = 0; y<ycs; y++)
			{
				for (int x = 0; x<xcs; x++)
				{
					COLORREF c = GetImagePixel(cx*xcs + x, cy*ycs + y);
					int mi = ClosestMatch(c, use, fc);
					SetPixel(cx*xcs + x, cy*ycs + y, use[mi]);
				}
			}
		}
	});

	SetPalette(old);
}

