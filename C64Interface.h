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

#include "atlimage.h"
#include "C64Col.h"
#include "narray.h"
#include "nstr.h"
#include "nmemfile.h"
#include "autoptr.h"
#include "bb.h"

#include <map>

#define CURRENT_GPX_FILE_VERSION 4

class SaveFormat
{
public:
	SaveFormat(nstr a, nstr b, bool gd, int w=0, int h=0, int typ=-1)
	{
		name = a;
		good = gd;

		size_t n;

		while((n=b.find(_T(';'))) !=-1)
		{
			ext.add(b.left(n));
			b.cut(0,n+1);
		}
		ext.add(b);

		type=typ;
		width = w;
		height = h;
	}

	nstr name;
	narray<nstr,int> ext;
	bool good;
	int width, height;
	int type;

	bool MatchExt(LPCTSTR ex)
	{
		for(int r=0;r<ext.count();r++)
		{
			if(!ext[r].cmpi(ex))
				return true;
		}

		return false;
	}

};

class CellInfo
{
public:
	int w;
	int h;
	BYTE col[6];
	int lock[6];
	int crippled[6];

	CellInfo()
	{
		w=-1;
		h=-1;
		for(int r=0;r<sizeof(col);r++)
		{
			col[r]=0xff;
			crippled[r]=lock[r]=-1;
		}
	}
};


class C64Interface
{
public:

	typedef enum
	{
		BITMAP,
		MC_BITMAP,
		SPRITE,
		MC_SPRITE,
		CHAR,
		MC_CHAR,
		UNUSED1,
		UNUSED2,
		UNRESTRICTED,
		W_UNRESTRICTED
	}tmode;

	typedef enum
	{
		REPLACE,
		NOTHING,
		CLOSEST
	}toverflow;

	C64Interface();
	virtual ~C64Interface();

	// Must overload
	virtual BYTE GetPixel(int x, int y) = NULL;
	virtual void SetPixel(int x, int y, BYTE color) = NULL;

	//Should overload
	virtual void GetPixelBatch(BYTE *p, int x, int y, int w, int h);	//Default slow implementation

	virtual void GetCellInfo(int cx, int cy, int w, int h, CellInfo *info);
	virtual void SetCellInfo(int cx, int cy, int w, int h, CellInfo *info);

	virtual int GetPixelWidth(void){return 1;};
	virtual void GetSaveFormats(narray<autoptr<SaveFormat>,int> &fmt);
	virtual bool CanOptimize(void){return false;}
	virtual int GetCharCount(void){return 0;}

	static void GetLoadFormats(narray<autoptr<SaveFormat>,int> &fmt);
	static nstr IdentifyFile(nmemfile &file){return _T("");};

	virtual void CustomCommand(int n){}

//protected:

	virtual void Import(CImage &img){}
	virtual void Save(nmemfile &file, LPCTSTR type);
	virtual void Load(nmemfile &file, LPCTSTR type, int version);

	virtual void SetFontDisplay(int mode) {}
	virtual int GetFontDisplay(void){return 0;}

public:

	int GetSizeX(void){return xsize;}
	int GetSizeY(void){return ysize;}
	int GetCellSizeX(void){return xcell;}
	int GetCellSizeY(void){return ycell;}
	int GetCellCountX(void){return xsize/xcell;}
	int GetCellCountY(void){return ysize/ycell;}

	BYTE GetBackground(void){return infouse[0].use != InfoUse::INFO_NO ? *infouse[0].pp[0] : 0;}

	int GuessBorderColor(void);
	BYTE GetBorderColor(void){return *border;};

	COLORREF GetPixelCR(int x, int y){return g_Vic2[GetPixel(x,y)];}

	void BeginHistory(void);
	void EndHistory(void);
	void InheritHistory(C64Interface *old);
	void DeleteHistory(void);

	bool CanUndo(void){return history_pos > 0 || history_previous;}
	bool CanRedo(void){return history_pos < history_redo.count() || history_next;}
	C64Interface *Undo(void);
	C64Interface *Redo(void);

