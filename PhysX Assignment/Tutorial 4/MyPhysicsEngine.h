#pragma once

#include "BasicActors.h"
#include <iostream>
#include <iomanip>

#include<array>
#include<memory>


#include "GameWorldHelper.h"


namespace PhysicsEngine
{
	using namespace std;

#pragma region PremadesetupStuff
	//a list of colours: Circus Palette
	static const PxVec3 color_palette[] = {
		PxVec3(46.f / 255.f,9.f / 255.f,39.f / 255.f),
		PxVec3(217.f / 255.f,0.f / 255.f,0.f / 255.f),
		PxVec3(255.f / 255.f,45.f / 255.f,0.f / 255.f),
		PxVec3(255.f / 255.f,140.f / 255.f,54.f / 255.f),
		PxVec3(4.f / 255.f,117.f / 255.f,111.f / 255.f) };

	//pyramid vertices
	static PxVec3 pyramid_verts[] = { PxVec3(0,1,0), PxVec3(1,0,0), PxVec3(-1,0,0), PxVec3(0,0,1), PxVec3(0,0,-1) };
	//pyramid triangles: a list of three vertices for each triangle e.g. the first triangle consists of vertices 1, 4 and 0
	//vertices have to be specified in a counter-clockwise order to assure the correct shading in rendering
	static PxU32 pyramid_trigs[] = { 1, 4, 0, 3, 1, 0, 2, 3, 0, 4, 2, 0, 3, 2, 1, 2, 4, 1 };

	class Pyramid : public ConvexMesh
	{
	public:
		Pyramid(PxTransform pose = PxTransform(PxIdentity), PxReal density = 1.f) :
			ConvexMesh(vector<PxVec3>(begin(pyramid_verts), end(pyramid_verts)), pose, density)
		{
		}
	};

	class PyramidStatic : public TriangleMesh
	{
	public:
		PyramidStatic(PxTransform pose = PxTransform(PxIdentity)) :
			TriangleMesh(vector<PxVec3>(begin(pyramid_verts), end(pyramid_verts)), vector<PxU32>(begin(pyramid_trigs), end(pyramid_trigs)), pose)
		{
		}
	};

#pragma endregion

	struct FilterGroup
	{
		enum Enum
		{
			GOLFBALL	= (1 << 0),
			GROUND		= (1 << 1),
			GOLFHOLE	= (1 << 2)
			//add more if you need
		};
	};



	

	
	class MySimulationEventCallback : public PxSimulationEventCallback
	{
	public:
		//an example variable that will be checked in the main simulation loop
		bool trigger;

		bool resetBallPos;
		bool golfBallInHole;


		MySimulationEventCallback() : trigger(false), resetBallPos(false) {}

		///Method called when the contact with the trigger object is detected.
		virtual void onTrigger(PxTriggerPair* pairs, PxU32 count) 
		{
			//you can read the trigger information here
			for (PxU32 i = 0; i < count; i++)
			{
				//filter out contact with the planes
				if (pairs[i].otherShape->getGeometryType() != PxGeometryType::ePLANE)
				{
					//check if eNOTIFY_TOUCH_FOUND trigger
					if (pairs[i].status & PxPairFlag::eNOTIFY_TOUCH_FOUND)
					{
						cerr << "onTrigger::eNOTIFY_TOUCH_FOUND" << endl;
						trigger = true;
					}
					//check if eNOTIFY_TOUCH_LOST trigger
					if (pairs[i].status & PxPairFlag::eNOTIFY_TOUCH_LOST)
					{
						cerr << "onTrigger::eNOTIFY_TOUCH_LOST" << endl;
						trigger = false;
					}
				}
			}
		}

