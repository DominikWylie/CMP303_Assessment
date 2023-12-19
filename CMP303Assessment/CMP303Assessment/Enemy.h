#pragma once

#include <iostream>
#include <SFML/graphics.hpp>
#include <cmath>

class Enemy
{

public:
	Enemy();
	~Enemy();

	void setPosition(sf::Vector2f position, float gameTime);
	sf::Vector2f getPosition();
	void shoot(sf::Vector2f rayEnd);
	void render(sf::RenderWindow* window);
	void update(float dt, float gameTime);

	bool getActive();
	void setActive(bool set);

	void setId(unsigned short id);
	unsigned short getId();

	void movementPrediction(float dt);
	void setNewPosition(sf::Vector2f newPos, float gameTime);

	void setTransparency(float transp);

private:

	sf::Vector2f normalize(sf::Vector2f vec);

	sf::CircleShape body;
	sf::VertexArray line;

	float fadeOutSpeed = 500.0f;
	float transparency = 0;
	float shootRayColor[3] = { 255, 0, 0 };

	bool active = false;

	sf::Vector2f targetPosition, displacementToTarget;
	float linearInterpolationValue = 20;

	unsigned short uniqueId = 0;
};

