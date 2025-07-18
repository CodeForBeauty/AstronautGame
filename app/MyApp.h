#pragma once

#include "App.h"
#include "Mesh.h"
#include "Text.h"

#include "Events.hpp"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "PhysixxSystem.h"
#include "PhysixxBody.h"

#include "AudioSystem.h"

#include <random>


using namespace glender;
using namespace physixx;
using namespace audioAL;
using namespace std;
using namespace lm;

// check for equality of 2 3D vectors
bool equal(const vec3& a, const vec3& b) {
	return a.x == b.x && a.y == b.y && a.z == b.z;
}

class MyApp : public App {
public:
	// Main audio listener
	AudioListener Listener = AudioListener();

	// Direct light, to have lighting
	shared_ptr<DirectLight> MainLight;

	// Physics engine instance
	PhysixxSystem Physics;

	// Player spawn data
	shared_ptr<ColliderBox> Player = nullptr;
	shared_ptr<ColliderBox> PlayerHead = nullptr;
	shared_ptr<ColliderBox> PlayerFeet = nullptr;
	float PlayerMoveSpeed = 8.0f;
	float PlayerJumpForce = 10.0f;
	float PlayerCameraSpeed = 4.0f;
	vec3 PlayerSize = { 1, 1.5f, 1 };
	vec3 PlayerStart = { 0, 20, 0 };
	bool PlayerCanJump = false;
	bool PlayerCanControl = true;
	vec3 UpVector = { 0, 1, 0 };

	MyApp() : App({ 1920, 1080 }) {
		// Making game full screen
		m_scene->SceneCamera.SetAutoResize(false);
		m_window->SetWindowMode(WindowedFullScreenMode);

		// Creating collider for player body
		Player = Physics.AddBody<ColliderBox>(100, PlayerSize, PlayerStart);
		Player->SetCanRotate(false);
		Player->SetBounciness(0);

		// Creating collider for player head
		PlayerHead = Physics.AddBody<ColliderBox>(1, vec3{ 0.1f, 0.1f, 0.1f });
		PlayerHead->SetSimulated(false);
		PlayerHead->SetColliding(false);
		PlayerHead->SetLocalPos({ 0, 1.8f, 0 });
		PlayerHead->SetParent(Player.get());

		PlayerFeet = Physics.AddBody<ColliderBox>(1, vec3{ 0.1f, 0.05f, 0.1f });
		PlayerFeet->SetSimulated(false);
		PlayerFeet->SetLocalPos({ 0, -1.65f, 0 });
		PlayerFeet->SetParent(Player.get());
	}

