#pragma once
#include "stdafx.h"
#include "DataBase.h"

Sector::Sector()
{
}

Sector::~Sector()
{
}

std::unordered_set<int> Sector::GetCurrentSector(Vector2& sec_pos)
{
	std::lock_guard<std::mutex> sl(m_mtxSector);
	return m_Sectors[sec_pos.x][sec_pos.y];
}

Vector2 Sector::PushSectorId(Vector2& pl_pos,int c_id)
{
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

	// �ݵ�� �Ҵ� �޾ƾ� �Ѵ�. �Ҵ� ���� ���Ѵٸ� �� �ڵ� �Ǵ� ��ǥ�� �߸� �Ȱ�
	assert(0);
	return Vector2(-1, -1);
}

void Sector::PopSectorId(Vector2& sec_pos,int c_id)
{
	std::lock_guard<std::mutex> ls(m_mtxSector);
	m_Sectors[sec_pos.x][sec_pos.y].erase(c_id);
}

bool Sector::InCurrentSector(Vector2& pl_pos, Vector2& sec_pos)
{
	if (pl_pos.y < (((sec_pos.y - 1) * COL_Y / SECTOR_COL) + (COL_Y / SECTOR_COL))) {
		//std::cout << "���͸� ������ϴ�.\n";
		return false;
	}
	if (pl_pos.x < (((sec_pos.x - 1) * ROW_X / SECTOR_ROW) + (ROW_X / SECTOR_ROW))) {
		//std::cout << "���͸� ������ϴ�.\n";
		return false;
	}

	if (pl_pos.y < (sec_pos.y * COL_Y / SECTOR_COL) + (COL_Y / SECTOR_COL)) {
		if (pl_pos.x < (sec_pos.x * ROW_X / SECTOR_ROW) + (ROW_X / SECTOR_ROW)) {
			std::lock_guard<std::mutex> sl(m_mtxSector);
			return true;
		}
	}
	//std::cout << "���͸� ������ϴ�.\n";
	return false;
}
