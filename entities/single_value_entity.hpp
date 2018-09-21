#pragma once

#include "entity.hpp"
#include "negatable_entity.hpp"
#include "exceptions.hpp"

class SingleValueEntity : public Entity, public NegatableEntity {
public:
	virtual string GetString();
	virtual string GetValue();
	virtual string GetAbsValue();
	virtual void SetValue(string value);
	virtual ~SingleValueEntity();
protected:
	string mValue;
};