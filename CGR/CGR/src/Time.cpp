#include "Time.h"
#include <wrl.h>

float Time::deltaTime = 0;
float Time::elapsedTime = 0;

LARGE_INTEGER mFrequency;
LARGE_INTEGER mCurrentTime;
LARGE_INTEGER mStartTime;
LARGE_INTEGER mLastTime;

Timer::Timer()
{
	if (!QueryPerformanceFrequency(&mFrequency))
	{
		throw new std::exception();
	}
	Reset();
}

void Timer::Reset()
{
	Update();
	mStartTime = mCurrentTime;
	mTotal = 0.0f;
	mDelta = 1.0f / 60.0f;
}

void Timer::Update()
{
	if (!QueryPerformanceCounter(&mCurrentTime))
	{
		throw new std::exception();
	}

	mTotal = static_cast<float>(
		static_cast<double>(mCurrentTime.QuadPart - mStartTime.QuadPart) /
		static_cast<double>(mFrequency.QuadPart));

	if (mLastTime.QuadPart == mStartTime.QuadPart)
	{
		//if timer was just reset, report time delta equivalent to 60hz frame time
		mDelta = 1.0f / 60.0f;
	}
	else
	{
		mDelta = static_cast<float>(
			static_cast<double>(mCurrentTime.QuadPart - mLastTime.QuadPart) /
			static_cast<double>(mFrequency.QuadPart));
	}

	mLastTime = mCurrentTime;
}