#pragma once

#include "operand_entity.hpp"

#include <vector>

using namespace std;

class FunctionEntity final : public OperandEntity, protected vector<Entity*> {
public:
	FunctionEntity(string value);
	static bool IsValid(string value);
	EntityType GetType() override;
	void SetValue(string value) override;

	// -- Arguments
	size_t GetArgumentsLength();
	bool HasArguments();
	void AddArgument(Entity *entity);
	Entity* PopArgument();
	vector<Entity*> GetArguments();
	void ClearArguments();
	// -- End arguments

	~FunctionEntity();
};
