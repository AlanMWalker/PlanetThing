#include "GodDebugComp.h"
#include <Input\KInput.h>
#include <KApplication.h>

#include "NetworkComms.h"
#include "PlayerLocomotive.h"
#include "GameSetup.h"

using namespace Krawler;
using namespace Krawler::Input;

GodDebugComp::GodDebugComp(Krawler::KEntity* pEntity, GameSetup* pSetup)
	: KComponentBase(pEntity), m_pSetup(pSetup)
{
}

KInitStatus GodDebugComp::init()
{
	m_pImgui = getEntity()->getComponent<imguicomp>();

	setStyle(*m_pImgui);

	return KInitStatus::Success;
}

void GodDebugComp::onEnterScene()
{
	auto pRenderer = GET_APP()->getRenderer();
	auto list = GET_SCENE()->getAllocatedEntityList();
	auto& blockedMap = BlockedMap::getInstance();
	for (auto item : list)
	{
		if (item->getTag() == L"Terrain")
		{
			sf::RectangleShape r;
			r = sf::RectangleShape(blockedMap.getTileSize());
			r.setFillColor(Colour(255, 0, 0, 100));
			r.setPosition(item->getTransform()->getPosition());
			m_colliderShapes.push_back(r);
		}
	}

	for (auto& r : m_colliderShapes)
	{
		pRenderer->addDebugShape(&r);
	}
}

void GodDebugComp::tick()
{
	// Just so game setup can handle spawning networked players on the 
	// correct thread we call a game setup tick since it's not 
	// a component, waking game setup up to check if it should make
	// a networked player visible
	m_pSetup->tick();

	if (KInput::JustPressed(KKey::Escape))
	{
		// close
		GET_APP()->closeApplication();
	}
	auto pPlayerEntity = GET_SCENE()->findEntity(L"Player");
	float* pMoveSpeed = &pPlayerEntity->getComponent<PlayerLocomotive>()->m_moveSpeed;
	handlePathClicks();

	static bool bDebugShapes = GET_APP()->getRenderer()->isShowingDebugDrawables();
	static bool bShowTileNodes = false;

	m_pImgui->update();
	m_pImgui->begin("-- God Debug Tools --");

	ImGui::Checkbox("Show Debug Shapes", &bDebugShapes);
	ImGui::Separator();

	ImGui::Checkbox("Visual Tile Nodes Mode", &bShowTileNodes);
	if (bShowTileNodes)
	{
		handleShowTileChildren();
	}
	ImGui::Separator();

	ImGui::Checkbox("Dodge Settings", &pShowDodgeSettings);
	{
		if (pShowDodgeSettings)
		{
			auto pDodgeTiming = &GET_SCENE()->findEntity(L"Player")->getComponent<PlayerLocomotive>()->m_dodgeTiming;
			auto pDodgeCooldown = &GET_SCENE()->findEntity(L"Player")->getComponent<PlayerLocomotive>()->m_dodgeCooldown;
			auto pDodgeSpeed = &GET_SCENE()->findEntity(L"Player")->getComponent<PlayerLocomotive>()->m_dodgeMultiplyer;

			ImGui::Text("Dodge Timing"); ImGui::SameLine(); ImGui::SliderFloat("", pDodgeTiming, 0.0f, 5.0f, "%.2f");
			ImGui::Text("Dodge Cooldown"); ImGui::SameLine(); ImGui::SliderFloat("", pDodgeCooldown, 0.0f, 5.0f, "%.2f");
			ImGui::Text("Dodge Speed"); ImGui::SameLine(); ImGui::SliderFloat("", pDodgeSpeed, 0.0f, 5.0f, "%.2f");
		}
	}
	ImGui::Separator();

	//if (sf::Joystick::isConnected(0))
	{
		ImGui::Text("yoyStick Axis Testing");
		{
			// Left Stick
			Vec2f in0 = Vec2f(sf::Joystick::getAxisPosition(0, sf::Joystick::X), sf::Joystick::getAxisPosition(0, sf::Joystick::Y));

			// Right Stick
			Vec2f in2 = Vec2f(sf::Joystick::getAxisPosition(0, sf::Joystick::Z), sf::Joystick::getAxisPosition(0, sf::Joystick::R));

			// in1.x = L2 analog | in1.y = R2 analog
			Vec2f in1 = Vec2f(sf::Joystick::getAxisPosition(0, sf::Joystick::V), sf::Joystick::getAxisPosition(0, sf::Joystick::U));

			// D-pad input
			Vec2f in3 = Vec2f(sf::Joystick::getAxisPosition(0, sf::Joystick::PovX), sf::Joystick::getAxisPosition(0, sf::Joystick::PovY) * -1.0f);

			ImGui::Text("LS, x = %f | y = %f", in0.x, in0.y);

			ImGui::Spacing();
			ImGui::Text("RS, x = %f | y = %f", in2.x, in2.y);
			ImGui::Spacing();
			ImGui::Text("Triggers, x = %f | y = %f", in1.x, in1.y);
			ImGui::Spacing();
			ImGui::Text("D-pad, x = %f | y = %f", in3.x, in3.y);
		}
		ImGui::Separator();
	}
	ImGui::InputFloat("Player Move Speed", pMoveSpeed);
	ImGui::Separator();

	static bool canSpawnIn = true;
	static char playerName[12];
	if (canSpawnIn)
	{
		ImGui::Text("Game Server Login");
		ImGui::InputText("Player Name", playerName, 12);
		canSpawnIn = !(ImGui::Button("Spawn to Server"));
		if (!canSpawnIn)
		{
			NetworkComms::get().setPlayerName(TO_WSTR(playerName));
			NetworkComms::get().spawnIn(pPlayerEntity->getTransform()->getPosition());
		}
	}
	networkImgui();
	m_pImgui->end();

	GET_APP()->getRenderer()->showDebugDrawables(bDebugShapes);
}

