#include "SimpleAI.h"
#include "Debug.h"
#include "../CSC8503Common/GameWorld.h"
#include "../../Common/Maths.h"
#define _USE_MATH_DEFINES
#include <math.h>

#define DRAW_DEBUG true

using namespace NCL;
using namespace CSC8503;

void SimpleAI::Update(float dt)
{
	if (target) {
		Vector3 force;
		force = Arrive(target->GetTransform().GetPosition());

		Vector3 magnitudeDir = Vector3::Cross(physicsObject->GetLinearVelocity(), Vector3(0, 1, 0)).Normalised() * avoid_magnitude;
		Debug::DrawLine(transform.GetPosition(), transform.GetPosition() + magnitudeDir.Normalised() * avoid_distance, Debug::BLUE);
		float magnitude = Avoid();
		//Vector3 newForce = (force * (1.0f - magnitude)) + (magnitudeDir * magnitude);
		//if (magnitude != 0.0f)
		//	physicsObject->AddForce(magnitudeDir * magnitude);
		//else
			physicsObject->AddForce((force * (1.0f - magnitude)) + (magnitudeDir * magnitude));
		// Forwards
		//Debug::DrawLine(transform.GetPosition(), transform.GetPosition() + physicsObject->GetLinearVelocity().Normalised() * avoid_distance, Debug::WHITE);
		//Debug::DrawLine(transform.GetPosition(), transform.GetPosition() + (magnitudeDir * magnitude), Debug::RED);

		RotateToVelocity();
	}
}

void SimpleAI::OnCollisionBegin(GameObject* otherObject, Vector3 localA, Vector3 localB, Vector3 normal)
{
	if (otherObject->GetName() == "projectile") {
		health -= 10.0f;
		if (health <= 0.0f) {
			health = 100.0f;
			transform.SetPosition(Vector3(0,3.0f,0));
		}
	}
}

Vector3 SimpleAI::Seek(Vector3 seekTarget)
{
	Vector3 force = (seekTarget - transform.GetPosition()).Normalised() * max_speed;
	force -= physicsObject->GetLinearVelocity();

	if (force.Length() > max_force) {
		force = force.Normalised() * max_force;
	}

	force.y = 0.0f;
	return force;
}

Vector3 SimpleAI::Arrive(Vector3 arriveTarget)
{
	Vector3 force = (arriveTarget - transform.GetPosition()).Normalised() * max_speed;
	force -= physicsObject->GetLinearVelocity();

	if (force.Length() > max_force) {
		force = force.Normalised() * max_force;
	}
	
	force.y = 0.0f;
	
	float distance = (arriveTarget - transform.GetPosition()).Length();
	float multiply = distance / arrive_radius;
	if (multiply > 1.0f)
		multiply = 1.0f;

	force *= multiply;

#if DRAW_DEBUG
	Debug::DrawSphere(arriveTarget, arrive_radius, Debug::RED);
#endif

	return force;
}

float SimpleAI::Avoid()
{
	// Raycast ahead, if it hits something, raycast to the sides, whichever gets the furthest, go in that direction.
	Ray forwardRay(transform.GetPosition(), physicsObject->GetLinearVelocity().Normalised() + Vector3(0,0.0000000000000001f,0));
	forwardRay.SetCollisionLayers(CollisionLayer::LAYER_ONE);
	RayCollision forwardCollision;

	if (!GameWorld::RaycastIgnoreObject(this, forwardRay, forwardCollision, true)) {
		return 0.0f;
	}
	float distance = (forwardCollision.collidedAt - transform.GetPosition()).Length();
	if (distance > avoid_distance) {
		return 0.0f;
	}

	float magnitude = 1.0f - (forwardCollision.collidedAt - transform.GetPosition()).Length() / avoid_distance;
	magnitude = Maths::Clamp(magnitude, 0.3f, 1.0f);

	Vector3 leftAngle = (Matrix4::Rotation(-45, Vector3(0, 1, 0)) * Matrix4::Translation(physicsObject->GetLinearVelocity())).GetPositionVector();
	Ray leftRay(transform.GetPosition(), leftAngle.Normalised() + Vector3(0, 0.000000000000000001f, 0));
	leftRay.SetCollisionLayers(CollisionLayer::LAYER_ONE);
	RayCollision leftCollision;
	bool leftHit = GameWorld::RaycastIgnoreObject(this, leftRay, leftCollision, true);

	Vector3 rightAngle = (Matrix4::Rotation(45, Vector3(0, 1, 0)) * Matrix4::Translation(physicsObject->GetLinearVelocity())).GetPositionVector();
	Ray rightRay(transform.GetPosition(), rightAngle.Normalised() + Vector3(0,0.000000000000000001f,0));
	rightRay.SetCollisionLayers(CollisionLayer::LAYER_ONE);
	RayCollision rightCollision;
	bool rightHit = GameWorld::RaycastIgnoreObject(this, rightRay, rightCollision, true);
	
	if (!leftHit && !rightHit)
		return 1.0f;
	else if (!leftHit)
		return 1.0f;
	else if (!rightHit)
		return -1.0f;

	return magnitude * (leftCollision.rayDistance < rightCollision.rayDistance ? -1.0f : 1.0f);
}

Vector3 SimpleAI::Pursue()
{
	Vector3 targetPos = target->GetTransform().GetPosition() + (target->GetPhysicsObject()->GetLinearVelocity().Normalised() * max_predict);

#if DRAW_DEBUG
	Debug::DrawSphere(targetPos, 1.0f, Debug::RED);
#endif

	return Seek(targetPos);
}

void SimpleAI::RotateToVelocity()
{
	float angle = acos(Vector3::Dot(physicsObject->GetLinearVelocity().Normalised(), Vector3(0, 0, -1)));
	angle *= (180.0f / M_PI);
	if (physicsObject->GetLinearVelocity().x > 0.0f)
		angle *= -1;
	transform.SetOrientation(Quaternion::AxisAngleToQuaterion(Vector3(0, 1, 0), angle));
}