	void Start() override {
		// Adding player controls
		PlayerFeet->OnCollision.AddListener(CollisionOverlap, [this](CollisionData data) {
			PlayerCanJump = true;
			});

		m_window->OnKeyboard.AddListener(KeyW, [this](InputEventType type, InputEventModifiers mods) {
			MovePlayer(Player->GetForward());
			});
		m_window->OnKeyboard.AddListener(KeyS, [this](InputEventType type, InputEventModifiers mods) {
			MovePlayer(-Player->GetForward());
			});
		m_window->OnKeyboard.AddListener(KeyD, [this](InputEventType type, InputEventModifiers mods) {
			MovePlayer(Player->GetRight());
			});
		m_window->OnKeyboard.AddListener(KeyA, [this](InputEventType type, InputEventModifiers mods) {
			MovePlayer(-Player->GetRight());
			});
		m_window->OnKeyboard.AddListener(KeySpace, [this](InputEventType type, InputEventModifiers mods) {
			if (type == InputEventRelease) {
				JumpPlayer();
			}
			});

		m_window->OnMousePos.AddListener(MousePosEventChange, [this](vec2 offset) {
			RotateHead(offset);
			});

		PlayerHead->OnRotation.AddListener(RotationChangeEvent, [this](Quaternion rot) {
			m_scene->SceneCamera.SetRotation(rot);
			});
		PlayerHead->OnPosition.AddListener(PositionChangeEvent, [this](vec3 pos) {
			m_scene->SceneCamera.SetPosition(pos);
			});

		// Spawning main scene light
		MainLight = m_scene->SpawnLight<DirectLight>(vec4{ 1.0f, 1.0f, 1.0f, 1.0f }, 5.0f,
			vec3{ 0.0f, 10.0f, 0.0f }, vec3{ 25.0f, 0.0f, 0.0f }, int2{1024, 1024});

		// Add UI interaction callback
		m_window->SetCursorLock(CursorDisable);
		m_window->OnMouseButton.AddListener(MouseLeftButton, [this](InputEventType type, InputEventModifiers modifiers) { Interact(type, modifiers); });
		m_window->OnMousePos.AddListener(MousePosEventMove, [this](vec2 pos) {
			lastMousePos.x = (int)pos.x;
			lastMousePos.y = (int)(m_window->GetWindowSize().y - pos.y);
			});


		// UI materials data
		UIFont = m_scene->LoadFont("Resources/Fonts/BAUHS93.TTF", { 1024, 1024 });
		UITextMat = m_scene->CreateTextMaterial(vec4{ 1, 0.15f, 0.1f, 1 }, 0.8f, 0.4f, 0.2f);
		UIBtnMat = m_scene->CreateSolidMaterial(vec4{ 0.7f, 0.7f, 0.8f, 1 }, 0.5f, 0.7f, 0.05f);

		GenerateLevel();

		// Create pause menu
		PauseMenu = new Transform();
		PauseMenu->SetParent(&m_scene->SceneCamera);
		PauseMenu->SetPosition({ 0, 0, 1.5f });

		shared_ptr<TextMesh> mainText = m_scene->SpawnMesh<TextMesh>(UITextMat, UIFont, string("Game Paused!"), 4);
		mainText->EntityTransform.SetParent(PauseMenu);
		mainText->EntityTransform.SetPosition({ -0.6f, 0.2f, 0 });

		shared_ptr<TextMesh> quitText = m_scene->SpawnMesh<TextMesh>(UITextMat, UIFont, string("Quit."), 3);
		quitText->EntityTransform.SetParent(PauseMenu);
		quitText->EntityTransform.SetPosition({ -0.17f, 0, 0 });

		shared_ptr<SolidMesh> quitBtn = m_scene->SpawnMesh<SolidMesh>(UIBtnMat);
		quitBtn->EntityTransform.SetParent(PauseMenu);
		quitBtn->EntityTransform.SetPosition({ 0, 0.05f, -0.1f });
		quitBtn->EntityTransform.SetScale({ 0.4f, 0.2f, 1 });

		QuitGameButtons.insert(quitText);
		QuitGameButtons.insert(quitBtn);

		//  Add game pause callback
		m_window->OnKeyboard.AddListener(KeyEscape, [this](InputEventType type, InputEventModifiers) { 
			if (type == InputEventPress) TogglePause(); });

		// Add background music
		shared_ptr<AudioSource> source = Listener.AddAudio("Resources/Audio/CalmBackground.wav");
		source->SetLooping(true);
		source->Play();
	}

	// Platforms that start falling after player collision
	unordered_map<shared_ptr<ColliderBox>, float> PlatformsToRemove = {};
	unordered_set<shared_ptr<ColliderBox>> RemoveThisFrame = {};

	bool GamePaused = false;
	// Skinned meshes to be animated in the end cutscene
	vector<shared_ptr<SkinnedMesh>> AnimatedMeshes = {};
	bool ShouldAnimate = false;
	float AnimationTime = 0;

	void Tick() override {
		if (GamePaused) {
			return;
		}
		// Update Physics engine
		Physics.Update(Delta);

		// Count each collided platfrom's time, and enable simulation after it's time reaches zero.
		for (pair<const shared_ptr<ColliderBox>, float>& platform : PlatformsToRemove) {
			PlatformsToRemove[platform.first] -= Delta;
			if (PlatformsToRemove[platform.first] <= 0) {
				platform.first->SetSimulated(true);
				RemoveThisFrame.insert(platform.first);
				shared_ptr<AudioSource> source = Listener.AddAudio("Resources/Audio/Shaking.wav");
				source->Play();
			}
		}
		for (const shared_ptr<ColliderBox>& platform : RemoveThisFrame) {
			PlatformsToRemove.erase(platform);
		}
		RemoveThisFrame.clear();

		// Animate end cutscene
		if (ShouldAnimate) {
			for (shared_ptr<SkinnedMesh>& mesh : AnimatedMeshes) {
				mesh->SetTime(AnimationTime);
			}
			AnimationTime += Delta;
		}
	}

