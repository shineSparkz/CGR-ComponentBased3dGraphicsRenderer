#ifndef __TIME_H__
#define __TIME_H__

#include <exception>

class Time
{
public:
	static float DeltaTime()
	{
		return deltaTime;
	}

	static float ElapsedTime()
	{
		return elapsedTime;
	}

	static bool CheckTimeElapsed(float& time_diff, const float time_against)
	{
		if ((elapsedTime - time_diff) > time_against)
		{
			time_diff = elapsedTime;
			return true;
		}
		return false;
	}

private:
	friend class Application;
	static float deltaTime;
	static float elapsedTime;
};

class Timer sealed
{
public:
	Timer();

	void Reset();
	void Update();

	float Total()
	{
		return mTotal;
	}

	float Delta()
	{
		return mDelta;
	}

private:
	float mTotal;
	float mDelta;
};

#endif
