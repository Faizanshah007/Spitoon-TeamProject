#ifdef _ORBIS
#include "PS4GameRenderer.h"
#include "../../Plugins/PlayStation4/PS4Mesh.h"
#include "../../Plugins/PlayStation4/PS4Shader.h"
#include "../../Plugins/PlayStation4/PS4Texture.h"
#endif

#include "TutorialGame.h"
#include "../CSC8503Common/GameWorld.h"
#ifdef _WIN64
#include "../../Plugins/OpenGLRendering/OGLMesh.h"
#include "../../Plugins/OpenGLRendering/OGLShader.h"
#include "../../Plugins/OpenGLRendering/OGLTexture.h"
#include "OGLGameRenderer.h"
#endif
#include "../../Common/TextureLoader.h"
#include "../../Common/Quaternion.h"
#include "../../Plugins/PlayStation4/InputBase.h"

TutorialGame::TutorialGame()	{
	world		= new GameWorld();
	renderer = new Renderer(*world);
	physics		= new PhysicsSystem(*world);
	input = NULL;

	forceMagnitude	= 30.0f;
	useGravity		= false;
	inSelectionMode = false;

	testStateObject = nullptr;

	state = PLAY;

	Debug::SetRendererAPI(renderer->GetRendererAPI());

	//physics->SetGravity(Vector3(0, 9.8f, 0));
	//physics->SetLinearDamping(10.0f);
	InitialiseAssets();
}

TutorialGame::TutorialGame(NCL::PS4::InputBase* input) : TutorialGame() {
	this->input = input;
}
/*

Each of the little demo scenarios used in the game uses the same 2 meshes, 
and the same texture and shader. There's no need to ever load in anything else
for this module, even in the coursework, but you can add it if you like!

*/
void TutorialGame::InitialiseAssets() {
	auto loadFunc = [](const string& name, MeshGeometry** into) {
#ifdef _ORBIS
		*into = new PS4::PS4Mesh(name);
#else
		*into = new OGLMesh(name);
#endif
		(*into)->SetPrimitiveType(GeometryPrimitive::Triangles);
		(*into)->UploadToGPU();
	};
	
	loadFunc("cube.msh"		 , &cubeMesh);
	loadFunc("sphere.msh"	 , &sphereMesh);
	loadFunc("Male1.msh"	 , &charMeshA);
	loadFunc("courier.msh"	 , &charMeshB);
	loadFunc("security.msh"	 , &enemyMesh);
	loadFunc("coin.msh"		 , &bonusMesh);
	loadFunc("capsule.msh"	 , &capsuleMesh);
	
#ifdef _WIN64
	basicTex	= (OGLTexture*)TextureLoader::LoadAPITexture("checkerboard.png");
	basicShader = new OGLShader("GameTechVert.glsl", "GameTechFrag.glsl");
	playerTex = (OGLTexture*)TextureLoader::LoadAPITexture("me.png");
#endif
#ifdef _ORBIS
	basicTex = (PS4::PS4Texture*)TextureLoader::LoadAPITexture("checkerboard.png");
	basicShader = PS4::PS4Shader::GenerateShader("/app0/Assets/Shaders/PS4/VertexShader.sb","/app0/Assets/Shaders/PS4/PixelShader.sb");
	playerTex = (PS4::PS4Texture*)TextureLoader::LoadAPITexture("me.png");
#endif
	InitCamera();
	InitWorld();
}

TutorialGame::~TutorialGame()	{
	delete cubeMesh;
	delete sphereMesh;
	delete charMeshA;
	delete charMeshB;
	delete enemyMesh;
	delete bonusMesh;

	delete basicTex;
	delete basicShader;

	delete physics;
	delete renderer;
	delete world;
}

void TutorialGame::UpdateGame(float dt) {
	Debug::SetRendererAPI(renderer->GetRendererAPI());
	switch (state) {
	case PLAY: UpdateGameWorld(dt); break;
	case PAUSE: UpdatePauseScreen(dt); break;
	case WIN: UpdateWinScreen(dt); break;
	case RESET: {
		InitCamera();
		InitWorld();
		selectionObject = nullptr;
		break;
	}
	}

	//Debug::DrawLine(Vector3(), Vector3(0, 20, 0), Debug::RED);
	//Debug::DrawLine(Vector3(), Vector3(360, 0, 0), Debug::RED);
	//Debug::DrawLine(Vector3(360, 0, 0), Vector3(360, 0, 360), Debug::RED);
	//Debug::DrawLine(Vector3(360, 0, 360), Vector3(0, 0, 360), Debug::RED);
	//Debug::DrawLine(Vector3(0, 0, 360), Vector3(0, 0, 0), Debug::RED);

	renderer->Update(dt);

	Debug::FlushRenderables(dt);
	renderer->Render();
}

