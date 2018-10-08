#pragma once

#include <string>

using namespace std;

class Entity {
public:
	typedef enum {
		INVALID_ENTITY = -1,
		FUNCTION_ENTITY,
		PARENTHESIS_ENTITY,
		COMPOUND_ENTITY,
		OPERAND_ENTITY,
		LITERAL_ENTITY,
	} EntityType;

	static constexpr const char* INVALID_ENTITY_STRING = "INVALID_ENTITY";
	static constexpr const char* FUNCTION_ENTITY_STRING = "FUNCTION_ENTITY";
	static constexpr const char* PARENTHESIS_ENTITY_STRING = "PARENTHESIS_ENTITY";
	static constexpr const char* COMPOUND_ENTITY_STRING = "COMPOUND_ENTITY";
	static constexpr const char* OPERAND_ENTITY_STRING = "OPERAND_ENTITY";
	static constexpr const char* LITERAL_ENTITY_STRING = "LITERAL_ENTITY";

	virtual EntityType GetType();
	virtual const char* GetTypeString();
	virtual string GetString();
	virtual ~Entity();
};
