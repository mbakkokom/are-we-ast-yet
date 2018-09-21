#include "single_value_entity.hpp"

string SingleValueEntity::GetString() {
	return IsNegative() ? "(-" + mValue + ")" : mValue;
}

string SingleValueEntity::GetValue() {
	return IsNegative() ? "-" + mValue : mValue;
}

string SingleValueEntity::GetAbsValue() {
	return mValue;
}

void SingleValueEntity::SetValue(string value) {
	throw ASTException("setting value on base SingleValueEntity");
}

SingleValueEntity::~SingleValueEntity() {}