void TutorialGame::UpdateGameWorld(float dt)
{

	if (!inSelectionMode) {
		world->GetMainCamera()->UpdateCamera(dt);
	}

	UpdateKeys();

#ifdef _ORBIS
	float frameSpeed = 10 * dt;
	Camera* cam = world->GetMainCamera();
	float deadzone = 0.2f;
	if (input->GetAxis(1).y < -deadzone || input->GetAxis(1).y > deadzone) {
		cam->SetPitch(cam->GetPitch() - input->GetAxis(1).y);
	}
	if (input->GetAxis(1).x < -deadzone || input->GetAxis(1).x > deadzone) {
		cam->SetYaw(cam->GetYaw() - input->GetAxis(1).x);
	}

	// Movement
	if (input->GetAxis(0).y < -deadzone) {
		cam->SetPosition(cam->GetPosition() + Matrix4::Rotation(cam->GetYaw(), Vector3(0, 1, 0)) * Vector3(0, 0, -1) * frameSpeed);
	}
	if (input->GetAxis(0).y > deadzone) {
		cam->SetPosition(cam->GetPosition() - Matrix4::Rotation(cam->GetYaw(), Vector3(0, 1, 0)) * Vector3(0, 0, -1) * frameSpeed);
	}
	if (input->GetAxis(0).x < -deadzone) {
		cam->SetPosition(cam->GetPosition() + Matrix4::Rotation(cam->GetYaw(), Vector3(0, 1, 0)) * Vector3(-1, 0, 0) * frameSpeed);
	}
	if (input->GetAxis(0).x > deadzone) {
		cam->SetPosition(cam->GetPosition() - Matrix4::Rotation(cam->GetYaw(), Vector3(0, 1, 0)) * Vector3(-1, 0, 0) * frameSpeed);
	}
#endif

	if (useGravity) {
		Debug::Print("(G)ravity on", Vector2(5, 95));
	}
	else {
		Debug::Print("(G)ravity off", Vector2(5, 95));
	}

	//SelectObject();
	//MoveSelectedObject(dt);
	physics->Update(dt);

	if (lockedObject != nullptr) {
		Vector3 objPos = lockedObject->GetTransform().GetPosition();
		Vector3 camPos = objPos + lockedOffset;

		Matrix4 temp = Matrix4::BuildViewMatrix(camPos, objPos, Vector3(0, 1, 0));

		Matrix4 modelMat = temp.Inverse();

		Quaternion q(modelMat);
		Vector3 angles = q.ToEuler(); //nearly there now!

		world->GetMainCamera()->SetPosition(camPos);
		world->GetMainCamera()->SetPitch(angles.x);
		world->GetMainCamera()->SetYaw(angles.y);

		//Debug::DrawAxisLines(lockedObject->GetTransform().GetMatrix(), 2.0f);
	}

	world->UpdateWorld(dt);
}

void TutorialGame::UpdatePauseScreen(float dt)
{
	renderer->DrawString("PAUSED", Vector2(5, 80), Debug::MAGENTA, 30.0f);
	renderer->DrawString("Press P to Unpause.", Vector2(5, 90), Debug::WHITE, 20.0f);
	renderer->DrawString("Press Esc to return to Main Menu.", Vector2(5, 95), Debug::WHITE, 20.0f);
}

void TutorialGame::UpdateWinScreen(float dt)
{
	renderer->DrawString("YOU WIN", Vector2(5, 80), Debug::MAGENTA, 30.0f);
	renderer->DrawString("Press R to Restart.", Vector2(5, 90), Debug::WHITE, 20.0f);
	renderer->DrawString("Press Esc to return to Main Menu.", Vector2(5, 95), Debug::WHITE, 20.0f);
}

