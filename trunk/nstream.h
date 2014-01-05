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


// nstream.h: interface for the nstream class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_NSTREAM_H__F3F970C6_1445_472B_951C_A1C1A03DA3FC__INCLUDED_)
#define AFX_NSTREAM_H__F3F970C6_1445_472B_951C_A1C1A03DA3FC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class nstream  
{
public:
	nstream(){};
	virtual ~nstream(){};

	virtual size_t write(const void *p, size_t l)=NULL;
	virtual size_t read(void *p, size_t l)=NULL;

	typedef enum
	{
		pos_begin=0,
		pos_current,
		pos_end
	}pos_type;

	virtual __int64 setpos(__int64 delta, pos_type how=pos_begin){return -1;}
	__int64 getpos(void){return setpos(0,pos_current);}

	inline nstream &operator<<(const bool &d){write(&d,sizeof(d));return *this;}
	inline nstream &operator<<(const float &d){write(&d,sizeof(d));return *this;}
	inline nstream &operator<<(const double &d){write(&d,sizeof(d));return *this;}
	inline nstream &operator<<(const wchar_t &d){write(&d,sizeof(d));return *this;}

	inline nstream &operator<<(const char &d){write(&d,sizeof(d));return *this;}
	inline nstream &operator<<(const short &d){write(&d,sizeof(d));return *this;}
	inline nstream &operator<<(const int &d){write(&d,sizeof(d));return *this;}
	inline nstream &operator<<(const long &d){write(&d,sizeof(d));return *this;}
	inline nstream &operator<<(const __int64 &d){write(&d,sizeof(d));return *this;}

	inline nstream &operator<<(const unsigned char &d){write(&d,sizeof(d));return *this;}
	inline nstream &operator<<(const unsigned short &d){write(&d,sizeof(d));return *this;}
	inline nstream &operator<<(const unsigned int &d){write(&d,sizeof(d));return *this;}
	inline nstream &operator<<(const unsigned long &d){write(&d,sizeof(d));return *this;}
	inline nstream &operator<<(const unsigned __int64 &d){write(&d,sizeof(d));return *this;}

#ifdef _NEWSTR_CLASS
	inline nstream &operator<<(const nstrc &o){writestring(o);return *this;}
	inline nstream &operator<<(const nstrw &o){writestring(o);return *this;}
#endif

	inline nstream &operator>>(bool &d){read(&d,sizeof(d));return *this;}
	inline nstream &operator>>(float &d){read(&d,sizeof(d));return *this;}
	inline nstream &operator>>(double &d){read(&d,sizeof(d));return *this;}
	inline nstream &operator>>(wchar_t &d){read(&d,sizeof(d));return *this;}

	inline nstream &operator>>(char &d){read(&d,sizeof(d));return *this;}
	inline nstream &operator>>(short &d){read(&d,sizeof(d));return *this;}
	inline nstream &operator>>(int &d){read(&d,sizeof(d));return *this;}
	inline nstream &operator>>(long &d){read(&d,sizeof(d));return *this;}
	inline nstream &operator>>(__int64 &d){read(&d,sizeof(d));return *this;}

	inline nstream &operator>>(unsigned char &d){read(&d,sizeof(d));return *this;}
	inline nstream &operator>>(unsigned short &d){read(&d,sizeof(d));return *this;}
	inline nstream &operator>>(unsigned int &d){read(&d,sizeof(d));return *this;}
	inline nstream &operator>>(unsigned long &d){read(&d,sizeof(d));return *this;}
	inline nstream &operator>>(unsigned __int64 &d){read(&d,sizeof(d));return *this;}

#ifdef _NEWSTR_CLASS
	inline nstream &operator>>(nstrc &o){readstring(o);return *this;}
	inline nstream &operator>>(nstrw &o){readstring(o);return *this;}
#endif

private:

#ifdef _NEWSTR_CLASS
	void writestring(const nstrc &o)
	{
		write(o,(o.len()+1)*sizeof(char));
	}
	void writestring(const nstrw &o)
	{
		write(o,(o.len()+1)*sizeof(unsigned short));
	}

	void readstring(nstrc &o)
	{
		o="";
		char b;
		for(;;)
		{
			read(&b,sizeof(char));
			if(!b)break;
			o+=b;
		}
	}
	void readstring(nstrw &o)
	{
		o=L"";
		unsigned short b;
		for(;;)
		{
			read(&b,sizeof(unsigned short));
			if(!b)break;
			o+=b;
		}
	}
#endif

};

#endif // !defined(AFX_NSTREAM_H__F3F970C6_1445_472B_951C_A1C1A03DA3FC__INCLUDED_)
