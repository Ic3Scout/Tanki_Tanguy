#include "pch.h"
#include "Core/UIButton.h"
#include "Core/UIDialogue.h"
#include "Gameplay/Player.h"
#include "Core/UIHealthBar.hpp"
#include "Core/UIManager.h"
#include "Core/UIText.hpp"

App::App() : m_gameManager(GameManager(false))
{
	s_pApp = this;
	CPU_CALLBACK_START(OnStart);
	CPU_CALLBACK_UPDATE(OnUpdate);
	CPU_CALLBACK_EXIT(OnExit);
	CPU_CALLBACK_RENDER(OnRender);

	m_pUIManager = UIManager::Get();
}

App::~App()
{
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void App::OnStart()
{
	// Resources
	m_font.Create(12);
	m_rts[0] = cpuEngine.CreateRT();
	
	m_gameManager.Start();
	cpuEngine.GetParticleData()->Create(1000000);
	cpuEngine.m_skyColor	= cpu::ToColor(0, 167, 230);
	cpuEngine.m_groundColor = cpu::ToColor(9, 176, 35);

	// UI
	m_colors = {	XMINT3(171, 75, 75),
					XMINT3(150, 255, 167),
					XMINT3(240, 234, 127),
					XMINT3(152, 176, 235),
					XMINT3(227, 152, 235),
					XMINT3(255, 255, 255),
					XMINT3(0, 0, 0),
	};
	
	m_pathGreen		= "../../res/Gameplay/vert.png";
	m_pathGray		= "../../res/Gameplay/gris.png";
	
	XMINT2 pos		= XMINT2(cpuDevice.GetWidth() / 2, 30);
	m_dialogueLog	= new UIDialogue();
	m_dialogueLog->AddInputText(m_font, pos, CPU_TEXT_CENTER, "Enter an address: ", m_addr);
	m_dialogueLog->AddInputText(m_font, pos, CPU_TEXT_CENTER, "Enter a port: ", m_port);
	m_dialogueLog->AddInputText(m_font, pos, CPU_TEXT_CENTER, "Enter a pseudo: ", m_pseudo);
	
	m_pErrorText = m_pUIManager->Create<UIText>();
	m_pErrorText->Init(XMINT2(cpuDevice.GetWidth() / 2, cpuDevice.GetHeight() - 14), CPU_TEXT_CENTER, "");
	m_pErrorText->Create(14, CPU_RED);
	m_pErrorText->SetActive(false);

	XMFLOAT3 posCam = XMFLOAT3(100.0f, 100.0f, 100.0f);
	cpuEngine.GetCamera()->transform.pos = posCam;
	
	m_pStartBtn = m_pUIManager->Create<UIButton>();
	m_pStartBtn->Init(XMINT2(cpuDevice.GetWidth() - 75, cpuDevice.GetHeight() - 75), XMINT2(50, 50), m_pathGreen, m_pathGray);

	m_pStartBtn->SetOnClickCallback([this]() {
		m_currentColor = (m_currentColor + 1) % m_colors.size();
	});

	XMFLOAT3 posModel	= XMFLOAT3(posCam.x + 3.f, posCam.y, posCam.z + 5.f);
	m_pModelPlayer = m_gameManager.CreateObject<Player>(false);
	m_pModelPlayer->Init(posModel, -1,  -2*XM_PI / 3, "",m_colors[m_currentColor]);
	
	m_pFloorMesh.CreateCube();
	m_pFloorMaterial.color = cpu::ToColor(9, 176, 35);
	for (int i = -(m_width / 2); i < m_width; i++)
	{
		for (int j = -(m_height / 2); j < m_height; j++)
		{
			cpu_entity* floor = cpuEngine.CreateEntity();
			floor->pMesh			= &m_pFloorMesh;
			floor->pMaterial		= &m_pFloorMaterial;
			floor->transform.pos	= XMFLOAT3(static_cast<float>(j) * 5, -1.0f, static_cast<float>(i) * 5);
			floor->transform.sca	= XMFLOAT3(5.0f, 1.0f, 5.0f);
		}
	}
}

void App::SetUI()
{
	m_pStartBtn->SetToBeDestroyed();
	
	UIHealthBar* uiBar = m_pUIManager->Create<UIHealthBar>();
	uiBar->Init(m_font, XMINT2(0.0f, cpuDevice.GetHeight() - 24), 24.0f, *m_pPlayer, m_pathGreen, m_pathGray);

	m_pUiScore = m_pUIManager->Create<UIText>();
	m_pUiScore->Init(XMINT2(cpuDevice.GetWidth() - 10, 5), CPU_TEXT_RIGHT, m_pPlayer->GetScore());
	m_pUiScore->SetFont(m_font);
	
	m_pRespawnText = m_pUIManager->Create<UIText>();
	m_pRespawnText->Init(XMINT2(cpuDevice.GetWidth() / 2, cpuDevice.GetHeight() / 2), CPU_TEXT_CENTER, "");
	m_pRespawnText->Create(12, CPU_GREEN);

	m_pPseudo = m_pUIManager->Create<UIText>();
	m_pPseudo->Init(XMINT2(0, 0), CPU_TEXT_LEFT, m_pseudo);
	m_pPseudo->SetFont(m_font);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool IsValidPort(const std::string& str, int& outPort)
{
	if (str.empty())
		return false;

	for (char c : str)
		if (!std::isdigit(static_cast<unsigned char>(c)))
			return false;
	
	outPort = std::stoi(str);
	return outPort > 0 && outPort <= 65535;
}

void App::Log()
{
	if (m_isRequested)
		return;
    
	m_dialogueLog->Update();

	if (m_dialogueLog->IsActive())
		return;
	
	int port;
	if (!IsValidPort(m_port, port) && m_port != "")
	{
		ShowError("Error Port");
		return;
	}
	
	if (m_client.ConnectToServer(m_addr.c_str(), std::stoi(m_port), m_pseudo.c_str(), m_colors[m_currentColor]))
		m_isRequested = true;
}

void App::ShowError(const std::string& error)
{
	m_isRequested = false;
	m_dialogueLog->Reset();
	m_pErrorText->SetText(error);
	m_pErrorText->SetActive(true);
}

void App::HandleConnectionRequest()
{
	if (!m_isRequested)
		return;
    
	if (m_client.m_isConnected)
	{
		OnConnectionSuccess();
		return;
	}
    
	m_requestProgress += cpuTime.delta;
	if (m_requestProgress >= m_requestTime)
	{
		ShowError("Connexion Failed !");
		m_requestProgress = 0.0f;
	}
}

void App::OnConnectionSuccess()
{
	m_isRequested		= false;
	m_isLogin			= true;
	m_requestProgress	= 0.0f;
    m_pErrorText->SetActive(false);

	m_pModelPlayer->SetNeedToBeDestroyed();
	m_id = m_client.GetServerInfo().playerId;
	m_gameManager.SetMainPlayer(m_id);
	m_pPlayer = m_gameManager.GetMainPlayer();
	m_pPlayer->SetPseudo(m_pseudo);
	m_pPlayer->SetColor(m_colors[m_currentColor].x, m_colors[m_currentColor].y, m_colors[m_currentColor].z);
	SetUI();
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void App::OnUpdate()
{
	if (cpuInput.IsKeyDown(VK_ESCAPE))
		cpuEngine.Quit();
	
	m_pUIManager->Update();
	if (m_pModelPlayer != nullptr)
		m_pModelPlayer->SetColor(
			m_colors[m_currentColor].x,
			m_colors[m_currentColor].y,
			m_colors[m_currentColor].z);
	
	if (!m_isLogin)
	{
		Log();
		HandleConnectionRequest();
		return;
	}
	
	m_client.SendPing();

	m_pUiScore->SetText(m_pPlayer->GetScore());
	m_gameManager.Update();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void App::OnExit()
{
	//CPU_DELPTR(m_pseudo);
	CPU_DELPTR(m_dialogueLog);
	CPU_DELPTR(m_pUIManager);
	m_colors.clear();
	PacketHeader header;
	header.packetType = PacketType::Disconnect;
	header.playerId = m_client.GetServerInfo().playerId;
	BufferWriter writer;
	SerializePacketHeader(writer, header);

	m_client.SendPacket((const char*)writer.data.data(), (int)writer.data.size());
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void App::OnRender(int pass)
{
	switch ( pass )
	{
		case CPU_PASS_PARTICLE_BEGIN:
			break;
		
		case CPU_PASS_PARTICLE_END:
			break;
		
		case CPU_PASS_UI_END:
		{
			m_pUIManager->Render();
				
			XMFLOAT2 screenPos;
			std::vector<Player*> players = m_gameManager.GetPlayers();
	
			if (m_isLogin == false)
				m_dialogueLog->Render();
	
			for (Player* player: players)
			{
				if (player == m_pPlayer)
					continue;

				if (player->IsDead())
					continue;
	
				if (m_gameManager.WorldToScreen(player->GetPos(), screenPos, cpuEngine.GetCamera()->matViewProj, cpuDevice.GetWidth(), cpuDevice.GetHeight()))
					cpuDevice.DrawText(&m_font, player->GetPseudo().c_str(), screenPos.x, screenPos.y - 25.0f, CPU_TEXT_CENTER );
			}

			if (m_pPlayer != nullptr)
			{
				if (m_pPlayer->IsDead())
				{
					m_pRespawnText->SetActive(true);
					m_respawnTime -= cpuTime.delta;
					std::string respawn = "Respawn in..." + std::to_string(static_cast<int>(m_respawnTime));
					m_pRespawnText->SetText(respawn);
				}
				else
				{
					m_respawnTime = 4.0f;
					m_pRespawnText->SetActive(false);
				}
				
			}

			if (!m_isLogin)
				cpuDevice.DrawText(&m_font, "TANKYTANGUY", static_cast<int>((cpuDevice.GetWidth() * 0.5f)), 10, CPU_TEXT_CENTER);

			if (m_pseudo != "")
			{
				if (m_pPseudo == nullptr)
					return;
				m_pPseudo->SetActive(true);
			}
			break;
		}
	}
}