	bool IsDirty(void){return dirty;}
	void SetDirty(void){dirty=true;}
	void ClearDirty(void){dirty=false;}

	LPCTSTR GetFileName(void){return file_name;}
	void SetFileName(LPCTSTR s){file_name=s;}
	nstr GetFileExt(void)
	{
		size_t n=file_name.rfind(_T('.'));
		if(n==-1)return _T("");
		return file_name.mid(n+1);
	}

	void Save(LPCTSTR pszFileName, LPCTSTR type);

	static nstr IdentifyFile(LPCTSTR pszFileName);
	static C64Interface *Load(LPCTSTR pszFileName, LPCTSTR type, tmode importmode, int width, int height);
	static C64Interface *CreateFromImage(CImage *img, int count, tmode type);
	static C64Interface *Allocate(tmode mode, int w, int h, int buffers=1);

	virtual C64Interface *CreateFromSelection(int x, int y, int w, int h);

	void Optimize(void);

	bool IsBitmap(void){return mode == BITMAP || mode == MC_BITMAP;}
	bool IsSprite(void){return mode == SPRITE || mode == MC_SPRITE;}
	bool IsChar(void){return mode == CHAR || mode == MC_CHAR;}
	bool IsMultiColor(void){return mode == MC_CHAR || mode == MC_SPRITE || mode == MC_BITMAP || mode == W_UNRESTRICTED;}
	bool IsSingleColor(void){return !IsMultiColor();}
	bool IsUnrestricted(void){return mode == W_UNRESTRICTED || mode == UNRESTRICTED;}
	tmode GetMode(void){return mode;}

	void RenderImage(CImage &img, int startx=0, int starty=0, int width=-1, int height=-1);

	void SetBackBuffer(int n);
	int GetBackBuffer(void){return rbackbufnum;}
	int GetBackBufferCount(void){return rbackbuffers;}
	void ClearBackBuffer(void);

	int GetPAR(void){return par;}
	double GetPARValue(void){return parvalue[par];}
	void SetPAR(int n){par=n;}

	toverflow GetOverflow(void){return overflow;}
	void SetOverflow(toverflow n){overflow = n;}

	void SetMetaInt(LPCSTR tag, int value){WCHAR buf[32];_itow(value, buf, 10);meta[tag]=buf;}
	int GetMetaInt(LPCSTR tag){return _wtoi(meta[tag]);}
	void SetMetaStr(LPCSTR tag, nstr value){meta[tag]=value;}
	nstr GetMetaStr(LPCSTR tag){return meta[tag];}

protected:

	BYTE *crippled;
	BYTE *lock;
	BYTE *border;
	BYTE *background;
	BYTE *ext;	//3

	BYTE *ext0;
	BYTE *ext1;
	BYTE *ext2;

	tmode mode;
	toverflow overflow;

	int xsize, ysize;				//Size in pixels
	int xcell, ycell;				//Size of cell in pixels
	int sizecell, offsetcell;		//Byte size of cell, byte size of cell offset

	int rmapsize, rcolorsize, rscreensize, rbackbuffers;
	int buffersize;

	int rbackbufnum;

	BYTE *buffer;
	BYTE *tbuffer;

	BYTE *map;
	BYTE *color;
	BYTE *screen;

	int par;	//0=PC, 1=PAL, 2=NTSC

	std::map<nstrc,nstrw> meta;

	class InfoUse
	{
	public:

		typedef enum
		{
			INFO_NO,
			INFO_VALUE,
			INFO_INDEX,
			INFO_INDEX_LOW,
			INFO_INDEX_HIGH
		}tuse;

		InfoUse(){use = INFO_NO;}

		tuse use;
		BYTE **pp;

	};

	InfoUse infouse[6];

	bool IsMaskGlobal(int mask){return infouse[mask].use == InfoUse::INFO_VALUE || crippled[mask];}
	bool IsMaskLocal(int mask){return !IsMaskGlobal(mask);}