		///Method called when the contact by the filter shader is detected.
		virtual void onContact(const PxContactPairHeader &pairHeader, const PxContactPair *pairs, PxU32 nbPairs)
		{
			cerr << "Contact found between " << pairHeader.actors[0]->getName() << " & " << pairHeader.actors[1]->getName() << endl;

			//check all pairs
			for (PxU32 i = 0; i < nbPairs; i++)
			{
				//check eNOTIFY_TOUCH_FOUND
				if (pairs[i].events & PxPairFlag::eNOTIFY_TOUCH_FOUND)
				{
					cerr << "onContact::eNOTIFY_TOUCH_FOUND" << endl;

					if (strcmp(pairHeader.actors[0]->getName(), "GolfBall") == 0 && (strcmp(pairHeader.actors[1]->getName(), "Ground") == 0))
						resetBallPos = true;
					if (strcmp(pairHeader.actors[0]->getName(), "GolfBall") == 0 && (strcmp(pairHeader.actors[1]->getName(), "Hole1") == 0))
						golfBallInHole = true;


					
				}
				//check eNOTIFY_TOUCH_LOST
				if (pairs[i].events & PxPairFlag::eNOTIFY_TOUCH_LOST)
				{
					cerr << "onContact::eNOTIFY_TOUCH_LOST" << endl;
				}
			}
		}

		virtual void onConstraintBreak(PxConstraintInfo *constraints, PxU32 count) {}
		virtual void onWake(PxActor **actors, PxU32 count) {}
		virtual void onSleep(PxActor **actors, PxU32 count) {}
	};


	//class MyCCDEventCallBack : public PxCCDContactModifyCallback
	//{
	//public:
	//	

	//	// Inherited via PxCCDContactModifyCallback
	//	virtual void onCCDContactModify(PxContactModifyPair * const pairs, PxU32 count) override
	//	{

	//		cerr << "CCD Callback" << endl;


	//	};

	//};


	//A simple filter shader based on PxDefaultSimulationFilterShader - without group filtering
	static PxFilterFlags CustomFilterShader( PxFilterObjectAttributes attributes0,	PxFilterData filterData0,
		PxFilterObjectAttributes attributes1,	PxFilterData filterData1,
		PxPairFlags& pairFlags,	const void* constantBlock,	PxU32 constantBlockSize)
	{
		// let triggers through
		if(PxFilterObjectIsTrigger(attributes0) || PxFilterObjectIsTrigger(attributes1))
		{
			pairFlags = PxPairFlag::eTRIGGER_DEFAULT;
			return PxFilterFlags();
		}

		pairFlags = PxPairFlag::eCONTACT_DEFAULT;
	
		//pairFlags |= PxPairFlag::eMODIFY_CONTACTS;
		//enable continous collision detection
		pairFlags |= PxPairFlag::eCCD_LINEAR;
		/*pairFlags |= PxPairFlag::eDETECT_CCD_CONTACT;
		pairFlags |= PxPairFlag::eSOLVE_CONTACT;
		pairFlags |= PxPairFlag::eDETECT_DISCRETE_CONTACT;*/

		//customise collision filtering here
		//e.g.

		// trigger the contact callback for pairs (A,B) where 
		// the filtermask of A contains the ID of B and vice versa.
		if((filterData0.word0 & filterData1.word1) && (filterData1.word0 & filterData0.word1))
		{
			//trigger onContact callback for this pair of objects
			pairFlags |= PxPairFlag::eNOTIFY_TOUCH_FOUND;
			pairFlags |= PxPairFlag::eNOTIFY_TOUCH_LOST;
//			pairFlags |= PxPairFlag::eNOTIFY_CONTACT_POINTS;
		}

		return PxFilterFlags();
	};


	class WBKScene : public Scene
	{
		

		Plane* plane;		
		MySimulationEventCallback* my_callback;
	  //MyCCDEventCallBack* myCCDcallback;

		Sphere* golfBall;
		Box* ballDirectionObj;
		PxVec3 directionVisOffset = PxVec3(0.0f,0.0f,0.01f);

		PxQuat rotation;	//rotation to fire goldball

		PxReal minPower = 10.0f;
		PxReal maxPower = 150.0f;;
		PxReal powerIncVal = 10.0f;
		PxReal power = 100.0f;//force to hit golfball with
		//PxVec3 lastBallPos = PxVec3(0.0f,0.0f,0.7f);

	

#pragma region levelVariables

