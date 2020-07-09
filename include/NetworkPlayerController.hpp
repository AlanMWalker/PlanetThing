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
	// Host
	void handlePlayerMoveHost(ServerClientMessage* scm);
	void handlePlayerFireHost(ServerClientMessage* scm);


	void handlePosUpdateClient(SatellitePositionUpdate* spu);
	void handleFireActivatedClient(FireActivated* fa);

	std::wstring m_playerUUID;

	std::list<Krawler::int32> m_moveSatQueue;

	bool m_bHasNewPos = false;
	bool m_bFireShot = false;

	float m_newTheta = 0.0f;
	float m_networkedStrength = 0.0f;
};