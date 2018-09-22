#include "operand_entity.hpp"

#include <regex>

using namespace std;

OperandEntity::OperandEntity() {}

OperandEntity::OperandEntity(string value) {
	SetValue(value);
}

bool OperandEntity::IsValid(string value) {
	regex pattern("^-{0,1}[A-Za-z_]{1}[A-Za-z0-9_]*$");
	return value != "inf" && value != "nan" && regex_match(value, pattern);
}

Entity::EntityType OperandEntity::GetType() {
	return OPERAND_ENTITY;
}

void OperandEntity::SetValue(string value) {
	if (IsValid(value)) {
		if (value[0] == '-') {
			mValue = value.substr(1);
			SetNegative(true);
		} else mValue = value;
	} else {
		throw ASTValueError("invalid value for operand");
	}
}

OperandEntity::~OperandEntity() {}