void GodDebugComp::onExitScene()
{
	m_tileAndChildren.clear();
	m_colliderShapes.clear();
}

void GodDebugComp::handlePathClicks()
{
	static int32 clickCount = 0;
	static Vec2f start, end;
	static bool bEnabled = false;

	if (KInput::JustPressed(KKey::P))
	{
		KPRINTF("Enabled path clicks\n");
		bEnabled = true;
	}

	if (bEnabled)
	{
		if (KInput::MouseJustPressed(KMouseButton::Left) && clickCount == 0)
		{
			KPRINTF("Left click registered \n");

			for (auto& s : m_drawnPath)
				GET_APP()->getRenderer()->removeDebugShape(&s);

			m_drawnPath.clear();

			start = KInput::GetMouseWorldPosition();
			m_drawnPath.push_back(sf::RectangleShape(Vec2f(10, 10)));
			m_drawnPath.back().setPosition(start);
			m_drawnPath.back().setOrigin(Vec2f(5, 5));
			++clickCount;
		}

		if (KInput::MouseJustPressed(KMouseButton::Right) && clickCount == 1)
		{
			KPRINTF("Right click registered \n");

			end = KInput::GetMouseWorldPosition();
			auto& blockedMap = BlockedMap::getInstance();
			auto path = blockedMap.getWalkablePath(start, end);

			for (auto& point : path)
			{
				m_drawnPath.push_back(sf::RectangleShape(Vec2f(10, 10)));
				m_drawnPath.back().setPosition(point);
				m_drawnPath.back().setOrigin(Vec2f(5, 5));
				m_drawnPath.back().setFillColor(Colour::Green);
			}

			for (auto& point : m_drawnPath)
			{
				GET_APP()->getRenderer()->addDebugShape(&point);
			}
			clickCount = 0;
			return;
		}
	}
}

