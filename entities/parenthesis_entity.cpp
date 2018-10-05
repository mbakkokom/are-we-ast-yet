#include "parenthesis_entity.hpp"

#include "exceptions.hpp"

ParenthesisEntity::ParenthesisEntity() {}

ParenthesisEntity::ParenthesisEntity(Entity *e, bool negative) {
	Set(e);
	SetNegative(negative);
}

Entity::EntityType ParenthesisEntity::GetType() {
	return PARENTHESIS_ENTITY;
}

Entity* ParenthesisEntity::Get() {
	return mValue;
}

int ParenthesisEntity::GetOperatorPrecedence() {
	return PRECEDENCE(PARENTHESIS);
}

string ParenthesisEntity::GetString() {
	return (IsNegative() ? "-(" : "(") + mValue->GetString() + ")";
}

void ParenthesisEntity::Set(Entity *e) {
	mValue = e;
}

void ParenthesisEntity::SetOperator(OperatorType type) {
	throw ASTException("setting operator on ParenthesisEntity");
}

ParenthesisEntity::~ParenthesisEntity() {
	if (mValue != nullptr)
		delete mValue;
}