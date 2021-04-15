#include "cPhysics.hpp"
#include "cComplexObject.hpp"
#include "cGameObjectFactory.hpp"
#include <time.h>
#include <random>

cPhysics::cPhysics()
{
	this->AsteroidSpawnRate = 0.6f;

	this->m_Gravity = glm::vec3(0.0f, -9.81f, 0.0f);
	this->m_Friction = glm::vec3(0.99f, 0.8f, 0.99f);
	this->LaserOrigin = glm::vec3(15.0f, 0.0f, 0.0f);
	this->secondCounter = 0.0f;
	this->laserCounter = 1000.0f;
	srand(time(NULL));

	//placing here for better performance on collision tests, since the station wont move.
	cModelObject* Station = pFindObjectByFriendlyName("ExposedParts");
	this->stationMatWorld = calculateWorldMatrix(Station);
	if (mpMesh.find(Station->meshName) != mpMesh.end())
	{
		CalculateTransformedMesh(*mpMesh[Station->meshName], this->stationMatWorld, this->stationMesh);
	}

	//find station length in a hacky manner!
	glm::vec3 testingPosX = glm::vec3(10000.0f, 0.0f, 0.0f);
	glm::vec3 testingNegX = glm::vec3(-10000.0f, 0.0f, 0.0f);
	glm::vec3 closestPosX = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 closestNegX = glm::vec3(0.0f, 0.0f, 0.0f);
	cPhysics::sPhysicsTriangle closestPosTriangle;
	cPhysics::sPhysicsTriangle closestNegTriangle;
	GetClosestTriangleToPoint(testingPosX, this->stationMesh, closestPosX, closestPosTriangle);
	GetClosestTriangleToPoint(testingNegX, this->stationMesh, closestNegX, closestNegTriangle);
	roughStationLength = closestPosTriangle.verts->x - closestNegTriangle.verts->x;

	//find station's big ring radius in a hacky manner!
	glm::vec3 testingPosZ = glm::vec3(0.0f, 0.0f, 10000.0f);
	glm::vec3 closestPosZ = glm::vec3(0.0f, 0.0f, 0.0f);
	GetClosestTriangleToPoint(testingPosZ, this->stationMesh, closestPosZ, closestPosTriangle);
	roughStationRadius = closestPosTriangle.verts->z;
	roughStationRadius += 5; //additional buffer to be safe
	
	return;
}

void cPhysics::setGravity(glm::vec3 newGravityValue)
{
	this->m_Gravity = newGravityValue;
	return;
}

glm::vec3 cPhysics::getGravity(void)
{
	return this->m_Gravity;
}

