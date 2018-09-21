#include "negatable_entity.hpp"

bool NegatableEntity::IsNegative() {
	return mIsNegative;
}

void NegatableEntity::SetNegative(bool negative) {
	mIsNegative = negative;
}