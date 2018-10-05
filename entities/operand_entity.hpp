#pragma once

#include "single_value_entity.hpp"

#include <string>

using namespace std;

class OperandEntity : public SingleValueEntity {
public:
	OperandEntity();
	OperandEntity(string value);
	static bool IsValid(string value);
	EntityType GetType() override;
	void SetValue(string value) override;
	~OperandEntity();
};