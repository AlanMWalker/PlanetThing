#pragma once 

#include <array>

#include <Krawler.h>
#include <Components/KCRenderableBase.h>
#include <Components/KCTransform.h>

#include <SFML/Graphics/VertexBuffer.hpp>
#include <SFML/Graphics/VertexArray.hpp>
#include <SFML/System/Clock.hpp>

constexpr Krawler::uint32 PARTICLE_COUNT = 50000;//10000;
constexpr Krawler::uint32 PARTICLE_ALIVE_TIME = 60; // seconds

class ProjectilePath :
	public Krawler::Components::KCRenderableBase
{
public:

	ProjectilePath(Krawler::KEntity* pEntity);
	~ProjectilePath() = default;

	virtual Krawler::KInitStatus init() override;
	virtual void tick() override;
	virtual void draw(sf::RenderTarget& rTarget, sf::RenderStates rStates) const override;
	void addPathPoint(const Krawler::Vec2f& position, const Krawler::Colour& col = Krawler::Colour::White);
	virtual void cleanUp();
	Krawler::Rectf getOnscreenBounds() const override { return Krawler::Rectf(1, 1, 1, 1); }

private:

	struct PathParticle
	{
		bool bInUse = false;
		int vertexIdx = 0;
		sf::Clock timer;
	};

	void setActive(PathParticle& pp, const Krawler::Vec2f& pos, const Krawler::Colour& col);
	void setInactive(PathParticle& pp);

	sf::VertexBuffer m_vBuffer;
	//sf::VertexArray m_vArray;
	std::vector<sf::Vertex> m_pathVertices;
	std::vector<PathParticle> m_pathData;

	Krawler::Components::KCTransform* m_pTransform = nullptr;
};