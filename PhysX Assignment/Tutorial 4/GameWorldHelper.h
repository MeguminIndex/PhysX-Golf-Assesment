#pragma once
#include "PhysicsEngine.h"

namespace PhysicsEngine
{

	class GameWorldHelper
	{

		

	public:
		static void ResetPosition(PxRigidBody* rb,PxTransform position)
		{
			//PxRigidBody* rb = ((PxRigidBody*)pairHeader.actors[0]);
			rb->setLinearVelocity(PxVec3(0.0f, 0.0f, 0.0f));
			rb->setGlobalPose(position);
		};


	private:


	};


}