	int32_t history_pos;
	narray<BYTE *,int32_t> history_undo;
	narray<BYTE *,int32_t> history_redo;
	C64Interface *history_previous, *history_next;

	void DeleteHistoryPrevious(void);
	void DeleteHistoryNext(void);

	void Create(int mapsize, int colorsize, int screensize, int backbuffers=1);
	void DuplicateGlobalsToBackBuffers(void);

	void Destroy();

	void RemapCell1(int cx, int cy, int maskfrom, int maskto);
	void RemapCell2(int cx, int cy, int maskfrom, int maskto);
	void RemapCell2(int cx, int cy, int w, int h, int maskfrom, int maskto);

	void SwapmaskCell2(int cx, int cy, int maskfrom, int maskto);
	void SwapmaskCell2(int cx, int cy, int w, int h, int maskfrom, int maskto);

	static BYTE RemapByte2(BYTE in, int maskfrom, int maskto);
	static BYTE SwapmaskByte2(BYTE in, int maskfrom, int maskto);

	void CountCell2(int cx, int cy, int *c);
	void CountCell2(int cx, int cy, int w, int h, int *c);

	void CountCell1(int cx, int cy, int *c);
	void CountCell1(int cx, int cy, int w, int h, int *c);

	BYTE GetCommonColorFromMask(int mask, BYTE *index, int size, bool high=false);

	BYTE GetColor(int ci, int infoindex);
	void SetColor(int ci, int infoindex, BYTE col);

	int ResolveMask1(int ci, BYTE &col, int pointmask);
	int ResolveMask2(int ci, BYTE &col, int pointmask, bool charmode=false);
	int ResolveRemask2(BYTE *c);
	int ResolveReplaceRemask2(BYTE *c);

	int GetMapIndexFromCell(int cx, int cy){return ((xsize/xcell) * cy + cx) * offsetcell;}
	int GetMapIndex(int x, int y){return 0 ;}

	void PushLocks(void){memcpy(savelock, lock, 6);}
	void PopLocks(void){memcpy(lock, savelock, 6);}

	void ResetHistory(void);

	//A wrapper class to Get- and SetPixel straight into the DIB rather than detour via CDC
	class CImageFast : public CImage
	{
	public:
		COLORREF GetPixel(int x, int y)
		{
			unsigned char *p = (unsigned char *)GetPixelAddress(x, y);
			return p[0] << 16 | p[1] << 8 | p[2];
		}

		void SetPixel(int x, int y, COLORREF color)
		{
			unsigned char *p = (unsigned char *)GetPixelAddress(x, y);
			p[0] = (color >> 16) & 255;
			p[1] = (color >> 8) & 255;
			p[2] = color & 255;
		}
	};

	class ImportHelper
	{
	public:
		ImportHelper(CImage &img, int x, int y, int xcellsize, int ycellsize, bool wide);
		~ImportHelper();

		BYTE GetPixel(int x, int y){return p[y*xsize+x];}
		void SetPixel(int x, int y, BYTE c){p[y*xsize+x]=c;}

		int CountColors(int cx, int cy, int w, int h, int *colcount);
		int CountTopColors(int cx, int cy, int w, int h, BYTE *col);

		int CountTopColorsPerCell(BYTE *col, int ceil=4);
		void ReduceColors(int maxcolor, BYTE *force, int forcecount);

		static int HighIndex(int *list, int count)
		{
			return highindex(list, count);
		}

	private:
		CImageFast &img;

		COLORREF GetImagePixel(int x, int y);

		int xsize, ysize, xcs, ycs;

		bool wide;

		int palette;
		BYTE *p;

		bool find(BYTE col, BYTE *p, int count)
		{
			for(int r=0;r<count;r++)
			{
				if(col == p[r])
					return true;
			}
			return false;
		}