void cPhysics::IntegrationStep(std::vector<cModelObject*>& vec_pGameObjects, float deltaTime)
{
	// MIDTERM Q2 - Asteroid Spawning
	this->secondCounter += deltaTime;
	if (this->secondCounter >= AsteroidSpawnRate) {
		this->secondCounter = 0.0f;
		//Build Asteroid
		iGameObject* theAsteroid = ObjectFactory.CreateGameObject("[MODELS]");

		switch (rand() % 3 + 0) {
		case 0:
			((cModelObject*)theAsteroid)->meshName = "Asteroid11";
			break;
		case 1:
			((cModelObject*)theAsteroid)->meshName = "Asteroid14";
			break;
		case 2:
			((cModelObject*)theAsteroid)->meshName = "Asteroid15";
			break;
		}
		((cModelObject*)theAsteroid)->friendlyName = "LaserFodder";

		std::random_device rd;
		std::mt19937 mt(rd());

		std::uniform_int_distribution<int> yzDist(-(int)(roughStationRadius*3), (int)(roughStationRadius*3));
		((cModelObject*)theAsteroid)->positionXYZ = glm::vec3(	(roughStationLength * 3),
																yzDist(mt),
																yzDist(mt));

		std::uniform_int_distribution<int> rotationDist(0, 314);
		((cModelObject*)theAsteroid)->rotationXYZ = glm::vec3(rotationDist(mt)/100.0f, rotationDist(mt) / 100.0f, rotationDist(mt) / 100.0f);
		((cModelObject*)theAsteroid)->scale = 0.1f;
		((cModelObject*)theAsteroid)->velocity = glm::vec3(rand() % -25 + -200, 0.0f, 0.0f);
		((cModelObject*)theAsteroid)->inverseMass = 1.0f;
		((cModelObject*)theAsteroid)->physicsShapeType = SPHERE;
	}

	this->laserCounter += deltaTime;
	for (unsigned int index = 0;
		index != vec_pGameObjects.size(); index++)
	{
		cModelObject* pCurObj = vec_pGameObjects[index];

		// MIDTERM Q3 explosion animation and impact residue
		if (pCurObj->friendlyName == "explosion")
		{
			pCurObj->scale *= 1.15;
			if (pCurObj->scale >= 20.0f) {
				pCurObj->scale = 2.0f;
				pCurObj->objectColourRGBA = glm::vec4(0.2f, 0.2f, 0.2f, 1.0f);
				pCurObj->specularColour = glm::vec4(0.2f, 0.2f, 0.2f, 1.0f);
				pCurObj->friendlyName = "impact";
			}
		}
		// MIDTERM Q4 no residue asteroid explosion
		else if (pCurObj->friendlyName == "LaserExplosion")
		{
			pCurObj->scale *= 1.15;
			if (pCurObj->scale >= 20.0f) {
				deletionQueue.push_back(pCurObj);
			}
		}
		// MIDTERM Q4 laser erase
		else if (pCurObj->friendlyName == "LaserSphere" && laserCounter >= 0.25f)
		{
			deletionQueue.push_back(pCurObj);
		}

		if (pCurObj->isVisible == false)
		{
			deletionQueue.push_back(pCurObj);
		}

		if (pCurObj->inverseMass != 0.0f && pCurObj->parentObject == NULL)
		{
			// Forward Explicit Euler Inetegration
			//NewVelocty += Velocity + ( Ax * DeltaTime )

			pCurObj->velocity.x += pCurObj->accel.x * deltaTime;
			pCurObj->velocity.y += pCurObj->accel.y * deltaTime;
			pCurObj->velocity.z += pCurObj->accel.z * deltaTime;

			pCurObj->prevPositionXYZ.x = pCurObj->positionXYZ.x;
			pCurObj->prevPositionXYZ.y = pCurObj->positionXYZ.y;
			pCurObj->prevPositionXYZ.z = pCurObj->positionXYZ.z;

			pCurObj->positionXYZ.x += pCurObj->velocity.x * deltaTime;
			pCurObj->positionXYZ.y += pCurObj->velocity.y * deltaTime;
			pCurObj->positionXYZ.z += pCurObj->velocity.z * deltaTime;
		}
	}

	for (unsigned int index = 0;
		index != g_vec_pComplexObjects.size(); 
		index++) {
		cComplexObject* pCurObj = g_vec_pComplexObjects[index];

		if (pCurObj->GetInverseMass() != 0.0f && pCurObj->parentObject == NULL)
		{
			// Forward Explicit Euler Inetegration
			//NewVelocty += Velocity + ( Ax * DeltaTime )

			glm::vec3 vel = pCurObj->GetVelocity();
			glm::vec3 accel = pCurObj->GetAcceleration();

			vel.x += accel.x * deltaTime;
			vel.y += (accel.y + this->m_Gravity.y) * deltaTime;
			vel.z += accel.z * deltaTime;
			
			glm::vec3 pos = pCurObj->getPosition();
			pCurObj->SetPreviousPosition(pos);

			pos.x += vel.x * deltaTime;
			pos.y += vel.y * deltaTime;
			pos.z += vel.z * deltaTime;

			pCurObj->SetPosition(pos);

			/*if (pCurObj->GetPosition().y < -5) {
				pCurObj->SetPosition(glm::vec3(0.0f, 50.0f, 0.0f));
				pCurObj->SetVelocity(glm::vec3(0.0f, 0.0f, 0.0f));
			}*/
		}
	}

	//quick and dirty, should change to iterators
	if (!deletionQueue.empty())
	{
		for (unsigned int i = 0; i < deletionQueue.size(); i++)
		{
			for (unsigned int j = 0; j != vec_pGameObjects.size(); j++)
			{
				if (deletionQueue[i] == vec_pGameObjects[j])
				{
					vec_pGameObjects.erase(vec_pGameObjects.begin() + (j));
					break;
				}
			}
		}	
	}

	return;
}

