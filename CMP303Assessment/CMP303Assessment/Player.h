#pragma once

#include "SFML/Graphics.hpp"


class Player
{
public:
	Player(sf::Vector2f position);
	~Player();

	bool update(sf::RenderWindow* window, sf::Event &event, float dt);
	void render(sf::RenderWindow* window);
	sf::Vector2f getLocation();
	void setLocation(sf::Vector2f loc);
	sf::Vector2f getRayEnd();
	bool getSend();
	void setSend(bool set);

private:
	void handleMovement(float dt);
	bool handleShoot(sf::RenderWindow* window, sf::Event& event, float dt);

	sf::CircleShape playerBody;
	sf::Vector2f previousPos;
	float playerSpeed = 400.0f;
	sf::Vector2i cursorPos;
	sf::Vector2f worldCursorPos;
	sf::VertexArray line;
	float fadeOutSpeed = 500.0f;
	float transparency = 0;
	float shootRayColor[3] = {255, 0, 0};
	bool leftMouseClicked = false;
	bool sendData = true;
	bool stopped = false, justStopped = false;
};

