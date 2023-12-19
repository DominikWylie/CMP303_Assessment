#pragma once

#include "SFML/Graphics.hpp"
#include "Player.h"
#include "Network.h"
#include "Enemy.h"

class Game
{
public:
	Game();

	void run();

private:
	void init();
	void processIncomingData(EnemyDataRecieve enemyData, float gameTime);

	sf::RenderWindow* window;

	sf::Clock dtClock, sendDataClock, gameClock;
	sf::Time deltaTime;
	int sendDataMilliseconds = 100;
	float gameTime;

	float dt = 0;

	Player player;
	const static int maxEnemies = 3;
	Enemy enemy[maxEnemies];
	Network network;
};

