#pragma once

#include "tiered_entity.hpp"

class CompoundEntity final : public TieredEntity {
public:
	typedef enum {
		LEFT_ENTITY,
		RIGHT_ENTITY,
	} EntityPosition;
	
	CompoundEntity();
	CompoundEntity(OperatorType type, Entity *left = nullptr, Entity *right = nullptr);
	EntityType GetType() override;
	Entity* Get(EntityPosition pos);
	string GetString() override;
	void Set(EntityPosition pos, Entity *value);
	~CompoundEntity();
private:
	Entity *mLeft = nullptr, *mRight = nullptr;
};