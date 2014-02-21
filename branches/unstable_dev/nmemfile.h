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


// nmemfile.h: interface for the nmemfile class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NMEMFILE_H__6C95F2EF_B3E8_44F9_8591_75AFCC10678F__INCLUDED_)
#define AFX_NMEMFILE_H__6C95F2EF_B3E8_44F9_8591_75AFCC10678F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "nstream.h"


class nmemfile : public nstream  
{
public:
	nmemfile(size_t initsize=0, size_t growsize=-1)
	{
		pos=leng=0;
		alloc=initsize;
		grow=growsize;

		if(alloc)
			ptr=(unsigned char *)malloc(alloc);
		else
			ptr=NULL;

		del=true;
	}

	nmemfile(void *p, size_t l, bool delp=false)
	{
		attach(p,l,delp);
	}

#ifdef _INC_STDIO

	//Load from the current FP to the end of file
	bool load(FILE *fp)
	{

		if(del)free(ptr);

		fpos_t current;
		fpos_t size;
		if(fgetpos(fp,&current))return false;
		_fseeki64(fp,0,SEEK_END);
		if(fgetpos(fp,&size))return false;
		_fseeki64(fp,current,SEEK_SET);
		size=size-current;

		ptr=(unsigned char *)malloc((size_t)size);
		if(fread(ptr,1,(size_t)size,fp)!=size)return false;

		pos=0;
		alloc=leng=(size_t)size;
		grow=-1;
		del=true;

		return true;
	}

	bool save(FILE *fp)
	{
		return fwrite(ptr,1,leng,fp)==leng?true:false;
	}

#endif

#ifdef NWINFILE_H
	//Load from the current pos to the end of file
	bool load(nwinfile &file)
	{
		if(del)free(ptr);

		fpos_t current=file.getpos();
		fpos_t size=file.getsize();
		size=size-current;

		ptr=(unsigned char *)malloc((size_t)size);
		if(file.read(ptr,(long)size)!=size)return false;

		pos=0;
		alloc=leng=(size_t)size;
		grow=-1;
		del=true;

		return true;
	}

	bool save(nwinfile &file)
	{
		return file.write(ptr,(long)leng)==leng?true:false;
	}

#endif

	virtual ~nmemfile()
	{
		if(del)
			free(ptr);
	}

	operator unsigned char *(){return ptr;}
	operator char *(){return (char *)ptr;}
	size_t len(void)const{return leng;}

	unsigned char *buf(void){return ptr;}
	void setlen(size_t l){leng=l;if(leng<pos)pos=leng;}

	void attach(void *p, size_t l, bool delp=false)
	{
		pos=0;
		alloc=leng=l;
		ptr=(unsigned char *)p;
		grow=-1;
		del=delp;
	}

	void *detach(void)
	{
		del=false;
		return ptr;
	}

	size_t write(const void *p, size_t l)
	{
		if(pos+l>alloc)
		{
			if(!del)
			{
				//Buffer full, cant write all of it
				memcpy(ptr+pos,p,l=alloc-pos);
				return l;
			}

			size_t needed=(pos+l)-alloc;
			size_t nalloc=grow!=-1?grow:(alloc?alloc:256);
			alloc+=needed>nalloc?needed:nalloc;

			ptr=(unsigned char *)realloc(ptr,alloc);
			//unsigned char *n=new unsigned char[alloc];
			//memcpy(n,ptr,leng);	//Move old buffer
			//delete [] ptr;
			//ptr=n;
		}

		memcpy(ptr+pos,p,l);
		pos+=l;

		if(pos>leng)leng=pos;

		return l;
	}

	size_t read(void *p, size_t l)
	{
		if(pos+l>leng)
			l=leng-pos;

		memcpy(p,ptr+pos,l);
		pos+=l;

		return l;
	}

	void *read(size_t l)
	{
		if(pos+l>leng)
			l=leng-pos;

		void *r=(void *)(ptr+pos);
		pos+=l;
		return r;
	}

	size_t sizeleft(void)const
	{
		return leng-pos;
	}

	__int64 setpos(__int64 delta, pos_type how=pos_begin)
	{
		__int64 newpos;

		switch(how)
		{
		case nstream::pos_begin:
			newpos=delta;
			break;
		case nstream::pos_current:
			newpos=pos+delta;
			break;
		case nstream::pos_end:
			newpos=leng+delta;
			break;
		default:
			return -1;
		}

		if(newpos<0 || newpos > __int64(leng))return -1;

		pos=(size_t)newpos;
		return newpos;
	}

private:
	unsigned char *ptr;
	size_t leng,alloc,pos,grow;
	bool del;

};

#endif // !defined(AFX_NMEMFILE_H__6C95F2EF_B3E8_44F9_8591_75AFCC10678F__INCLUDED_)
