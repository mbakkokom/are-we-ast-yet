#pragma once

#include <string>

using namespace std;

class Entity {
public:
	typedef enum {
		INVALID_ENTITY = -1,
		CALLER_ENTITY,
		PARENTHESIS_ENTITY,
		COMPOUND_ENTITY,
		OPERAND_ENTITY,
		LITERAL_ENTITY,
	} EntityType;

	virtual EntityType GetType();
	virtual string GetTypeString();
	virtual string GetString();
	virtual ~Entity();
};