	// Player movement functions
	void MovePlayer(vec3 direction) {
		if (!PlayerCanControl || GamePaused) {
			return;
		}

		Player->Move(Delta * PlayerMoveSpeed * direction);
	}
	void JumpPlayer() {
		if (!PlayerCanControl || !PlayerCanJump || GamePaused) {
			return;
		}

		Player->SetVelocity(Player->GetUp() * PlayerJumpForce);
		PlayerCanJump = false;

		shared_ptr<AudioSource> source = Listener.AddAudio("Resources/Audio/JumpSound.wav");
		source->Play();
	}
	vec2 HeadRot = {};
	void RotateHead(vec2 offset) {
		if (GamePaused) {
			return;
		}

		HeadRot += PlayerCameraSpeed * (float)PIrad * Delta * offset;

		Player->SetRotation(Quaternion({ 0, 1, 0 }, HeadRot.x));
		PlayerHead->SetLocalRot(Quaternion({ 1, 0, 0 }, HeadRot.y));
	}

	// Function to easily spawn boxes in the scene, with appropriate colliders
	shared_ptr<ColliderBox> SpawnBox(vec3 position, Quaternion rotation, vec3 size, shared_ptr<Material> material, float mass = 1.0f, bool simulated = true) {
		shared_ptr<ColliderBox> box = Physics.AddBody<ColliderBox>(mass, size, position);
		box->SetRotation(rotation);
		box->SetSimulated(simulated);

		shared_ptr<SolidMesh> cube = m_scene->SpawnCube(material);
		box->OnPosition.AddListener(PositionChangeEvent, [box, cube](vec3 pos) { cube->EntityTransform.SetPosition(pos); });
		box->OnRotation.AddListener(RotationChangeEvent, [box, cube](Quaternion rot) { cube->EntityTransform.SetRotation(rot); });
		cube->EntityTransform.SetScale(size);
		cube->EntityTransform.SetPosition(position);
		cube->EntityTransform.SetRotation(rotation);

		return box;
	}

	// UI
	shared_ptr<FontMap> UIFont = nullptr;
	shared_ptr<Material> UITextMat = nullptr;
	shared_ptr<Material> UIBtnMat = nullptr;
	unordered_set<shared_ptr<Mesh>> QuitGameButtons = {};

	// UI interaction
	int2 lastMousePos = { 0, 0 };
	void Interact(InputEventType inputType, InputEventModifiers inputModifiers) {
		if (inputType != InputEventRelease) {
			return;
		}
		// Sample appropriate index of an object from GBuffer
		int* pixel = m_renderer->GetGBufferPixel<int>(lastMousePos.x, lastMousePos.y, GBufferIndex);
		if (pixel[0] <= 0) {
			return;
		}

		// Check if that object is quit game button
		shared_ptr<Mesh> interacted = m_scene->SceneMeshes[pixel[0] - 1];
		if (QuitGameButtons.contains(interacted)) {
			m_shouldClose = true;
		}
		// Needs to be freed
		free(pixel);
	}

	Transform* PauseMenu = nullptr;

	void TogglePause() {
		if (!PlayerCanControl) {
			return;
		}

		GamePaused = !GamePaused;

		// Move pause menu either to front of the camera or behind
		if (GamePaused) {
			PauseMenu->SetPosition({0, 0, -1.5f});
			m_window->SetCursorLock(CursorFree);
		}
		else {
			PauseMenu->SetPosition({ 0, 0, 1.5f });
			m_window->SetCursorLock(CursorDisable);
		}
	}

