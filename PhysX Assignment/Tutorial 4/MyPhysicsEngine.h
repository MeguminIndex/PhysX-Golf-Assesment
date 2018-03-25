#pragma once

#include "BasicActors.h"
#include <iostream>
#include <iomanip>

namespace PhysicsEngine
{
	using namespace std;

	//a list of colours: Circus Palette
	static const PxVec3 color_palette[] = {PxVec3(46.f/255.f,9.f/255.f,39.f/255.f),PxVec3(217.f/255.f,0.f/255.f,0.f/255.f),
		PxVec3(255.f/255.f,45.f/255.f,0.f/255.f),PxVec3(255.f/255.f,140.f/255.f,54.f/255.f),PxVec3(4.f/255.f,117.f/255.f,111.f/255.f)};

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




	struct FilterGroup
	{
		enum Enum
		{
			ACTOR0		= (1 << 0),
			ACTOR1		= (1 << 1),
			ACTOR2		= (1 << 2)
			//add more if you need
		};
	};

	///A customised collision class, implemneting various callbacks
	class MySimulationEventCallback : public PxSimulationEventCallback
	{
	public:
		//an example variable that will be checked in the main simulation loop
		bool trigger;

		MySimulationEventCallback() : trigger(false) {}

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
			cerr << "Contact found between " << pairHeader.actors[0]->getName() << " " << pairHeader.actors[1]->getName() << endl;

