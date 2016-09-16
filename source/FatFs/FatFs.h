/*
 * A class to wrap FatFs library from ChaN
 * Copyright (c) 2014 by Jean-Michel Gallego
 *
 * Use version R0.10c of FatFs updated at November 26, 2014
 *
 * This Library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with the Arduino SdSpiCard Library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

#ifndef FATFS_H
#define FATFS_H

#include <Arduino.h>
#include "ff.h"
#include "diskio.h"

#define MAX_FILES	10

class FatFsClass
{
public:
  FatFsClass() {};

  bool     begin( void );
  int32_t  capacity();
  int32_t  free();
  uint8_t  error();

  bool     mkdir( char * path );
  bool     rmdir( char * path );
  bool     remove( char * path );
  bool     remove( const char * path ) {return remove((char*)path);};
  bool     rename( char * oldName, char * newName );
  bool     exists( char * path );
  bool     isDir( char * path );
  bool     timeStamp( char * path, uint16_t year, uint8_t month, uint8_t day,
                      uint8_t hour, uint8_t minute, uint8_t second );
  bool     getFileModTime( char * path, uint16_t * pdate, uint16_t * ptime );

private:
  FATFS    ffs;
};

extern FatFsClass FatFs;

class DirFs
{
public:
  DirFs();
  ~DirFs();

  bool     openDir( char * dirPath );
  bool     openDir( const char * dirPath ) {return openDir((char*)dirPath);};
  bool     closeDir();
  bool     nextFile();
  bool     rewind();
  bool     isDirectory();
  char *   fileName();
  uint32_t fileSize();
  uint16_t fileModDate();
  uint16_t fileModTime();

private:
  FILINFO  finfo;
  DIR      dir;
};

class FileFs: public FatFsClass, public DirFs
{
public:
  FileFs() { for(int j=0; j<MAX_FILES; j++){_open[j] = false;} };

  int     	open(char * fileName, uint8_t mode = FA_OPEN_EXISTING );
  int		open(const char * fileName, uint8_t mode = FA_OPEN_EXISTING) {return open((char*)fileName, mode);};
  bool     	close(int hlr);
  bool 		isOpen(int hlr) { return((hlr >= 0) ? _open[hlr] : false); };

  uint32_t 	write( int hlr, void * buf, uint32_t lbuf );
  int      	writeString(int hlr, char * str );
  bool     	writeChar( int hlr, char car );

  uint32_t 	read( int hlr, void * buf, uint32_t lbuf );
  int16_t  	readString( int hlr, char * buf, int len );
  char     	readChar(int hlr);
  uint16_t 	readInt(int hlr);
  uint16_t 	readHex(int hlr);

  uint32_t 	curPosition(int hlr);
  bool     	seekSet( int hlr, uint32_t cur );

  uint32_t 	fileSize(int hlr);
  using DirFs::fileSize;

private:
  FIL      ffile[MAX_FILES];
  bool		_open[MAX_FILES];
};

#endif // FATFS_H
