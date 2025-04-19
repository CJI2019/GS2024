#pragma once
class CLua
{
public:
	CLua();
	~CLua();

	lua_State* m_lua_State;

	std::string m_name;
	static bool isActive;

public:
	bool ErrorCheck(int retval);
	void Register_Functions(lua_State* L);

	void SetScriptInfo(int id);
};

