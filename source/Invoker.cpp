#include "Invoker.hpp"
#include "SockSmeller.hpp"

using namespace Krawler;

Invoker::Invoker(KEntity* pEntity)
	: KComponentBase(pEntity)
{
}

KInitStatus Invoker::init()
{
	return KInitStatus::Success;
}

void Invoker::tick()
{
	// Network invoke on main thread
	std::mutex& m = SockSmeller::get().getQueueMutex();
	std::lock_guard<std::mutex> g(m);

	auto& subQueue = SockSmeller::get().getSubscribersQueue();
	auto& subData = SockSmeller::get().getSubscriberData();

	KCHECK(subData.size() == subQueue.size());
	for (uint64 i = 0; i < subQueue.size(); ++i)
	{
		subQueue[i](subData[i]);
	}

	for (auto& d : subData)
	{
		KFREE(d);
	}

	subData.clear();
	subQueue.clear();
}