		vector<Box*> boxes;				
		//Box infomation variables
			vector<PxVec3> positions = {
			PxVec3( 0.0f, 0.3f,-10.0f),
			PxVec3( 5.0f, 0.3f, 100.0f),
			PxVec3(-5.0f, 0.3f, 100.0f),
			PxVec3( 0.0f, 0.3f, 100.0f),
			PxVec3(0.0f, 1.0f, 60.0f)
			
			};
			vector<PxVec3> dimensions = {
				PxVec3( 5.0f, 1.0f, 0.2f),
				PxVec3( 0.2f, 1.0f, 110.0f),
				PxVec3( 0.2f, 1.0f, 110.0f),
				PxVec3( 5.0f, 0.2f, 110.0f),
				PxVec3(5.0f, 0.2f, 3.0f)
				
			};
			vector<PxQuat> mapRotations = {
				PxQuat(PxIdentity),
				PxQuat(PxIdentity),
				PxQuat(PxIdentity),
				PxQuat(PxIdentity),
				PxQuat(-0.349066,PxVec3(1.0f,0.0f,0.0f))

			};


			Cloth* cloth;

			CompoundHole* hole1;
			
			//WINDMILL
			CompoundWindMill* windMill;
			RevoluteJoint* jointRevBladeOne;
			RevoluteJoint* jointRevBladeTwo;
			Box* bladeOne;
			Box* bladeTwo;
			//END WINDMILL

			PyramidStatic* pyramidObsitcle;

#pragma endregion



	public:
		GameWorldHelper gameWorldH;

		//specify your custom filter shader here
		//PxDefaultSimulationFilterShader by default
		WBKScene() : Scene(CustomFilterShader) {};

		
		void SetVisualisation()
		{
			px_scene->setVisualizationParameter(PxVisualizationParameter::eSCALE, 1.0f);
			px_scene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_SHAPES, 1.0f);
			px_scene->setVisualizationParameter(PxVisualizationParameter::eACTOR_AXES,1.0f);
			px_scene->setVisualizationParameter(PxVisualizationParameter::eBODY_LIN_VELOCITY, 1.0f);
			
			//cloth visualisation
			px_scene->setVisualizationParameter(PxVisualizationParameter::eCLOTH_HORIZONTAL, 1.0f);
			px_scene->setVisualizationParameter(PxVisualizationParameter::eCLOTH_VERTICAL, 1.0f);
			px_scene->setVisualizationParameter(PxVisualizationParameter::eCLOTH_BENDING, 1.0f);
			px_scene->setVisualizationParameter(PxVisualizationParameter::eCLOTH_SHEARING, 1.0f);


			//enable Continmuous COllision detection in my scene
			px_scene->setFlag(PxSceneFlag::eENABLE_CCD,true);
		
	

		}

		
		virtual void CustomInit()
		{
			SetVisualisation();
			//GetMaterial()->setStaticFriction(1.54f);
			GetMaterial()->setDynamicFriction(.2f);
			GetMaterial()->setRestitution(0.2f);
		//	GetMaterial()->setFrictionCombineMode(PxCombineMode::e);
			//Initialise and set the customised event callback
			my_callback = new MySimulationEventCallback();
			px_scene->setSimulationEventCallback(my_callback);
			//myCCDcallback = new MyCCDEventCallBack();
			//px_scene->setCCDContactModifyCallback(myCCDcallback);

		//	gameWorldH = GameWorldHelper();
		
			
			

			//create plane for floor
			plane = new Plane();
			plane->Name("Ground");
			plane->Color(PxVec3(210.f / 255.f, 210.f / 255.f, 210.f / 255.f));
			plane->SetupFiltering(FilterGroup::GROUND, FilterGroup::GOLFBALL);
			Add(plane);

			

			//create goldball
			golfBall = new Sphere(PxTransform(PxVec3(0.0f, 5.5f,0.0f)),0.2f);
			golfBall->Color(color_palette[0]);
			golfBall->Name("GolfBall");
			((PxRigidBody*)golfBall->Get())->setRigidBodyFlag(PxRigidBodyFlag::eENABLE_CCD, true);
			((PxRigidBody*)golfBall->Get())->isRigidDynamic()->setAngularDamping(.8f);

			Add(golfBall);
			golfBall->SetupFiltering(FilterGroup::GOLFBALL,FilterGroup::GROUND | FilterGroup::GOLFHOLE);

			

			//init rotation
			rotation = PxQuat(1.5708f,PxVec3(1.0f,0.0f,0.0f));

			//create the object used to indicate ball firing direction
			ballDirectionObj = new Box(PxTransform(PxVec3(0.0f, 0.5f, 0.0f)),PxVec3(0.05f, 1.05f, 0.05f),0.01f);
			ballDirectionObj->SetKinematic(true);
			ballDirectionObj->SetTrigger(true);
			
			ballDirectionObj->Color(color_palette[0]);
			Add(ballDirectionObj);




			//initialise level;
			InitLevel();
		}