void TutorialGame::UpdateKeys() {
	/*if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F1)) {
		InitWorld(); //We can reset the simulation at any time with F1
		selectionObject = nullptr;
		lockedObject	= nullptr;
	}

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F2)) {
		InitCamera(); //F2 will reset the camera to a specific default place
	}

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::G)) {
		useGravity = !useGravity; //Toggle gravity!
		physics->UseGravity(useGravity);
	}

	//Running certain physics updates in a consistent order might cause some
	//bias in the calculations - the same objects might keep 'winning' the constraint
	//allowing the other one to stretch too much etc. Shuffling the order so that it
	//is random every frame can help reduce such bias.
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F9)) {
		world->ShuffleConstraints(true);
	}
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F10)) {
		world->ShuffleConstraints(false);
	}

	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F7)) {
		world->ShuffleObjects(true);
	}
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::F8)) {
		world->ShuffleObjects(false);
	}*/

	if (lockedObject) {
		LockedObjectMovement();
	}
	else {
		DebugObjectMovement();
	}
}

void TutorialGame::LockedObjectMovement() {
	Matrix4 view		= world->GetMainCamera()->BuildViewMatrix();
	Matrix4 camWorld	= view.Inverse();

	Vector3 rightAxis = Vector3(camWorld.GetColumn(0)); //view is inverse of model!

	//forward is more tricky -  camera forward is 'into' the screen...
	//so we can take a guess, and use the cross of straight up, and
	//the right axis, to hopefully get a vector that's good enough!

	Vector3 fwdAxis = Vector3::Cross(Vector3(0, 1, 0), rightAxis);
	fwdAxis.y = 0.0f;
	fwdAxis.Normalise();

	Vector3 charForward  = lockedObject->GetTransform().GetOrientation() * Vector3(0, 0, 1);
	Vector3 charForward2 = lockedObject->GetTransform().GetOrientation() * Vector3(0, 0, 1);

	float force = 100.0f;

	//if (Window::GetKeyboard()->KeyDown(NCL::KeyboardKeys::UP)) {
	//	selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 0, -1) * forceMagnitude * 0.1);
	//}
	//if (Window::GetKeyboard()->KeyDown(NCL::KeyboardKeys::DOWN)) {
	//	selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 0, 1) * forceMagnitude * 0.1);
	//}
	//if (Window::GetKeyboard()->KeyDown(NCL::KeyboardKeys::LEFT)) {
	//	selectionObject->GetPhysicsObject()->AddForce(Vector3(-1, 0, 0) * forceMagnitude * 0.1);
	//}
	//if (Window::GetKeyboard()->KeyDown(NCL::KeyboardKeys::RIGHT)) {
	//	selectionObject->GetPhysicsObject()->AddForce(Vector3(1, 0, 0) * forceMagnitude * 0.1);
	//}
	//if (Window::GetKeyboard()->KeyDown(NCL::KeyboardKeys::SHIFT)) {
	//	selectionObject->GetPhysicsObject()->AddForce(Vector3(0, -1, 0) * forceMagnitude * 0.1);
	//}
	//if (Window::GetKeyboard()->KeyDown(NCL::KeyboardKeys::SPACE)) {
	//	selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 1, 0) * forceMagnitude * 0.1);
	//}
}

void TutorialGame::DebugObjectMovement() {
//If we've selected an object, we can manipulate it with some key presses
	if (inSelectionMode && selectionObject) {
		//Twist the selected object!
		//if (Window::GetKeyboard()->KeyDown(KeyboardKeys::LEFT)) {
		//	selectionObject->GetPhysicsObject()->AddTorque(Vector3(-10, 0, 0));
		//}

		//if (Window::GetKeyboard()->KeyDown(KeyboardKeys::RIGHT)) {
		//	selectionObject->GetPhysicsObject()->AddTorque(Vector3(10, 0, 0));
		//}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NUM7)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(0, 10, 0));
		}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NUM8)) {
			selectionObject->GetPhysicsObject()->AddTorque(Vector3(0, -10, 0));
		}

		//if (Window::GetKeyboard()->KeyDown(KeyboardKeys::RIGHT)) {
		//	selectionObject->GetPhysicsObject()->AddTorque(Vector3(10, 0, 0));
		//}

		//if (Window::GetKeyboard()->KeyDown(KeyboardKeys::UP)) {
		//	selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 0, -10));
		//}

		//if (Window::GetKeyboard()->KeyDown(KeyboardKeys::DOWN)) {
		//	selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 0, 10));
		//}

		if (Window::GetKeyboard()->KeyDown(KeyboardKeys::NUM5)) {
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, -10, 0));
		}
	}

}

