#pragma once

#include "single_value_entity.hpp"

class LiteralEntity final : public SingleValueEntity {
public:
	LiteralEntity(string value);
	static bool IsValid(string value);
	EntityType GetType() override;
	void SetValue(string value) override;
	~LiteralEntity();
};