void GodDebugComp::handleShowTileChildren()
{
	auto& blockedMap = BlockedMap::getInstance();

	if (KInput::MouseJustPressed(KMouseButton::Left))
	{
		const Vec2f mousePos = KInput::GetMouseWorldPosition();
		for (auto& s : m_tileAndChildren)
		{
			GET_APP()->getRenderer()->removeDebugShape(&s);
		}
		m_tileAndChildren.clear();
		auto positions = blockedMap.getTileChildren(mousePos);
		auto tilesize = blockedMap.getTileSize();
		for (auto p : positions)
		{
			m_tileAndChildren.push_back(sf::RectangleShape(tilesize));
			if (m_tileAndChildren.size() == 1)
			{
				m_tileAndChildren.back().setFillColor(Colour{ 0,255,255, 80 });
			}
			else
			{
				m_tileAndChildren.back().setFillColor(Colour{ 0,255, 255, 80 });
			}
			m_tileAndChildren.back().setPosition(p);
		}

		for (auto& t : m_tileAndChildren)
		{
			GET_APP()->getRenderer()->addDebugShape(&t);
		}

	}
}

void GodDebugComp::networkImgui()
{
	static bool bShowNetworkDebugs = false;
	ImGui::Checkbox("Show Network Debugs", &bShowNetworkDebugs);
	if (bShowNetworkDebugs)
	{
		auto timeInMs = NetworkComms::get().getServerResponseTime().asMilliseconds();
		ImGui::Text("Server Response Time: %d ms", timeInMs);

	}
}

void GodDebugComp::setStyle(imguicomp ImGui)
{
	ImGuiStyle* style = &ImGui::GetStyle();

	style->WindowPadding = ImVec2(15, 15);
	style->WindowRounding = 0.0f;
	style->FramePadding = ImVec2(5, 5);
	style->FrameRounding = 0.0f;
	style->ItemSpacing = ImVec2(12, 8);
	style->ItemInnerSpacing = ImVec2(8, 6);
	style->IndentSpacing = 25.0f;
	style->ScrollbarSize = 15.0f;
	style->ScrollbarRounding = 0.0f;
	style->GrabMinSize = 5.0f;
	style->GrabRounding = 0.0;

	style->Colors[ImGuiCol_Text] = ImVec4(0.80f, 0.80f, 0.83f, 1.00f);
	style->Colors[ImGuiCol_TextDisabled] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
	style->Colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
	style->Colors[ImGuiCol_ChildWindowBg] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
	style->Colors[ImGuiCol_PopupBg] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
	style->Colors[ImGuiCol_Border] = ImVec4(0.80f, 0.80f, 0.83f, 0.88f);
	style->Colors[ImGuiCol_BorderShadow] = ImVec4(0.92f, 0.91f, 0.88f, 0.00f);
	style->Colors[ImGuiCol_FrameBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
	style->Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
	style->Colors[ImGuiCol_FrameBgActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
	style->Colors[ImGuiCol_TitleBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
	style->Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(1.00f, 0.98f, 0.95f, 0.75f);
	style->Colors[ImGuiCol_TitleBgActive] = ImVec4(0.07f, 0.07f, 0.09f, 1.00f);
	style->Colors[ImGuiCol_MenuBarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
	style->Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
	style->Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
	style->Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
	style->Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
	style->Colors[ImGuiCol_CheckMark] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
	style->Colors[ImGuiCol_SliderGrab] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
	style->Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
	style->Colors[ImGuiCol_Button] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
	style->Colors[ImGuiCol_ButtonHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
	style->Colors[ImGuiCol_ButtonActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
	style->Colors[ImGuiCol_Header] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
	style->Colors[ImGuiCol_HeaderHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
	style->Colors[ImGuiCol_HeaderActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
	style->Colors[ImGuiCol_Column] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
	style->Colors[ImGuiCol_ColumnHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
	style->Colors[ImGuiCol_ColumnActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
	style->Colors[ImGuiCol_ResizeGrip] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	style->Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
	style->Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
	style->Colors[ImGuiCol_PlotLines] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
	style->Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
	style->Colors[ImGuiCol_PlotHistogram] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
	style->Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
	style->Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.25f, 1.00f, 0.00f, 0.43f);
	style->Colors[ImGuiCol_ModalWindowDarkening] = ImVec4(1.00f, 0.98f, 0.95f, 0.73f);
}
