#include "Network.h"

Network::Network()
{
}

Network::~Network()
{
}

void Network::init()
{

	
	serverUdpSocket.bind(serverUdpPort);

	listener.setBlocking(false);
	serverUdpSocket.setBlocking(false);

	std::cout << "SFML Version: " << SFML_VERSION_MAJOR << "." << SFML_VERSION_MINOR << "." << SFML_VERSION_PATCH << std::endl;

	listenForNewClients();
}

void Network::checkForNewClients()
{

	//new client initialised to 0, finds next avalable at end of new client set up
	sf::Socket::Status listenerAcceptStatus;
	if (newClient < maxClients && newClient >= 0) {
		listenerAcceptStatus = listener.accept(client[newClient].tcpSocket);
	}
	else {
		//if its full just set it to nothing coming through, space is checked at end of function
		listenerAcceptStatus = sf::Socket::NotReady;

	}

	if (listenerAcceptStatus == sf::Socket::Done) {

		client[newClient].ip = client[newClient].tcpSocket.getRemoteAddress();
		//remote port is identifier for each clients data in
		client[newClient].tcpPort = client[newClient].tcpSocket.getRemotePort();
		client[newClient].active = true;
		client[newClient].newData = true;

		//set up udp have tcpsocket blocking on clientCheck side
		client[newClient].udpPort = startingUdpPort + newClient;

		std::cout << "accepted new client. slot " << newClient <<". tcp port/id: " << client[newClient].tcpPort << " udp: " << client[newClient].udpPort << "\n";

		//send clientCheck port
		sf::Packet packet;
		//udp port is only for sending from server/recievng at clientCheck
		packet << client[newClient].udpPort << serverUdpPort;

		sf::Socket::Status udpPortSendStatus = client[newClient].tcpSocket.send(packet);

		if (udpPortSendStatus == sf::Socket::Done) {
			client[newClient].tcpSocket.setBlocking(false);

			std::cout << "sent client port: " << client[newClient].udpPort << " and server port: " << serverUdpPort << "\n";
		}
		else {
			std::cout << "failed to send udp setup" << udpPortSendStatus << "\n";
		}

		//send new enemy data to all other clients 
		for (int i = 0; i < maxClients; i++) {
			//check if its active, if not loop to next
			if (client[i].active && i != newClient) {
				
				sf::Packet newEnemyPacket;
				PacketType newEnemy = NEW_ENEMY;
				//unique id is the tcp port
				newEnemyPacket << newEnemy << client[newClient].tcpPort;
				client[i].tcpSocket.send(newEnemyPacket);
				std::cout << "new client sent as enemy to " << i << "\n";

			}
		}

		//send new client all other enemy data

		for (int i = 0; i < maxClients; i++) {
			//if its not the new client and enemy is active
			if (i != newClient && client[i].active) {

				sf::Packet newEnemyPacket;
				PacketType newEnemy = NEW_ENEMY;
				//unique id is the tcp port
				newEnemyPacket << newEnemy << client[i].tcpPort;
				client[newClient].tcpSocket.send(newEnemyPacket);
				std::cout << "new client is sent enemy no." << i << "\n";

				sf::Packet packet;

				packet << client[i].position.x << client[i].position.y << client[i].tcpPort;

				sf::Socket::Status locationSendStatus = serverUdpSocket.send(packet, client[newClient].ip, client[newClient].udpPort);
				if (locationSendStatus != sf::Socket::Done) {
					std::cout << "enemy location send failed" << locationSendStatus << "\n";
				}
			}
		}

		checkForNextAvalableClient();

	}
	else if (listenerAcceptStatus == sf::Socket::NotReady)
	{
		//std::cout << "no clientCheck to connect\n";
	}
	else {
		std::cout << "failed to accept client " << listenerAcceptStatus << "\n";
	}
}

void Network::listenForNewClients()
{
	broadcastServer();

	sf::Socket::Status listenStatus = listener.listen(listenerPort);
	if (listenStatus != sf::Socket::Done) {
		std::cout << "Tcp listener failed: " << listenStatus << "\n";
	}
}

