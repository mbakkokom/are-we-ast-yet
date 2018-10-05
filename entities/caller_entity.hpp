#pragma once

#include "operand_entity.hpp"

class CallerEntity final : public OperandEntity {
public:
	CallerEntity(string value);
	static bool IsValid(string value);
	EntityType GetType() override;
	void SetValue(string value) override;
	~CallerEntity();
};