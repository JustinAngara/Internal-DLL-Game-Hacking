#pragma once
#include <string>
class Feature
{
public:
	Feature(std::string n = "name me", std::string d = "give me a desc", bool f = false)
		  : m_name(n), m_desc(d), m_isFlagged(f) { }

public:

public:
	void setName(std::string n) { m_name = n; }
	void setDesc(std::string d) { m_desc = d; }
	void setFlag(bool f)        { m_isFlagged = f; }
public:
	std::string getName() { return m_name; }
	std::string getDesc() { return m_desc; }
	bool        getFlag() { return m_isFlagged; }

private:
	// name, desc, isFlagged
	std::string m_name;
	std::string m_desc;
	bool        m_isFlagged;

};

// Don't know if we need yet
//namespace Container
//{
//}
