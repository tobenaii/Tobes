#include "p2Collision.h"
#include <vector>
#include <iostream>
#include "p2Body.h"
fn collisionsFunctionArray[] = 
{
p2CollideCircles, //0 + 0 =  0
p2CollidePolygonAndCircle, //1 + 0 = 1
p2CollidePolygons, //1 + 1 = 2
p2CollideEdgeAndCircle, //3 + 0 = 3
p2CollideEdgeAndPolygon, //3 + 1 = 4
};

void ResolveCollision(const CollisionData& data)
{
	float j = (Vector2::Dot(data.relativeVelocity * -(0.6f + 1), data.normal)) / (Vector2::Dot(data.normal, data.normal * (1 / data.bodyA->GetMass() + 1 / data.bodyB->GetMass())));
	//const float min = 0;
	//float max = data.bodyA->GetMass() + data.bodyB->GetMass();
	//float moveScaleA = (data.bodyA->GetMass() - min) / (max - min);
	//float moveScaleB = (data.bodyB->GetMass() - min) / (max - min);
	float moveScaleA = 0.5f;
	float moveScaleB = 0.5f;
	if (data.bodyA->GetType() != p2_dynamicBody)
	{
		moveScaleA = 0;
		moveScaleB = 1;
	}
	if (data.bodyB->GetType() != p2_dynamicBody)
	{
		moveScaleA = 1;
		moveScaleB = 0;
	}
	data.bodyA->SetPosition(data.bodyA->GetPosition() - (data.normal * (data.overlap * moveScaleA)));
	data.bodyB->SetPosition(data.bodyB->GetPosition() + (data.normal * (data.overlap * moveScaleB)));

	data.bodyA->ApplyForce(data.normal * (j), data.point);
	data.bodyB->ApplyForce(data.normal * (j) * -1, data.point);
}

void CheckCollisions(const std::vector<p2Fixture*> fixtures)
{
	size_t fixtureCount = fixtures.size();
	for (int f1 = 0; f1 < fixtureCount - 1; f1++)
	{
		if (fixtures[f1] == nullptr)
			continue;
		for (int f2 = f1 + 1; f2 < fixtureCount; f2++)
		{
			if (fixtures[f2] == nullptr)
				continue;
			CollisionData data;
			int collisionId = fixtures[f1]->GetShape()->GetShapeID() + fixtures[f2]->GetShape()->GetShapeID();
			if (collisionId > 4)
				continue;
			collisionsFunctionArray[collisionId](&data, fixtures[f1]->GetShape(), fixtures[f1]->GetBodyPos(), fixtures[f2]->GetShape(), fixtures[f2]->GetBodyPos());
			if (data.collision)
			{
				data.normal = data.normal / Vector2::Length(data.normal);
				
				data.relativeVelocity = fixtures[f1]->GetBodyVelocity() - fixtures[f2]->GetBodyVelocity();
				data.bodyA = fixtures[f1]->GetBody();
				data.bodyB = fixtures[f2]->GetBody();
				data.bodyA->m_isColliding = true;
				data.bodyB->m_isColliding = true;
				data.bodyA->m_collision = data.bodyB;
				data.bodyB->m_collision = data.bodyA;
				ResolveCollision(data);
			}
		}
	}
}

void p2CollideCircles(CollisionData* data, const p2Shape* circleA, const Vector2& bodyPosA, const p2Shape * circleB, const Vector2& bodyPosB)
{
	const p2CircleShape* cA = dynamic_cast<const p2CircleShape*>(circleA);
	const p2CircleShape* cB = dynamic_cast<const p2CircleShape*>(circleB);

	Vector2 diff = (bodyPosB + cB->m_pos) - (bodyPosA + cA->m_pos);
	float length = Vector2::Length(diff);
	float intersect = cA->m_radius + cB->m_radius - length;

	if (intersect > 0)
	{
		data->collision = true;
		data->normal = diff;
		Vector2 norm = diff / length;
		data->point = norm * cA->m_radius;
		data->overlap = intersect;
	}
}

