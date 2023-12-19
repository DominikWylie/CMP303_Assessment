#pragma once

#include <SFML/Network.hpp>
#include <SFML/Graphics.hpp>
#include <iostream>
#include <chrono>
#include <thread>
#include <map>

//twin in client network.h
enum PacketType {
	RAY_VECTOR2F,
	NEW_ENEMY,
	ENEMY_DISSCONECTED,
	HIT
};

struct Client {
	sf::TcpSocket tcpSocket;
	unsigned short tcpPort;
	sf::IpAddress ip;
	//sf::UdpSocket udpSocket;
	//udp port to send client udp data
	unsigned short udpPort;

	sf::Vector2f position;
	sf::Vector2f previousPos = sf::Vector2f(0, 0);
	int radius = 25; //i should make that more dynamic when i have time. i intend to keep all circles 25 anyway

	//a simpler way of tracking clients that have been dissconected than moving data 
	bool active = false, newData = true, hit = false;
};

class Network
{
private:
	sf::TcpListener listener;

	static const int maxClients = 4;
	Client client[maxClients];

	int clientsConnected = 0;

	unsigned short startingUdpPort = 52000;
	int newClientCheckIntervalMilliseconds = 500, recieveSendDataMilliseconds = 100;

	sf::UdpSocket serverUdpSocket;
	unsigned short serverUdpPort = 50000;
	int udpPort = 54000;

	short listenerPort = 2000;
	short clientPort;

	int newClient = 0;

	sf::Clock NewClientClock, recieveSendDataClock;

public:
	Network();
	~Network();

	void init();
	void run();

private:
	void communicateLoactionData();
	void communicateShootData();
	void checkForNewClients();
	void listenForNewClients();
	void broadcastServer();
	void checkForNextAvalableClient();
	void collisionDetection(int shooter, sf::Vector2f rayEnd);
	bool pointCircle(float px, float py, float cx, float cy, float r);
	void communicateHit(int clientNo);
};

