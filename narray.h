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



#if !defined(AFX_NARRAY_H__355057A2_A99E_11D4_9E00_0040F634D74E__INCLUDED_)
#define AFX_NARRAY_H__355057A2_A99E_11D4_9E00_0040F634D74E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


/*
	A simple array for objects of compiler default types
	An object must support operator= and accept =NULL for clearing
*/
template<class T, class S=size_t> 
class narray
{
public:
	narray(){c=0;a=0;index=NULL;}

	narray(S n){c=0;a=0;index=NULL;count(n);}

	narray(const narray &i)
	{
		c=0;a=0;index=NULL;
		*this=i;
	}

	~narray()
	{
		if(index)
			delete [] index;
	}

	T &operator[](S n)
	{
#ifdef NARRAY_SAFE
		if(n>=c)count(n+1);
#endif
		return index[n];
	}

	T &getat(S n)
	{
#ifdef NARRAY_SAFE
		if(n>=c)count(n+1);
#endif
		return index[n];
	}

	void setsafe(S n, T o)
	{
		if(n>=c)count(n+1);
		index[n]=o;
	}

	S count(void)const{return c;}
	S count(S n)
	{
		if(n>c)
		{
			if(n>a)
			{
#ifdef NARRAY_COMPACT
				S na=n;
#else
				S na=a?a*2:64;
				na=na<n?n+16:na;
#endif

				T *newindex=new T[na];
				if(index)
				{
					S r;
					for(r=0;r<c;r++)
						newindex[r]=index[r];
				
					delete [] index;
				}
				index=newindex;
				a=na;
			}
		}
		else if(n<c)
		{
			S r;
			for(r=n;r<c;r++)
				index[r]=NULL;
		}
		return c=n;
	}

	S insert(S n, const T &s)
	{
		count(c+1);

		S r;
		for(r=c-1;r>n;r--)
		{
			index[r]=index[r-1];
		}
		index[n]=s;
		return n;
	}

	S sortinsert(const T &o)
	{
		S start=0, end=c, mid;
		if(!end)
		{
			insert(0,o);
		}
		else for(;;)
		{
			mid=start+((end-start)>>1);
			if(mid==start)
			{
				if(o<index[mid])
					return insert(mid,o);	//Insert before
				else
					return insert(mid+1,o);	//Insert after
			}
			if(o<index[mid])
			{
				end=mid;
			}
			else
			{
				start=mid;
			}
		}
		return 0;
	}

	S sortinsert_ptr(const T &o)
	{
		S start=0, end=c, mid;
		if(!end)
		{
			insert(0,o);
		}
		else for(;;)
		{
			mid=start+((end-start)>>1);
			if(mid==start)
			{
				if((*o)<(*index[mid]))
					return insert(mid,o);	//Insert before
				else
					return insert(mid+1,o);	//Insert after
			}
			if((*o)<(*index[mid]))
			{
				end=mid;
			}
			else
			{
				start=mid;
			}
		}
		return 0;
	}


	S find(const T &o)
	{
		S r;
		for(r=0;r<c;r++)
		{
			if(index[r]==o)return r;
		}
		return -1;
	}

	S find_ptr(const T &o)
	{
		S r;
		for(r=0;r<c;r++)
		{
			if(*index[r]==*o)return r;
		}
		return -1;
	}

	S rfind(const T &o)
	{
		S r=c;
		while(r>0)
		{
			--r;
			if(index[r]==o)return r;
		}
		return -1;
	}

	S rfind_ptr(const T &o)
	{
		S r=c;
		while(r>0)
		{
			--r;
			if(*index[r]==*o)return r;
		}
		return -1;
	}

	S sortfind(T &o, S *in=NULL)		//in is set if not found of where it should be inserted
	{
		S start=0, end=c, mid;

		if(!end)
		{
			if(in)*in=0;
			return -1;
		}

		for(;;)
		{
			mid=start+((end-start)>>1);

			if(mid==start)
			{
				if(o==index[mid])return mid;

				if(in)
				{
					if(o<index[mid])*in=mid;	//Insert before
					else *in=mid+1;
				}

				return -1;
			}
			
			if(o<index[mid])
			{
				end=mid;
			}
			else
			{
				start=mid;
			}
		}
	}

	S sortfind_ptr(const T &o, S *in=NULL)		//in is set if not found of where it should be inserted
	{
		S start=0, end=c, mid;

		if(!end)
		{
			if(in)*in=0;
			return -1;
		}

		for(;;)
		{
			mid=start+((end-start)>>1);

			if(mid==start)
			{
				if(*o==*index[mid])return mid;

				if(in)
				{
					if((*o)<(*index[mid]))*in=mid;	//Insert before
					else *in=mid+1;
				}

				return -1;
			}
			
			if((*o)<(*index[mid]))
			{
				end=mid;
			}
			else
			{
				start=mid;
			}
		}
	}

	template<class K> S sortfind_ptr_t(const K &o, S *in=NULL)		//in is set if not found of where it should be inserted
	{
		S start=0, end=c, mid;

		if(!end)
		{
			if(in)*in=0;
			return -1;
		}

		for(;;)
		{
			mid=start+((end-start)>>1);

			if(mid==start)
			{
				if(*index[mid]==o)return mid;

				if(in)
				{
					if(*index[mid]<o)*in=mid+1;
					else *in=mid;	//Insert before
				}

				return -1;
			}
			
			if(*index[mid]<o)
			{
				start=mid;
			}
			else
			{
				end=mid;
			}
		}
	}

	void remove(S n)
	{
		if(n>=c)return;
		S r;
		for(r=n;r<c-1;r++)
		{
			index[r]=index[r+1];
		}
		count(c-1);
	}

	void remove(S n, S l)		//Remove range n and l following, n and 1 is same as remove n
	{
		if(n>=c||l==0)return;
		if(n+l>c)l=c-n;
		S r;
		for(r=n;r<c-l;r++)
		{
			index[r]=index[r+l];
		}
		count(c-l);
	}

	narray &operator=(const narray &i)
	{
		S r;
		count(i.c);

		for(r=0;r<c;r++)
			index[r]=i.index[r];

		return *this;
	}

	narray &operator=(int n)	//This operator will force the array size and clear all if 0
	{
		if(!n)
		{
			if(index)
				delete [] index;

			c=a=0;
			index=NULL;
		}
		else count(n);

		return *this;
	}

	narray &operator+=(const narray &i)
	{
		S r,t=count();
		count(t+i.c);

		for(r=0;r<i.c;r++,t++)
			index[t]=i.index[r];

		return *this;
	}

	bool operator==(const narray &o)const
	{
		if(c!=o.c)return false;

		for(S r=0;r<c;r++)
		{
			if(!(index[r]==o.index[r]))return false;
		}

		return true;
	}

	bool operator!=(const narray &o)const{return !((*this)==o);}

	S add(const T &s)
	{
		return insert(count(),s);
	}

	T *getarray(void)
	{
		return index;
	}

	T *detach(void)
	{
		T *i = index;
		index = NULL;
		return i;
	}

	static S typesize(void)
	{
		return sizeof(T);
	}

	void freeall(void)
	{
		if(index)delete [] index;
		c=0;a=0;index=NULL;
	}

	//operator T*(){return getarray();}  bad for [] operator

private:
	S c,a;
	T *index;
};

#endif // !defined(AFX_NARRAY_H__355057A2_A99E_11D4_9E00_0040F634D74E__INCLUDED_)

