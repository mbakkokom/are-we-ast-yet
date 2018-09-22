#pragma once

#include "entity.hpp"

class TieredEntity : public Entity {
public:
	typedef enum {
		OPERATOR_INVALID = -1,
		DIRECTIVE_ARGS,
		DIRECTIVE_CALL,
		ARITHMETIC_ADD,
		ARITHMETIC_SUB,
		ARITHMETIC_MUL,
		ARITHMETIC_DIV,
		ARITHMETIC_MOD,
		ARITHMETIC_POW,
		PARENTHESIS,
		OPERATOR_SET
	} OperatorType;

	typedef enum {
		ASSOC_INVALID = -1,
		ASSOC_LEFT,
		ASSOC_RIGHT
	} Associativity;
	
	static OperatorType OPERATOR(char op);
	static int PRECEDENCE(OperatorType type);
	static Associativity ASSOCIATIVE(OperatorType type);
	OperatorType GetOperator();
	string GetOperatorString();
	virtual int GetOperatorPrecedence();
	virtual int GetOperatorAssociativity();
	virtual void SetOperator(OperatorType type);
private:
	OperatorType mOperatorType = OPERATOR_INVALID;
};