void TutorialGame::InitCamera() {
	world->GetMainCamera()->SetNearPlane(0.1f);
	world->GetMainCamera()->SetFarPlane(500.0f);
	world->GetMainCamera()->SetPitch(-15.0f);
	world->GetMainCamera()->SetYaw(315.0f);
	world->GetMainCamera()->SetPosition(Vector3(-60, 40, 60));
	lockedObject = nullptr;
}

void TutorialGame::InitWorld() {
	world->ClearAndErase();
	physics->Clear();

	GameObject* a = AddCubeToWorld(Vector3(15, 2, 15), Vector3(1, 1, 1), true, 10.0f, 1, false, true);
	GameObject* b = AddCubeToWorld(Vector3(10, 2, 15), Vector3(1, 1, 1), true, 10.0f, 1, false, true);
	//a->GetRenderObject()->SetColour(Debug::CYAN);
	//b->GetRenderObject()->SetColour(Debug::CYAN);

	GameObject* c = AddCubeToWorld(Vector3(10, 2, 10), Vector3(1, 1, 1), false, 10.0f, 1, false, true);
	GameObject* d = AddCubeToWorld(Vector3(15, 2, 10), Vector3(1, 1, 1), false, 10.0f, 1, false, true);
	c->GetPhysicsObject()->SetFriction(false);
	d->GetPhysicsObject()->SetFriction(false);

	// Floor
	GameObject* e = AddCubeToWorld(Vector3(0, -2, 0), Vector3(250, 2, 250), false, 0, 0);

	GameObject* cap1 = AddCapsuleToWorld(Vector3(15, 5, 0), 3.0f, 1.5f);
	GameObject* cap2 = AddCapsuleToWorld(Vector3(10, 5, 0), 3.0f, 1.5f);
	cap1->SetDynamic(true);
	cap2->SetDynamic(true);

	GameObject* sphere1 = AddSphereToWorld(Vector3(10, 5, 20), 1.0f, 10.0f, false, false, true);
	GameObject* sphere2 = AddSphereToWorld(Vector3(15, 5, 20), 1.0f, 10.0f, false, false, true);

	a->SetCollisionLayers(CollisionLayer::LAYER_ONE);
	b->SetCollisionLayers(CollisionLayer::LAYER_ONE);
	c->SetCollisionLayers(CollisionLayer::LAYER_ONE);
	d->SetCollisionLayers(CollisionLayer::LAYER_ONE);
	e->SetCollisionLayers(CollisionLayer::LAYER_ONE);
	cap1->SetCollisionLayers(CollisionLayer::LAYER_ONE | CollisionLayer::LAYER_TWO);
	cap2->SetCollisionLayers(CollisionLayer::LAYER_ONE | CollisionLayer::LAYER_TWO);
	sphere1->SetCollisionLayers(CollisionLayer::LAYER_ONE);
	sphere2->SetCollisionLayers(CollisionLayer::LAYER_ONE);

	physics->BuildStaticList();
}

/*

A single function to add a large immoveable cube to the bottom of our world

*/
GameObject* TutorialGame::AddFloorToWorld(const Maths::Vector3& position) {
	GameObject* floor = new GameObject("Floor");

	Vector3 floorSize	= Vector3(20, 2, 100);
	AABBVolume* volume	= new AABBVolume(floorSize);
	floor->SetBoundingVolume((CollisionVolume*)volume);
	floor->GetTransform()
		.SetScale(floorSize * 2)
		.SetPosition(position);
		//.SetOrientation(Quaternion(0.25f, 0, 0, 1));

	floor->SetRenderObject(new RenderObject(&floor->GetTransform(), cubeMesh, basicTex, basicShader));
	floor->SetPhysicsObject(new PhysicsObject(&floor->GetTransform(), floor->GetBoundingVolume()));

	floor->GetPhysicsObject()->SetInverseMass(0);
	floor->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(floor);
	floor->SetCollisionLayers(CollisionLayer::LAYER_ONE);
	return floor;
}

