#pragma once

#include <SFML/Network.hpp>
#include <thread>
#include <chrono>
#include <iostream>

#include "Player.h"

//twin in server network.h
enum PacketType {
	RAY_VECTOR2F,
	NEW_ENEMY,
	ENEMY_DISSCONECTED,
	HIT
};

struct EnemyDataRecieve {
	sf::Vector2f location;
	sf::Vector2f rayEnd;
	unsigned short uniqueId;
	bool active = true;
	bool newEnemy = false;
	bool noMoveData = true, noShootData = true;
	bool hit = false;
	float transparency = 0;
};

class Network
{
private:
	sf::TcpSocket tcpSocket;
	unsigned short tcpListenerPort = 2000;

	sf::UdpSocket udpSocket;
	unsigned short udpPort;

	unsigned short serverUdpPort;

	sf::IpAddress serverIp;

	sf::Vector2f previousLocation = sf::Vector2f(0, 0);

	sf::UdpSocket testSocket;

	void connectToServer();
	void findServer();

public:
	Network();
	~Network();

	void init();
	void communicateShootData(sf::Vector2f endLoc);
	void communicateLoactionData(Player& player);
	sf::Vector2f checkForLocationData();

	EnemyDataRecieve recieveEnemyData();
	

};

