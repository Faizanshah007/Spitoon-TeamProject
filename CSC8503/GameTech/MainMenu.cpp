#include "MainMenu.h"
#include "../CSC8503Common/InputHandler.h"
#include "NetworkedGame.h"
#include "TrainingGame.h"

#include "../../Common/Assets.h"
using namespace NCL;
using namespace CSC8503;

MainMenu::MainMenu() 
	: renderer(RendererBase()), networkedLevel(nullptr), trainingLevel(nullptr) {
	pushMachine = new PushdownMachine((PushdownState*)this);
	inputHandler = new InputHandler();

	AxisCommand* m = new MenuMoveCommand(this);
	inputHandler->BindAxis(0, m);
	Command* enter = new MenuEnterCommand(this);
	inputHandler->BindButton(Input::JUMP, enter);
	AudioManager* a = NCL::AudioManager::GetInstance();
	a->Initialize();
	a->LoadSound(Assets::AUDIODIR + "menu_music.ogg", false, true, true);
	a->LoadSound(Assets::AUDIODIR + "menu_move.ogg", false, false, false);
	a->LoadSound(Assets::AUDIODIR + "menu_select.ogg", false, false, false);
	bgm = NCL::BGMManager::GetInstance();
	bgm->PlaySongFade(Assets::AUDIODIR + "menu_music.ogg", 0.5f);
}

MainMenu::~MainMenu() {
	delete pushMachine;
	delete inputHandler;
}

PushdownState::PushdownResult MainMenu::OnUpdate(float dt, PushdownState** newState) {
	UpdateMenu(dt);
	if (!pressed) return PushdownState::PushdownResult::NoChange;
	switch (selectedItem)
	{
	default:
		return PushdownState::PushdownResult::NoChange;
		break;
	case 0:
		if (networkedLevel) delete networkedLevel;
		networkedLevel = new NetworkedGame();
		*newState = new LevelState(networkedLevel);
		pressed = false;
		bgm->PlaySong(Assets::AUDIODIR + "game_music.ogg");
		return PushdownState::PushdownResult::Push;
		break;
	case 1:
		if (trainingLevel) delete trainingLevel;
		trainingLevel = new TrainingGame();
		*newState = new LevelState(trainingLevel);
		bgm->PlaySong(Assets::AUDIODIR + "game_music.ogg");
		pressed = false;
		return PushdownState::PushdownResult::Push;
		break;
	case 2:
		return PushdownState::PushdownResult::Exit;
		break;
	}
}

void MainMenu::UpdateMenu(float dt) {
	NCL::AudioManager::GetInstance()->Update();
	renderer.Render();

	float framed = (renderer.GetFrameNumber() / 240.f);
	float size = (50.0f * abs(sin(framed))) + 40.0f;

	if (renderer.GetFrameNumber() % 5 == 1) {
		inputHandler->HandleInput();
	}

	renderer.DrawString("Spitoon", { 20,30 }, { 0.6f,0.3f,0.8f,1.0f },  size );

	auto drawMenuOption = [=](const std::string& string, const Maths::Vector2 pos, int selection, int menuNumber) {
		if (selection == menuNumber) {
			renderer.DrawString(string, pos, { 1.0f,0.2f,0.2f,1.0f });
		}
		else {
			renderer.DrawString(string, pos, { 0.8f,0.8f,0.8f,1.0f });
		}
	};

	drawMenuOption("Start Networked Game", { 30, 50 }, selectedItem, 0);
	drawMenuOption("Start Training Game", { 30, 60 }, selectedItem, 1);
	drawMenuOption("Quit", { 30, 70 }, selectedItem, 2);
}

void MainMenu::HandleMenuMove(const Vector2 axis) {
	if (axis.y > 0.1f) {
		selectedItem += 1;
		if (selectedItem < 3) {
			AudioManager::GetInstance()->StartPlayingSound(Assets::AUDIODIR + "menu_move.ogg");
		}
	}
	if (axis.y < -0.1f) {
		selectedItem -= 1;
		if (selectedItem > -1) {
			AudioManager::GetInstance()->StartPlayingSound(Assets::AUDIODIR + "menu_move.ogg");
		}
	}
	selectedItem = std::clamp(selectedItem, 0, 2);
}

void MainMenu::HandleMenuPress() {
	pressed = true;
	AudioManager::GetInstance()->StartPlayingSound(Assets::AUDIODIR + "menu_select.ogg");
}

bool MainMenu::UpdateGame(float dt) {
	NCL::AudioManager::GetInstance()->Update();
	bool val = false;
	!pushMachine->Update(dt) ? val = false : val =  true;
	return val;
}