void p2CollidePolygonAndCircle(CollisionData* data, const p2Shape * polygonA, const Vector2& bodyPosA, const p2Shape * circleB, const Vector2& bodyPosB)
{
	const p2PolygonShape* pA = dynamic_cast<const p2PolygonShape*>(polygonA);
	if (!pA)
	{
		p2CollidePolygonAndCircle(data, circleB, bodyPosB, polygonA, bodyPosA);
		return;
	}

	const p2CircleShape* cB = dynamic_cast<const p2CircleShape*>(circleB);

	for (int i = 0; i < pA->GetVertexCount(); i++)
	{
		p2EdgeShape edge;
		edge.Setp1(bodyPosA + pA->GetVertex(i));
		edge.Setp2(bodyPosA + pA->GetVertex((i == pA->GetVertexCount() - 1)?0:i + 1));
		p2CollideEdgeAndCircle(data, &edge, Vector2(0,0), circleB, bodyPosB);
		if (data->collision)
		{
			data->normal *= -1;
			return;
		}
	}
}

void p2CollidePolygons(CollisionData* data, const p2Shape * polygonA, const Vector2& bodyPosA, const p2Shape * polygonB, const Vector2& bodyPosB)
{
	const p2PolygonShape* pA = dynamic_cast<const p2PolygonShape*>(polygonA);
	const p2PolygonShape* pB = dynamic_cast<const p2PolygonShape*>(polygonB);

	//if (pA->GetVertexCount() < pB->GetVertexCount())
	//{
	//	const p2PolygonShape* b = pB;
	//	pB = pA;
	//	pA = b;
	//}

	bool isColliding = false;
	float overlap1 = 9999;
	float overlap2 = 9999;
	Vector2 collisionNorm;
	Vector2 point;
	float minOverlap = 99999;
	Vector2 collisionPointA;
	for (int i = 0; i < pA->GetVertexCount() + pB->GetVertexCount(); i++)
	{
		p2EdgeShape edge;
		if (i < pA->GetVertexCount())
		{
			edge.Setp1(pA->GetVertex(i));
			int i2 = (i == pA->GetVertexCount() - 1) ? 0 : i + 1;
			edge.Setp2(pA->GetVertex(i2));
		}
		else
		{
			edge.Setp1(pB->GetVertex(i - pA->GetVertexCount()));
			int i2 = (i == pA->GetVertexCount() + pB->GetVertexCount() - 1) ? 0 : i - pA->GetVertexCount() + 1;
			edge.Setp2(pB->GetVertex(i2));
		}
		Vector2 dir = Vector2::Normalize(edge.Getp2() - edge.Getp1());

		Vector2 normal = Vector2(dir.y, -dir.x);
		
		float highestProjA;
		float lowestProjA;
		Vector2 point;

		float proj = Vector2::Dot(pA->GetVertex(0) + bodyPosA, normal);
		lowestProjA = proj;
		highestProjA = proj;
		Vector2 highestPointA;
		Vector2 lowestPointA;
		for (int i = 1; i < pA->GetVertexCount(); i++)
		{
			proj = Vector2::Dot(pA->GetVertex(i) + bodyPosA, normal);
			if (proj > highestProjA)
			{
				highestProjA = proj;
				highestPointA = pA->GetVertex(i);
			}
			if (proj < lowestProjA)
			{
				lowestProjA = proj;
				lowestPointA = pA->GetVertex(i);
			}
		}

		float highestProjB;
		float lowestProjB;
		proj = Vector2::Dot(pB->GetVertex(0) + bodyPosB, normal);
		lowestProjB = proj;
		highestProjB = proj;
		for (int i = 1; i < pB->GetVertexCount(); i++)
		{
			proj = Vector2::Dot(pB->GetVertex(i) + bodyPosB, normal);
			if (proj > highestProjB)
			{
				highestProjB = proj;
			}
			if (proj < lowestProjB)
			{
				lowestProjB = proj;
			}
		}

		bool seperated = (highestProjB < lowestProjA) || (highestProjA < lowestProjB);
		if (!seperated)
		{
			if (highestProjB > lowestProjA)
				overlap1 = abs(highestProjB - lowestProjA);
			if (highestProjA > lowestProjB)
				overlap2 = abs(highestProjA - lowestProjB);
			if (overlap1 < minOverlap)
			{
				collisionPointA = lowestPointA;
				collisionNorm = normal;
				minOverlap = overlap1;
			}
			if (overlap2 < minOverlap)
			{
				collisionPointA = highestPointA;
				collisionNorm = normal;
				minOverlap = overlap2;
			}
			isColliding = true;
		}
		else
		{
			isColliding = false;
			break;
		}
	}

	if (isColliding)
	{
		Vector2 dir = Vector2::Normalize(bodyPosB - bodyPosA);
		if (Vector2::Dot(dir, collisionNorm) < 0)
			collisionNorm *= -1;
		data->overlap = minOverlap;
		data->point = collisionPointA;
		data->normal = collisionNorm;
		data->collision = isColliding;
	}		
}

