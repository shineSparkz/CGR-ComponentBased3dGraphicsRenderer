#ifndef __MY_RECT_H__
#define __MY_RECT_H__

#include <iostream>
#include "types.h"
#include "math_utils.h"

class Rect
{
public:
	explicit Rect();
	explicit Rect(int32 l, int32 r, int32 t, int32 b);
	~Rect();

	friend std::ostream& operator <<(std::ostream& o, const Rect& r);

	Rect Expand(int32 l, int32 r, int32 t, int32 b);
	Rect operator*(int32 scalar);

	bool Outside(const Rect& other);
	bool CompletelyContains(const Rect& other);
	bool Contains(const Vec2& pos) const;

	void Set(int32 l, int32 r, int32 b, int32 t);
	int Width() const;
	int Height() const;
	void Translate(int32 x, int32 y);
	void Clip(const Rect& clipTo);
	bool Intersects(const Rect& other) const;

public:
	int32 left, right, top, bottom;
};

INLINE std::ostream& operator <<(std::ostream& o, const Rect& r)
{
	o << "MyRect: " << " L:" << r.left << " R:" << r.right <<
		" T:" << r.top << " B:" << r.bottom << "\n";
	return o;
}

INLINE Rect::Rect() :
	left(0),
	right(0),
	top(0),
	bottom(0)
{
}

INLINE Rect::Rect(int32 l, int32 r, int32 t, int32 b) :
	left(l),
	right(r),
	top(t),
	bottom(b)
{
}

INLINE Rect::~Rect()
{
}

INLINE Rect Rect::Expand(int32 l, int32 r, int32 t, int32 b)
{
	return Rect(this->left - l, this->right + r,
		this->top - t, this->bottom + b);
}

INLINE Rect Rect::operator*(int32 scalar)
{
	int w = this->Width() * scalar;
	int h = this->Height() * scalar;
	return Rect(
		this->left - (w / 2),
		this->right + (w / 2),
		this->top - (h / 2),
		this->bottom + (h / 2)
		);
}

INLINE void Rect::Set(int32 l, int32 r, int32 t, int32 b)
{
	left = l; right = r; bottom = b; top = t;
}

INLINE int32 Rect::Width() const
{
	return right - left;
}

INLINE int32 Rect::Height() const
{
	return bottom - top;
}

INLINE void Rect::Translate(int32 x, int32 y)
{
	left += x;
	right += x;
	top += y;
	bottom += y;
}

INLINE void Rect::Clip(const Rect& clipTo)
{
	left = Maths::Max(clipTo.left, left);
	right = Maths::Min(clipTo.right, right);
	top = Maths::Max(clipTo.top, top);
	bottom = Maths::Min(clipTo.bottom, bottom);
}

INLINE bool Rect::Intersects(const Rect& other) const
{
	if ( left > other.right || right <= other.left || top > other.bottom || bottom <= other.top )
		return false;
	return true;
}

INLINE bool Rect::CompletelyContains(const Rect& other)
{
	if (other.left > left && other.right < right &&
		other.top > top && other.bottom < bottom)
		return true;

	return false;
}

INLINE bool Rect::Contains(const Vec2& pos) const
{
	int x = (int)pos.x;
	int y = (int)pos.y;

	return (x >= left && x < right && y >= top && y < bottom);
}

INLINE bool Rect::Outside(const Rect& other)
{
	if (left > other.right || right < other.left ||
		bottom < other.top || top > other.bottom)
		return true;
	return false;}

class RectF
{
public:
	explicit RectF();
	explicit RectF(float l, float r, float t, float b);
	~RectF() {}

	friend std::ostream& operator <<(std::ostream& o, const RectF& r);

	RectF Expand(float l, float r, float t, float b);
	RectF operator*(float scalar);

	bool Outside(const RectF& other);
	bool CompletelyContains(const RectF& other);
	bool Contains(const Vec2& pos) const;

	void Set(float l, float r, float t, float b);
	float Width() const;
	float Height() const;
	void Translate(float x, float y);
	void Clip(const RectF& clipTo);
	bool Intersects(const RectF& other) const;

public:
	float left, right, top, bottom;
};

INLINE std::ostream& operator <<(std::ostream& o, const RectF& r)
{
	o << "MyRect: " << " L:" << r.left << " R:" << r.right <<
		" T:" << r.top << " B:" << r.bottom << "\n";
	return o;
}

INLINE RectF::RectF() :
	left(0),
	right(0),
	top(0),
	bottom(0)
{
}

INLINE RectF::RectF(float l, float r, float t, float b) :
	left(l),
	right(r),
	top(t),
	bottom(b)
{
}

INLINE Rect::~Rect()
{
}

INLINE RectF RectF::Expand(float l, float r, float t, float b)
{
	return RectF(this->left - l, this->right + r,
		this->top - t, this->bottom + b);
}

INLINE RectF RectF::operator*(float scalar)
{
	float w = this->Width() * scalar;
	float h = this->Height() * scalar;
	return RectF(
		this->left - (w / 2),
		this->right + (w / 2),
		this->top - (h / 2),
		this->bottom + (h / 2)
	);
}

INLINE void RectF::Set(float l, float r, float t, float b)
{
	left = l; right = r; bottom = b; top = t;
}

INLINE float RectF::Width() const
{
	return right - left;
}

INLINE float RectF::Height() const
{
	return bottom - top;
}

INLINE void RectF::Translate(float x, float y)
{
	left += x;
	right += x;
	top += y;
	bottom += y;
}

INLINE void RectF::Clip(const RectF& clipTo)
{
	left = Maths::Max(clipTo.left, left);
	right = Maths::Min(clipTo.right, right);
	top = Maths::Max(clipTo.top, top);
	bottom = Maths::Min(clipTo.bottom, bottom);
}

INLINE bool RectF::Intersects(const RectF& other) const
{
	if ( left > other.right || right <= other.left || top > other.bottom || bottom <= other.top )
		return false;
	return true;
}

INLINE bool RectF::CompletelyContains(const RectF& other)
{
	if ( other.left > left && other.right < right &&
		other.top > top && other.bottom < bottom )
		return true;

	return false;
}

INLINE bool RectF::Contains(const Vec2& pos) const
{
	return (pos.x >= left && pos.x < right && pos.y >= top && pos.y < bottom);
}

INLINE bool RectF::Outside(const RectF& other)
{
	if ( left > other.right || right < other.left ||
		bottom < other.top || top > other.bottom )
		return true;
	return false;
}
#endif