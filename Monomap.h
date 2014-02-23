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

#include <Windows.h>
#include <cstdint>


class Monomap
{
public:

	Monomap(int x, int y)
	{
		pitch = ((x+31) & ~31)/sizeof(uint32_t);
		pmap = new uint32_t[y * pitch];
		ZeroMemory(pmap, sizeof(uint32_t) * y * pitch);
	}

	~Monomap(void)
	{
		delete [] pmap;
	}

	BYTE GetPixel(int x, int y)
	{
		return (pmap[y*pitch+x/32] & 1<<(x&31)) >> (x&31);
	}

	void SetPixel(int x, int y)
	{
		pmap[y*pitch+x/32] |= 1<<(x&31);
	}

private:
	uint32_t *pmap;
	int pitch;
};