/*

Builds a game object that uses a sphere mesh for its graphics, and a bounding sphere for its
rigid body representation. This and the cube function will let you build a lot of 'simple' 
physics worlds. You'll probably need another function for the creation of OBB cubes too.

*/
GameObject* TutorialGame::AddSphereToWorld(const Maths::Vector3& position, float radius, float inverseMass, bool rubber, bool hollow, bool dynamic) {
	GameObject* sphere = new GameObject("Sphere");
	
	// Raycasting Tutorial 1 - Further Work Part 1
	//sphere->SetLayer(1);

	Vector3 sphereSize = Vector3(radius, radius, radius);
	SphereVolume* volume = new SphereVolume(radius);
	sphere->SetBoundingVolume((CollisionVolume*)volume);

	sphere->GetTransform()
		.SetScale(sphereSize)
		.SetPosition(position);

	sphere->SetRenderObject(new RenderObject(&sphere->GetTransform(), sphereMesh, basicTex, basicShader));
	sphere->SetPhysicsObject(new PhysicsObject(&sphere->GetTransform(), sphere->GetBoundingVolume()));

	sphere->GetPhysicsObject()->SetInverseMass(inverseMass);
	if (hollow)
		sphere->GetPhysicsObject()->InitHollowSphereInertia();
	else
		sphere->GetPhysicsObject()->InitSphereInertia();

	if (rubber)
		sphere->GetPhysicsObject()->SetElasticity(0.9f);
	else
		sphere->GetPhysicsObject()->SetElasticity(0.2f);

	world->AddGameObject(sphere);
	sphere->SetDynamic(dynamic);
	return sphere;
}

GameObject* TutorialGame::AddCapsuleToWorld(const Maths::Vector3& position, float halfHeight, float radius, float inverseMass) {
	GameObject* capsule = new GameObject("Capsule");

	CapsuleVolume* volume = new CapsuleVolume(halfHeight, radius);
	capsule->SetBoundingVolume((CollisionVolume*)volume);

	capsule->GetTransform()
		.SetScale(Vector3(radius* 2, halfHeight, radius * 2))
		.SetPosition(position);

	capsule->SetRenderObject(new RenderObject(&capsule->GetTransform(), capsuleMesh, basicTex, basicShader));
	capsule->SetPhysicsObject(new PhysicsObject(&capsule->GetTransform(), capsule->GetBoundingVolume()));

	capsule->GetPhysicsObject()->SetInverseMass(inverseMass);
	capsule->GetPhysicsObject()->InitCubeInertia();

	world->AddGameObject(capsule);

	return capsule;
}

GameObject* TutorialGame::AddCubeToWorld(const Maths::Vector3& position, Maths::Vector3 dimensions, bool OBB, float inverseMass, int layer, bool isTrigger, bool dynamic) {
	GameObject* cube = new GameObject();
	if (OBB) {
		OBBVolume* volume = new OBBVolume(dimensions);
		cube->SetBoundingVolume((CollisionVolume*)volume);
	}
	else {
		AABBVolume* volume = new AABBVolume(dimensions);
		cube->SetBoundingVolume((CollisionVolume*)volume);
	}

	cube->GetTransform()
		.SetPosition(position)
		.SetScale(dimensions * 2);


	cube->SetRenderObject(new RenderObject(&cube->GetTransform(), cubeMesh, basicTex, basicShader));
	cube->SetPhysicsObject(new PhysicsObject(&cube->GetTransform(), cube->GetBoundingVolume()));

	cube->GetPhysicsObject()->SetInverseMass(inverseMass);
	cube->GetPhysicsObject()->InitCubeInertia();
	cube->GetPhysicsObject()->SetElasticity(0.2f);

	world->AddGameObject(cube);
	switch (layer)
	{
	default:
		cube->SetCollisionLayers(CollisionLayer::LAYER_ONE);
		break;
	case 1:
		cube->SetCollisionLayers(CollisionLayer::LAYER_ONE);
		break;
	case 2:
		cube->SetCollisionLayers(CollisionLayer::LAYER_TWO);
		break;
	case 3:
		cube->SetCollisionLayers(CollisionLayer::LAYER_THREE);
		break;
	}
	cube->SetTrigger(isTrigger);
	cube->SetDynamic(dynamic);
	return cube;
}

void TutorialGame::InitSphereGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, float radius) {
	for (int x = 0; x < numCols; ++x) {
		for (int z = 0; z < numRows; ++z) {
			Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);
			AddSphereToWorld(position, radius, 1.0f);
		}
	}
	AddFloorToWorld(Vector3(0, -2, 0));
}

void TutorialGame::InitMixedGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing) {
	float sphereRadius = 1.0f;
	Vector3 cubeDims = Vector3(1, 1, 1);

	for (int x = 0; x < numCols; ++x) {
		for (int z = 0; z < numRows; ++z) {
			Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);

			if (rand() % 2) {
				AddCubeToWorld(position, cubeDims, false, 10.0f, 1, false, true);
			}
			else {
				AddSphereToWorld(position, sphereRadius, 10.0f, rand() % 2, false, true);
			}
		}
	}
}

