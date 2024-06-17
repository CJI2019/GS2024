#pragma once

class CTile
{
public:
	CTile();
	CTile(long a, long b, COLORREF c) {
		x = a;
		y = b;
		color = c;
	}
	~CTile();

	long x = 0;
	long y = 0;
	COLORREF color = RGB(0, 0, 0);
};

