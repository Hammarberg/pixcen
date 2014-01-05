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
#ifndef _NEWSTR_CLASS
#define _NEWSTR_CLASS

#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#if defined(_MSC_VER)

#define NSTRWIN
#include <windows.h>

#elif defined(__GNUC__)

#define NSTROSX
#include <ctype.h>
#include <xlocale.h>
#include <libkern/OSTypes.h>
#include <libkern/OSAtomic.h>

#endif

#define NSTRCLASS template<class NCHAR> 
NSTRCLASS class NSTR;
#define NSTRTYPE NSTR<NCHAR>

typedef NSTR<char> nstrc;
typedef NSTR<wchar_t> nstrw;

#ifdef _UNICODE
typedef nstrw nstr;
#define _N(x)      L ## x
#else
typedef nstrc nstr;
#define _N(x)      x
#endif

template<class NCHAR>
class nstrptr
{
public:
	size_t l;		//String len
	size_t a;		//Allocated
	volatile long r;//Reference count
	NCHAR b[1];

	static int refsize(void)
	{
		return sizeof(size_t)+sizeof(size_t)+sizeof(long);
	}
};

#define NSTRPTR nstrptr<NCHAR>

NSTRCLASS
class NSTR
{
public:
	NSTR(){p=&nullstr;}
	NSTR(const char *);
	NSTR(const NSTR<char> &o);
	NSTR(const wchar_t *);
	NSTR(const NSTR<wchar_t> &o);
#ifdef _WINDOWS_
	NSTR(UINT nID, int size=256);
#endif

	~NSTR(){deref(p);}

	NSTR &operator=(const NSTR<char> &o);
	NSTR &operator=(const char *s);
	NSTR &operator=(int s);
	NSTR &operator=(const NSTR<wchar_t> &o);
	NSTR &operator=(const wchar_t *s);

	void copy(const NSTR &o){*this=o;}
	void cpy(const NSTR &o){*this=o;}
	void cpyn(const NCHAR *s, size_t len);

	NSTR operator+(const NSTR &o)const;
	NSTR operator+(const NCHAR *s)const;
	NSTR &operator+=(const NSTR &o);
	NSTR &operator+=(const NCHAR *s);
	NSTR &operator+=(NCHAR c);

	inline size_t len(void)const{return p->l;}
	inline size_t count(void)const{return p->l;}
	inline size_t size(void)const{return p->l;}

	size_t alloc(size_t l){unsharealloc(l);return p->a;}
	size_t alloc(void)const{return p->a;}
	size_t optimize(void);
	long referencecount(void)const{return p->r;}

	NCHAR *lockbuf(size_t len=-1);
	void releasebuf(size_t len=-1);

	void upper(void);
	void lower(void);

	//Reverse for Motorola Little Endian <-> Big Endian
#if defined(NSTRWIN)
	void wordswap(void){nwordswap(p);}
#endif
	void limit(size_t n);

	int cmp(const NSTR &o)const{return nstrcmp(p->b,o.p->b);}
	int rcmp(const NSTR &o)const{return nstrcmp(o.p->b,p->b);}
	int rcmpi(const NSTR &o)const{return nstrcmpi(o.p->b,p->b);}
	int cmpi(const NSTR &o)const{return nstrcmpi(p->b,o.p->b);}
	int cmpn(const NSTR &o, size_t l)const{return nstrcmpn(p->b,o.p->b,l);}
	int cmpni(const NSTR &o, size_t l)const{return nstrcmpni(p->b,o.p->b,l);}

	int cmp(const NCHAR *s)const{return nstrcmp(p->b,s);}
	int rcmp(const NCHAR *s)const{return nstrcmp(s,p->b);}
	int cmpi(const NCHAR *s)const{return nstrcmpi(p->b,s);}
	int cmpn(const NCHAR *s, size_t l)const{return nstrcmpn(p->b,s,l);}
	int cmpni(const NCHAR *s, size_t l)const{return nstrcmpni(p->b,s,l);}

	//Case insensitive wildcard compare ? and *, not good for all DOS style file names without preparation
	//For instance "thefile" would not match "*.*", you have to add a '.' for extension like "thefile."
	bool cmpcard(const NCHAR *car, bool caseinsensitive=true)const{return ncmpcard(p->b,car,caseinsensitive);}

	NSTR mid(size_t first, size_t l=-1)const;
	NSTR left(size_t l)const{return mid(0,l);}
	NSTR right(size_t l)const{return mid(l>p->l?0:p->l-l);};