void cPhysics::GetClosestTriangleToPoint(Point pointXYZ, cMesh& mesh, glm::vec3& closestPoint, sPhysicsTriangle& closestTriangle)
{
	float closestDistanceSoFar = FLT_MAX;

	for (unsigned int triIndex = 0;
		triIndex != mesh.vecTriangles.size();
		triIndex++)
	{
		sPlyTriangle& curTriangle = mesh.vecTriangles[triIndex];

		sPlyVertexXYZ_N triVert1 = mesh.vecVertices[curTriangle.vert_index_1];
		sPlyVertexXYZ_N triVert2 = mesh.vecVertices[curTriangle.vert_index_2];
		sPlyVertexXYZ_N triVert3 = mesh.vecVertices[curTriangle.vert_index_3];

		Point triVertPoint1;
		triVertPoint1.x = triVert1.x;
		triVertPoint1.y = triVert1.y;
		triVertPoint1.z = triVert1.z;

		Point triVertPoint2;
		triVertPoint2.x = triVert2.x;
		triVertPoint2.y = triVert2.y;
		triVertPoint2.z = triVert2.z;

		Point triVertPoint3;
		triVertPoint3.x = triVert3.x;
		triVertPoint3.y = triVert3.y;
		triVertPoint3.z = triVert3.z;

		glm::vec3 curClosetPoint = ClosestPtPointTriangle(pointXYZ,
			triVertPoint1, triVertPoint2, triVertPoint3);

		float distanceNow = glm::distance(curClosetPoint, pointXYZ);

		if (distanceNow <= closestDistanceSoFar)
		{
			closestDistanceSoFar = distanceNow;

			closestPoint = curClosetPoint;

			closestTriangle.verts[0].x = triVert1.x;
			closestTriangle.verts[0].y = triVert1.y;
			closestTriangle.verts[0].z = triVert1.z;
			closestTriangle.verts[1].x = triVert2.x;
			closestTriangle.verts[1].y = triVert2.y;
			closestTriangle.verts[1].z = triVert2.z;
			closestTriangle.verts[2].x = triVert3.x;
			closestTriangle.verts[2].y = triVert3.y;
			closestTriangle.verts[2].z = triVert3.z;

			glm::vec3 triVert1Norm = glm::vec3(triVert1.nx, triVert1.ny, triVert1.nz);
			glm::vec3 triVert2Norm = glm::vec3(triVert2.nx, triVert2.ny, triVert2.nz);
			glm::vec3 triVert3Norm = glm::vec3(triVert3.nx, triVert3.ny, triVert3.nz);

			closestTriangle.normal = (triVert1Norm + triVert2Norm + triVert3Norm) / 3.0f;
		}

	}

	return;
}

