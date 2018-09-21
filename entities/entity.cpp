#include "entity.hpp"
#include "exceptions.hpp"

Entity::EntityType Entity::GetType() {
	return INVALID_ENTITY;
}

string Entity::GetTypeString() {
	switch(this->GetType()) {
	case COMPOUND_ENTITY:
		return "COMPOUND_ENTITY";
		break;
	case OPERAND_ENTITY:
		return "OPERAND_ENTITY";
		break;
	case LITERAL_ENTITY:
		return "LITERAL_ENTITY";
		break;
	case INVALID_ENTITY:
	default:
		return "INVALID_ENTITY";
	}
}

string Entity::GetString() {
	throw ASTException("getting string value on base Entity");
}

Entity::~Entity() {}