		static int highindex(int *list, int count)
		{
			int high = 0;
			int best = -1;
			for(int r=0;r<count;r++)
			{
				if(list[r] > high)
				{
					high = list[r];
					best = r;
				}
			}

			return best;
		}

	};

private:

	BYTE savelock[6];

	static const double parvalue[3];

	nstr file_name;

	bool dirty;

	void PlayHistory(BYTE *p);

	void GetMaskInfo(int cx, int cy, int w, int h, int infoindex, CellInfo &info);
	void SetMaskInfo(int cx, int cy, int w, int h, int infoindex, CellInfo &info);

	bool mymemspn_low(const BYTE *ptr, BYTE value, int num)
	{
		for(;num>1;num--,ptr++)
		{
			if(((*ptr)&0x0f)!=value)
				return false;
		}
		return true;
	}

	bool mymemspn_high(const BYTE *ptr, BYTE value, int num)
	{
		for(;num>1;num--,ptr++)
		{
			if(((*ptr)&0xf0)!=value)
				return false;
		}
		return true;
	}
};


class MCBitmap : public C64Interface
{
friend class C64Interface;
friend class Bitmap;
public:

	MCBitmap(int x=160, int y=200, int backbuffers=1);
	virtual ~MCBitmap();

protected:

	BYTE GetPixel(int x, int y);
	void SetPixel(int x, int y, BYTE col);
	void GetPixelBatch(BYTE *p, int x, int y, int w, int h) override;
	int GetPixelWidth(void){return 2;}
	void GetSaveFormats(narray<autoptr<SaveFormat>,int> &fmt);
	bool CanOptimize(void){return true;}
	void Import(CImage &img);
	void Load(nmemfile &file, LPCTSTR type, int version);
	void Save(nmemfile &file, LPCTSTR type);

	static void GetLoadFormats(narray<autoptr<SaveFormat>,int> &fmt);

	static nstr IdentifyFile(nmemfile &file);

private:
	inline BYTE GetPixelInternal(int x, int y);

	static int DecompressKoalaStream(const BYTE *stream, int stream_size, BYTE *buffer, int buffer_size);
	static int CompressKoalaStream(const BYTE *stream, int stream_size, BYTE *buffer, int buffer_size);

	int GetMask(int x, int y);
};

class Bitmap : public C64Interface
{
friend class C64Interface;
public:

	Bitmap(int x=320, int y=200, int backbuffers=1);
	virtual ~Bitmap();

protected:

	BYTE GetPixel(int x, int y);
	void SetPixel(int x, int y, BYTE col);
	void GetPixelBatch(BYTE *p, int x, int y, int w, int h) override;
	void GetSaveFormats(narray<autoptr<SaveFormat>,int> &fmt);
	bool CanOptimize(void){return true;}
	void Import(CImage &img);
	void Load(nmemfile &file, LPCTSTR type, int version);
	void Save(nmemfile &file, LPCTSTR type);

	static void GetLoadFormats(narray<autoptr<SaveFormat>,int> &fmt);

	static nstr IdentifyFile(nmemfile &file);

private:
	inline BYTE GetPixelInternal(int x, int y);

	int GetMask(int x, int y);
};


class Unrestricted : public C64Interface
{
friend class C64Interface;
public:

	Unrestricted(int x=320, int y=200, bool wide = false, int backbuffers=1);
	virtual ~Unrestricted();

protected:

	BYTE GetPixel(int x, int y);
	void SetPixel(int x, int y, BYTE col);
	void GetPixelBatch(BYTE *p, int x, int y, int w, int h) override;
	void GetCellInfo(int cx, int cy, int w, int h, CellInfo *info);
	int GetPixelWidth(void){return mode == W_UNRESTRICTED ? 2 : 1;}
	void GetSaveFormats(narray<autoptr<SaveFormat>,int> &fmt);
	bool CanOptimize(void){return false;}
	void Import(CImage &img);
	void Save(nmemfile &file, LPCTSTR type);