	void ShowWinScreen() {
		if (!PlayerCanControl || GamePaused) {
			return;
		}

		m_window->SetCursorLock(CursorFree);

		PlayerCanControl = false;

		// Create win screen
		shared_ptr<TextMesh> mainText = m_scene->SpawnMesh<TextMesh>(UITextMat, UIFont, string("You Won!"), 4);
		mainText->EntityTransform.SetParent(&m_scene->SceneCamera);
		mainText->EntityTransform.SetPosition({ -0.4f, 0.2f, -1.5f });

		shared_ptr<TextMesh> quitText = m_scene->SpawnMesh<TextMesh>(UITextMat, UIFont, string("Quit."), 3);
		quitText->EntityTransform.SetParent(&m_scene->SceneCamera);
		quitText->EntityTransform.SetPosition({ -0.17f, 0, -1.5f });

		shared_ptr<SolidMesh> quitBtn = m_scene->SpawnMesh<SolidMesh>(UIBtnMat);
		quitBtn->EntityTransform.SetParent(&m_scene->SceneCamera);
		quitBtn->EntityTransform.SetPosition({ 0, 0.05f, -1.6f });
		quitBtn->EntityTransform.SetScale({ 0.4f, 0.2f, 1 });

		QuitGameButtons.insert(quitText);
		QuitGameButtons.insert(quitBtn);

		// Show cutscene
		ShouldAnimate = true;

		// Play shuttle engine sound effect
		shared_ptr<AudioSource> source = Listener.AddAudio("Resources/Audio/EngineSound.wav");
		source->SetLooping(true);
		source->Play();
	}

	void ShowLoseScreen() {
		if (!PlayerCanControl || GamePaused) {
			return;
		}

		m_window->SetCursorLock(CursorFree);

		PlayerCanControl = false;

		// Create lose screen
		shared_ptr<TextMesh> mainText = m_scene->SpawnMesh<TextMesh>(UITextMat, UIFont, string("Game Over."), 4);
		mainText->EntityTransform.SetParent(&m_scene->SceneCamera);
		mainText->EntityTransform.SetPosition({ -0.5f, 0.2f, -1.5f });

		shared_ptr<TextMesh> quitText = m_scene->SpawnMesh<TextMesh>(UITextMat, UIFont, string("Quit."), 3);
		quitText->EntityTransform.SetParent(&m_scene->SceneCamera);
		quitText->EntityTransform.SetPosition({ -0.17f, 0, -1.5f });

		shared_ptr<SolidMesh> quitBtn = m_scene->SpawnMesh<SolidMesh>(UIBtnMat);
		quitBtn->EntityTransform.SetParent(&m_scene->SceneCamera);
		quitBtn->EntityTransform.SetPosition({ 0, 0.05f, -1.6f });
		quitBtn->EntityTransform.SetScale({ 0.4f, 0.2f, 1 });
		
		QuitGameButtons.insert(quitText);
		QuitGameButtons.insert(quitBtn);

		// Play death sound effect
		shared_ptr<AudioSource> source = Listener.AddAudio("Resources/Audio/Hurt.wav");
		source->Play();
	}

	// Level generation data
	float MinPlatformOffset = 10;
	float MaxPlatformOffset = 15;
	vec3 PlatformSize = { 5, 1, 5 };
	float PlatformLifeTime = 5.0f;
	float PlatformUpOffset = 3.0f;
	unordered_set<shared_ptr<ColliderBox>> Platforms = {};
	shared_ptr<PbrMaterial> PlatformMaterial = nullptr;
	shared_ptr<PbrMaterial> LavaMaterial = nullptr;
	shared_ptr<PbrMaterial> EndPlatformMat = nullptr;