void TutorialGame::InitCubeGridWorld(int numRows, int numCols, float rowSpacing, float colSpacing, const Maths::Vector3& cubeDims) {
	for (int x = 1; x < numCols+1; ++x) {
		for (int z = 1; z < numRows+1; ++z) {
			Vector3 position = Vector3(x * colSpacing, 10.0f, z * rowSpacing);
			AddCubeToWorld(position, cubeDims, 1.0f);
		}
	}
}

void TutorialGame::InitDefaultFloor() {
	AddFloorToWorld(Vector3(0, -2, 0));
}

void TutorialGame::InitGameExamples() {
	AddPlayerToWorld(Vector3(0, 5, 0));
	AddCapsuleToWorld(Vector3(15, 5, 0), 3.0f, 1.5f, 1.0f);
}

GameObject* TutorialGame::AddPlayerToWorld(const Maths::Vector3& position) {
	float meshSize = 3.0f;
	float inverseMass = 0.5f;

	GameObject* character = new GameObject("Player");

	AABBVolume* volume = new AABBVolume(Vector3(0.3f, 0.85f, 0.3f) * meshSize);

	character->SetBoundingVolume((CollisionVolume*)volume);

	character->GetTransform()
		.SetScale(Vector3(meshSize, meshSize, meshSize))
		.SetPosition(position);

	if (rand() % 2) {
		character->SetRenderObject(new RenderObject(&character->GetTransform(), charMeshA, nullptr, basicShader));
	}
	else {
		character->SetRenderObject(new RenderObject(&character->GetTransform(), charMeshB, nullptr, basicShader));
	}
	character->SetPhysicsObject(new PhysicsObject(&character->GetTransform(), character->GetBoundingVolume()));

	character->GetPhysicsObject()->SetInverseMass(inverseMass);
	character->GetPhysicsObject()->InitSphereInertia();

	world->AddGameObject(character);

	//lockedObject = character;

	return character;
}


/*

Every frame, this code will let you perform a raycast, to see if there's an object
underneath the cursor, and if so 'select it' into a pointer, so that it can be 
manipulated later. Pressing Q will let you toggle between this behaviour and instead
letting you move the camera around. 

*/
bool TutorialGame::SelectObject() {
	if (Window::GetKeyboard()->KeyPressed(KeyboardKeys::Q)) {
		inSelectionMode = !inSelectionMode;
		if (inSelectionMode) {
			Window::GetWindow()->ShowOSPointer(true);
			Window::GetWindow()->LockMouseToWindow(false);
		}
		else {
			Window::GetWindow()->ShowOSPointer(false);
			Window::GetWindow()->LockMouseToWindow(true);
		}
	}
	if (inSelectionMode) {
		renderer->DrawString("Press Q to change to camera mode!", Vector2(5, 85));

		if (Window::GetMouse()->ButtonDown(NCL::MouseButtons::LEFT)) {
			if (selectionObject) {	//set colour to deselected;
				selectionObject->GetRenderObject()->SetColour(Vector4(1, 1, 1, 1));
				//selectionObject->SetLayer(0);
				selectionObject = nullptr;
				lockedObject	= nullptr;
			}

			Ray ray = CollisionDetection::BuildRayFromMouse(*world->GetMainCamera());

			RayCollision closestCollision;
			if (world->Raycast(ray, closestCollision, true)) {
				selectionObject = (GameObject*)closestCollision.node;
				selectionObject->GetRenderObject()->SetColour(Vector4(0, 1, 0, 1));
				//selectionObject->SetLayer(1);
				//Ray r(selectionObject->GetTransform().GetPosition(), Vector3(0,0,-1));
				//RayCollision col;

				// Doesn't work for cubes for some reason idk

				//if (world->Raycast(r, col, true)) {
				//	((GameObject*)col.node)->GetRenderObject()->SetColour(Vector4(0, 0, 1, 1));
				//	Debug::DrawLine(r.GetPosition(), col.collidedAt, Vector4(1, 0, 0, 1), 5.0f);
				//}

				Debug::DrawLine(ray.GetPosition(), closestCollision.collidedAt, Vector4(1, 0, 0, 1), 5.0f);
				return true;
			}
			else {
				return false;
			}
		}
	}
	else {
		renderer->DrawString("Press Q to change to select mode!", Vector2(5, 85));
	}

	if (lockedObject) {
		renderer->DrawString("Press L to unlock object!", Vector2(5, 80));
	}

	else if(selectionObject){
		renderer->DrawString("Press L to lock selected object object!", Vector2(5, 80));
	}

	if (Window::GetKeyboard()->KeyPressed(NCL::KeyboardKeys::L)) {
		if (selectionObject) {
			if (lockedObject == selectionObject) {
				lockedObject = nullptr;
			}
			else {
				lockedObject = selectionObject;
			}
		}

	}

	return false;
}