void cPhysics::GetClosestTrianglesToSphere(Point pointXYZ, float distanceRange, cMesh& mesh,
	std::vector<glm::vec3>& vecClosestPoints, std::vector<sPhysicsTriangle>& vecClosestTriangles)
{
	float closestDistanceSoFar = FLT_MAX;
	sPhysicsTriangle closestTriangle;

	for (unsigned int triIndex = 0;
		triIndex != mesh.vecTriangles.size();
		triIndex++)
	{
		sPlyTriangle& curTriangle = mesh.vecTriangles[triIndex];

		sPlyVertexXYZ_N triVert1 = mesh.vecVertices[curTriangle.vert_index_1];
		sPlyVertexXYZ_N triVert2 = mesh.vecVertices[curTriangle.vert_index_2];
		sPlyVertexXYZ_N triVert3 = mesh.vecVertices[curTriangle.vert_index_3];

		Point triVertPoint1;
		triVertPoint1.x = triVert1.x;
		triVertPoint1.y = triVert1.y;
		triVertPoint1.z = triVert1.z;

		Point triVertPoint2;
		triVertPoint2.x = triVert2.x;
		triVertPoint2.y = triVert2.y;
		triVertPoint2.z = triVert2.z;

		Point triVertPoint3;
		triVertPoint3.x = triVert3.x;
		triVertPoint3.y = triVert3.y;
		triVertPoint3.z = triVert3.z;

		glm::vec3 curClosetPoint = ClosestPtPointTriangle(pointXYZ,
			triVertPoint1, triVertPoint2, triVertPoint3);

		float distanceNow = glm::distance(curClosetPoint, pointXYZ);

		if (distanceNow <= closestDistanceSoFar)
		{
			closestDistanceSoFar = distanceNow;

			closestTriangle.verts[0].x = triVert1.x;
			closestTriangle.verts[0].y = triVert1.y;
			closestTriangle.verts[0].z = triVert1.z;
			closestTriangle.verts[1].x = triVert2.x;
			closestTriangle.verts[1].y = triVert2.y;
			closestTriangle.verts[1].z = triVert2.z;
			closestTriangle.verts[2].x = triVert3.x;
			closestTriangle.verts[2].y = triVert3.y;
			closestTriangle.verts[2].z = triVert3.z;

			glm::vec3 triVert1Norm = glm::vec3(triVert1.nx, triVert1.ny, triVert1.nz);
			glm::vec3 triVert2Norm = glm::vec3(triVert2.nx, triVert2.ny, triVert2.nz);
			glm::vec3 triVert3Norm = glm::vec3(triVert3.nx, triVert3.ny, triVert3.nz);

			closestTriangle.normal = (triVert1Norm + triVert2Norm + triVert3Norm) / 3.0f;

			if (distanceNow <= distanceRange)
			{
				vecClosestPoints.push_back(curClosetPoint);
				vecClosestTriangles.push_back(closestTriangle);
			}

		}

	}

	return;
}


void cPhysics::TestForCollisions(std::vector<cModelObject*>& vec_pGameObjects)
{
	// This will store all the collisions in this frame
	//std::vector<sCollisionInfo> vecCollisions;

	//sCollisionInfo collisionInfo;

	for (unsigned int outerLoopIndex = 0;
		outerLoopIndex != vec_pGameObjects.size(); outerLoopIndex++)
	{
		for (unsigned int innerLoopIndex = 0;
			innerLoopIndex != vec_pGameObjects.size(); innerLoopIndex++)
		{
			cModelObject* pA = vec_pGameObjects[outerLoopIndex];
			cModelObject* pB = vec_pGameObjects[innerLoopIndex];

			// Note that if you don't respond to the 
			// collision here, then you will get the same
			// result twice (Object "A" with "B" and later, 
			// object "B" with "A" - but it's the same collison

			//if (pA->getUniqueID() == pB->getUniqueID())
			if (pA->friendlyName == pB->friendlyName)
			{
				// It's the same object
			}

			else if (pA->physicsShapeType == SPHERE && pB->physicsShapeType == SPHERE)				
			{
				DoSphereSphereCollisionTest(pA, pB);
			}

			// MIDTERM Q4 - Laser Blast
			else if ( g_MidtermLaserBlast
				&& this->laserCounter >= 0.25
				&& pA->physicsShapeType == SPHERE
				&& pA->isVisible
				&& pA->positionXYZ.x < roughStationLength * 0.5	
				&& pA->positionXYZ.x > -roughStationLength * 0.2) //scan early and ignor anything past the frontt bit of the station when turned on.
			{
				//Check if the asteroid is within the space station rings radius + a small buffer.
				float pythagoreanPoint = (pA->positionXYZ.z * pA->positionXYZ.z) + (pA->positionXYZ.y * pA->positionXYZ.y);
				if (pythagoreanPoint < (roughStationRadius * roughStationRadius))
				{
					if (!MidtermFutureHitScan(pA))
						pA->physicsShapeType == UNKNOWN;
				}
			}
			
			//MIDTERM Q3 - Collide with space station
			// some additional qualifiers to help not murder framerate...
			// Tried to do mesh-mesh first but that wasn't working out to well for me!
			else if (pA->physicsShapeType == SPHERE
				&& pB == pFindObjectByFriendlyName("ExposedParts")
				&& pA->positionXYZ.x < roughStationLength * 0.1
				&& pA->isVisible)
			{
				DoSphereMeshCollisionTest(pA, pB);
			}
			else if (pA->physicsShapeType == MESH
				&& pB == pFindObjectByFriendlyName("ExposedParts")	// only check vs the exposed mesh
				&& pA->positionXYZ.x < roughStationLength			// if its within striking distance of station
				&& pA->isVisible)									// and is still visible
			{
				DoMeshMeshCollisionTest(pA, pB);
			}


			// MIDTERM Q2 - Delete when far enough away
			else if (	pA->isVisible == true
						&& pA->inverseMass != 0.0f)
			{
				if (pA->positionXYZ.x < -roughStationLength * 2)
				{
					deletionQueue.push_back(pA);
				}
			}
		}
	}
}

