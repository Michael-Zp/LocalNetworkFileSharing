#pragma once

#include <chrono>
#include <random>

class MyRandom
{
public:
	MyRandom(bool seedIt = true)
	{
		if (seedIt)
		{
			RandomEngine.seed((unsigned int)std::chrono::system_clock::now().time_since_epoch().count());
		}
	}

	uint64_t operator()()
	{
		return Random(RandomEngine);
	}

private:
	std::uniform_int_distribution<uint64_t> Random;
	std::default_random_engine RandomEngine;
};