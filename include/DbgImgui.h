#pragma once
//external
#include "imgui-SFML.h"
#include "imgui.h"
#include <Utilities\KDebug.h>

// engine
#include <Krawler.h>
#include <KApplication.h>

static void processEvent(const sf::Event& e)
{
	ImGui::SFML::ProcessEvent(e);
}

// custom imgui component
class imguicomp : public Krawler::KComponentBase
{
public:
	imguicomp(Krawler::KEntity* pEntity) : Krawler::KComponentBase(pEntity) {}
	~imguicomp() = default;

	virtual Krawler::KInitStatus init() override
	{
		ImGui::SFML::Init(*Krawler::KApplication::getApp()->getRenderWindow());
		std::function<void(void)> subLastDraw = std::bind(&imguicomp::draw, this);
		Krawler::KApplication::getApp()->subscribeToEventQueue(processEvent);
		Krawler::KApplication::getApp()->getRenderer()->subscribeLastDrawCallback(subLastDraw);
		m_bWasInitSuccessful = true;
		return Krawler::KInitStatus::Success;
	}

	virtual void cleanUp() override
	{
		ImGui::SFML::Shutdown();
	}

	// call to invoke imgui::begin
	void begin(const std::string& name)
	{
		if (!m_bBeginCalled)
		{
			ImGui::Begin(name.c_str());
			m_bBeginCalled = true;
		}
	}

	// call to invoke imgui::end
	void end()
	{
		if (!m_bEndCalled)
		{
			m_bEndCalled = true;
			ImGui::End();
		}
	}

	// Call to invoke imgui::sfml::update
	void update()
	{
		if (!m_bUpdateRun)
		{
			ImGui::SFML::Update(*Krawler::KApplication::getApp()->getRenderWindow(), sf::seconds(1.0f / (float)(Krawler::KApplication::getApp()->getGameFPS())));
			m_bUpdateRun = true;
		}
	}

private:

	void draw()
	{
		// If init is unsuccessful we'll avoid invoking render since this can force an abrupt halt 
		// instead we'll print that we're skipping an imgui render call 
		if (m_bWasInitSuccessful)
		{
			static bool hasBeenPrint = false;
			if (!hasBeenPrint)
			{
				Krawler::KPrintf(L"Skipping imgui draw\n");
				hasBeenPrint = true;
			}
		}

		// If the update hasn't been run yet, we should totally avoid calling imgui draw
		// this is an applicating halting mistake to make.
		if (!m_bUpdateRun)
		{
			return;
		}


		// If begin was not called, do not try render. 
		// This is an application halting mistake to make.
		if (!m_bBeginCalled)
		{
			return;
		}

		// If end was not called, do not try render. 
		// This is an application halting mistake to make.
		if (!m_bEndCalled)
		{
			return;
		}

		ImGui::SFML::Render(*Krawler::KApplication::getApp()->getRenderWindow());
		m_bUpdateRun = false;
		m_bBeginCalled = false;
		m_bEndCalled = false;
	}

	bool m_bUpdateRun = false;
	bool m_bWasInitSuccessful = false;
	bool m_bBeginCalled = false;
	bool m_bEndCalled = false;
};
