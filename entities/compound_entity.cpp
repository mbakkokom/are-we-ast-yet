#include "compound_entity.hpp"

#include "exceptions.hpp"

CompoundEntity::CompoundEntity() {}

CompoundEntity::CompoundEntity(OperatorType type, Entity *left, Entity *right) {
	SetOperator(type);
	Set(LEFT_ENTITY, left);
	Set(RIGHT_ENTITY, right);
}

Entity::EntityType CompoundEntity::GetType() {
	return COMPOUND_ENTITY;
}

Entity* CompoundEntity::Get(EntityPosition pos) {
	switch(pos) {
	case LEFT_ENTITY:
		return mLeft;
		break;
	case RIGHT_ENTITY:
		return mRight;
		break;
	default:
		// not suppose to be here.
		return nullptr;
	}
}

string CompoundEntity::GetString() {
	string ls, rs;
	Entity *le = Get(LEFT_ENTITY), *re = Get(RIGHT_ENTITY);

	if (le == nullptr)
		ls = "NULL";
	else
		ls = le->GetString();

	if (re == nullptr)
		rs = "NULL";
	else
		rs = re->GetString();

	return ls + GetOperatorString() + rs;
}

void CompoundEntity::Set(EntityPosition pos, Entity *value) {
	if (value != nullptr && value->GetType() == Entity::INVALID_ENTITY) {
		throw ASTValueError("invalid entity");
	}

	switch(pos) {
	case LEFT_ENTITY:
		mLeft = value;
		break;
	case RIGHT_ENTITY:
		mRight = value;
		break;
	default:
		// not suppose to be here.
		break;
	}
}

CompoundEntity::~CompoundEntity() {
	if (mLeft != nullptr)
		delete mLeft;

	if (mRight != nullptr)
		delete mRight;
}