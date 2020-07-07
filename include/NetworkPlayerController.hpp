#pragma once 

#include <mutex>
#include <atomic>

#include "BaseController.hpp"
#include "SockSmeller.hpp"


class NetworkPlayerController
	: public BaseController
{
public:

	NetworkPlayerController(Krawler::KEntity* pEntity, CelestialBody* pHost);
	~NetworkPlayerController() = default;

	virtual Krawler::KInitStatus init() override;
	virtual void onEnterScene() override;
	virtual void tick() override;

	void setUUID(const std::wstring& uuid) { m_playerUUID = uuid; }

protected:

private:

	void handlePlayerMoveHost(ServerClientMessage* scm);
	void handlePosUpdate(SatellitePositionUpdate* spu);

	std::wstring m_playerUUID;

	std::list<Krawler::int32> m_moveSatQueue;
	std::mutex m_networkMtx;

	atombool m_bHasNewPos = false;
	float m_newTheta = 0.0f;
};