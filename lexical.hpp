#pragma once

#include "entities/entities.hpp"

class ASTLex {
public:
	ASTLex();
	Entity* GetEntityFrom(string code);
	Entity* Parse(string code);
	void LeftAssociate(Entity **HEAD, CompoundEntity *TMP);
	void RightAssociate(Entity **HEAD, CompoundEntity *TMP);
	~ASTLex();
protected:
	void CLEANUP(Entity *e);
};