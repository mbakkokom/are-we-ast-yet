#pragma once

#include "entities/entities.hpp"

class ASTLex {
public:
	ASTLex();
	Entity* GetEntityFrom(string code);
	Entity* Parse(string code, char separator='\n');
	void LeftAssociate(Entity **HEAD, CompoundEntity *TMP);
	void RightAssociate(Entity **HEAD, CompoundEntity *TMP);
	string GetPostfix(Entity *e);
	~ASTLex();
protected:
	void CLEANUP(Entity *e);
};