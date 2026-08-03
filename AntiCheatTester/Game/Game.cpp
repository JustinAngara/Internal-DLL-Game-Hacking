#include "Game.h"
#include <iostream>
#include <Windows.h>

// random pseudo logic
void Game::Run() 
{
	
	std::cout << "=========================================\n";
	std::cout << "this is my score: " << m_score     << '\n';
	std::cout << "this is my age: "   << m_age       << '\n';
	std::cout << "this is my tick: "  << m_tickCount << '\n';
	std::cout << "=========================================\n";
	
	m_tickCount++;
	if (m_tickCount % 5 == 0)
	{
		m_age++;
		m_score += 100;
	}

	Sleep(800);

}