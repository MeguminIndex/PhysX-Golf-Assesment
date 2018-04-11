#pragma once
#include "BasicActors.h"

namespace PhysicsEngine
{


	class MovingObject
	{
	public:

		Box* movingObj;
		PxReal maxDistOffSet = 1.0f;
		PxReal speed = 0.8f;
		MovingObject(const PxTransform& pose = PxTransform(PxIdentity), PxVec3 dimensions = PxVec3(.5f, .5f, .5f), PxReal density = 1.f){
		
			cout << "Constructing MovingBall Object" << endl;

			movingObj = new Box(pose, dimensions, density);
			movingObj->SetKinematic(true);
			movingObj->Name("MovingBox");

			initPos = pose.p;

			mBox01 = (PxRigidBody*)movingObj->Get();

		};
		~MovingObject()
		{
		
		};

	
		void Update(PxReal dt)
		{
			//PxRigidBody* mBox01 = (PxRigidBody*)movingObj->Get();
			PxTransform trans = mBox01->getGlobalPose();

			if (trans.p.x < initPos.x + maxDistOffSet && hitEnd == false)
				trans.p.x += speed * dt;
			else
				hitEnd = true;

			if (hitEnd == true && trans.p.x > initPos.x - maxDistOffSet)
				trans.p.x -= speed * dt;
			else
				hitEnd = false;

			mBox01->setGlobalPose(trans);
		};

		void ResetInitPos(PxVec3 newPos)
		{
			initPos = newPos;
		};

	private:

		PxVec3 initPos;
		PxRigidBody* mBox01;
		bool hitEnd = false;

	};

}

