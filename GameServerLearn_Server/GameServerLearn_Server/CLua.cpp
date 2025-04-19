#include "stdafx.h"
#include "CLua.h"

bool CLua::isActive = true;

CLua::CLua()
{
	if (!isActive) return;
	m_lua_State = luaL_newstate();
	luaL_openlibs(m_lua_State);
	luaL_loadfile(m_lua_State, "Object.lua");
	ErrorCheck(lua_pcall(m_lua_State, 0, 0, 0));
}

CLua::~CLua()
{
	if (!isActive) return;
	lua_close(m_lua_State);
}

bool CLua::ErrorCheck(int retval)
{
	if (retval != 0) {
		const char* errorMsg = lua_tostring(m_lua_State, -1); // ���� �޽��� ��������
		cout << "Lua ���� ����: " << errorMsg << endl;
		lua_pop(m_lua_State, 1); // ���ÿ��� ���� �޽��� ����
		return false;
	}
	return true;
}

void CLua::Register_Functions(lua_State* L)
{
	//lua_register(L, "cpp_add", cpp_add);  // 'cpp_add'��� �̸����� �Լ� ���
}

void CLua::SetScriptInfo(int id)
{
	if (!isActive) {
		m_name = "obj_" + std::to_string(id);
		return;
	}

	lua_getglobal(m_lua_State, "Set_info");
	lua_pushinteger(m_lua_State, id);
	if (ErrorCheck(lua_pcall(m_lua_State, 1, 1, 0))) {
		m_name = lua_tostring(m_lua_State, -1);
		//cout << id << " -> " << name << endl;
		
		//int ret_id = lua_tointeger(m_lua_State, -1);
		//cout << id << " <=> " << ret_id << endl;
		lua_pop(m_lua_State, 1);
	}

}
