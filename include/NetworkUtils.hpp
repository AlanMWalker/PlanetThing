#pragma once

#include <chrono>
#include <Krawler.h>

static Krawler::int64 timestamp()
{
	return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}