	void insert(size_t index, NCHAR c);
	void insert(size_t index, const NCHAR *s);

	void cut(size_t first, size_t l);

	inline NCHAR lastchar(void)const{return p->l?p->b[p->l-1]:0;}
	inline void ensurelast(NCHAR c){if(lastchar()!=c)(*this)+=c;}
#ifdef NSTRWIN
	void reverse(void){unshare();nstrrev(p->b);}
#endif
	void empty(void){(*this)=NULL;}

	bool isnotempty(void)const{return p->l!=0;}
	bool isempty(void)const{return p->l==0;}
	bool isnull(void)const{return (p==&nullstr);}

	void trimleft(const NCHAR *s=NULL);
	void trimright(const NCHAR *s=NULL);
	void trim(const NCHAR *s=NULL){trimright(s);trimleft(s);}

	size_t find(NCHAR c, size_t start=0)const;
	size_t rfind(NCHAR c, size_t start=-1)const;
	size_t find(const NCHAR *s, size_t start=0)const;
	size_t findi(const NCHAR *s, size_t start=0)const;
	size_t findany(const NCHAR *s, size_t start=0)const;
	size_t rfindany(const NCHAR *s, size_t start=-1)const;

	NCHAR getat(size_t n)const{return p->b[n];}
	NCHAR operator[](size_t n)const{return p->b[n];}
	NCHAR operator[](int n)const{return p->b[n];}
	void setat(size_t n, NCHAR c){unshare();p->b[n]=c;}

	operator const NCHAR *()const{return (const NCHAR *)p->b;}
	inline const NCHAR *cstr(void)const{return (const NCHAR *)p->b;}

	NSTR &operator<<(NSTR &o){(*this)+=o;return *this;}
	NSTR &operator<<(const NCHAR *s){(*this)+=s;return *this;}
#ifdef NSTRWIN
	NSTR &operator<<(long n){NCHAR b[16];nitoa(n,b,10);(*this)+=(const NCHAR *)b;return *this;}
	NSTR &operator<<(unsigned long n){NCHAR b[16];nutoa(n,b,10);(*this)+=(const NCHAR *)b;return *this;}
	NSTR &operator<<(int n){NCHAR b[16];nitoa(n,b,10);(*this)+=(const NCHAR *)b;return *this;}
	NSTR &operator<<(NCHAR n){(*this)+=n;return *this;}
	NSTR &operator<<(unsigned int n){NCHAR b[16];nutoa(n,b,10);(*this)+=(const NCHAR *)b;return *this;}
	NSTR &operator<<(__int64 n){NCHAR b[24];ni64toa(n,b,10);(*this)+=(const NCHAR *)b;return *this;}
	NSTR &operator<<(unsigned __int64 n){NCHAR b[24];nui64toa(n,b,10);(*this)+=(const NCHAR *)b;return *this;}
#endif
	size_t format(const NCHAR *format, ...);

	bool load(FILE *fp);
#ifdef _WINDOWS_
	bool load(UINT uID, int size=256)
	{
		int n=load(lockbuf(size),size,uID);
		releasebuf();
		return n!=NULL;
	}
#endif
	
	//-------------------------------------------------------
	// Private section
	//-------------------------------------------------------
	
private:
	friend class NSTR<char>;
	friend class NSTR<wchar_t>;

	NSTRPTR *p;

	NSTR(NSTRPTR *t){p=t;}
	
	static inline size_t ninterlockeddecrement(volatile long *p)
	{
#if defined(NSTRWIN)
		return InterlockedDecrement(p);
#elif defined(NSTROSX)
		return ::OSAtomicDecrement32((volatile int32_t *)p);
#else
		(*p)--;
		return *p;
#endif
	}
	
	static inline size_t ninterlockedincrement(volatile long *p)
	{
#if defined(NSTRWIN)
		return InterlockedIncrement(p);
#elif defined(NSTROSX)
		return ::OSAtomicIncrement32((volatile int32_t *)p);
#else
		(*p)++;
		return *p;
#endif
	}
	
	static inline void deref(NSTRPTR *p)    
	{
		if(p!=&nullstr) 
		{
			if(!ninterlockeddecrement(&p->r))        
			{
				free(p);
			}
		}
	}

	static inline void ref(NSTRPTR *p)
	{
		if(p!=&nullstr)
		{
			ninterlockedincrement(&p->r);
		}
	}

	static NSTRPTR *newblock(size_t l)
	{
		size_t al=l+32;
		NSTRPTR *n=(NSTRPTR *)malloc(NSTRPTR::refsize()+(al)*sizeof(NCHAR));
		n->r=1;
		n->a=al;
		return n;
	}