void Network::broadcastServer()
{
	sf::UdpSocket broadcastSocket;
	sf::IpAddress serverIP = sf::IpAddress::getLocalAddress();
	unsigned short serverPort = broadcastSocket.getLocalPort();

	sf::Packet broadcastPacket;
	broadcastPacket << serverIP.toString() << serverPort;

	broadcastSocket.send(broadcastPacket, sf::IpAddress::Broadcast, 55000);


}

void Network::checkForNextAvalableClient()
{
	//if it is active check next, if not return, thats the next avalable
	newClient = 0;

	while (client[newClient].active) {
		newClient++;

		if (newClient >= maxClients) {
			//-1 means no space
			newClient = -1;
			break;
		}
	}
}

void Network::communicateLoactionData() {


	sf::IpAddress senderAdressUDP;
	unsigned short senderPortUDP;
	sf::Packet recievedPacket;

	//recieve location data
	sf::Socket::Status stat = serverUdpSocket.receive(recievedPacket, senderAdressUDP, senderPortUDP);
	if (stat == sf::Socket::Done) {
		for (int i = 0; i <= maxClients; i++) {

			if (senderPortUDP == client[i].udpPort) {
				recievedPacket >> client[i].position.x >> client[i].position.y;
				client[i].newData = true;

			}
		}
	}
	else if (stat == sf::Socket::NotReady) {
		//do nothing
	}
	else if (stat == sf::Socket::Disconnected) {
		//udp dosnt dissconnect - deal with dissconection at tcp port (shoot)
	}
	else {
		std::cout << "error " << stat << "\n";
	}

	//sending
	for (int send = 0; send < maxClients; send++) {
		//send data to
		if (client[send].active) {
			//data to send
			for (int data = 0; data < maxClients; data++) {
				//send if client is active, client is not the data and its new data. 
				if (client[data].active 
					&& send != data 
					&& client[data].newData) {


					sf::Packet packet;

					packet << client[data].position.x << client[data].position.y << client[data].tcpPort;

					sf::Socket::Status locationSendStatus = serverUdpSocket.send(packet, client[send].ip, client[send].udpPort);
					if (locationSendStatus != sf::Socket::Done) {
						std::cout << "enemy location send failed" << locationSendStatus << "\n";
					}
				}
			}
		}
	}

	//one evreything is sent all data is old. this saves sending useless data
	for (int i = 0; i < maxClients; i++) {
		client[i].newData = false;
	}
}

void Network::communicateShootData()
{
	sf::Packet recievePacket;
	sf::Packet sendPacket;

	sf::Vector2f rayEnd;
	PacketType packetType;
	int tempPacketType;

	//loop through all tcp sockets to get new data
	for (int currentClient = 0; currentClient < maxClients; currentClient++) {
		if (client[currentClient].active) {
			//try and recieve dat from socket
			sf::Socket::Status recieveStatus = client[currentClient].tcpSocket.receive(recievePacket);
			if (recieveStatus == sf::Socket::Done) {
				std::cout << "ray Recieved \n";
				recievePacket >> tempPacketType;
				packetType = static_cast<PacketType>(tempPacketType);
				if (packetType == RAY_VECTOR2F) {
					recievePacket >> rayEnd.x >> rayEnd.y;

					//collision detection
					collisionDetection(currentClient, rayEnd);

					sendPacket << RAY_VECTOR2F << rayEnd.x << rayEnd.y << client[currentClient].tcpPort;
					//current client has shot, send to others
					for (int i = 0; i < maxClients; i++) {
						//send do evreyone but the sender client
						if (i != currentClient && client[i].active) {
							sf::Socket::Status sendStatus = client[i].tcpSocket.send(sendPacket);
							if (sendStatus != sf::Socket::Done) {
								std::cout << "ray send failed" << sendStatus << "\n";
							}
							std::cout << "ray recived from " << currentClient << "sent to" << i << "\n";
						}
					}
				}
			}
			else if (recieveStatus == sf::Socket::Disconnected) {
				//client dissconected
				client[currentClient].active = false;
				std::cout << "client no." << currentClient << "has dissconected\n";
				//contact all other clients about disconnect
				for (int i = 0; i < maxClients; i++) {
					sf::Packet dissconectPacket;
					dissconectPacket << ENEMY_DISSCONECTED << client[currentClient].tcpPort;
					sf::Socket::Status sendDisconnectStatus = client[i].tcpSocket.send(dissconectPacket);
				}
				//run check for avalable clients, if previously there were no more slots
				checkForNextAvalableClient();
			}
		}
	}
}

