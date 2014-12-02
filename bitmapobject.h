// BitMapObject.h
#ifndef BITMAPOBJECT_H
#define BITMAPOBJECT_H
#pragma once

// we need this for window stuff
#include <windows.h>

class BitMapObject
{
private:
	// memory dc
	HDC hdcMemory;
	
	// new bitmap!
	HBITMAP hbmNewBitMap;

	// old bitmap.
	HBITMAP hbmOldBitMap;

	// width & height as integers.
	int iWidth;
	int iHeight;

public:
	// constructor
	BitMapObject();

	// destructor
	~BitMapObject();

	// loads bitmap from a file
	void Load(HDC hdcCompatible, LPCTSTR lpszFilename);

	// creates a blank bitmap
	void Create(HDC hdcCompatible, int width, int height);

	// destroys bitmap and dc
	void Destroy();

	// return width
	int GetWidth();

	// return height
	int GetHeight();

	// converts to HDC
	operator HDC();
};
#endif