		//Custom udpate function
		virtual void CustomUpdate(PxReal dt)
		{


			PxRigidBody* golfBallRB = (PxRigidBody*)golfBall->Get();


			//update the position of the reticle/direction visual
			PxTransform pose = golfBallRB->getGlobalPose();
			//pose.p += directionVisOffset;
			pose.q = rotation;
				((PxRigidBody*)ballDirectionObj->Get())->setGlobalPose(pose);
			

				if (my_callback->resetBallPos)
				{
					cout << "Reset Ball position" << endl;

					GameWorldHelper::ResetPosition(golfBallRB, PxTransform(gameWorldH.lastBallPos));
					my_callback->resetBallPos = false;

				}

				if (golfBallRB->getLinearVelocity().magnitude() < 0.6f)
				{
					golfBallRB->setAngularVelocity(PxVec3(0.0f, 0.0f, 0.0f));//hacky way to solve infinate roll problem 
					//golfBallRB->setLinearVelocity(PxVec3(0.0f,0.0f,0.0f));
				}

				//wcout << "GolfBall Lin Vel:" << endl;
				//GameWorldHelper::PrintVector(golfBallRB->getAngularVelocity());

				if(golfBallRB->getAngularVelocity().isZero())
				{
				//	cout << "Update Golfballs Last Position" << endl;
					gameWorldH.lastBallPos = (golfBallRB->getGlobalPose()).p;
				}

			/*	if (golfBallRB->isRigidDynamic()->isSleeping())
				{
					cout << "Update Golfballs Last Position" << endl;
					gameWorldH.lastBallPos = (golfBallRB->getGlobalPose()).p;
				}*/
		//		cout << "GolfBALL Vel:: X:" << golfBallRB->getLinearVelocity().x <<" Y: " <<
		//			golfBallRB->getLinearVelocity().y<< " Z: " << golfBallRB->getLinearVelocity().z << endl;



				if (my_callback->golfBallInHole)
				{
					cout << "Golf Ball is in the Hole!" << endl;

					my_callback->golfBallInHole = false;
				}



		}

		/// An example use of key release handling
		void ExampleKeyReleaseHandler(int key)
		{
		
			switch (toupper(key))
			{
				//implement your own
				case 'R':
					break;

				default:
					break;
			}

		}

		/// An example use of key presse handling
		void ExampleKeyPressHandler(int key)
		{

			switch (toupper(key))
			{
				case 'Q':
				{
					PxTransform pose = ((PxRigidBody*)ballDirectionObj->Get())->getGlobalPose();
					rotation *= PxQuat(0.1f, PxVec3(0.0f, 0.f, 1.0f));
					pose.q = rotation;
					((PxRigidBody*)ballDirectionObj->Get())->setGlobalPose(pose);

					//cerr <<"Ball Rotation: " << " X: " + std::to_string(pose.q.x) << " Y: " + std::to_string(pose.q.y) << " Z: " + std::to_string(pose.q.z) << endl;

					break;
				}
				case 'E':
				{
					PxTransform pose = ((PxRigidBody*)ballDirectionObj->Get())->getGlobalPose();
					rotation *= PxQuat(-0.1f, PxVec3(0.0f, 0.f, 1.0f));
					pose.q = rotation;
					((PxRigidBody*)ballDirectionObj->Get())->setGlobalPose(pose);

				//	cerr << "Ball Rotation: " << " X: " + std::to_string(pose.q.x) << " Y: " + std::to_string(pose.q.y) << " Z: " + std::to_string(pose.q.z) << endl;

					break;
				}

				case 'F':
				{

					PxVec3 direction = rotation.getBasisVector1();
					((PxRigidBody*)golfBall->Get())->addForce(direction*power);
					cerr << "Ball Direct??: " << " X: " + std::to_string(direction.x) << " Y: " + std::to_string(direction.y) << " Z: " + std::to_string(direction.z) << endl;

					gameWorldH.player1Strokes += 1;
					cout << "Number of Strokes: " << gameWorldH.player1Strokes << endl;
					break;
				}
				case 'R':
				{
					//((PxRigidBody*)golfBall->Get())->setGlobalPose(PxTransform(PxVec3(0.0f,1.5f,0.0f)));
					GameWorldHelper::ResetPosition((PxRigidBody*)golfBall->Get(), PxTransform(gameWorldH.lastBallPos));
					break;
				}

				case 'G':
				{
					cout << "Hitting Power incrimented to:  " << power << endl;
					power += powerIncVal;
					if (power > maxPower)
						power = maxPower;

					break;
				}
				case 'H':
				{
					cout << "Hitting Power incrimented to:  " << power << endl;
					power -= powerIncVal;
					if (power < minPower)
						power = minPower;
					break;
				}

			}
		}


