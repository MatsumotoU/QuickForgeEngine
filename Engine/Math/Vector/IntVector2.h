#pragma once
class IntVector2 {
public:
	int x;
	int y;

	bool operator==(const IntVector2& rhs) const {
		return x == rhs.x && y == rhs.y;
	}
};