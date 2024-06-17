#pragma once

#define SECTOR_ROW 100
#define SECTOR_COL 100

class Sector {
public:
	Sector();
	~Sector();

	std::unordered_set<int> GetCurrentSector(Vector2& sec_pos);
	Vector2 AllocSectorId(Vector2& pl_pos, int c_id);

	void PushSectorId(Vector2& sec_pos, int c_id);
	void PopSectorId(Vector2& sec_pos, int c_id);
	bool InCurrentSector(Vector2& pl_pos, Vector2& sec_pos);

private:
	// [0][1] -> [id0,id1,id2....]
	// �˻� ������ ����Ͽ� unordered_set�� ����Ѵ�.
	std::array<std::array<std::unordered_set<int>, SECTOR_COL>, SECTOR_ROW> m_Sectors;
	std::mutex m_mtxSector;
};