	C64Interface *CreateFromSelection(int x, int y, int w, int h);

	static void GetLoadFormats(narray<autoptr<SaveFormat>,int> &fmt);

private:
	inline BYTE GetPixelInternal(int x, int y);
};

class CommonFont : public C64Interface
{
friend class C64Interface;

public: 
	enum tfontdisplay : int
	{
		FONT_1X1 = 0,
		FONT_1X2,
		FONT_2X1,
		FONT_2X2
	};

protected:
	CommonFont();
	virtual ~CommonFont();

	volatile int charcount;

	int CreateCharScreen(narray<BYTE,int> &c, narray<BYTE,int> &s, bool space = false);

	void GetSaveFormats(narray<autoptr<SaveFormat>,int> &fmt);
	void Save(nmemfile &file, LPCTSTR type);
	void Load(nmemfile &file, LPCTSTR type, int version);

	void SetFontDisplay(int mode) override;
	int GetFontDisplay(void) override {return fontdisplay;}

	void TranslateFontDisplay(int &cx, int &cy);

private:
	CEvent quit;
	HANDLE thread;

	tfontdisplay fontdisplay;

	virtual int GetCharCount(void){return charcount;}
	static unsigned int __stdcall threadentry(void *in);
};


class MCFont : public CommonFont
{
friend class C64Interface;
public:

	MCFont(int x, int y, int backbuffers=1);
	virtual ~MCFont();

protected:

	BYTE GetPixel(int x, int y);
	void SetPixel(int x, int y, BYTE col);
	void GetSaveFormats(narray<autoptr<SaveFormat>,int> &fmt);
	void Import(CImage &img);
	void Load(nmemfile &file, LPCTSTR type, int version);
	void Save(nmemfile &file, LPCTSTR type);
	int GetPixelWidth(void){return 2;};

	static void GetLoadFormats(narray<autoptr<SaveFormat>,int> &fmt);

private:

	int GetMask(int x, int y);
};

class SFont : public CommonFont
{
friend class C64Interface;
public:

	SFont(int x, int y, int backbuffers=1);
	virtual ~SFont();

protected:

	BYTE GetPixel(int x, int y);
	void SetPixel(int x, int y, BYTE col);
	void GetSaveFormats(narray<autoptr<SaveFormat>,int> &fmt);
	void Import(CImage &img);
	void Load(nmemfile &file, LPCTSTR type, int version);
	void Save(nmemfile &file, LPCTSTR type);

	void CustomCommand(int n);

	static void GetLoadFormats(narray<autoptr<SaveFormat>,int> &fmt);

private:

	int GetMask(int x, int y);
};

class Sprite : public C64Interface
{
friend class C64Interface;
public:
	Sprite(int x, int y, int backbuffers=1);
	virtual ~Sprite();

protected:
	BYTE GetPixel(int x, int y);
	void SetPixel(int x, int y, BYTE color);

	static void GetLoadFormats(narray<autoptr<SaveFormat>,int> &fmt);
	void GetSaveFormats(narray<autoptr<SaveFormat>,int> &fmt);
	void Load(nmemfile &file, LPCTSTR type, int version);
	void Save(nmemfile &file, LPCTSTR type);
	void Import(CImage &img);

private:
	int GetMask(int x, int y);

};

class MCSprite : public C64Interface
{
friend class C64Interface;
public:
	MCSprite(int x, int y, int backbuffers=1);
	virtual ~MCSprite();

protected:
	BYTE GetPixel(int x, int y);
	void SetPixel(int x, int y, BYTE color);

	static void GetLoadFormats(narray<autoptr<SaveFormat>,int> &fmt);
	void GetSaveFormats(narray<autoptr<SaveFormat>,int> &fmt);
	void Load(nmemfile &file, LPCTSTR type, int version);
	void Save(nmemfile &file, LPCTSTR type);
	void Import(CImage &img);

	int GetPixelWidth(void){return 2;};

private:

	int GetMask(int x, int y);
};