bool cPhysics::DoSphereSphereCollisionTest(cModelObject* pA, cModelObject* pB)//, sCollisionInfo& collisionInfo)
{
	if (glm::length(pA->positionXYZ - pB->positionXYZ) <= pA->scale + pB->scale)
	{

		float m1, m2, x1, x2;
		glm::vec3 v1temp, v1, v2, v1x, v2x, v1y, v2y, x(pA->positionXYZ - pB->positionXYZ);

		glm::normalize(x);
		v1 = pA->velocity;
		x1 = dot(x, v1);
		v1x = x * x1;
		v1y = v1 - v1x;
		m1 = pA->inverseMass;

		x = x * -1.0f;
		v2 = pB->velocity;
		x2 = dot(x, v2);
		v2x = x * x2;
		v2y = v2 - v2x;
		m2 = pB->inverseMass;

		pA->positionXYZ = pA->prevPositionXYZ;
		pA->velocity = glm::vec3(v1x * (m1 - m2) / (m1 + m2) + v2x * (2 * m2) / (m1 + m2) + v1y) / 4.0f;

		if (pB->parentObject == NULL) {
			pB->positionXYZ = pB->prevPositionXYZ;
			pB->velocity = glm::vec3(v1x * (2 * m1) / (m1 + m2) + v2x * (m2 - m1) / (m1 + m2) + v2y) / 4.0f;
		}
		else {
			((cComplexObject*)pB->parentObject)->SetPosition(((cComplexObject*)pB->parentObject)->GetPosition());
			((cComplexObject*)pB->parentObject)->SetVelocity(glm::vec3(v1x * (2 * m1) / (m1 + m2) + v2x * (m2 - m1) / (m1 + m2) + v2y) / 4.0f);
		}
		
		return true;
	}

	return false;
}

