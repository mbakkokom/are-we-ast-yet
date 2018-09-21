#include "tiered_entity.hpp"

TieredEntity::OperatorType TieredEntity::OPERATOR(char op) {
	if (op == '+') {
		return ARITHMETIC_ADD;
	} else if (op == '-') {
		return ARITHMETIC_SUB;
	} else if (op == '*') {
		return ARITHMETIC_MUL;
	} else if (op == '/') {
		return ARITHMETIC_DIV;
	} else if (op == '%') {
		return ARITHMETIC_MOD;
	} else if (op == '^') {
		return ARITHMETIC_POW;
	} else if (op == '=') {
		return OPERATOR_SET;
	} else {
		return OPERATOR_NONE;
	}
}

int TieredEntity::PRECEDENCE(OperatorType type) {
	switch(type) {
	case ARITHMETIC_ADD:
		return 3;
		break;
	case ARITHMETIC_SUB:
		return 3;
		break;
	case ARITHMETIC_MUL:
		return 2;
		break;
	case ARITHMETIC_DIV:
		return 2;
		break;
	case ARITHMETIC_MOD:
		return 2;
		break;
	case ARITHMETIC_POW:
		return 1;
		break;
	case PARENTHESIS:
		return 0;
	case OPERATOR_SET:
		return -1;
	default:
		return -1;
	}
}

TieredEntity::Associativity TieredEntity::ASSOCIATIVE(OperatorType type) {
	switch(type) {
	case ARITHMETIC_ADD:
		return ASSOC_LEFT;
		break;
	case ARITHMETIC_SUB:
		return ASSOC_LEFT;
		break;
	case ARITHMETIC_MUL:
		return ASSOC_LEFT;
		break;
	case ARITHMETIC_DIV:
		return ASSOC_LEFT;
		break;
	case ARITHMETIC_MOD:
		return ASSOC_LEFT;
		break;
	case ARITHMETIC_POW:
		return ASSOC_LEFT;
		break;
	case PARENTHESIS:
		return ASSOC_LEFT;
	case OPERATOR_SET:
		return ASSOC_RIGHT;
	default:
		return ASSOC_INVALID;
	}
}

TieredEntity::OperatorType TieredEntity::GetOperator() {
	return mOperatorType;
}

string TieredEntity::GetOperatorString() {
	switch(mOperatorType) {
	case ARITHMETIC_ADD:
		return "+";
		break;
	case ARITHMETIC_SUB:
		return "-";
		break;
	case ARITHMETIC_MUL:
		return "*";
		break;
	case ARITHMETIC_DIV:
		return "/";
		break;
	case ARITHMETIC_MOD:
		return "%";
		break;
	case ARITHMETIC_POW:
		return "^";
		break;
	case OPERATOR_SET:
		return "=";
		break;
	default:
		return "UNKNOWN_OPERATOR";
	}
}

int TieredEntity::GetOperatorPrecedence() {
	return PRECEDENCE(mOperatorType);
}

int TieredEntity::GetOperatorAssociativity() {
	return ASSOCIATIVE(mOperatorType);
}

void TieredEntity::SetOperator(OperatorType type) {
	mOperatorType = type;
}