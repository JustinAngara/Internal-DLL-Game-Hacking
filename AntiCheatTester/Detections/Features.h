#pragma once
#include <string>
#include <vector>
#include "../Vars.h"

class Feature
{
public:
	Feature(std::string n = "name me", std::string d = "give me a desc", bool f = false)
		  : m_name(n), m_desc(d), m_isFlagged(f) { }

public:
	void setName(std::string n) { m_name = n; }
	void setDesc(std::string d) { m_desc = d; }
	void setFlag(bool f)        { m_isFlagged = f; }
public:
	std::string getName() { return m_name; }
	std::string getDesc() { return m_desc; }
	bool        getFlag() { return m_isFlagged; }

public:
	void callCheckForDetection() // basically anytime you do ts you run it idrk how to explain
	{ 
		if (m_func())
		{
			setFlag(true);
			return;
		}
		setFlag(false);
	} 
private:
	// name, desc, isFlagged
	std::string m_name;
	std::string m_desc;
	bool        m_isFlagged;
	TestDetFunc m_func;

};

