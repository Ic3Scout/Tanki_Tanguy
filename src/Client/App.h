#pragma once

#include "Gameplay/Core/GameManager.h"
#include "Network/ClientSocket.h"

class Player;
class GameManager;
class UIManager;
class UIDialogue;
class UIText;
class UIButton;

class App
{
public:
	App();
	virtual ~App();

	static App& GetInstance() { return *s_pApp; }

	void OnStart();
	void OnUpdate();
	void ShowError(const std::string& error);
	void SetUI();
	void HandleConnectionRequest();
	void OnConnectionSuccess();
	void OnExit();
	void OnRender(int pass);

	void Log();
	void SetLog(bool _state) { m_isLogin = _state; }
	
	std::string m_addr;
	std::string m_port;
	std::string m_pseudo;
	
private:
	inline static App* s_pApp	= nullptr;
	GameManager m_gameManager;

	UIDialogue* m_dialogueLog	= nullptr;
	UIText* m_pUiScore			= nullptr;
	UIText* m_pRespawnText		= nullptr;
	UIText* m_pErrorText		= nullptr;
	UIText* m_pPseudo			= nullptr;
	ClientSocket m_client;
	
	float m_requestTime			= 2.0f;
	float m_requestProgress		= 0.0f;
	float m_respawnTime			= 4.0f;
	
	bool m_isLogin				= false;
	bool m_isRequested			= false;
	bool m_isRunning			= false;

	float m_width				= 50.0f;
	float m_height				= 50.0f;
	
	// Resources
	cpu_font m_font;
	cpu_rt* m_rts[1];

	// Player
	int m_id = -1;
	Player* m_pPlayer			= nullptr;
	Player* m_pModelPlayer		= nullptr;
	
	// UI
	UIManager* m_pUIManager;
	UIButton* m_pStartBtn		= nullptr;
	std::string m_pathGreen;
	std::string m_pathGray;

	int m_currentColor = 0;
	std::vector<XMINT3> m_colors;

	// Map
	cpu_mesh m_pFloorMesh;
	cpu_material m_pFloorMaterial;
};

// =======STATE-MACHINE======= //
/*struct StateShipGlobal
{
	void OnEnter(Ship& cur, int from);
	void OnExecute(Ship& cur);
	void OnExit(Ship& cur, int to);
};

struct StateShipIdle
{
	void OnEnter(Ship& cur, int from);
	void OnExecute(Ship& cur);
	void OnExit(Ship& cur, int to);
};

struct StateShipBlink
{
	void OnEnter(Ship& cur, int from);
	void OnExecute(Ship& cur);
	void OnExit(Ship& cur, int to);
};*/
