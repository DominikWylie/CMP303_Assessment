#include "Game.h"

Game::Game() : player(sf::Vector2f(300, 300))
{
    window = new sf::RenderWindow(sf::VideoMode(1300, 800), "CMP303Assessment");

	init();
}

void Game::init() {
    network.init();
}

void Game::run() {

    while (window->isOpen())
    {

        sf::Event event;
        while (window->pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window->close();
        }

        deltaTime = dtClock.restart();
        dt = deltaTime.asSeconds();
        gameTime = gameClock.getElapsedTime().asSeconds();

        sf::Vector2i cursorPos = sf::Mouse::getPosition(*window);
        sf::Vector2f worldCursorPos = window->mapPixelToCoords(cursorPos);

        //if returns true player has fired
        if (player.update(window, event, dt)) {
            network.communicateShootData(player.getRayEnd());
        }
        for (int i = 0; i < maxEnemies; i++) {
            enemy[i].update(dt, gameTime);
        }
        //network.communicateShootData(player.getLocation());
        if (sendDataClock.getElapsedTime().asMilliseconds() >= sendDataMilliseconds) {
            network.communicateLoactionData(player);
            sendDataClock.restart();
        }
        //EnemyDataRecieve enemyData = network.recieveEnemyData();
        processIncomingData(network.recieveEnemyData(), gameTime);

        window->clear(sf::Color(18, 33, 43)); // Color background
        player.render(window);

        for (int i = 0; i < maxEnemies; i++) {
            enemy[i].render(window);
            //enemy[i].getPosition();
            //std::cout << "enemy " << enemy[i].getPosition().x << ", " << enemy[i].getPosition().y << "\n";
        }

        window->display();
    }

}

void Game::processIncomingData(EnemyDataRecieve enemyData, float gameTime)
{

    if (enemyData.newEnemy) {
        //get the next non active
        int nextAvalable = 0;
        while (enemy[nextAvalable].getActive()) {
            nextAvalable++;
            if (nextAvalable >= maxEnemies) {
                std::cout << "error exeeded max enemies\n";
                return;
            }
        }
        enemy[nextAvalable].setActive(true);
        enemy[nextAvalable].setNewPosition(enemyData.location, gameTime);
        enemy[nextAvalable].setId(enemyData.uniqueId);

        std::cout << "enemy added\n";
        
        int enemyNum = 0;
        while (enemyNum < maxEnemies) {
            if (enemy[enemyNum].getActive()) {
                std::cout << "enemy slot " << enemyNum << " active - " << enemy[enemyNum].getId() << "\n";
            }
            else {
                std::cout << "enemy slot " << enemyNum << " inactive x\n";
            }
            enemyNum++;
        }
    }

    //enemy disconected
    if (!enemyData.active) {
        for (int i = 0; i < maxEnemies; i++) {
            if (enemyData.uniqueId == enemy[i].getId()) {
                enemy[i].setActive(false);
                return;
            }
        }
    }

    //if it isnt new and is active, set locaton and shoot
    if (!enemyData.noMoveData) {
        for (int i = 0; i < maxEnemies; i++) {
            if (enemyData.uniqueId == enemy[i].getId()) {
                enemy[i].setPosition(enemyData.location, gameTime);
                //std::cout << "update: " << enemyData.uniqueId << " - " << enemyData.location.x << ", " << enemyData.location.y << "\n";
            }
        }
    }
    else {
        //std::cout << "no move data\n";
    }

    if (!enemyData.noShootData) {
        for (int i = 0; i < maxEnemies; i++) {
            if (enemyData.uniqueId == enemy[i].getId()) {
                enemy[i].shoot(enemyData.rayEnd);
                enemy[i].setTransparency(enemyData.transparency);
            }
        }
    }

    if (enemyData.hit) {
        //died - back to spawn
        player.setLocation(sf::Vector2f(5, 5));
        //send updated location
        network.communicateLoactionData(player);
    }

}