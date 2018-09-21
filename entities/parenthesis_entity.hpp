#pragma once

#include "tiered_entity.hpp"
#include "negatable_entity.hpp"

class ParenthesisEntity final : public TieredEntity, public NegatableEntity {
public:
	ParenthesisEntity();
	ParenthesisEntity(Entity *e, bool negative=false);
	EntityType GetType() override;
	Entity* Get();
	virtual int GetOperatorPrecedence() override;
	string GetString() override;
	void Set(Entity *e);
	virtual void SetOperator(OperatorType type) override;
private:
	Entity *mValue;
};
