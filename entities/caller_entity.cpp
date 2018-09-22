#include "caller_entity.hpp"

#include <regex>

using namespace std;

CallerEntity::CallerEntity(string value) : OperandEntity(value) {}

bool CallerEntity::IsValid(string value) {
	regex pattern("^-{0,1}[A-Za-z_]{1}[A-Za-z0-9]*$");
	return value != "inf" && value != "nan" && regex_match(value, pattern);
}

Entity::EntityType CallerEntity::GetType() {
	return CALLER_ENTITY;
}

void CallerEntity::SetValue(string value) {
	if (IsValid(value)) {
		if (value[0] == '-') {
			mValue = value.substr(1);
			SetNegative(true);
		} else mValue = value;
	} else {
		throw ASTValueError("invalid directive");
	}
}

CallerEntity::~CallerEntity() {}