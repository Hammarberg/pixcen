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

// autoptr.h: interface for the autoptr class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_AUTOPTR_H__BB9C2421_D5A9_11D4_9E05_0040F634D74E__INCLUDED_)
#define AFX_AUTOPTR_H__BB9C2421_D5A9_11D4_9E05_0040F634D74E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#pragma warning( push )
#pragma warning( disable : 4284 )

#include <assert.h>


//--------------------------------------------------------------------------------------

template<class T> 
class autoptr  
{
public:
	autoptr(const autoptr &p)
	{
		b=p.b;
		if(b)
		{
#ifdef _MT
			InterlockedIncrement(&b->r);
#else
			b->r++;
#endif
		}
	}
	autoptr(T *p)
	{
		if(p)
		{
			b=new chk;
			b->r=1;
			b->p=p;
		}
		else b=NULL;
	}
	autoptr()
	{
		b=NULL;
	}
	~autoptr()
	{
		deref();
	}

	bool operator>(const autoptr &o)const
	{
		return (*b->p)>(*o.b->p);
	}

	bool operator<(const autoptr &o)const
	{
		return (*b->p)<(*o.b->p);
	}

	bool operator==(const autoptr &o)const
	{
		return (*b->p)==(*o.b->p);
	}

	autoptr &operator=(T *p)
	{
		deref();

		if(p)
		{
			b=new chk;
			b->r=1;
			b->p=p;
		}
		else b=NULL;
		return *this;
	}

	autoptr &operator=(const autoptr &o)
	{
		deref();
		if(o.b)
		{
			b=o.b;
#ifdef _MT
			InterlockedIncrement(&b->r);
#else
			if(b)b->r++;
#endif
		}
		else b=NULL;
		return *this;
	}

	T *operator->(){return b->p;}
	T &operator*(){return *b->p;}

	operator T*(){if(b)return b->p;return NULL;}

	bool isnull(void)const
	{
		if(b)
		{
			if(b->p)return false;
			return true;
		}

		return true;
	}

	void incref(void)
	{
		if(b)
		{
#ifdef _MT
			InterlockedIncrement(&b->r);
#else
			b->r++;
#endif
		}
	}

private:

	class chk
	{
	public:
		long r;
		T *p;
	};

	chk *b;

	void deref(void)
	{
		if(b)
		{
#ifdef _MT
			if(!InterlockedDecrement(&b->r))
#else
			if(!--b->r)
#endif
			{
				if(b->p)delete b->p;
				delete b;
				b=NULL;
			}
		}
	}

};


#pragma warning( pop )

#endif // !defined(AFX_AUTOPTR_H__BB9C2421_D5A9_11D4_9E05_0040F634D74E__INCLUDED_)
