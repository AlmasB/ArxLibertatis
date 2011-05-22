/*
===========================================================================
ARX FATALIS GPL Source Code
Copyright (C) 1999-2010 Arkane Studios SA, a ZeniMax Media company.

This file is part of the Arx Fatalis GPL Source Code ('Arx Fatalis Source Code'). 

Arx Fatalis Source Code is free software: you can redistribute it and/or modify it under the terms of the GNU General Public 
License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

Arx Fatalis Source Code is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied 
warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with Arx Fatalis Source Code.  If not, see 
<http://www.gnu.org/licenses/>.

In addition, the Arx Fatalis Source Code is also subject to certain additional terms. You should have received a copy of these 
additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Arx 
Fatalis Source Code. If not, please request a copy in writing from Arkane Studios at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing Arkane Studios, c/o 
ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.
===========================================================================
*/
//////////////////////////////////////////////////////////////////////////////////////
//   @@        @@@        @@@                @@                           @@@@@     //
//   @@@       @@@@@@     @@@     @@        @@@@                         @@@  @@@   //
//   @@@       @@@@@@@    @@@    @@@@       @@@@      @@                @@@@        //
//   @@@       @@  @@@@   @@@  @@@@@       @@@@@@     @@@               @@@         //
//  @@@@@      @@  @@@@   @@@ @@@@@        @@@@@@@    @@@            @  @@@         //
//  @@@@@      @@  @@@@  @@@@@@@@         @@@@ @@@    @@@@@         @@ @@@@@@@      //
//  @@ @@@     @@  @@@@  @@@@@@@          @@@  @@@    @@@@@@        @@ @@@@         //
// @@@ @@@    @@@ @@@@   @@@@@            @@@@@@@@@   @@@@@@@      @@@ @@@@         //
// @@@ @@@@   @@@@@@@    @@@@@@           @@@  @@@@   @@@ @@@      @@@ @@@@         //
// @@@@@@@@   @@@@@      @@@@@@@@@@      @@@    @@@   @@@  @@@    @@@  @@@@@        //
// @@@  @@@@  @@@@       @@@  @@@@@@@    @@@    @@@   @@@@  @@@  @@@@  @@@@@        //
//@@@   @@@@  @@@@@      @@@      @@@@@@ @@     @@@   @@@@   @@@@@@@    @@@@@ @@@@@ //
//@@@   @@@@@ @@@@@     @@@@        @@@  @@      @@   @@@@   @@@@@@@    @@@@@@@@@   //
//@@@    @@@@ @@@@@@@   @@@@             @@      @@   @@@@    @@@@@      @@@@@      //
//@@@    @@@@ @@@@@@@   @@@@             @@      @@   @@@@    @@@@@       @@        //
//@@@    @@@  @@@ @@@@@                          @@            @@@                  //
//            @@@ @@@                           @@             @@        STUDIOS    //
//////////////////////////////////////////////////////////////////////////////////////                                                                                     
//////////////////////////////////////////////////////////////////////////////////////
// HERMESMain
//////////////////////////////////////////////////////////////////////////////////////
//
// Description:
//		HUM...hum...
//
// Updates: (date) (person) (update)
//
// Code: Sébastien Scieux
//
// Copyright (c) 1999 ARKANE Studios SA. All rights reserved
//////////////////////////////////////////////////////////////////////////////////////

#ifndef ARX_IO_PAKMANAGER_H
#define ARX_IO_PAKMANAGER_H

#include <stddef.h>
#include <string>
#include <vector>

struct PakFileHandle;
class PakReader;
class PakDirectory;

void * PAK_FileLoadMalloc( const std::string& name, size_t& sizeLoaded );
void * PAK_FileLoadMallocZero( const std::string& name, size_t& sizeLoaded );

bool PAK_AddPak(const std::string& pakfile);

PakFileHandle * PAK_fopen(const std::string& filename);
size_t PAK_fread(void * buffer, size_t size, size_t count, PakFileHandle * stream);
int PAK_fclose(PakFileHandle * stream);
long PAK_ftell(PakFileHandle * stream);
bool PAK_DirectoryExist(const std::string& name);
bool PAK_FileExist(const std::string& name);
int PAK_fseek(PakFileHandle * fic, int offset, long origin);

void PAK_Close();

class PakManager {
	
private:
	
	std::vector<PakReader*> loadedPaks;
	
public:
	
	PakManager();
	~PakManager();

	bool AddPak( const std::string& pakname );
	bool RemovePak( const std::string& pakname );
	bool Read( const std::string& filename, void* buffer );
	void* ReadAlloc( const std::string& filename, size_t& sizeRead );
	size_t GetSize( const std::string& filename );
	PakFileHandle * fOpen( const std::string& filename );
	int fClose(PakFileHandle * fh);
	int fRead(void * buffer, size_t size, size_t count, PakFileHandle * fh);
	int fSeek(PakFileHandle * fh, int offset, long whence );
	int fTell(PakFileHandle * fh);

	bool GetDirectories( const std::string& name, std::vector<PakDirectory*>& directories );
	bool ExistDirectory( const std::string& name );
	bool ExistFile( const std::string& name );
	
};

#endif // ARX_IO_PAKMANAGER_H
