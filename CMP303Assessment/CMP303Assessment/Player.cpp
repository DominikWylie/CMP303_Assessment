#include "Player.h"

Player::Player(sf::Vector2f position) : playerBody(25, 20), line(sf::Lines, 2)
{

	playerBody.setPosition(position);
	previousPos = position;
	line[0].color = sf::Color::Red;
	line[1].color = sf::Color::Yellow;
	line[0].position = sf::Vector2f(0, 0);
	line[1].position = sf::Vector2f(0, 0);

}

Player::~Player()
{
}

bool Player::update(sf::RenderWindow* window, sf::Event &event, float dt)
{
	handleMovement(dt);
	return handleShoot(window, event, dt);
}

void Player::render(sf::RenderWindow* window)
{
	window->draw(line);
	window->draw(playerBody);
}

sf::Vector2f Player::getLocation()
{
	return playerBody.getPosition();
}

void Player::setLocation(sf::Vector2f loc)
{
	playerBody.setPosition(loc);
}

sf::Vector2f Player::getRayEnd()
{
	return line[1].position;
}

bool Player::getSend()
{
	return sendData;
}

void Player::setSend(bool set)
{
	sendData = set;
}

void Player::handleMovement(float dt)
{
	previousPos = playerBody.getPosition();

	//seperate so if pressing both it cancels out
	sf::Vector2f moveTo = sf::Vector2f(0, 0);
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
		moveTo += sf::Vector2f(0, -playerSpeed);
	}

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
		moveTo += sf::Vector2f(0, playerSpeed);
	}

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
		moveTo += sf::Vector2f(-playerSpeed, 0);
	}	
	
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
		moveTo += sf::Vector2f(playerSpeed, 0);
	}

	if (moveTo != sf::Vector2f(0, 0)) {
		playerBody.setPosition(
			sf::Vector2f(
				playerBody.getPosition().x + (moveTo.x * dt),
				playerBody.getPosition().y + (moveTo.y * dt)
			)
		);
		sendData = true;
	}

	//so after player stops it sends the same location once more so the other players know they stopped
	if (previousPos == playerBody.getPosition() && !stopped) {
		justStopped = true;
		sendData = true;
	}
	else if (previousPos == playerBody.getPosition() && justStopped) {
		stopped = true;
		justStopped = false;
	}
	else if (previousPos != playerBody.getPosition() && stopped) {
		stopped = false;
	}

}

bool Player::handleShoot(sf::RenderWindow* window, sf::Event& event, float dt)
{
	//seperate flag so it still runs the line fade at the bottom of the function
	bool fired = false;
	if (event.type == sf::Event::MouseButtonPressed) {
		if (event.mouseButton.button == sf::Mouse::Left && !leftMouseClicked) {
			cursorPos = sf::Mouse::getPosition(*window);
			worldCursorPos = window->mapPixelToCoords(cursorPos);

			sf::Vector2f rayStart = sf::Vector2f(playerBody.getPosition().x + playerBody.getRadius(), playerBody.getPosition().y + playerBody.getRadius());
			sf::Vector2f rayCursor = worldCursorPos;
			sf::Vector2f rayVector = rayCursor - rayStart;
			//sf::Vector2f rayEnd = rayStart + sf::Vector2f(rayCursor.x * 3, rayCursor.y * 3);

			float sqrt = std::sqrt(rayVector.x * rayVector.x + rayVector.y * rayVector.y);
			sf::Vector2f rayNormalised = rayVector / sqrt;
			sf::Vector2f rayEnd = sf::Vector2f((rayNormalised.x * 2000) + rayStart.x, (rayNormalised.y * 2000) + rayStart.y);
			//sf::Vector2f rayEnd = rayStart + 3.0f * rayVector;

			line[0].position = rayStart;
			//line[1].position = worldCursorPos;
			line[1].position = rayEnd;
			transparency = 255;
			leftMouseClicked = true;
			fired = true;
		}
	}
	else if (event.type == sf::Event::MouseButtonReleased)
	{
		if (event.mouseButton.button == sf::Mouse::Left)
		{
			leftMouseClicked = false;
		}
	}

	if (transparency > 0) {
		transparency -= (fadeOutSpeed * dt);
		line[0].color = sf::Color(shootRayColor[0], shootRayColor[1], shootRayColor[2], transparency);
		line[1].color = sf::Color(shootRayColor[0], shootRayColor[1], shootRayColor[2], transparency);
		if (transparency < 0) {
			transparency = 0;
		}
	}

	return fired;
}