/*
If an object has been clicked, it can be pushed with the right mouse button, by an amount
determined by the scroll wheel. In the first tutorial this won't do anything, as we haven't
added linear motion into our physics system. After the second tutorial, objects will move in a straight
line - after the third, they'll be able to twist under torque aswell.
*/
void TutorialGame::MoveSelectedObject(float dt) {
	renderer->DrawString("Click Force: " + std::to_string(forceMagnitude), Vector2(10, 20));
	forceMagnitude += Window::GetMouse()->GetWheelMovement() * 100.0f;

	if (!selectionObject)
		return;

	if (Window::GetMouse()->ButtonPressed(NCL::MouseButtons::RIGHT)) {
		Ray ray = CollisionDetection::BuildRayFromMouse(*world->GetMainCamera());
		RayCollision closestCollision;
		if (world->Raycast(ray, closestCollision, true)) {
			if (closestCollision.node == selectionObject) {
				selectionObject->GetPhysicsObject()->AddForceAtPosition(ray.GetDirection() * forceMagnitude, closestCollision.collidedAt);
			}
		}
	}

	Vector3 pos = selectionObject->GetTransform().GetPosition();
	string s = "Pos: " + std::to_string(pos.x) + ", " + std::to_string(pos.y) + ", " + std::to_string(pos.z) + ")";
	renderer->DrawString(s, Vector2(5, 5));

	if (Window::GetKeyboard()->KeyHeld(NCL::KeyboardKeys::F))
		selectionObject->Interact(dt);

	if (Window::GetKeyboard()->KeyDown(NCL::KeyboardKeys::UP)) {
		if (selectionObject->GetName() == "player")
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 0, -1) * ((Player*)selectionObject)->GetSpeed() * 0.1);
		else
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 0, -1) * forceMagnitude * 0.1);
	}
	if (Window::GetKeyboard()->KeyDown(NCL::KeyboardKeys::DOWN)) {
		if (selectionObject->GetName() == "player")
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 0, 1) * ((Player*)selectionObject)->GetSpeed() * 0.1);
		else
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 0, 1) * forceMagnitude * 0.1);
	}
	if (Window::GetKeyboard()->KeyDown(NCL::KeyboardKeys::LEFT)) {
		if (selectionObject->GetName() == "player")
			selectionObject->GetPhysicsObject()->AddForce(Vector3(-1, 0, 0) * ((Player*)selectionObject)->GetSpeed() * 0.1);
		else
			selectionObject->GetPhysicsObject()->AddForce(Vector3(-1, 0, 0) * forceMagnitude * 0.1);
	}
	if (Window::GetKeyboard()->KeyDown(NCL::KeyboardKeys::RIGHT)) {
		if (selectionObject->GetName() == "player")
			selectionObject->GetPhysicsObject()->AddForce(Vector3(1, 0, 0) * ((Player*)selectionObject)->GetSpeed() * 0.1);
		else
			selectionObject->GetPhysicsObject()->AddForce(Vector3(1, 0, 0) * forceMagnitude * 0.1);
	}
	if (Window::GetKeyboard()->KeyDown(NCL::KeyboardKeys::SHIFT)) {
		if (selectionObject->GetName() == "player")
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, -1, 0) * ((Player*)selectionObject)->GetSpeed() * 0.1);
		else
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, -1, 0) * forceMagnitude * 0.1);
	}
	if (Window::GetKeyboard()->KeyDown(NCL::KeyboardKeys::SPACE)) {
		if (selectionObject->GetName() == "player")
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 1, 0) * ((Player*)selectionObject)->GetSpeed() * 0.1);
		else
			selectionObject->GetPhysicsObject()->AddForce(Vector3(0, 1, 0) * forceMagnitude * 0.1);
	}
}