			//check all pairs
			for (PxU32 i = 0; i < nbPairs; i++)
			{
				//check eNOTIFY_TOUCH_FOUND
				if (pairs[i].events & PxPairFlag::eNOTIFY_TOUCH_FOUND)
				{
					cerr << "onContact::eNOTIFY_TOUCH_FOUND" << endl;
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
		//enable continous collision detection
//		pairFlags |= PxPairFlag::eCCD_LINEAR;
		
		
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

	///Custom scene class
	class MyScene : public Scene
	{
		Plane* plane;
		Cloth* cloth;
		Box* box;
		MySimulationEventCallback* my_callback;
		
	public:
		//specify your custom filter shader here
		//PxDefaultSimulationFilterShader by default
		MyScene() : Scene() {};

		///A custom scene class
		void SetVisualisation()
		{
			px_scene->setVisualizationParameter(PxVisualizationParameter::eSCALE, 1.0f);
			px_scene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_SHAPES, 1.0f);

			//cloth visualisation
			px_scene->setVisualizationParameter(PxVisualizationParameter::eCLOTH_HORIZONTAL, 1.0f);
			px_scene->setVisualizationParameter(PxVisualizationParameter::eCLOTH_VERTICAL, 1.0f);
			px_scene->setVisualizationParameter(PxVisualizationParameter::eCLOTH_BENDING, 1.0f);
			px_scene->setVisualizationParameter(PxVisualizationParameter::eCLOTH_SHEARING, 1.0f);
		}

		//Custom scene initialisation
		virtual void CustomInit() 
		{
			SetVisualisation();			

			GetMaterial()->setDynamicFriction(.2f);

			///Initialise and set the customised event callback
			my_callback = new MySimulationEventCallback();
			px_scene->setSimulationEventCallback(my_callback);

			plane = new Plane();
			plane->Color(PxVec3(210.f/255.f,210.f/255.f,210.f/255.f));
			Add(plane);

			cloth = new Cloth(PxTransform(PxVec3(-4.f,9.f,0.f)), PxVec2(8.f,8.f), 40, 40);
			cloth->Color(color_palette[2]);
			Add(cloth);

			box = new Box(PxTransform(PxVec3(0.f,2.f,0.f)),PxVec3(2.f,2.f,2.f));
			box->Color(color_palette[3]);
			Add(box);

			//setting custom cloth parameters
			//((PxCloth*)cloth->Get())->setStretchConfig(PxClothFabricPhaseType::eBENDING, PxClothStretchConfig(1.f));
			((PxCloth*)cloth->Get())->setStretchConfig(PxClothFabricPhaseType::eVERTICAL, PxClothStretchConfig(1.0f));
			((PxCloth*)cloth->Get())->setStretchConfig(PxClothFabricPhaseType::eHORIZONTAL, PxClothStretchConfig(0.9f));
			((PxCloth*)cloth->Get())->setStretchConfig(PxClothFabricPhaseType::eSHEARING, PxClothStretchConfig(0.75f));
			((PxCloth*)cloth->Get())->setStretchConfig(PxClothFabricPhaseType::eBENDING, PxClothStretchConfig(0.5f));
			

		}

		//Custom udpate function
		virtual void CustomUpdate(PxReal dt)
		{
			PxTransform posT = ((PxCloth*)cloth->Get())->getGlobalPose();
			posT.p.x += 0.1;

			//Setting GlobalPos
			//((PxCloth*)cloth->Get())->setGlobalPose(posT);
			//((PxCloth*)cloth->Get())->setTargetPose(posT);
		}

		/// An example use of key release handling
		void ExampleKeyReleaseHandler(int key)
		{
			cerr << "I am realeased!" << endl;

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
			cerr << "I am pressed!" << endl;

			switch (toupper(key))
			{
			case 'R':
			{
				PxTransform posT = ((PxCloth*)cloth->Get())->getGlobalPose();
				posT.p.x += 0.8;
				//((PxCloth*)cloth->Get())->setGlobalPose(posT);
				((PxCloth*)cloth->Get())->setTargetPose(posT);
				break;
			}
			case 'F':
			{
				((PxCloth*)cloth->Get())->setExternalAcceleration(PxVec3(5.0f, 0.0f, 0.0f));
				break;
			}
			}
		}
	};


	class WBKScene : public Scene
	{
		Plane* plane;		
		MySimulationEventCallback* my_callback;



		Sphere* golfBall;
		Box* box;
		PxQuat rotation;
		
		PxReal power = 10.0f;


	public:
		

		//specify your custom filter shader here
		//PxDefaultSimulationFilterShader by default
		WBKScene() : Scene() {};

		///A custom scene class
		void SetVisualisation()
		{
			px_scene->setVisualizationParameter(PxVisualizationParameter::eSCALE, 1.0f);
			px_scene->setVisualizationParameter(PxVisualizationParameter::eCOLLISION_SHAPES, 1.0f);

			//cloth visualisation
			px_scene->setVisualizationParameter(PxVisualizationParameter::eCLOTH_HORIZONTAL, 1.0f);
			px_scene->setVisualizationParameter(PxVisualizationParameter::eCLOTH_VERTICAL, 1.0f);
			px_scene->setVisualizationParameter(PxVisualizationParameter::eCLOTH_BENDING, 1.0f);
			px_scene->setVisualizationParameter(PxVisualizationParameter::eCLOTH_SHEARING, 1.0f);
		}

		//Custom scene initialisation
		virtual void CustomInit()
		{
			SetVisualisation();

			GetMaterial()->setDynamicFriction(.2f);

			///Initialise and set the customised event callback
			my_callback = new MySimulationEventCallback();
			px_scene->setSimulationEventCallback(my_callback);

			plane = new Plane();
			plane->Color(PxVec3(210.f / 255.f, 210.f / 255.f, 210.f / 255.f));
			Add(plane);


			golfBall = new Sphere(PxTransform(PxVec3(0.0f, 0.5f,0.0f)),0.2f);
			golfBall->Color(color_palette[0]);
			Add(golfBall);

			rotation = PxQuat(1.5708f,PxVec3(1.0f,0.0f,0.0f));

			box = new Box(PxTransform(PxVec3(0.0f, 0.5f, 0.0f)),PxVec3(0.05f, 1.05f, 0.05f),0.01f);			box->SetKinematic(true);
			box->SetTrigger(true);
			box->Color(color_palette[0]);
			Add(box);


		}

		//Custom udpate function
		virtual void CustomUpdate(PxReal dt)
		{

			PxTransform pose = ((PxRigidBody*)golfBall->Get())->getGlobalPose();
			pose.q = rotation;
				((PxRigidBody*)box->Get())->setGlobalPose(pose);
			
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
					PxTransform pose = ((PxRigidBody*)box->Get())->getGlobalPose();
					rotation *= PxQuat(0.1f, PxVec3(0.0f, 0.f, 1.0f));
					pose.q = rotation;
					((PxRigidBody*)box->Get())->setGlobalPose(pose);

					cerr <<"Ball Rotation: " << " X: " + std::to_string(pose.q.x) << " Y: " + std::to_string(pose.q.y) << " Z: " + std::to_string(pose.q.z) << endl;

					break;
				}
				case 'E':
				{
					PxTransform pose = ((PxRigidBody*)box->Get())->getGlobalPose();
					rotation *= PxQuat(-0.1f, PxVec3(0.0f, 0.f, 1.0f));
					pose.q = rotation;
					((PxRigidBody*)box->Get())->setGlobalPose(pose);

					cerr << "Ball Rotation: " << " X: " + std::to_string(pose.q.x) << " Y: " + std::to_string(pose.q.y) << " Z: " + std::to_string(pose.q.z) << endl;

					break;
				}

				case 'F':
				{

					PxVec3 tmp = PxVec3(1.0f, 0.f, 0.0f);
					PxReal f = 1.0f;

					//cerr << "Ball Direct??: " << " X: " + std::to_string(tmp.x) << " Y: " + std::to_string(tmp.y) << " Z: " + std::to_string(tmp.z) << endl;


					rotation.toRadiansAndUnitAxis(f,tmp);

					PxVec3 direction = rotation.getBasisVector1(); //PxVec3(1.0f, 0.f, 0.0f) ;

					

					((PxRigidBody*)golfBall->Get())->addForce(direction*power);

					cerr << "Ball Direct??: " << " X: " + std::to_string(tmp.x) << " Y: " + std::to_string(tmp.y) << " Z: " + std::to_string(tmp.z) << endl;


					break;
				}


			}
		}
	};
}
