#include "Network.h"

Network::Network()
{
}

Network::~Network()
{
	tcpSocket.disconnect();
}

void Network::init()
{

	connectToServer();
}

void Network::communicateShootData(sf::Vector2f endLoc)
{


	sf::Packet shootpacket;
	shootpacket << RAY_VECTOR2F << endLoc.x << endLoc.y;
	sf::Socket::Status shootStatus = tcpSocket.send(shootpacket);
	if ( shootStatus != sf::Socket::Done) {
		std::cout << "failed to send data \n";
	}

	std::cout << "shoot: " << endLoc.x << ", " << endLoc.y << " - " << shootStatus << "\n";

}

sf::Vector2f Network::checkForLocationData()
{

	return sf::Vector2f(0.0f, 0.0f);
}

void Network::findServer() {
	sf::UdpSocket findServerSocket;
	findServerSocket.bind(55000);

	sf::Packet finderPacket;
	unsigned short serverPort;

	findServerSocket.receive(finderPacket, serverIp, serverPort);

}

void Network::connectToServer()
{

	findServer();

	sf::Socket::Status statusTcp = tcpSocket.connect(serverIp, tcpListenerPort);

	if (statusTcp != sf::Socket::Done) {
		//std::cout << "Failed to bind connect " << statusTcp << "\n";
	}
	else if (statusTcp == sf::Socket::Done) {
		std::cout << "socket connected\n";
	}

	//connect udp for location data
	tcpSocket.setBlocking(true);
	sf::Packet packet;
	sf::Socket::Status statusUdp = tcpSocket.receive(packet);
	if (statusUdp != sf::Socket::Done) {
		std::cout << "cant connect udpSocet " << statusUdp << "\n";
	}
	else {

		packet >> udpPort >> serverUdpPort;
		udpSocket.unbind();
		sf::Socket::Status udpStatus = udpSocket.bind(udpPort);
		std::cout << "udp bind stat: " << udpStatus << " udp port:" << udpPort << "\n";
		std::cout << "server port: " << serverUdpPort << "\n";

		udpSocket.setBlocking(false);
		tcpSocket.setBlocking(false);
	}

}

void Network::communicateLoactionData(Player& player)
{

	//if its the same as last tick dont send
	if (!player.getSend()) {
		return;
	}

	sf::Packet packet;

	packet << player.getLocation().x << player.getLocation().y;
	if (udpSocket.send(packet, serverIp, serverUdpPort) != sf::Socket::Done) {
		std::cout << "failed to send packet\n";
		return;
	}

	player.setSend(false);

}

EnemyDataRecieve Network::recieveEnemyData()
{
	EnemyDataRecieve enemyData;

	sf::Packet tcpPacket;
	sf::Socket::Status tcpStatus = tcpSocket.receive(tcpPacket);

	PacketType packetType;
	if (tcpStatus == sf::Socket::Done) {
		int tempPacketType;
		tcpPacket >> tempPacketType;
		packetType = static_cast<PacketType>(tempPacketType);
		//in its only the id its a new enemy
		if (packetType == NEW_ENEMY) {
			tcpPacket >> enemyData.uniqueId;
			enemyData.newEnemy = true;
			std::cout << "new enemy: " << enemyData.uniqueId << "\n";
		}
		//if its a ray, meaning shoot, includes identity
		else if (packetType == RAY_VECTOR2F) {
			tcpPacket >> enemyData.rayEnd.x >> enemyData.rayEnd.y >> enemyData.uniqueId;
			enemyData.transparency = 255;
			enemyData.noShootData = false;
			std::cout << "recieved shoot data\n";
		}
		else if (packetType == ENEMY_DISSCONECTED) {
			tcpPacket >> enemyData.uniqueId;
			enemyData.active = false;
			std::cout << "enemy dissconected " << enemyData.uniqueId;
		}
		else if (packetType == HIT) {
			//meaning player hit
			enemyData.hit = true;
		}
		else {
			std::cout << "enemy data read error";
		}
	}
	else if (tcpStatus == sf::Socket::NotReady) {
		enemyData.noShootData = true;
	}
	else{
		std::cout << "tcp recieve error - " << tcpStatus << "\n";
	}

	sf::Packet udpPacket;
	sf::IpAddress ip;
	unsigned short port;
	sf::Socket::Status udpStatus = udpSocket.receive(udpPacket, ip, port);
	if (udpStatus == sf::Socket::Done) {
		udpPacket >> enemyData.location.x >> enemyData.location.y >> enemyData.uniqueId;
		enemyData.noMoveData = false;
		//std::cout << "enemy location recieved: " << enemyData.uniqueId << " - " << enemyData.location.x << ", " << enemyData.location.y << "\n";
	}
	else if (udpStatus == sf::Socket::NotReady) {
		//std::cout << "not ready status - ";
		enemyData.noMoveData = true;
	}
	else {
		std::cout << "udp recieve error - " << udpStatus << "\n";
	}

	return enemyData;
}
