#include "ProjectilePath.hpp"
#include <KEntity.h>

using namespace Krawler;
using namespace Krawler::Components;

ProjectilePath::ProjectilePath(Krawler::KEntity* pEntity)
	: KCRenderableBase(pEntity)
{
}

KInitStatus ProjectilePath::init()
{
	if (!m_vBuffer.create(PARTICLE_COUNT))
	{
		return KInitStatus::Failure;
	}
	/*m_vArray.resize(PARTICLE_COUNT);
	m_vArray.setPrimitiveType(sf::PrimitiveType::Points);*/

	m_vBuffer.setPrimitiveType(sf::PrimitiveType::Points);
	m_vBuffer.setUsage(sf::VertexBuffer::Usage::Stream);

	m_pathData.resize(PARTICLE_COUNT);
	m_pathVertices.resize(PARTICLE_COUNT);

	//for (auto& pp : m_pathData)
	for (int i = 0; i < PARTICLE_COUNT; ++i)
	{
		auto& pp = m_pathData[i];
		setInactive(pp);
		pp.vertexIdx = i;
	}
	//m_vBuffer.update(&m_pathVertices[0]);
	m_pTransform = getEntity()->m_pTransform;
	return KInitStatus::Success;
}

void ProjectilePath::tick()
{
	for (auto& pp : m_pathData)
	{
		if (pp.timer.getElapsedTime().asSeconds() > PARTICLE_ALIVE_TIME)
		{
			setInactive(pp);
		}
	}

	// For now update the whole buffer
	// but later on optimise to only update 
	// parts which have recently become active
	m_vBuffer.update(&m_pathVertices[0]);
	
}

void ProjectilePath::draw(sf::RenderTarget& rTarget, sf::RenderStates rStates) const
{
	rStates.transform *= m_pTransform->getTransform();
	rStates.texture = nullptr;
	//rStates.shader = getShader();
	rTarget.draw(m_vBuffer, rStates);
	//rTarget.draw(m_vArray, rStates);
}

void ProjectilePath::addPathPoint(const Vec2f& position, const Colour& col)
{
	auto result = std::find_if(m_pathData.begin(), m_pathData.end(), [](const PathParticle& pp)  -> bool {
		return !pp.bInUse;
		});

	if (result == m_pathData.end())
	{
		return;
	}

	setActive(*result, position, col);
}

void ProjectilePath::cleanUp()
{
	//m_vArray.clear();
	m_vBuffer.create(0);
}

void ProjectilePath::setActive(PathParticle& pp, const Vec2f& pos, const Colour& col)
{
	pp.bInUse = true;
	pp.timer.restart();
	//m_vArray[pp.vertexIdx].color = col;
	//m_vArray[pp.vertexIdx].position = pos;

	m_pathVertices[pp.vertexIdx].color = col;
	m_pathVertices[pp.vertexIdx].position = pos;
}

void ProjectilePath::setInactive(PathParticle& pp)
{
	pp.bInUse = false;
	m_pathVertices[pp.vertexIdx].color = sf::Color::Transparent;
	//m_vArray[pp.vertexIdx].color = sf::Color::Transparent;
}

