#pragma once
#include "Game.h"
#include <vector>
#include <memory>
class GameManagement
{
public:

	GameManagement(std::shared_ptr<Game> g)
		: m_selectedGame(g) { m_listOfGames.push_back(std::move(g)); }
	~GameManagement() = default;
	
	const Game& getCurrentGame() const; 
	void setCurrentGame(std::shared_ptr<Game> g);
	void addGame(std::shared_ptr<Game> g); 
	const std::vector<std::shared_ptr<Game>>& getListOfGames() const;  
	
	void runCurrentGame();



private:
	std::shared_ptr<Game> m_selectedGame;
	std::vector<std::shared_ptr<Game>> m_listOfGames;
};