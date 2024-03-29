#pragma once
#include "p2Shape.h"
#include "Tobes/Common/Math/Vector2.h"

struct p2FixtureDef
{
	p2FixtureDef()
	{
		shape = nullptr;
		restitution = 0;
		density = 1;
	}

	const p2Shape* shape;
	float restitution;
	float density;
};

class p2Body;

class p2Fixture
{
private:
	friend class p2Body;
	friend class p2World;

	p2Fixture(p2Body* body, const p2FixtureDef* def) { m_body = body; m_shape = def->shape->Clone(); m_restitution = def->restitution; m_density = def->density; }
	p2Body* m_body;
	p2Shape* m_shape;
	float m_restitution;
	float m_density;
	void UpdateRotation(float rotation);

	//TODO: Friction

public:
	p2Shape* GetShape() const;
	p2Body* GetBody() const;
	Vector2 GetBodyPos() const;
	Vector2 GetBodyVelocity() const;
	p2MassData* GetMassData() const;
	p2Shape::Type GetShapeType() { return m_shape->GetType(); }
};
