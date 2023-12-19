#include "Enemy.h"

Enemy::Enemy() : body(25, 20), line(sf::Lines, 2)
{

}

Enemy::~Enemy()
{
}

void Enemy::setPosition(sf::Vector2f position, float gameTime)
{

	targetPosition = position;

}

sf::Vector2f Enemy::getPosition()
{
	return body.getPosition();
}

void Enemy::shoot(sf::Vector2f rayEnd)
{
	line[0] = sf::Vector2f(body.getPosition().x + body.getRadius(), body.getPosition().y + body.getRadius());;
	line[1] = rayEnd;
	transparency = 255;
}

void Enemy::render(sf::RenderWindow* window)
{
	if (active) {
		window->draw(body);
		window->draw(line);
	}

	if (transparency > 0) {
		std::cout << "enemy " << uniqueId << " transparrency: " << transparency << "\n";
	}
}

void Enemy::update(float dt, float gameTime)
{
	if (transparency > 0) {
		transparency -= (fadeOutSpeed * dt);
		line[0].color = sf::Color(shootRayColor[0], shootRayColor[1], shootRayColor[2], transparency);
		line[1].color = sf::Color(shootRayColor[0], shootRayColor[1], shootRayColor[2], transparency);
		if (transparency < 0) {
			transparency = 0;
		}
	}

	movementPrediction(dt);

}

bool Enemy::getActive()
{
	return active;
}

void Enemy::setActive(bool set)
{
	active = set;
}

void Enemy::setId(unsigned short id)
{
	uniqueId = id;
}

unsigned short Enemy::getId()
{
	return uniqueId;
}

void Enemy::movementPrediction(float dt)
{

	displacementToTarget = targetPosition - body.getPosition();

	body.setPosition(body.getPosition() + ((displacementToTarget * linearInterpolationValue) * dt));

}

void Enemy::setNewPosition(sf::Vector2f newPos, float gameTime)
{

	targetPosition = newPos;

}

void Enemy::setTransparency(float transp)
{
	transparency = transp;
}

sf::Vector2f Enemy::normalize(sf::Vector2f vec)
{
	vec /= std::sqrt(vec.x * vec.x + vec.y * vec.y);

	return vec;
}
