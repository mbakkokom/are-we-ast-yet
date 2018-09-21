#pragma once

class NegatableEntity {
public:
	bool IsNegative();
	void SetNegative(bool negative);
private:
	bool mIsNegative = false;
};