	inline void renewblock(size_t l)
	{
		size_t al=l+32;
		p=(NSTRPTR *)realloc(p,NSTRPTR::refsize()+(p->a=al)*sizeof(NCHAR));
	}

	inline static size_t nstrlen(const char *a){return strlen(a);}
	inline static size_t nstrlen(const wchar_t *a){return wcslen(a);}

	inline static int nstrcmp(const char *a, const char *b){return strcmp(a,b);}
	inline static int nstrcmp(const wchar_t *a, const wchar_t *b){return wcscmp(a,b);}

	inline static int nstrcmpn(const char *a, const char *b, size_t l){return strncmp(a,b,l);}
	inline static int nstrcmpn(const wchar_t *a, const wchar_t *b, size_t l){return wcsncmp(a,b,l);}
#if defined(NSTRWIN)
	inline static int nstrcmpi(const char *a, const char *b){return _stricmp(a,b);}
	inline static int nstrcmpi(const wchar_t *a, const wchar_t *b){return _wcsicmp(a,b);}
	
	inline static int nstrcmpni(const char *a, const char *b, size_t l){return _strnicmp(a,b,l);}
	inline static int nstrcmpni(const wchar_t *a, const wchar_t *b, size_t l){return _wcsnicmp(a,b,l);}
#elif defined(NSTROSX)
	inline static int nstrcmpi(const char *a, const char *b){return strcasecmp(a,b);}
	inline static int nstrcmpi(const wchar_t *a, const wchar_t *b)
	{
		int n;
		for(;*a||*b;a++,b++)
		{
			n=ntolower(*a)-ntolower(*b);
			if(n)return n;
		}
		return 0;
	}
	
	inline static int nstrcmpni(const char *a, const char *b, size_t l){return strncasecmp(a,b,l);}
	inline static int nstrcmpni(const wchar_t *a, const wchar_t *b, size_t l)
	{
		int n;
		for(;l!=0&&(*a||*b);a++,b++,l--)
		{
			n=ntolower(*a)-ntolower(*b);
			if(n)return n;
		}
		return 0;
	}
#endif
	inline static char ntolower(char a){return tolower(a);}
	inline static wchar_t ntolower(wchar_t a){return towlower(a);}
	inline static char ntoupper(char a){return toupper(a);}
	inline static wchar_t ntoupper(wchar_t a){return towupper(a);}
#if defined(NSTRWIN)
	inline static void nstrupr(char *a){_strupr(a);}
	inline static void nstrupr(wchar_t *a){_wcsupr(a);}
	
	inline static void nstrlwr(char *a){_strlwr(a);}
	inline static void nstrlwr(wchar_t *a){_wcslwr(a);}
#else
	inline static void nstrupr(NCHAR *a){while(*a){*a=ntoupper(*a);a++;}}
	inline static void nstrlwr(NCHAR *a){while(*a){*a=ntolower(*a);a++;}}
#endif
#if defined(NSTRWIN)	
	inline static char *nstrrev(char *a){return _strrev(a);}
	inline static wchar_t *nstrrev(wchar_t *a){return _wcsrev(a);}
#endif
	static int nisspace(char a){return isspace(a);}
	static int nisspace(wchar_t a){return iswspace(a);}

	inline static char *nstrchr(const char *a, char c){return strchr((char *)a,c);}
	inline static wchar_t *nstrchr(const wchar_t *a, wchar_t c){return wcschr((wchar_t *)a,c);}

	inline static char *nstrstr(char *a, const char *c){return strstr(a,c);}
	inline static wchar_t *nstrstr(wchar_t *a, const wchar_t *c){return wcsstr(a,c);}

	inline static char *nstrpbrk(char *a, const char *c){return strpbrk(a,c);}
	inline static wchar_t *nstrpbrk(wchar_t *a, const wchar_t *c){return wcspbrk(a,c);}
#if defined(NSTRWIN)
	inline static char *nitoa(long n, char *s, int r){return _ltoa(n,s,r);}
	inline static wchar_t *nitoa(long n, wchar_t *s, int r){return _ltow(n,s,r);}

	inline static char *nutoa(long n, char *s, int r){return _ultoa(n,s,r);}
	inline static wchar_t *nutoa(long n, wchar_t *s, int r){return _ultow(n,s,r);}

	inline static char *ni64toa(__int64 n, char *s, int r){return _i64toa(n,s,r);}
	inline static wchar_t *ni64toa(__int64 n, wchar_t *s, int r){return _i64tow(n,s,r);}

