#pragma once
#include <string>
// typicaly a feature will access some sort of game variable from hacked games
// m_offset ...
// we should have a reference towards that game varialbe specefically

class Feature
{
public:
private:
	std::string m_name;
	bool m_isOn; // needs to point to somewhere in data 

};