bool cPhysics::DoSphereMeshCollisionTest(cModelObject* pA, cModelObject* pB)//, sCollisionInfo& collisionInfo)
{
	glm::vec3 closestPoint = glm::vec3(0.0f, 0.0f, 0.0f);
	cPhysics::sPhysicsTriangle closestTriangle;

	GetClosestTriangleToPoint(pA->positionXYZ, this->stationMesh, closestPoint, closestTriangle);
	
	float distance = glm::length(pA->positionXYZ - closestPoint);
	
	//MIDTERM Q3 - Collide with space station
	//hardcoded roughly equal 1/2 bounding box dimensions in meshlab...
	if (pA->meshName == "Asteroid11" && distance <= pA->scale * 60) {
		pA->isVisible = false;
		MidtermExplosion(closestPoint);
	}
	if (pA->meshName == "Asteroid14" && distance <= pA->scale * 26.5) {
		pA->isVisible = false;
		MidtermExplosion(closestPoint);	}

	if (pA->meshName == "Asteroid15" && distance <= pA->scale * 90) {
		pA->isVisible = false;
		MidtermExplosion(closestPoint);
	}

	else if (distance <= pA->scale && pA->parentObject == NULL)
	{
		pA->positionXYZ.y = pA->prevPositionXYZ.y;

		float distanceAfter = glm::length(pA->positionXYZ - closestPoint);

		glm::vec3 velocityVector = glm::normalize(pA->velocity);

		glm::vec3 reflectionVec = glm::reflect(velocityVector, closestTriangle.normal);
		reflectionVec = glm::normalize(reflectionVec);

		reflectionVec *= m_Friction;

		float speed = glm::length(pA->velocity);

		pA->velocity = reflectionVec * speed;

		// Dynamic hack
		//if (pB->friendlyName != "TheGround")
		if (pB->friendlyName == "MovingCube" || pB->friendlyName == "NotAxisAlignedAnnulus")
		{
			Vector newVec = glm::normalize(pA->positionXYZ - closestPoint);
			pA->velocity += newVec * 5.0f;
		}

		return true;
	}

	return false;
}

bool cPhysics::DoMeshMeshCollisionTest(cModelObject* pA, cModelObject* pB) {
	
	GetClosestTriangleToPoint(pA->positionXYZ, stationMesh, stationClosestPoint, stationClosestTriangle);
	
	glm::vec3 asteroidClosestPoint = glm::vec3(0.0f, 0.0f, 0.0f);
	cPhysics::sPhysicsTriangle asteroidclosestTriangle;
	glm::mat4 asteroidMatWorld = calculateWorldMatrix(pA);
	cMesh asteroidMesh;
	if (mpMesh.find(pA->meshName) != mpMesh.end()) {
		CalculateTransformedMesh(*mpMesh[pA->meshName], asteroidMatWorld, asteroidMesh);
	}
	else {
		return false;
	}

	GetClosestTriangleToPoint(pB->positionXYZ, asteroidMesh, asteroidClosestPoint, asteroidclosestTriangle);

	glm::vec3 distance = stationClosestPoint - asteroidClosestPoint;

	if (distance.x < 0.5f && distance.y < 0.5f && distance.z < 0.5f)
	{
		printf("Collision: x%f, y:%f, z:%f\n", asteroidClosestPoint.x, asteroidClosestPoint.y, asteroidClosestPoint.z);
		pA->isVisible = false;

		iGameObject* theImpact = ObjectFactory.CreateMediatedGameObject("model", "sphere", "explosion");
		((cModelObject*)theImpact)->positionXYZ = asteroidClosestPoint;
		((cModelObject*)theImpact)->scale = 2.0f;
		((cModelObject*)theImpact)->objectColourRGBA = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
		((cModelObject*)theImpact)->physicsShapeType = UNKNOWN;
	}
}

void cPhysics::MidtermExplosion(glm::vec3 closestPoint)
{
	iGameObject* theImpact = ObjectFactory.CreateMediatedGameObject("model", "sphere", "explosion");
	((cModelObject*)theImpact)->positionXYZ = closestPoint;
	((cModelObject*)theImpact)->scale = 2.0f;
	((cModelObject*)theImpact)->objectColourRGBA = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
	((cModelObject*)theImpact)->physicsShapeType = UNKNOWN;
}