	inline static char *nui64toa(unsigned __int64 n, char *s, int r){return _ui64toa(n,s,r);}
	inline static wchar_t *nui64toa(unsigned __int64 n, wchar_t *s, int r){return _ui64tow(n,s,r);}
#endif
	size_t _nstrprintf(const char *format, va_list argptr);
	size_t _nstrprintf(const wchar_t *format, va_list argptr);

	static bool ncmpcard(const NCHAR *str, const NCHAR *car, bool caseinsensitive);

	static void copyconvert( nstrptr<char> *p, const char *s){memcpy((void *)p->b,(const void *)s,(p->l+1)*sizeof(NCHAR));}
	static void copyconvert( nstrptr<wchar_t> *p, const wchar_t *s){memcpy((void *)p->b,(const void *)s,(p->l+1)*sizeof(NCHAR));}
	static void copyconvert( nstrptr<char> *p, const wchar_t *s){if(wcstombs(p->b,s,p->l+1)==-1)*p->b=0,p->l=0;}
	static void copyconvert( nstrptr<wchar_t> *p, const char *s){if(mbstowcs(p->b,s,p->l+1)==-1)*p->b=0,p->l=0;}

	void assigncopyconvert( nstrptr<char> *p2, nstrptr<char> *op){ref(p=op);}
	void assigncopyconvert( nstrptr<wchar_t> *p2, nstrptr<wchar_t> *op){ref(p=op);}

	void assigncopyconvert( nstrptr<char> *p2, nstrptr<wchar_t> *op)
	{
		size_t l=nstrlen(op->b);
		p=newblock(l);
		p->l=l;
		copyconvert(p,op->b);
	}
	void assigncopyconvert( nstrptr<wchar_t> *p2, nstrptr<char> *op)
	{
		size_t l=nstrlen(op->b);
		p=newblock(l);
		p->l=l;
		copyconvert(p,op->b);
	}

	inline static void nwordswap( nstrptr<char> *p)
	{
		//No effect on ascii atrings
	}

	inline static void nwordswap( nstrptr<wchar_t> *p)
	{
		size_t r;
		char *l,t;
		for(r=0;r<p->l;r++)
		{
			l=(char *)&p->b[r];
			t=l[0];
			l[0]=l[1];
			l[1]=t;
		}
	}

	void unshare(void)
	{
		if(p->r>1)
		{
			NSTRPTR *n=newblock(p->l);
			n->l=p->l;
			memcpy((void *)n->b,(const void *)p->b,(p->l+1)*sizeof(NCHAR));
			deref(p);
			p=n;
		}
	}

	void unsharealloc(size_t newsize)
	{
		if(p->r>1)
		{
			NSTRPTR *n=newblock(newsize);
			n->l=newsize<p->l?newsize:p->l;
			memcpy((void *)n->b,(const void *)p->b,(n->l+1)*sizeof(NCHAR));
			deref(p);
			p=n;
		}
		else if(newsize+1>p->a)
		{
			renewblock(newsize);
		}
	}

#ifdef _WINDOWS_
	static int load(char *buf, int max, UINT uID)
	{
		return ::LoadStringA(GetModuleHandle(NULL),uID,buf,max);
	}

	static int load(wchar_t *buf, int max, UINT uID)
	{
		return ::LoadStringW(GetModuleHandle(NULL),uID,buf,max);
	}
#endif
	static nstrptr<NCHAR> nullstr;

};

NSTRCLASS static inline NSTRTYPE operator+(const NCHAR * s, const NSTRTYPE &a){return NSTRTYPE(s)+a;}

#ifdef NSTRICOMPARE

