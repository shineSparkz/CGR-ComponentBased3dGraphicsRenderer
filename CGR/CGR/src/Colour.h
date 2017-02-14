#ifndef __COLOUR_H__
#define __COLOUR_H__

#include "types.h"
#include "math_utils.h"

class Colour
{
public:
	byte r{ (byte)MAX_UNSIGNED_TYPE(byte) };
	byte g{ (byte)MAX_UNSIGNED_TYPE(byte) };
	byte b{ (byte)MAX_UNSIGNED_TYPE(byte) };
	byte a{ (byte)MAX_UNSIGNED_TYPE(byte) };

	Colour() 
	{}

	Colour(byte R, byte G, byte B, byte A) 
	{
		this->Set(R, G, B, A);
	}

	Colour(byte R, byte G, byte B)
	{
		this->Set(R, G, B, (byte)MAX_UNSIGNED_TYPE(byte));
	}

	Colour(byte RGBA)
	{
		this->Set(RGBA, RGBA, RGBA);
	}

	void Set(byte R, byte G, byte B, byte A)
	{
		this->r = Maths::Clamp(R, (byte)MIN_UNSIGNED_TYPE(byte), (byte)MAX_UNSIGNED_TYPE(byte));
		this->g = Maths::Clamp(G, (byte)MIN_UNSIGNED_TYPE(byte), (byte)MAX_UNSIGNED_TYPE(byte));
		this->b = Maths::Clamp(B, (byte)MIN_UNSIGNED_TYPE(byte), (byte)MAX_UNSIGNED_TYPE(byte));
		this->a = Maths::Clamp(A, (byte)MIN_UNSIGNED_TYPE(byte), (byte)MAX_UNSIGNED_TYPE(byte));
	}

	void Set(byte R, byte G, byte B)
	{
		this->Set(R, G, B, (byte)MAX_UNSIGNED_TYPE(byte));
	}

	Vec4 Normalize() const
	{
		return Vec4(N*(float)this->r, N*(float)this->g, N*(float)this->b, N*(float)this->a);
	}

	static Colour Red()
	{
		return Colour((byte)MAX_UNSIGNED_TYPE(byte),0,0);
	}

	static Colour Green()
	{
		return Colour(0, (byte)MAX_UNSIGNED_TYPE(byte), 0);
	}

	static Colour Blue()
	{
		return Colour(0, 0, (byte)MAX_UNSIGNED_TYPE(byte));
	}

	static Colour Pink()
	{
		return Colour((byte)MAX_UNSIGNED_TYPE(byte), 0, 128);
	}

	static Colour White()
	{
		return Colour((byte)MAX_UNSIGNED_TYPE(byte));
	}

	static Colour Black()
	{
		return Colour(0);
	}

private :
	const static float N;
};

#endif