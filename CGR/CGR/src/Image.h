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
	byte* m_Data;
	dword m_Width;
	dword m_Height;
	int32 m_NumBytes;
};

INLINE dword Image::Width() const
{
	return m_Width;
}

INLINE dword Image::Height() const
{
	return m_Height;
}

INLINE byte* Image::Data() const
{
	return m_Data;
}

INLINE int Image::NumBytes() const
{
	return m_NumBytes;
}

INLINE byte* Image::GetPixel(int x, int y) const
{
	int offset = (x + (y * ((int)this->m_Width))) * this->m_NumBytes;
	return m_Data + offset;
}

#endif
