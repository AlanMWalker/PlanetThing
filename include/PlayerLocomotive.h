#pragma once

#include <Krawler.h>
#include <Components\KCSprite.h>
#include <SFML\Graphics\RectangleShape.hpp>
#include <KComponent.h>
#include <Components\KCBoxCollider.h>

#include "DbgImgui.h"


struct DbgLine
	: public sf::Drawable
{
	DbgLine()
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

class PlayerLocomotive 
	: public Krawler::KComponentBase
{
public:

	PlayerLocomotive(Krawler::KEntity* pEntity);
	~PlayerLocomotive() = default;

	virtual Krawler::KInitStatus init() override;
	virtual void onEnterScene() override;
	virtual void tick() override;

private:

	imguicomp* m_pImgui = nullptr;
	Krawler::Components::KCSprite* m_pSprite = nullptr;
	sf::RectangleShape m_shape;
	DbgLine m_line;
	
	void resolve(const Krawler::KCollisionDetectionData& collData);

	Krawler::Components::KCColliderBaseCallback m_callback = [this](const Krawler::KCollisionDetectionData& collData)
	{
		resolve(collData);
	};

	float m_moveSpeed = 100.0f;
};