//Case insensitive operators
NSTRCLASS static inline bool operator==(const NSTRTYPE &a,const NSTRTYPE &b){return (a.cmpi(b)==false);}
NSTRCLASS static inline bool operator==(const NSTRTYPE &a,const NCHAR * s){return (a.cmpi(s)==false);}
NSTRCLASS static inline bool operator==(const NCHAR * s,const NSTRTYPE &b){return (b.rcmpi(s)==false);}
NSTRCLASS static inline bool operator!=(const NSTRTYPE &a,const NSTRTYPE &b){return (a.cmpi(b)!=false);}
NSTRCLASS static inline bool operator!=(const NSTRTYPE &a,const NCHAR * s){return (a.cmpi(s)!=false);}
NSTRCLASS static inline bool operator!=(const NCHAR * s,const NSTRTYPE &b){return (b.rcmpi(s)!=false);}
NSTRCLASS static inline bool operator<(const NSTRTYPE &a,const NSTRTYPE &b){return (a.cmpi(b)<0);}
NSTRCLASS static inline bool operator<(const NSTRTYPE &a,const NCHAR * s){return (a.cmpi(s)<0);}
NSTRCLASS static inline bool operator<(const NCHAR * s,const NSTRTYPE &b){return (b.rcmpi(s)<0);}
NSTRCLASS static inline bool operator>(const NSTRTYPE &a,const NSTRTYPE &b){return (a.cmpi(b)>0);}
NSTRCLASS static inline bool operator>(const NSTRTYPE &a,const NCHAR * s){return (a.cmpi(s)>0);}
NSTRCLASS static inline bool operator>(const NCHAR * s,const NSTRTYPE &b){return (b.rcmpi(s)>0);}
NSTRCLASS static inline bool operator<=(const NSTRTYPE &a,const NSTRTYPE &b){return !(a>b);}
NSTRCLASS static inline bool operator<=(const NSTRTYPE &a,const NCHAR * s){return !(a>s);}
NSTRCLASS static inline bool operator<=(const NCHAR * s,const NSTRTYPE &b){return !(s>b);}
NSTRCLASS static inline bool operator>=(const NSTRTYPE &a,const NSTRTYPE &b){return !(a<b);}
NSTRCLASS static inline bool operator>=(const NSTRTYPE &a,const NCHAR * s){return !(a<s);}
NSTRCLASS static inline bool operator>=(const NCHAR * s,const NSTRTYPE &b){return !(s<b);}

#else

//Case sensitive operators
NSTRCLASS static inline bool operator==(const NSTRTYPE &a,const NSTRTYPE &b){return (a.cmp(b)==false);}
NSTRCLASS static inline bool operator==(const NSTRTYPE &a,const NCHAR * s){return (a.cmp(s)==false);}
NSTRCLASS static inline bool operator==(const NCHAR * s,const NSTRTYPE &b){return (b.rcmp(s)==false);}
NSTRCLASS static inline bool operator!=(const NSTRTYPE &a,const NSTRTYPE &b){return (a.cmp(b)!=false);}
NSTRCLASS static inline bool operator!=(const NSTRTYPE &a,const NCHAR * s){return (a.cmp(s)!=false);}
NSTRCLASS static inline bool operator!=(const NCHAR * s,const NSTRTYPE &b){return (b.rcmp(s)!=false);}
NSTRCLASS static inline bool operator<(const NSTRTYPE &a,const NSTRTYPE &b){return (a.cmp(b)<0);}
NSTRCLASS static inline bool operator<(const NSTRTYPE &a,const NCHAR * s){return (a.cmp(s)<0);}
NSTRCLASS static inline bool operator<(const NCHAR * s,const NSTRTYPE &b){return (b.rcmp(s)<0);}
NSTRCLASS static inline bool operator>(const NSTRTYPE &a,const NSTRTYPE &b){return (a.cmp(b)>0);}
NSTRCLASS static inline bool operator>(const NSTRTYPE &a,const NCHAR * s){return (a.cmp(s)>0);}
NSTRCLASS static inline bool operator>(const NCHAR * s,const NSTRTYPE &b){return (b.rcmp(s)>0);}
NSTRCLASS static inline bool operator<=(const NSTRTYPE &a,const NSTRTYPE &b){return !(a>b);}
NSTRCLASS static inline bool operator<=(const NSTRTYPE &a,const NCHAR * s){return !(a>s);}
NSTRCLASS static inline bool operator<=(const NCHAR * s,const NSTRTYPE &b){return !(s>b);}
NSTRCLASS static inline bool operator>=(const NSTRTYPE &a,const NSTRTYPE &b){return !(a<b);}
NSTRCLASS static inline bool operator>=(const NSTRTYPE &a,const NCHAR * s){return !(a<s);}
NSTRCLASS static inline bool operator>=(const NCHAR * s,const NSTRTYPE &b){return !(s<b);}

#endif //NSTRICOMPARE

//------------------------------------------------------------------------------------

#ifdef _WINDOWS_
NSTRCLASS NSTRTYPE::NSTR(UINT nID, int size)
{
	p=&nullstr;
	load(nID,size);
}
#endif
NSTRCLASS NSTRTYPE::NSTR(const NSTR<char> &o)
{
	if(o.p!=&o.nullstr)
		assigncopyconvert(p,o.p);
	else
		p=&nullstr;
}

