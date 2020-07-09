#pragma once 

#include <Krawler.h>
#include <KComponent.h>
#include <KEntity.h>

class Invoker
	: public Krawler::KComponentBase
{
public:

	Invoker(Krawler::KEntity* pEntity);
	~Invoker() = default; 

	virtual Krawler::KInitStatus init() override;
	virtual void tick() override;

private:

};
