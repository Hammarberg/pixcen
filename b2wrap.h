/*
Pixcen - A windows platform low level pixel editor for C64
Copyright (C) 2019  John Hammarberg (crt@nospam.binarybone.com)

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

struct B2File
{
	size_t size;
	BYTE *data;
};

bool B2Crunch(B2File *aSource, B2File *aTarget, unsigned short startAdress);

static void B2FreeFile(B2File *aFile)
{
	free(aFile->data);
}
