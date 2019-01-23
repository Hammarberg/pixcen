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

#pragma once

#include "atlimage.h"

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
