#pragma once
#include "PhysicsEngine.h"

namespace PhysicsEngine
{

	class GameWorldHelper
	{

		

	public:

		GameWorldHelper()
		{
			cout << "GameWorldConstruct" << endl;
			lastBallPos = PxVec3(0.0f, 1.0f, 0.0f);
		};

		PxVec3 lastBallPos; 

		static void ResetPosition(PxRigidBody* rb,PxTransform position)
		{			
			
			rb->setLinearVelocity(PxVec3(0.0f, 0.0f, 0.0f));
			rb->setGlobalPose(position);

			cout << rb->getName() << "Positition reset X: " << rb->getGlobalPose().p.x <<" Y: " << rb->getGlobalPose().p.y<< " Z: " << rb->getGlobalPose().p.z << endl;
		};


		static void PrintVector(PxVec3 vector)
		{
			cout << "X : "<<vector.x<<" Y :" << vector.y <<" Z :" <<vector.z << endl;

		}

	private:


	};


}