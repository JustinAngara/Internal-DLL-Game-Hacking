#include "GameManagement.h"

const Game& GameManagement::getCurrentGame() const  
{
    return *m_selectedGame;  
}

void GameManagement::setCurrentGame(std::shared_ptr<Game> g)
{
    m_selectedGame = g;

    // only add if not already in the list
    auto it = std::find(m_listOfGames.begin(), m_listOfGames.end(), g);
    
    if (it == m_listOfGames.end())
    {
        m_listOfGames.push_back(std::move(g));
    }
}

void GameManagement::addGame(std::shared_ptr<Game> g)  
{
    m_listOfGames.push_back(std::move(g));
}

const std::vector<std::shared_ptr<Game>>& GameManagement::getListOfGames() const
{
    return m_listOfGames;
}

void GameManagement::runCurrentGame()
{
    m_selectedGame->run();
}