void Network::collisionDetection(int shooter, sf::Vector2f rayEnd)
{
	for (int i = 0; i < maxClients; i++) {
		if (client[i].active && i != shooter) {
			//not my code 
			//https://www.jeffreythompson.org/collision-detection/line-circle.php

			float cx = client[i].position.x;      // circle position (set by mouse)
			float cy = client[i].position.y;
			float r = client[i].radius;     // circle radius

			float x1 = client[shooter].position.x;    // coordinates of line
			float y1 = client[shooter].position.y;
			float x2 = rayEnd.x;
			float y2 = rayEnd.y;

			// Check if the shooter is shooting towards the target
			sf::Vector2f shooterToTarget = client[i].position - client[shooter].position;
			sf::Vector2f shootDirection = rayEnd - client[shooter].position;
			float dotProduct = shooterToTarget.x * shootDirection.x + shooterToTarget.y * shootDirection.y;

			if (dotProduct < 0) {
				// The shooter is not shooting towards the target
				continue;
			}


			// is either end INSIDE the circle?
  // if so, return true immediately
			bool inside1 = pointCircle(x1, y1, cx, cy, r);
			bool inside2 = pointCircle(x2, y2, cx, cy, r);
			if (inside1 || inside2) {
				communicateHit(i);
				std::cout << "hit\n";
				return;
			}

			// get length of the line
			float distX = x1 - x2;
			float distY = y1 - y2;
			float len = sqrt((distX * distX) + (distY * distY));

			// get dot product of the line and circle
			float dot = (((cx - x1) * (x2 - x1)) + ((cy - y1) * (y2 - y1))) / pow(len, 2);

			// find the closest point on the line
			float closestX = x1 + (dot * (x2 - x1));
			float closestY = y1 + (dot * (y2 - y1));

			//i dont care really thats a bit overkill
			// is this point actually on the line segment?
			// if so keep going, but if not, return false
			//bool onSegment = linePoint(x1, y1, x2, y2, closestX, closestY);
			//if (!onSegment) return false;

			// get distance to closest point
			distX = closestX - cx;
			distY = closestY - cy;
			float distance = sqrt((distX * distX) + (distY * distY));

			if (distance <= r) {
				communicateHit(i);
				std::cout << "hit\n";
				return;
			}
			client[i].hit = false;
			std::cout << "miss\n";

		}
	}
}

bool Network::pointCircle(float px, float py, float cx, float cy, float r) {

	// get distance between the point and circle's center
	// using the Pythagorean Theorem
	float distX = px - cx;
	float distY = py - cy;
	float distance = sqrt((distX * distX) + (distY * distY));

	// if the distance is less than the circle's
	// radius the point is inside!
	if (distance <= r) {
		return true;
	}
	return false;
}

void Network::communicateHit(int clientNo)
{
	client[clientNo].hit = true;

	sf::Packet hitPacket;
	hitPacket << HIT;
	sf::Socket::Status sendDisconnectStatus = client[clientNo].tcpSocket.send(hitPacket);
}

void Network::run()
{

	if (NewClientClock.getElapsedTime().asMilliseconds() >= newClientCheckIntervalMilliseconds) {
		//check for clients, listen for clients was run once in init
		checkForNewClients();
		NewClientClock.restart();
		//listen at the end so it will listen during the wait time
		listenForNewClients();
	}

	if (recieveSendDataClock.getElapsedTime().asMilliseconds() >= recieveSendDataMilliseconds) {
		recieveSendDataClock.restart();

	}

	communicateLoactionData();
	communicateShootData();

}
