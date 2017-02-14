#ifndef __IMAGE_H__
#define __IMAGE_H__

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "types.h"
#include <string>

class Image
{
public:
	Image();
	Image(Image& rhs);
	virtual ~Image();

	bool LoadImg(const char* file_path);

	dword Width() const;
	dword Height() const;
	byte* Data() const;
	int32 NumBytes() const;
	byte* GetPixel(int x, int y) const;

private:
	byte* data = nullptr;
	dword width = 0, height = 0;
	int32 num_bytes;
};

INLINE dword Image::Width() const
{
	return width;
}

INLINE dword Image::Height() const
{
	return height;
}

INLINE byte* Image::Data() const
{
	return data;
}

INLINE int Image::NumBytes() const
{
	return num_bytes;
}

INLINE byte* Image::GetPixel(int x, int y) const
{
	int offset = (x + (y * ((int)this->width))) * this->num_bytes;
	return data + offset;
}

#endif#pragma once
