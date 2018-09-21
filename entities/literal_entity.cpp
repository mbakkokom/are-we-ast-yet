#include "literal_entity.hpp"

#include <regex>

using namespace std;

LiteralEntity::LiteralEntity(string value) {
	SetValue(value);
}

bool LiteralEntity::IsValid(string value) {
	regex pattern("^-{0,1}[0-9]+(\\.[0-9]+){0,1}$");
	return value == "inf" || value == "nan" || regex_match(value, pattern);
}

Entity::EntityType LiteralEntity::GetType() {
	return LITERAL_ENTITY;
}

void LiteralEntity::SetValue(string value) {
	if (LiteralEntity::IsValid(value)) {
		if (value[0] == '-') {
			mValue = value.substr(1);
			SetNegative(true);
		} else mValue = value;
	} else {
		throw ASTValueError("invalid value for literal");
	}
}

LiteralEntity::~LiteralEntity() {}