void p2CollideEdgeAndCircle(CollisionData* data, const p2Shape * edgeA, const Vector2& bodyPosA, const p2Shape * circleB, const Vector2& bodyPosB)
{
	const p2EdgeShape* eA = dynamic_cast<const p2EdgeShape*>(edgeA);
	if (!eA)
	{
		p2CollideEdgeAndCircle(data, circleB, bodyPosB, edgeA, bodyPosA);
		return;
	}

	const p2CircleShape* cB = dynamic_cast<const p2CircleShape*>(circleB);

	Vector2 bPos = bodyPosB + cB->m_pos;
	Vector2 lineDirNorm = eA->Getp2() - eA->Getp1();
	float lineLength = Vector2::Length(lineDirNorm);
	lineDirNorm /= Vector2::Length(lineDirNorm);
	Vector2 startToCircle = bPos - eA->Getp1();
	float proj = Vector2::Dot(startToCircle, lineDirNorm);
	Vector2 closestPoint;
	if (proj > lineLength)
		closestPoint = eA->Getp2();
	else if (proj < 0)
		closestPoint = eA->Getp1();
	else
		closestPoint = (eA->Getp1() + (lineDirNorm * proj));

	float overLap = Vector2::Length(closestPoint - bPos);
	if (overLap < cB->m_radius)
	{
		data->collision = true;
		data->normal = bodyPosB - closestPoint;
		data->point = closestPoint;
		data->overlap = cB->m_radius - overLap;
	}
}

void p2CollideEdgeAndPolygon(CollisionData* data, const p2Shape * edgeA, const Vector2& bodyPosA, const p2Shape * polygonB, const Vector2& bodyPosB)
{
	const p2EdgeShape* eA = dynamic_cast<const p2EdgeShape*>(edgeA);
	if (!eA)
	{
		p2CollideEdgeAndPolygon(data, polygonB, bodyPosB, edgeA, bodyPosA);
		return;
	}
	const p2PolygonShape* pB = dynamic_cast<const p2PolygonShape*>(polygonB);

	p2PolygonShape poly;
	poly.AddVertex(eA->Getp1());
	poly.AddVertex(eA->Getp2());
	
	const Vector2 dir = eA->Getp2() - eA->Getp1();
	Vector2 bpa = eA->Getp1() + (Vector2::Normalize(dir) * (Vector2::Length(dir) / 2));

	p2PolygonShape localEdge;
	localEdge.AddVertex(eA->Getp1() - bpa);
	localEdge.AddVertex(eA->Getp2() - bpa);
	p2CollidePolygons(data, &localEdge, bpa, polygonB, bodyPosB);

	//for (int i = 0; i < pB->GetVertexCount(); i++)
	//{
	//	p2EdgeShape e1;
	//	e1.Setp1(Vector2(pB->GetVertex(i)));
	//	e1.Setp2(Vector2(pB->GetVertex((i == pB->GetVertexCount() - 1)?(0):(i + 1))));
	//
	//	float x1 = eA->Getp1().x;
	//	float x2 = eA->Getp2().x;
	//	float x3 = e1.Getp1().x + bodyPosB.x;
	//	float x4 = e1.Getp2().x + bodyPosB.x;
	//	float y1 = eA->Getp1().y;
	//	float y2 = eA->Getp2().y;
	//	float y3 = e1.Getp1().y + bodyPosB.y;
	//	float y4 = e1.Getp2().y + bodyPosB.y;
	//
	//	float l1 = ((x4 - x3)*(y1 - y3) - (y4 - y3)*(x1 - x3)) / ((y4 - y3)*(x2 - x1) - (x4 - x3)*(y2 - y1));
	//	float l2 = ((x2 - x1)*(y1 - y3) - (y2 - y1)*(x1 - x3)) / ((y4 - y3)*(x2 - x1) - (x4 - x3)*(y2 - y1));
	//	if (l1 >= 0 && l1 <= 1 && l2 >= 0 && l2 <= 1)
	//	{
	//		data->collision = true;
	//		Vector2 intersection(x1 + (l1 * (x2 - x1)), y1 + (l1 * (y2 - y1)));
	//		data->normal = bodyPosB - intersection;

	//		return;
	//	}
	//}
}
