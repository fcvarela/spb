#include <iostream>
#include <Common.h>
#include <GameSceneManager.h>

// provision a singleton
static GameSceneManager *gGameSceneManager;
GameSceneManager *getGameSceneManager() {
	if (!gGameSceneManager)
		gGameSceneManager = new GameSceneManager();

	return gGameSceneManager;
}

GameSceneManager::GameSceneManager() {
	std::cerr << "GameSceneManager alloc" << std::endl;
}

// and the rest
bool GameSceneManager::init() {
	std::cerr << "GameSceneManager init" << std::endl;

	// read config and set initial camera position
	this->camera = new Camera();

	return true;
}

GameSceneManager::~GameSceneManager() {
	std::cerr << "GameSceneManager cleanup" << std::endl;
}

void GameSceneManager::step() {
	// reposition camera
	this->camera->setPerspective();

	// update the view frustum
	calculateFrustum(this->frustum);
}