	void GenerateLevel() {
		PlatformMaterial = m_scene->CreateMaterial<PbrMaterial>(vec3{}, 0.5f, 0.8f, 0.15f);
		shared_ptr<Texture> moonTex = m_scene->LoadTexture("Resources/Textures/MoonTexture.jpg");
		shared_ptr<Texture> moonNormal = m_scene->LoadTexture("Resources/Textures/MoonNormal.png");
		PlatformMaterial->SetTexture(TextureAlbedo, moonTex);
		PlatformMaterial->SetTexture(TextureNormal, moonNormal);

		random_device seeder;
		mt19937 engine(seeder());
		// random number between 15 - 25
		uniform_int_distribution<int> randomNum(25, 35);
		int platformsNum = randomNum(engine);
		vec3 directions[4] = {
			{ 1,  0,  0},
			{ 0,  0,  1},
			{-1,  0,  0},
			{ 0,  0, -1},
		};
		uniform_int_distribution<int> randomDir(0, 3);
		vec3 direction = { 0, 0, 0 };
		vec3 currentPosition = { 0, 0, 0 };
		uniform_real_distribution<float> randomOffset(MinPlatformOffset, MaxPlatformOffset);
		for (int i = 0; i < platformsNum; i++) {
			// Calculate position to spawn platform
			float offset = randomOffset(engine);
			currentPosition += direction * offset;

			// Spawn platform
			shared_ptr<ColliderBox> box = SpawnBox(currentPosition, {}, PlatformSize, PlatformMaterial, 10.0f, false);
			box->OnCollision.AddListener(CollisionTouch, [box, this](CollisionData data) {
				if (!PlatformsToRemove.contains(box)) {
					PlatformsToRemove[box] = PlatformLifeTime;
				}
				});

			Platforms.insert(box);

			// Move up with each platform
			currentPosition.y += PlatformUpOffset;

			// Get next direction for platform spawning
			int randomDirInd = randomDir(engine);
			vec3 invDir = -direction;
			// Make sure to not place platform back
			while (equal(directions[randomDirInd], invDir)) {
				randomDirInd = randomDir(engine);
			}
			direction = directions[randomDirInd];
		}

		// Spawn end platform
		EndPlatformMat = m_scene->CreateMaterial<PbrMaterial>(vec3{ 0.2f, 0.4f, 0.8f }, 0.5f, 0.6f, 0.3f);
		EndPlatformMat->SetTexture(TextureAlbedo, moonTex);
		EndPlatformMat->SetTexture(TextureNormal, moonNormal);

		vec3 endPlatformPos = currentPosition + (direction * randomOffset(engine));
		shared_ptr<ColliderBox> levelEnd = SpawnBox(endPlatformPos, {}, PlatformSize, EndPlatformMat, 10.0f, false);
		levelEnd->OnCollision.AddListener(CollisionOverlap, [this](CollisionData data) {
			if (data.Other == Player.get()) ShowWinScreen();
			});
		
		// Spawn shuttle from fbx file
		FileLoadData loadedData = m_scene->LoadFile("Resources/Models/Spaceship/Transport_Shuttle_fbx.fbx");
		AnimatedMeshes = loadedData.SkinnedMeshes;
		for (shared_ptr<SolidMesh>& mesh : loadedData.SolidMeshes) {
			m_scene->RemoveMesh(mesh);
		}
		vec3 shuttlePos = endPlatformPos + direction * 10;

		Quaternion shuttleRot = Quaternion({ 0, 1, 0 }, acosf(dot({ 0, 0, -1 }, direction)));
		for (shared_ptr<SkinnedMesh>& mesh : loadedData.SkinnedMeshes) {
			mesh->EntityTransform.SetPosition(shuttlePos);
			mesh->EntityTransform.SetRotation(shuttleRot);
			mesh->PlayAnimation(1);
			mesh->SetTime(0);
		}

		// Spawn out of bounds death box
		LavaMaterial = m_scene->CreateMaterial<PbrMaterial>(vec3{ 0.8f, 0.2f, 0.1f }, 0.2f, 0.2f, 0.3f);
		shared_ptr<Texture> lavaTex = m_scene->LoadTexture("Resources/Textures/Lava.jpg");
		shared_ptr<Texture> lavaNormal = m_scene->LoadTexture("Resources/Textures/LavaNormal.png");
		LavaMaterial->SetTexture(TextureAlbedo, lavaTex);
		LavaMaterial->SetTexture(TextureNormal, lavaNormal);

		shared_ptr<ColliderBox> outOfLevel = SpawnBox({ 0, -40, 0 }, {}, { 1000, 5, 1000 }, LavaMaterial, 10.0f, false);
		outOfLevel->OnCollision.AddListener(CollisionOverlap, [this](CollisionData data) {
			if (data.Other == Player.get()) ShowLoseScreen();
			else {
				Physics.RemoveBody(data.Other);
			}
			});
	}
};