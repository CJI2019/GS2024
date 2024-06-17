#pragma once
#include "stdafx.h"
#include "Sector.h"


Sector::Sector()
{
}

Sector::~Sector()
{
}

std::unordered_set<int> Sector::GetCurrentSector(Vector2& sec_pos)
{
	std::lock_guard<std::mutex> sl(m_mtxSector);
	std::unordered_set<int> pSector;

	for (int col = -1; col < 2;++col) {// -1 ~ +1
		for (int row = -1; row < 2; ++row) {// -1 ~ +1
			if ((sec_pos.x + row >= 0 && sec_pos.x + row < SECTOR_ROW) &&
				(sec_pos.y + col >= 0 && sec_pos.y + col < SECTOR_COL))
			{
				for (auto& obj_id : m_Sectors[sec_pos.x + row][sec_pos.y + col]) {
					pSector.insert(obj_id);
				}
			}
		}
	}

	return pSector;

	//return m_Sectors[sec_pos.x][sec_pos.y];
}

Vector2 Sector::AllocSectorId(Vector2& pl_pos, int c_id)
{
	if (pl_pos.x >= 0 && pl_pos.y >= 0) {
		for (int y = 0; y < SECTOR_COL; ++y) {
			for (int x = 0; x < SECTOR_ROW;++x) {

				if (pl_pos.y < (y * COL_Y / SECTOR_COL) + (COL_Y / SECTOR_COL)) {
					if (pl_pos.x < (x * ROW_X / SECTOR_ROW) + (ROW_X / SECTOR_ROW)) {
						std::lock_guard<std::mutex> sl(m_mtxSector);
						m_Sectors[x][y].insert(c_id);
						return Vector2(x, y);
					}
				}

			}
		}
	}

	// 반드시 할당 받아야 한다. 할당 받지 못한다면 위 코드 또는 좌표가 잘못 된것
	assert(0);
	return Vector2(-1, -1);
}

void Sector::PushSectorId(Vector2& sec_pos, int c_id)
{
	std::lock_guard<std::mutex> ls(m_mtxSector);
	m_Sectors[sec_pos.x][sec_pos.y].insert(c_id);
}

void Sector::PopSectorId(Vector2& sec_pos, int c_id)
{
	std::lock_guard<std::mutex> ls(m_mtxSector);
	m_Sectors[sec_pos.x][sec_pos.y].erase(c_id);
}

bool Sector::InCurrentSector(Vector2& pl_pos, Vector2& sec_pos)
{
	if (pl_pos.y < (((sec_pos.y - 1) * COL_Y / SECTOR_COL) + (COL_Y / SECTOR_COL))) {
		//std::cout << "섹터를 벗어났습니다.\n";
		return false;
	}
	if (pl_pos.x < (((sec_pos.x - 1) * ROW_X / SECTOR_ROW) + (ROW_X / SECTOR_ROW))) {
		//std::cout << "섹터를 벗어났습니다.\n";
		return false;
	}

	if (pl_pos.y < (sec_pos.y * COL_Y / SECTOR_COL) + (COL_Y / SECTOR_COL)) {
		if (pl_pos.x < (sec_pos.x * ROW_X / SECTOR_ROW) + (ROW_X / SECTOR_ROW)) {
			std::lock_guard<std::mutex> sl(m_mtxSector);
			return true;
		}
	}
	//std::cout << "섹터를 벗어났습니다.\n";
	return false;
}