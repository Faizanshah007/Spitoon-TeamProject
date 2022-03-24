#include <stdlib.h>
unsigned int sceLibcHeapExtendedAlloc = 1;			/* Switch to dynamic allocation */
size_t       sceLibcHeapSize = 256 * 1024 * 1024;	/* Set up heap area upper limit as 256 MiB */

#include "../../Plugins/PlayStation4/PS4Window.h"
#include "../../Plugins/PlayStation4/Ps4AudioSystem.h"
#include "../../Plugins/PlayStation4/PS4Input.h"
#include "../../Common/Window.h"

#include "TutorialGame.h"

#include "../CSC8503Common/StateMachine.h"
#include "../CSC8503Common/StateTransition.h"
#include "../CSC8503Common/State.h"

#include "../CSC8503Common/NavigationGrid.h"

#include "TutorialGame.h"
#include "MainMenu.h"
#include "../CSC8503Common/BehaviourAction.h"
#include "../CSC8503Common/BehaviourSequence.h"
#include "../CSC8503Common/BehaviourSelector.h"

#include "../CSC8503Common/PushdownState.h"
#include "../CSC8503Common/PushdownMachine.h"
#include <iostream>

#include "NetworkedGame.h"
#include "LoadingScreen.h"

using namespace NCL;
using namespace CSC8503;
using namespace NCL;
using namespace NCL::PS4;

class WinGame : public PushdownState {
public:
	WinGame(TutorialGame* g) : g(g) {};

	PushdownResult OnUpdate(float dt, PushdownState** newState) override {
		g->UpdateGame(dt);

		//if (g->GetQuit()) {
		//	g->ResetGame();
		//	g->UpdateGame(dt);
		//	return PushdownResult::Pop;
		//}

		//if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::ESCAPE)) {
		//	return PushdownResult::Reset;
		//}

		return PushdownResult::NoChange;
	}

	void OnAwake() override {
		g->SetState(WIN);
	}

protected:
	TutorialGame* g;
};

class PauseGame : public PushdownState {
public:
	PauseGame(TutorialGame* g) : g(g) {};

	PushdownResult OnUpdate(float dt, PushdownState** newState) override {
		g->UpdateGame(dt);

		if (!g->GetPaused()) {
			return PushdownResult::Pop;
		}

		if (g->GetQuit()) {
			return PushdownResult::Reset;
		}
		return PushdownResult::NoChange;
	}

	void OnAwake() override {
		g->SetState(PAUSE);
	}

protected:
	TutorialGame* g;
};

class Game : public PushdownState {
public:
	Game(TutorialGame* g) : g(g) {};

	PushdownResult OnUpdate(float dt, PushdownState** newState) override {
		g->UpdateGame(dt);

		if (g->Win()) {
			*newState = new WinGame(g);
			return PushdownResult::Push;
		}
		

		if (g->GetPaused()) {
			*newState = new PauseGame(g);
			return PushdownResult::Push;
		}

		return PushdownResult::NoChange;
	}

	void OnAwake() override {
		g->SetState(PLAY);
	}

protected:
	TutorialGame* g;
};

class Menu : public PushdownState {
public:
	Menu(MainMenu* m, NetworkedGame* g, NetworkedGame* h) : m(m), tg(g), ng(h) {};

	PushdownResult OnUpdate(float dt, PushdownState** newState) override {
		m->UpdateGame(dt);

		if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::NUM1)) {
			*newState = new Game(tg);
			return PushdownResult::Push;
		}

		if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::NUM2)) {
			*newState = new Game(ng);
			return PushdownResult::Push;
		}

		if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::ESCAPE)) {
			return PushdownResult::Exit;
		}

		return PushdownResult::NoChange;
	}

protected:
	NetworkedGame* tg;
	NetworkedGame* ng;
	MainMenu* m;
};

class Loading : public PushdownState
{
public:
	Loading(LoadingScreen* l) : ls(l) {};
	PushdownResult OnUpdate(float dt, PushdownState** newState) override 
	{
		LoadingScreen::SetInstancesToLoad(3);
		LoadingScreen::SetCompletionState(false);
		LoadingScreen::UpdateGame(dt);

		m = new MainMenu();
		ng = new NetworkedGame();
		tg = new NetworkedGame();

		LoadingScreen::SetCompletionState(true);
		
		*newState = new Menu(m, tg, ng);
		return PushdownResult::Push;
	}

protected:
	LoadingScreen* ls;
	NetworkedGame* tg;
	NetworkedGame* ng;
	MainMenu* m;
};

/*

The main function should look pretty familar to you!
We make a window, and then go into a while loop that repeatedly
runs our 'game' until we press escape. Instead of making a 'renderer'
and updating it, we instead make a whole game, and repeatedly update that,
instead.

This time, we've added some extra functionality to the window class - we can
hide or show the

*/

int main() {
#ifdef _WIN64
	Window* w = Window::CreateGameWindow("CSC8503 Game technology!", 1280, 720);
#endif
#ifdef _ORBIS
	Window* w = (PS4Window*)Window::CreateGameWindow("PS4 Example Code", 1920, 1080);
	Ps4AudioSystem* audioSystem = new Ps4AudioSystem(8);
#endif

		
	if (!w->HasInitialised()) {
		return -1;
	}	
	srand(time(NULL));
	w->ShowOSPointer(false);
	w->LockMouseToWindow(true);

	float avgTimeWait = 3.0f;
	float curTimeWait = 3.0f;
	float totalTime = 0.0f;
	int totalFrames = 0;

	w->SetTitle("Loading");

	LoadingScreen* l = new LoadingScreen();
	PushdownMachine p = new Loading(l);	
	//NetworkedGame* h = new NetworkedGame();
		
	w->GetTimer()->GetTimeDeltaSeconds(); //Clear the timer so we don't get a large first dt!
	float smallestFrameRate = 144.0f;
	while (w->UpdateWindow()) {

		float dt = w->GetTimer()->GetTimeDeltaSeconds();
		if (dt > 0.1f) {	//Skipping large time delta
			continue;	//must have hit a breakpoint or something to have a 1 second frame time!
		}

		float frameRate = (1.0f / dt);
		if (frameRate < smallestFrameRate)
			smallestFrameRate = frameRate;

		w->SetTitle("Gametech frame time:" + std::to_string(1000.0f * dt) + " | Gametech frame rate:" + std::to_string(frameRate));

		curTimeWait -= dt;
		totalTime += dt;
		totalFrames++;

		//h->UpdateGame(dt);

		if (!p.Update(dt)) {
			return 0;
		}
	}
	Window::DestroyGameWindow();
}