		void InitLevel()
		{
			

			int sizeP = positions.size();
			cerr << "Number of box positions: " << sizeP << endl;
			for (int i = 0; i < sizeP; i++)
			{
				Box* tmpBox;
				if(i > dimensions.size()-1)
					tmpBox = new Box(PxTransform(positions[i], mapRotations[i]));
				else
				tmpBox = new Box(PxTransform(positions[i], mapRotations[i]), dimensions[i]);

				tmpBox->Color(color_palette[4]);
				tmpBox->SetKinematic(true);
				boxes.push_back(tmpBox);
				Add(boxes[i]);


			}


			pyramidObsitcle = new PyramidStatic(PxTransform(0.0f, 0.4f, 3.0f));
			Add(pyramidObsitcle);

			cloth = new Cloth(PxTransform(PxVec3(-5.0f, 11.0f, 80.0f)), PxVec2(10.f, 10.0), 40, 40);
			cloth->Color(color_palette[2]);
			((PxCloth*)cloth->Get())->setClothFlag(PxClothFlag::eSWEPT_CONTACT, true);

		//	cloth->SetTrigger(false);

			Add(cloth);

			((PxCloth*)cloth->Get())->setStretchConfig(PxClothFabricPhaseType::eVERTICAL, PxClothStretchConfig(1.0f));
			((PxCloth*)cloth->Get())->setStretchConfig(PxClothFabricPhaseType::eHORIZONTAL, PxClothStretchConfig(0.9f));
			((PxCloth*)cloth->Get())->setStretchConfig(PxClothFabricPhaseType::eSHEARING, PxClothStretchConfig(0.75f));
			((PxCloth*)cloth->Get())->setStretchConfig(PxClothFabricPhaseType::eBENDING, PxClothStretchConfig(0.5f));

			//WINDMILL
			windMill = new CompoundWindMill(PxTransform(0.0f,6.5f,140.0f, PxQuat(3.14159f, PxVec3(0.0f, 1.0f, 0.0f))));
			windMill->SetKinematic(true);

			//WINDMILL BLADES

		
			PxTransform windMillTrans = ((PxRigidBody*)windMill->Get())->getGlobalPose();
			windMillTrans.p.z += 2.0f;
			windMillTrans.p.y += 5.0f;

			PxVec3 bladesDim = PxVec3(0.1f, 10.f, 1.1f);

			bladeOne = new Box(windMillTrans, bladesDim);
			
			PxQuat rot = windMillTrans.q;
			windMillTrans.q *= PxQuat(1.5708f, PxVec3(0.0f, 0.0f, 1.0f));
			((PxRigidBody*)bladeOne->Get())->setGlobalPose(windMillTrans);


			//propella JOINTS
			jointRevBladeOne = new RevoluteJoint(windMill, PxTransform(PxVec3(0.f, 5.f, 4.f), PxQuat(PxPi / 2, PxVec3(0.f, 1.f, 0.f))), bladeOne, PxTransform(PxVec3(0.f, 0.f, 0.f)));
			

			jointRevBladeOne->DriveVelocity(10.0f);
		

			Add(windMill);
			Add(bladeOne);
			

			hole1 = new CompoundHole(PxTransform(PxVec3(0.0f,0.1f,214.0f)));
			hole1->SetKinematic(true);
			hole1->Color(color_palette[4]);

			hole1->SetupFiltering(FilterGroup::GOLFHOLE,FilterGroup::GOLFBALL,0);//set the filtering so when the golfball hits shape 0 (the floor of the hole object) we get a collision event;
			hole1->Name("Hole1");
			Add(hole1);

		}

	};
}