void cPhysics::MidtermLaserBlast(cModelObject* pA)
{
	//Draw explosion - different name and colour from MidtermExplosion()
	iGameObject* theImpact = ObjectFactory.CreateMediatedGameObject("model", "sphere", "LaserExplosion");
	((cModelObject*)theImpact)->positionXYZ = pA->positionXYZ;
	((cModelObject*)theImpact)->velocity = pA->velocity * 0.5f;
	((cModelObject*)theImpact)->scale = 2.0f;
	((cModelObject*)theImpact)->inverseMass = 1.0f;
	((cModelObject*)theImpact)->objectColourRGBA = glm::vec4(1.0f, 0.0f, 0.4f, 1.0f);
	((cModelObject*)theImpact)->physicsShapeType = UNKNOWN;

	//Draw laser line!
	glm::vec3 direction = glm::normalize(pA->positionXYZ - this->LaserOrigin);
	glm::vec3 drawPos = this->LaserOrigin;

	bool draw = true;
	while (draw) {
		drawPos += direction * 0.25f;

		iGameObject* theImpact = ObjectFactory.CreateMediatedGameObject("model", "sphere", "LaserSphere");
		((cModelObject*)theImpact)->positionXYZ = drawPos;
		((cModelObject*)theImpact)->scale = 0.5f;
		((cModelObject*)theImpact)->objectColourRGBA = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
		((cModelObject*)theImpact)->physicsShapeType = UNKNOWN;

		glm::vec3 test = pA->positionXYZ - drawPos;
		if (test.x < 2 && test.y < 2 && test.z < 2)
			draw = false;
	}

	laserCounter = 0.0f;
	pA->isVisible = false;
}

// MIDTERM BONUS - Only shoot the ones that are going to hit
bool cPhysics::MidtermFutureHitScan(cModelObject* pA)
{
	glm::vec3 oldPosition = pA->positionXYZ;
	glm::vec3 closestPoint = glm::vec3(0.0f, 0.0f, 0.0f);
	cPhysics::sPhysicsTriangle closestTriangle;
	float length = roughStationLength * 0.1; //just eyeballed, seems to be around the last exposed part that can be hit

	//trying to increment by the highest number I can to save on performance,
	//while accounting for the estimated radius of the asteroid.
	if (pA->meshName == "Asteroid11") {
		for (pA->positionXYZ.x = length;
			pA->positionXYZ.x > -length;
			pA->positionXYZ.x -= 30) {
			GetClosestTriangleToPoint(pA->positionXYZ, this->stationMesh, closestPoint, closestTriangle);
			float distance = glm::length(pA->positionXYZ - closestPoint);
			if (distance <= pA->scale * 60) {
				pA->positionXYZ = oldPosition;
				MidtermLaserBlast(pA);
				return true;
			}
		}
		length = roughStationLength * 0.4;
		for (pA->positionXYZ.x = roughStationLength * 0.3;
			pA->positionXYZ.x > -length;
			pA->positionXYZ.x -= 30) {
			GetClosestTriangleToPoint(pA->positionXYZ, this->stationMesh, closestPoint, closestTriangle);
			float distance = glm::length(pA->positionXYZ - closestPoint);
			if (distance <= pA->scale * 60) {
				pA->positionXYZ = oldPosition;
				MidtermLaserBlast(pA);
				return true;
			}
		}
	}
	else if (pA->meshName == "Asteroid14") {
		for (pA->positionXYZ.x = length;
			pA->positionXYZ.x > -length;
			pA->positionXYZ.x -= 20) {
			GetClosestTriangleToPoint(pA->positionXYZ, this->stationMesh, closestPoint, closestTriangle);
			float distance = glm::length(pA->positionXYZ - closestPoint);
			if (distance <= pA->scale * 26.5) {
				pA->positionXYZ = oldPosition;
				MidtermLaserBlast(pA);
				return true;
			}
		}
		length = roughStationLength * 0.4;
		for (pA->positionXYZ.x = roughStationLength * 0.3;
			pA->positionXYZ.x > -length;
			pA->positionXYZ.x -= 20) {
			GetClosestTriangleToPoint(pA->positionXYZ, this->stationMesh, closestPoint, closestTriangle);
			float distance = glm::length(pA->positionXYZ - closestPoint);
			if (distance <= pA->scale * 26.5) {
				pA->positionXYZ = oldPosition;
				MidtermLaserBlast(pA);
				return true;
			}
		}
	}
	else if (pA->meshName == "Asteroid15") {
		for (pA->positionXYZ.x = length;
			pA->positionXYZ.x > -length;
			pA->positionXYZ.x -= 45) {
			GetClosestTriangleToPoint(pA->positionXYZ, this->stationMesh, closestPoint, closestTriangle);
			float distance = glm::length(pA->positionXYZ - closestPoint);
			if (distance <= pA->scale * 90) {
				pA->positionXYZ = oldPosition;
				MidtermLaserBlast(pA);
				return true;
			}
		}
		length = roughStationLength * 0.4;
		for (pA->positionXYZ.x = roughStationLength * 0.3;
			pA->positionXYZ.x > -length;
			pA->positionXYZ.x -= 45) {
			GetClosestTriangleToPoint(pA->positionXYZ, this->stationMesh, closestPoint, closestTriangle);
			float distance = glm::length(pA->positionXYZ - closestPoint);
			if (distance <= pA->scale * 90) {
				pA->positionXYZ = oldPosition;
				MidtermLaserBlast(pA);
				return true;
			}
		}
	}

	pA->positionXYZ = oldPosition;
	return false;
}

