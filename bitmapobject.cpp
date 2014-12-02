#include "bitmapobject.h"

BitMapObject::BitMapObject()
{
	hdcMemory = NULL;
	hbmNewBitMap = NULL;
	hbmOldBitMap = NULL;
	iWidth = 0;
	iHeight = 0;
}

BitMapObject::~BitMapObject()
{
	// if the hdcMemory hasn't been destroyed, do so
	if(hdcMemory)
		Destroy();
}

void BitMapObject::Load(HDC hdcCompatible, LPCTSTR lpszFilename)
{
	// if hdcMemory isn't null, make it so captain!
	if(hdcMemory)
		Destroy();

	// create memory dc.
	hdcMemory = CreateCompatibleDC(hdcCompatible);

	// load the bitmap
	hbmNewBitMap = (HBITMAP)LoadImage(NULL, lpszFilename, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

	// shove the image into the dc
	hbmOldBitMap = (HBITMAP)SelectObject(hdcMemory, hbmNewBitMap);

	// grab the bitmap's properties
	BITMAP bmp;
	GetObject(hbmNewBitMap, sizeof(BITMAP), (LPVOID) &bmp);

	// grab the width & height
	iWidth = bmp.bmWidth;
	iHeight = bmp.bmHeight;
}

void BitMapObject::Create(HDC hdcCompatible, int width, int height)
{
	// if hdcMemory isn't null, blow it up!
	if(hdcMemory)
		Destroy();

	// create the memory dc.
	hdcMemory = CreateCompatibleDC(hdcCompatible);

	// create the bitmap
	hbmNewBitMap = CreateCompatibleBitmap(hdcCompatible, width, height);

	// shove the image into the dc
	hbmOldBitMap = (HBITMAP)SelectObject(hdcMemory, hbmNewBitMap);

	// change the width and height.
	iWidth = width;
	iHeight = height;
}

void BitMapObject::Destroy()
{
	// restore old bitmap
	SelectObject(hdcMemory, hbmOldBitMap);

	// delete new bitmap.
	DeleteObject(hbmNewBitMap);

	// delete device context.
	DeleteDC(hdcMemory);

	//set members to 0/NULL
	hdcMemory = NULL;
	hbmNewBitMap = NULL;
	hbmOldBitMap = NULL;
	iWidth = 0;
	iHeight = 0;
}

BitMapObject::operator HDC()
{
	//return hdcMemory.
	return(hdcMemory);
}

int BitMapObject::GetWidth()
{
	// return width
	return(iWidth);
}

int BitMapObject::GetHeight()
{
	// return height
	return(iHeight);
}