NSTRCLASS NSTRTYPE::NSTR(const NSTR<wchar_t> &o)
{
	if(o.p!=&o.nullstr)
		assigncopyconvert(p,o.p);
	else
		p=&nullstr;
}

NSTRCLASS NSTRTYPE &NSTRTYPE::operator=(const NSTR<char> &o)
{
	deref(p);
	if(o.p!=&o.nullstr)
		assigncopyconvert(p,o.p);
	else
		p=&nullstr;
	return *this;
}

NSTRCLASS NSTRTYPE &NSTRTYPE::operator=(const NSTR<wchar_t> &o)
{
	deref(p);
	if(o.p!=&o.nullstr)
		assigncopyconvert(p,o.p);
	else
		p=&nullstr;
	return *this;
}

NSTRCLASS NSTRTYPE::NSTR(const char *s)
{
	if(!s)
		p=&nullstr;
	else
	{
		size_t l=nstrlen(s);
		p=newblock(l);
		p->l=l;
		copyconvert(p,s);
	}
}

NSTRCLASS NSTRTYPE::NSTR(const wchar_t *s)
{
	if(!s)
		p=&nullstr;
	else
	{
		size_t l=nstrlen(s);
		p=newblock(l);
		p->l=l;
		copyconvert(p,s);
	}
}

NSTRCLASS NSTRTYPE &NSTRTYPE::operator=(const char *s)
{
	if(!s)
	{
		deref(p);
		p=&nullstr;
	}
	else
	{
		size_t l=nstrlen(s);
		unsharealloc(l);
		p->l=l;
		copyconvert(p,s);
	}
	return *this;
}

NSTRCLASS void NSTRTYPE::cpyn(const NCHAR *s, size_t len)
{
	if(!s || !len)
	{
		deref(p);
		p=&nullstr;
	}
	else
	{
		size_t l=0;
		unsharealloc(len);
		while(l<len && (*s))
		{
			p->b[l]=*s;
			l++;
			s++;
		}
		p->l=l;
		p->b[l]=0;
	}
}

NSTRCLASS NSTRTYPE &NSTRTYPE::operator=(const wchar_t *s)
{
	deref(p);
	if(!s)
		p=&nullstr;
	else
	{
		size_t l=nstrlen(s);
		p=newblock(l);
		p->l=l;
		copyconvert(p,s);
	}
	return *this;
}

NSTRCLASS NSTRTYPE &NSTRTYPE::operator=(int c)
{
	if(c)
	{
		NCHAR b[2]={c,0};
		(*this)=(const NCHAR *)b;
	}
	else
	{
		deref(p);
		p=&nullstr;
	}
	return *this;
}

NSTRCLASS NSTRTYPE NSTRTYPE::operator+(const NSTR &o)const
{
	size_t l=p->l+o.p->l;
	NSTRPTR *n=newblock(l);
	n->l=l;
	memcpy((void *)n->b,(void *)p->b,p->l*sizeof(NCHAR));
	memcpy((void *)(n->b+p->l),(void *)o.p->b,(o.p->l+1)*sizeof(NCHAR));
	return NSTR(n);
}

NSTRCLASS NSTRTYPE NSTRTYPE::operator+(const NCHAR *s)const
{
	if(!s)return *this;
	size_t sl=nstrlen(s);
	size_t l=p->l+sl;
	NSTRPTR *n=newblock(l);
	n->l=l;
	memcpy(n->b,p->b,p->l*sizeof(NCHAR));
	memcpy(n->b+p->l,s,(sl+1)*sizeof(NCHAR));

	return NSTR(n);
}

NSTRCLASS NSTRTYPE &NSTRTYPE::operator+=(const NSTR &o)
{
	size_t sl=o.p->l;
	size_t l=p->l+sl;
	unsharealloc(l);
	memcpy(p->b+p->l,o.p->b,(sl+1)*sizeof(NCHAR));
	p->l=l;
	return *this;
}

NSTRCLASS NSTRTYPE &NSTRTYPE::operator+=(const NCHAR *s)
{
	size_t sl=nstrlen(s);
	size_t l=p->l+sl;
	unsharealloc(l);
	memcpy(p->b+p->l,s,(sl+1)*sizeof(NCHAR));
	p->l=l;
	return *this;
}

NSTRCLASS NSTRTYPE &NSTRTYPE::operator+=(NCHAR c)
{
	size_t l=p->l+1;
	unsharealloc(l);
	p->b[p->l]=c;
	p->b[l]=0;
	p->l=l;
	return *this;
}

NSTRCLASS NCHAR *NSTRTYPE::lockbuf(size_t len)
{
	if(len==-1)len=p->l;
	unsharealloc(len);
	p->l=len;
	return p->b;
}

