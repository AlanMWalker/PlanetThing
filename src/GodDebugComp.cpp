#include "GodDebugComp.h"
#include <Input\KInput.h>
#include <KApplication.h>

#include "ClientPoll.h"
#include "PlayerLocomotive.h"
#include "GameSetup.h"
#include "ServerPoll.h"
#include "ClientPoll.h"
#include "Camera.h"

using namespace Krawler;
using namespace Krawler::Input;

GodDebugComp::GodDebugComp(Krawler::KEntity* pEntity, GameSetup* pSetup, ServerPoll* pServerPoll, ClientPoll* pClientPoll)
	: KComponentBase(pEntity), m_pSetup(pSetup), m_pServerPoll(pServerPoll), m_pClientPoll(pClientPoll)
{
}

KInitStatus GodDebugComp::init()
{
	m_pImgui = getEntity()->getComponent<imguicomp>();
	return KInitStatus::Success;
}

void GodDebugComp::onEnterScene()
{
	setStyle(*m_pImgui);

	auto pRenderer = GET_APP()->getRenderer();
	auto list = GET_SCENE()->getAllocatedEntityList();
	auto& blockedMap = BlockedMap::getInstance();
	for (auto item : list)
	{
		if (item->getTag() == L"Terrain")
		{
			sf::RectangleShape r;
			r = sf::RectangleShape(blockedMap.getTileSize());
			r.setFillColor(Colour::Transparent);
			r.setPosition(item->getTransform()->getPosition());
			m_terrainColliderShapes.push_back(r);
		}
	}

	for (auto& r : m_terrainColliderShapes)
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
	handleImgui();
}

void GodDebugComp::onExitScene()
{
	m_tileAndChildren.clear();
	m_terrainColliderShapes.clear();
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
		if (!m_pServerPoll->isServerRunning())
		{
			auto timeInMs = m_pClientPoll->getServerResponseTime().asMilliseconds();
			ImGui::Text("Server Response Time: %d ms", timeInMs);
		}
	}
}

void GodDebugComp::handleImgui()
{
	if (KInput::JustPressed(KKey::Escape))
	{
		// close
		GET_APP()->closeApplication();
	}
	

	static bool bDebugShapes = GET_APP()->getRenderer()->isShowingDebugDrawables();
	static bool bShowTileNodes = false;

	m_pImgui->update();
	ImGui::PushFont(m_pImguiFont);
	m_pImgui->begin("-- God Debug Tools --");
	ImGui::Checkbox("Show Debug Shapes", &bDebugShapes);
	if (bDebugShapes)
	{
		bool before = m_bShowTerrainColliders;
		ImGui::Checkbox("Show Terrain Colliders", &m_bShowTerrainColliders);
		if (before != m_bShowTerrainColliders && m_bShowTerrainColliders)
		{
			for (auto& shape : m_terrainColliderShapes)
			{
				shape.setFillColor(Colour(255, 0, 0, 100));
			}
		}
		else if (before != m_bShowTerrainColliders && !m_bShowTerrainColliders)
		{
			for (auto& shape : m_terrainColliderShapes)
			{
				shape.setFillColor(Colour::Transparent);
			}
		}
	}
	ImGui::Separator();

	if (m_pServerPoll->isServerRunning())
	{
		showServerDebugs();
	}
	else
	{
		showNonServerDebugs();
	}

	ImGui::Checkbox("Visual Tile Nodes Mode", &bShowTileNodes);
	if (bShowTileNodes)
	{
		handleShowTileChildren();
	}
	ImGui::Separator();
		
	networkImgui();
	ImGui::PopFont();
	m_pImgui->end();

	GET_APP()->getRenderer()->showDebugDrawables(bDebugShapes);
}

void GodDebugComp::handleServerDebugCamera()
{
	Vec2f dir;

	if (KInput::Pressed(KKey::W))
	{
		dir.y = -1.0f;
	}

	if (KInput::Pressed(KKey::S))
	{
		dir.y = 1.0f;
	}


	if (KInput::Pressed(KKey::A))
	{
		dir.x = -1.0f;
	}

	if (KInput::Pressed(KKey::D))
	{
		dir.x = 1.0f;
	}

	dir = Normalise(dir);
	getEntity()->getComponent<Camera>()->moveCamera(dir * m_freeCameraMoveSpeed);
}

void GodDebugComp::showNonServerDebugs()
{
	auto pPlayerEntity = GET_SCENE()->findEntity(L"Player");
	float* pMoveSpeed = &pPlayerEntity->getComponent<PlayerLocomotive>()->m_moveSpeed;
	handlePathClicks();
	ImGui::Checkbox("Dodge Settings", &m_bShowDodgeSettings);
	{
		if (m_bShowDodgeSettings)
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
	ImGui::InputFloat("Player Move Speed", pMoveSpeed);
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
			m_pClientPoll->setPlayerName(TO_WSTR(playerName));
			m_pClientPoll->spawnIn(pPlayerEntity->getTransform()->getPosition());
		}
	}
}

void GodDebugComp::showServerDebugs()
{
	handleServerDebugCamera();

	ImGui::InputFloat("Free Camera Movement Speed", &m_freeCameraMoveSpeed);
	ImGui::Separator();
}

