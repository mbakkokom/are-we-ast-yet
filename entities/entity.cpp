#include "entity.hpp"
#include "exceptions.hpp"

Entity::EntityType Entity::GetType() {
	return INVALID_ENTITY;
}

const char* Entity::GetTypeString() {
	switch(this->GetType()) {
	case COMPOUND_ENTITY:
		return COMPOUND_ENTITY_STRING;
		break;
	case OPERAND_ENTITY:
		return OPERAND_ENTITY_STRING;
		break;
	case LITERAL_ENTITY:
		return LITERAL_ENTITY_STRING;
		break;
	case FUNCTION_ENTITY:
		return FUNCTION_ENTITY_STRING;
		break;
	case PARENTHESIS_ENTITY:
		return PARENTHESIS_ENTITY_STRING;
		break;
	case INVALID_ENTITY:
	default:
		return INVALID_ENTITY_STRING;
	}
}

string Entity::GetString() {
	throw ASTException("getting string value on base Entity");
}

Entity::~Entity() {}