NSTRCLASS void NSTRTYPE::releasebuf(size_t end)
{
	if(end==-1)
	{
		p->b[p->a-1]=0;
		p->l=nstrlen(p->b);
	}
	else
	{
		p->b[end]=0;
		p->l=end;
	}
}

NSTRCLASS void NSTRTYPE::upper(void)
{
	unshare();
	nstrupr(p->b);
}

NSTRCLASS void NSTRTYPE::lower(void)
{
	unshare();
	nstrlwr(p->b);
}

NSTRCLASS void NSTRTYPE::limit(size_t nl)
{
	if(nl<p->l)
	{
		unshare();
		p->b[p->l=nl]=0;
	}
}

NSTRCLASS bool NSTRTYPE::ncmpcard(const NCHAR *str, const NCHAR *car, bool caseinsensitive)
{
	int r,l;
	const NCHAR *lop;
	for(;;str++,car++)
	{
		switch(*car)
		{
		case '\0':
			if(!*str)return true;   //Both card and string ended, all matched.
			return false;                   //The string didn't end, no match
		case '*':
			if(!car[1])return true; //The wildcard ends with a *, all match and the rest is match
			for(lop=str;;lop++)
			{
				if(*lop)
				{
					if(ncmpcard(lop,car+1,caseinsensitive))return true;
				}
				else break;     //Else skip the *
			}
			break;
		case '?':
			if(!*str)return false;  //Any letter is a match but end of string
			break;
		default:
			r=*car,l=*str;
			if(caseinsensitive)
			{
				if(ntolower((NCHAR)r)!=ntolower((NCHAR)l))return false; //char by char comparison failed
			}
			else
			{ 
				if(r!=l)return false; //char by char comparison failed
			}
			break;
		}
	}
}

NSTRCLASS NSTRTYPE NSTRTYPE::mid(size_t first, size_t l)const
{
	if(first>=p->l)return NSTR(&nullstr);
	size_t al=p->l-first;
	if(l>al)l=al;
	NSTRPTR *n=newblock(l);
	n->l=l;
	memcpy(n->b,p->b+first,l*sizeof(NCHAR));
	n->b[l]=0;
	return NSTR(n);
}

NSTRCLASS void NSTRTYPE::insert(size_t index, NCHAR c)
{
	size_t l=1,nl=p->l+1;
	unsharealloc(nl);
	memmove(p->b+index+l,p->b+index,((p->l-index)+1)*sizeof(NCHAR));
	p->b[index]=c;
	p->l=nl;
}

NSTRCLASS void NSTRTYPE::insert(size_t index, const NCHAR *s)
{
	size_t l=nstrlen(s),nl=p->l+l;
	unsharealloc(nl);
	memmove(p->b+index+l,p->b+index,((p->l-index)+1)*sizeof(NCHAR));
	memcpy(p->b+index,s,l*sizeof(NCHAR));
	p->l=nl;
}

NSTRCLASS void NSTRTYPE::cut(size_t first, size_t l)
{
	unshare();
	if(first+l>p->l)l=p->l-first;
	memmove(p->b+first,p->b+first+l,(p->l-(first+l)+1)*sizeof(NCHAR));
	p->l-=l;
}

NSTRCLASS void NSTRTYPE::trimleft(const NCHAR *s)
{
	unshare();

	NCHAR *r=p->b;

	if(!s)
	{
		while(nisspace(*r))r++;
	}
	else
	{
		while(*r)
		{
			if(!nstrchr(s,*r))break;
			r++;
		}
	}

	memmove(p->b,r,((p->l=(p->l-(r-p->b)))+1)*sizeof(NCHAR));
}

NSTRCLASS void NSTRTYPE::trimright(const NCHAR *s)
{
	if(!p->l)return;
	unshare();

	NCHAR *r=p->b+p->l-1;

	if(!s)
	{
		do
		{
			if(!nisspace(*r))break;
			r--;
		}
		while(r>=p->b);
	}
	else
	{
		do
		{
			if(!nstrchr(s,*r))break;
			r--;
		}
		while(r>=p->b);
	}

	p->l=(r+1)-p->b;
	p->b[p->l]=0;
}

NSTRCLASS size_t NSTRTYPE::find(NCHAR c, size_t start)const
{
	if(start<p->l)
	{
		NCHAR *t=nstrchr(p->b+start,c);
		if(t)return (size_t)(t-p->b);
	}
	return -1;
}