void GodDebugComp::setStyle(imguicomp ImGui)
{
	// Sorry if there's any errors here, I translated this back by hand.
	auto& io = ImGui::GetIO();
	m_pImguiFont = io.Fonts->AddFontFromFileTTF("res/font//ChakraPetch-SemiBold.ttf", 15.0f);
	ImGui::SFML::UpdateFontTexture();
	//io.Fonts->AddFontDefault(m_pImguiFont->ConfigData);

	auto& style = ImGui::GetStyle();
	style.FrameRounding = 4.0f;
	style.WindowBorderSize = 0.0f;
	style.PopupBorderSize = 0.0f;
	style.GrabRounding = 4.0f;

	ImVec4* colors = style.Colors;

	/// 0 = FLAT APPEARENCE
	/// 1 = MORE "3D" LOOK
	int is3D = 1;

	colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
	colors[ImGuiCol_TextDisabled] = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
	colors[ImGuiCol_ChildBg] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
	colors[ImGuiCol_WindowBg] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
	colors[ImGuiCol_PopupBg] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
	colors[ImGuiCol_Border] = ImVec4(0.12f, 0.12f, 0.12f, 0.71f);
	colors[ImGuiCol_BorderShadow] = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
	colors[ImGuiCol_FrameBg] = ImVec4(0.42f, 0.42f, 0.42f, 0.54f);
	colors[ImGuiCol_FrameBgHovered] = ImVec4(0.42f, 0.42f, 0.42f, 0.40f);
	colors[ImGuiCol_FrameBgActive] = ImVec4(0.56f, 0.56f, 0.56f, 0.67f);
	colors[ImGuiCol_TitleBg] = ImVec4(0.19f, 0.19f, 0.19f, 1.00f);
	colors[ImGuiCol_TitleBgActive] = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);
	colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.17f, 0.17f, 0.17f, 0.90f);
	colors[ImGuiCol_MenuBarBg] = ImVec4(0.335f, 0.335f, 0.335f, 1.000f);
	colors[ImGuiCol_ScrollbarBg] = ImVec4(0.24f, 0.24f, 0.24f, 0.53f);
	colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.52f, 0.52f, 0.52f, 1.00f);
	colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.76f, 0.76f, 0.76f, 1.00f);
	colors[ImGuiCol_CheckMark] = ImVec4(0.65f, 0.65f, 0.65f, 1.00f);
	colors[ImGuiCol_SliderGrab] = ImVec4(0.52f, 0.52f, 0.52f, 1.00f);
	colors[ImGuiCol_SliderGrabActive] = ImVec4(0.64f, 0.64f, 0.64f, 1.00f);
	colors[ImGuiCol_Button] = ImVec4(0.54f, 0.54f, 0.54f, 0.35f);
	colors[ImGuiCol_ButtonHovered] = ImVec4(0.52f, 0.52f, 0.52f, 0.59f);
	colors[ImGuiCol_ButtonActive] = ImVec4(0.76f, 0.76f, 0.76f, 1.00f);
	colors[ImGuiCol_Header] = ImVec4(0.38f, 0.38f, 0.38f, 1.00f);
	colors[ImGuiCol_HeaderHovered] = ImVec4(0.47f, 0.47f, 0.47f, 1.00f);
	colors[ImGuiCol_HeaderActive] = ImVec4(0.76f, 0.76f, 0.76f, 0.77f);
	colors[ImGuiCol_Separator] = ImVec4(0.000f, 0.000f, 0.000f, 0.137f);
	colors[ImGuiCol_SeparatorHovered] = ImVec4(0.700f, 0.671f, 0.600f, 0.290f);
	colors[ImGuiCol_SeparatorActive] = ImVec4(0.702f, 0.671f, 0.600f, 0.674f);
	colors[ImGuiCol_ResizeGrip] = ImVec4(0.26f, 0.59f, 0.98f, 0.25f);
	colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.67f);
	colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
	colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
	colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
	colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
	colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
	colors[ImGuiCol_TextSelectedBg] = ImVec4(0.73f, 0.73f, 0.73f, 0.35f);
	colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
	colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
	colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
	colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
	colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);

	style.PopupRounding = 3;

	style.WindowPadding = ImVec2(4, 4);
	style.FramePadding = ImVec2(6, 4);
	style.ItemSpacing = ImVec2(6, 2);

	style.ScrollbarSize = 18;

	style.WindowBorderSize = 1;
	style.ChildBorderSize = 1;
	style.PopupBorderSize = 1;
	style.FrameBorderSize = is3D;

	style.WindowRounding = 3;
	style.ChildRounding = 3;
	style.FrameRounding = 3;
	style.ScrollbarRounding = 2;
	style.GrabRounding = 3;

#ifdef IMGUI_HAS_DOCK 
	style.TabBorderSize = is3D;
	style.TabRounding = 3;

	colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.38f, 0.38f, 0.38f, 1.00f);
	colors[ImGuiCol_Tab] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
	colors[ImGuiCol_TabHovered] = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);
	colors[ImGuiCol_TabActive] = ImVec4(0.33f, 0.33f, 0.33f, 1.00f);
	colors[ImGuiCol_TabUnfocused] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
	colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.33f, 0.33f, 0.33f, 1.00f);
	colors[ImGuiCol_DockingPreview] = ImVec4(0.85f, 0.85f, 0.85f, 0.28f);

	if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}
#endif
}