glm::mat4 cPhysics::calculateWorldMatrix(cModelObject* pCurrentObject)
{

	glm::mat4 matWorld = glm::mat4(1.0f);


	// TRANSLATION
	glm::mat4 matTrans = glm::translate(glm::mat4(1.0f),
		glm::vec3(pCurrentObject->positionXYZ.x,
			pCurrentObject->positionXYZ.y,
			pCurrentObject->positionXYZ.z));
	matWorld = matWorld * matTrans;


	// ROTATION
	glm::mat4 rotateX = glm::rotate(glm::mat4(1.0f),
		pCurrentObject->rotationXYZ.x,			// Angle 
		glm::vec3(1.0f, 0.0f, 0.0f));
	matWorld = matWorld * rotateX;

	glm::mat4 rotateY = glm::rotate(glm::mat4(1.0f),
		pCurrentObject->rotationXYZ.y,			// Angle 
		glm::vec3(0.0f, 1.0f, 0.0f));
	matWorld = matWorld * rotateY;

	glm::mat4 rotateZ = glm::rotate(glm::mat4(1.0f),
		pCurrentObject->rotationXYZ.z,			// Angle 
		glm::vec3(0.0f, 0.0f, 1.0f));
	matWorld = matWorld * rotateZ;


	// SCALE
	glm::mat4 scale = glm::scale(glm::mat4(1.0f),
		glm::vec3(pCurrentObject->scale,
			pCurrentObject->scale,
			pCurrentObject->scale));
	matWorld = matWorld * scale;


	return matWorld;
}

void cPhysics::CalculateTransformedMesh(cMesh& originalMesh, glm::mat4 matWorld,
	cMesh& mesh_transformedInWorld)
{
	mesh_transformedInWorld = originalMesh;

	for (std::vector<sPlyVertexXYZ_N>::iterator itVert = mesh_transformedInWorld.vecVertices.begin();
		itVert != mesh_transformedInWorld.vecVertices.end(); itVert++)
	{
		glm::vec4 vertex = glm::vec4(itVert->x, itVert->y, itVert->z, 1.0f);

		glm::vec4 vertexWorldTransformed = matWorld * vertex;

		itVert->x = vertexWorldTransformed.x;
		itVert->y = vertexWorldTransformed.y;
		itVert->z = vertexWorldTransformed.z;

		glm::mat4 matWorld_Inv_Transp = glm::inverse(glm::transpose(matWorld));

		glm::vec4 normal = glm::vec4(itVert->nx, itVert->ny, itVert->nz, 1.0f);

		glm::vec4 normalWorldTransformed = matWorld_Inv_Transp * normal;

		itVert->nx = normalWorldTransformed.x;
		itVert->ny = normalWorldTransformed.y;
		itVert->nz = normalWorldTransformed.z;
	}

	return;
}