NSTRCLASS size_t NSTRTYPE::rfind(NCHAR c, size_t start)const
{
	if(start==-1)start=len()-1;
	while(start!=-1)
	{
		if(p->b[start]==c)break;
		start--;
	}
	return start;
}

NSTRCLASS size_t NSTRTYPE::find(const NCHAR *c, size_t start)const
{
	if((int)start<p->l)
	{
		NCHAR *t=nstrstr(p->b+start,c);
		if(t)return (int)(t-p->b);
	}
	return -1;
}

NSTRCLASS size_t NSTRTYPE::findi(const NCHAR *c, size_t start)const
{
	size_t l=nstrlen(c);

	NCHAR sstr[3]={0,0,0};
	sstr[0]=tolower(c[0]);
	sstr[1]=toupper(c[0]);

	NCHAR *t=p->b+start;
	for(;;)
	{
		t=nstrpbrk(t,sstr);
		if(!t)break;
		if(!nstrcmpni(t,c,l))return (size_t)(t-p->b);
		t++;
	}
	return -1;
}

NSTRCLASS size_t NSTRTYPE::findany(const NCHAR *c, size_t start)const
{
	if((int)start<p->l)
	{
		NCHAR *t=nstrpbrk(p->b+start,c);
		if(t)return (size_t)(t-p->b);
	}
	return -1;
}

NSTRCLASS size_t NSTRTYPE::rfindany(const NCHAR *c, size_t start)const
{
	if(start==-1)start=len()-1;
	const NCHAR *i;
	while(start!=-1)
	{
		i=c;
		for(;*i;i++)
		{
			if(p->b[start]==*i)return start;
		}
		start--;
	}
	return start;
}

NSTRCLASS bool NSTRTYPE::load(FILE *fp)
{
	long cur=ftell(fp),end;
	if(fseek(fp,0,SEEK_END ))return false;
	end=ftell(fp);
	if(fseek(fp,cur,SEEK_SET ))return false;

	limit(0);       //Empty the string

	alloc((end-cur)+8);

	NCHAR *w=p->b;

	int c;
	if(sizeof(NCHAR)==sizeof(wchar_t))
	{
		for(;;w++)
		{
			if((c=fgetwc(fp))==WEOF)break;
			*w=(NCHAR)c;
		}
	}
	else
	{
		for(;;w++)
		{
			if((c=fgetc(fp))==EOF)break;
			*w=(NCHAR)c;
		}
	}

	*w=0;

	p->l=(int)(w-p->b);

	return true;
}

NSTRCLASS size_t NSTRTYPE::format(const NCHAR *format, ...)
{
	va_list arglist;
	va_start(arglist, format);
	return _nstrprintf(format,arglist);
}

NSTRCLASS size_t NSTRTYPE::_nstrprintf(const char *format, va_list argptr)
{
#if defined(NSTRWIN)
	int size=_vscprintf(format, argptr);
	if(size<0)throw size;
	unsharealloc(size+16);
	return p->l=vsprintf_s(p->b,p->a,format,argptr);
#elif defined(NSTROSX)
	int size=vsnprintf(p->b,0,format, argptr);
	if(size<0)throw size;
	unsharealloc(size+16);
	return p->l=vsnprintf(p->b,p->a,format,argptr);
#endif
}

NSTRCLASS size_t NSTRTYPE::_nstrprintf(const wchar_t *format, va_list argptr)
{
#if defined(NSTRWIN)
	int size=_scwprintf(format, argptr);
	if(size<0)throw size;
	unsharealloc(size+16);
	return p->l=vswprintf_s(p->b,p->a,format,argptr);
#elif defined(NSTROSX)
	//TODO make better. vswprintf can not estimate like vsnprintf can
	int size=0,r;
	do
	{
		size+=64;
		unsharealloc(size);
		r=vswprintf(p->b,p->a,format, argptr);
	}
	while(r<0);
	return p->l=r;
#endif
}

//Same thing as unsharealloc but will shrink the buffer even if reference count is 1
NSTRCLASS size_t NSTRTYPE::optimize(void)
{
	size_t newsize=p->l;

	if(p->r>1)
	{
		NSTRPTR *n=newblock(newsize);
		n->l=newsize<p->l?newsize:p->l;
		memcpy((void *)n->b,(const void *)p->b,(n->l+1)*sizeof(NCHAR));
		deref(p);
		p=n;
	}
	else
	{
		renewblock(newsize);
	}
	return p->a;
}

NSTRCLASS nstrptr<NCHAR> NSTRTYPE::nullstr={0,0,2,0};

#endif
