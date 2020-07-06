#pragma once

#include <KEntity.h>
#include <Physics/KPhysicsWorld2D.h>
#include <Components/KCBody.h>

#include <SFML/Graphics.hpp>

#include "Blackboard.hpp"
#include "ProjectilePath.hpp"
#include "NewtonianGravity.hpp"
#include "LocalPlayerController.hpp"
#include "ServerPackets.hpp"
#include "SockSmeller.hpp"


namespace sf
{
	class Text;
}

struct DbgLineDraw
	: public sf::Drawable
{
	DbgLineDraw()
	{

		arr.setPrimitiveType(sf::PrimitiveType::Lines);
		arr.resize(2);
		arr[0].position = Krawler::Vec2f();
		arr[1].position = Krawler::Vec2f();
	}

	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const
	{
		target.draw(arr);
	}

	void setStartEnd(const Krawler::Vec2f& start, const Krawler::Vec2f& end)
	{
		arr[0].position = start;
		arr[1].position = end;
	}

	void setCol(const Krawler::Colour& c)
	{
		arr[0].color = c;
		arr[1].color = c;
	}
	sf::VertexArray arr;

};

class CelestialBody;


class GameSetup :
	public Krawler::KComponentBase
{
public:

	enum class GameType
	{
		Local,
		Networked
	};

	GameSetup();
	~GameSetup();

	virtual Krawler::KInitStatus init() override;
	virtual void onEnterScene() override;
	virtual void tick() override;
	virtual void fixedTick() override;
	virtual void cleanUp() override;

	float colScale = 10000;
	// Invoked by MenuSetup before it transitions into the single player game
	void setAIPlayerCount(Krawler::int32 count);

	// Invoked by LobbySetup before it transitions into the networked game
	void setNetworkPlayerCount(Krawler::int32 count) { m_networkedCount = count; }
	void setGameType(GameType type) { m_gameType = type; }
	void setLevelGen(const GeneratedLevel& level) { m_genLevel = level; }

private:
	
	float PPM = 10.0f;

	void createGod();
	void zoomAt(const Krawler::Vec2f& pos, float zoom);
	void setBackgroundShaderParams();

	void createCelestialBodies();

	void setupLevelLocal();
	void setupLevelNetworkedHost();
	void setupLevelNetworkedClient();

	void manageGameState();

	void manageNetworked(); 
	void manageLocal();

	Krawler::KEntity* m_pPlayerPlanet = nullptr;
	std::vector<Krawler::KEntity*> m_networkedPlanets;
	std::vector<Krawler::KEntity*> m_aiPlanets;
	std::vector<Krawler::KEntity*> m_satellites;
	std::vector<Krawler::KEntity*> m_moons;

	DbgLineDraw line;
	ProjectilePath* m_pPath = nullptr;
	sf::View m_defaultView;

	GameType m_gameType = GameType::Local;

	Krawler::KEntity* m_pObject = nullptr;
	Krawler::KEntity* m_pBackground = nullptr;
	sf::Shader* m_pBackgroundShader = nullptr;
	Krawler::Physics::KPhysicsWorld2D* m_pPhysicsWorld = nullptr;
	LocalPlayerController* m_playerController = nullptr;

	NewtonianGravity m_newton;
	GeneratedLevel m_genLevel;

	Krawler::int32 m_aiCount = Blackboard::MIN_AI;
	Krawler::int32 m_networkedCount = Blackboard::MIN_NETWORKED;

	std::vector<std::wstring> m_lobbyPlayers;
	Krawler::uint64 m_currentPlayerTurnIdx = 0;
};