#pragma once
#ifndef __UTILS_H__
#define __UTILS_H__

#include <string>
#include <sstream>
#include <fstream>
#include <vector>

#include "types.h"

namespace util

{
	INLINE std::string get_file_extension(const std::string& fileIn)
	{
		size_t index = 0;

		// Determine what type it is
		for (size_t i = fileIn.size() - 1; i >= 0; --i)
		{
			if (fileIn[i] == '.')
			{
				index = i;
				break;
			}
		}

		if (index <= 0)
		{
			return "";
		}

		std::string file_type;
		for (size_t i = index + 1; i < fileIn.size(); ++i)
		{
			file_type += fileIn[i];
		}

		return file_type;
	}

	INLINE int32 str_hash(const std::string& str)
	{
		int32 hash = 0;

		for (int32 i = 0; i < (int32)str.size(); ++i)
		{
			hash += (str[i] * (i + 119));
		}

		return hash;
	}

	INLINE std::string str_to_lower(const std::string& str)
	{
		std::string s_out;
		s_out.resize(str.size());

		for (size_t i = 0; i < str.size(); ++i)
		{
			char c = str[i];

			if (c >= 0x41 && c <= 0x5A)
				c += 32;

			s_out[i] = c;
		}

		return s_out;
	}

	INLINE std::string bool_to_str(bool b)
	{
		return b ? "true" : "false";
	}

	INLINE bool str_contains(const std::string& str, char c)
	{
		for (size_t i = 0; i < str.size(); ++i)
		{
			if (str[i] == c)
				return true;
		}

		return false;
	}

	INLINE int32 str_to_int(char* str)
	{
		int32 value = 0;
		while ( *str != NULL )
		{
			value = (value * 10) + *str - '0';
			++str;
		}
		return value;
	}

	INLINE float str_to_float(const std::string& str)
	{
		return (float)atof(str.c_str());
	}

	INLINE std::vector<std::string> split_str(const std::string& s, char token)
	{
		std::stringstream ss(s);
		std::vector<std::string> split_line;

		while ( ss.good() )
		{
			std::string substr;
			getline(ss, substr, token);
			split_line.push_back(substr);
		}

		return split_line;
	}
	
	template <typename Type> INLINE std::string to_str(const Type& t)
	{
		std::stringstream os;
		os << t;
		return os.str();
	}

	INLINE std::string vec3_to_str(const Vec3& v)
	{
		return "(" + util::to_str(v.x) + "," + util::to_str(v.y) + "," + util::to_str(v.z) + ")";
	}

	template <class T> INLINE void quick_removal(size_t index_to_remove, std::vector<T>& list_, bool isPointers)
	{
		if ( index_to_remove != list_.size() - 1 )
		{
			std::swap(list_[index_to_remove], list_.back());
			if ( isPointers )
				delete list_[list_.size() - 1];
			list_.pop_back();
		}
		else
		{
			if ( isPointers )
				delete list_[list_.size() - 1];
			list_.pop_back();
		}
	}
}

namespace random
{
	int RandomRange(int32 min, int32 max);
	float RandFloat(float min, float max);
}
#endif