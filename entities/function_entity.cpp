#include "function_entity.hpp"

#include <regex>

using namespace std;

FunctionEntity::FunctionEntity(string value) : OperandEntity(value) {}

bool FunctionEntity::IsValid(string value) {
	regex pattern("^-{0,1}[A-Za-z_]{1}[A-Za-z0-9]*$");
	return value != "inf" && value != "nan" && regex_match(value, pattern);
}

Entity::EntityType FunctionEntity::GetType() {
	return FUNCTION_ENTITY;
}

void FunctionEntity::SetValue(string value) {
	if (IsValid(value)) {
		if (value[0] == '-') {
			mValue = value.substr(1);
			SetNegative(true);
		} else mValue = value;
	} else {
		throw ASTValueError("invalid directive");
	}
}

size_t FunctionEntity::GetArgumentsLength() {
	return this->size();
}

bool FunctionEntity::HasArguments() {
	return !this->empty();
}

void FunctionEntity::AddArgument(Entity *entity) {
	this->push_back(entity);
}

Entity* FunctionEntity::PopArgument() {
	Entity *ret = this->at(this->size()-1);
	this->pop_back();

	return ret;
}

vector<Entity*> FunctionEntity::GetArguments() {
	vector<Entity*> ret;
	for (vector<Entity*>::iterator it = this->begin(); it != this->end(); ++it) {
		ret.push_back(*it);
	}
	return ret;
}

void FunctionEntity::ClearArguments() {
	while(!this->empty()) {
		delete PopArgument();
	}
}


FunctionEntity::~FunctionEntity() {
	ClearArguments();
}
