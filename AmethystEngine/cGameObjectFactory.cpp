#include "cGameObjectFactory.hpp"
#include "Globals.hpp"

#include "cModelObject.hpp"
#include "cLightObject.hpp"
#include "cComplexObject.hpp"

iGameObject* cGameObjectFactory::CreateGameObject(std::string type) {
	if (type == "[LIGHTS]") {
		return new cLightObject(g_vec_pLightObjects);
	}

	if (type == "[MODELS]") {
		return new cModelObject(g_vec_pGameObjects);
	}
	
	if (type == "[COMPLEX]") {
		return new cComplexObject(g_vec_pComplexObjects);
	}
}

// MEDIATOR SPECIFIC CREATION FUNCTION
// THIS SHOULD BE OVERLOADED SO THAT IT CAN BE PASSED NUMEROUS INITIALIZATION VARIABLES AT A LATER DATE
// CURRENTLY THIS WILL INITIALIZE TO DEFAULT SETTINGS
iGameObject* cGameObjectFactory::CreateMediatedGameObject(std::string type, std::string mesh, std::string name) {
	if (type == "light")
		return new cLightObject(g_vec_pLightObjects, name);

	if (type == "model")
		return new cModelObject(g_vec_pGameObjects, mesh, name);

	if (type == "complex")
		return new cComplexObject(g_vec_pComplexObjects);
}



//
//void cGameObjectFactory::CreateGameObject(std::string objectType) {
//	iGameObject* pGameObject = NULL;
//
//	if (objectType == "model") {
//		pGameObject = new cModelObject(g_vec_pGameObjects);
//	}
//	//if (objectType == "physics")
//	//{
//	//	//pGameObject = new cGraphicsObject();
//	//}
//	//else if (objectType == "graphics")
//	//{
//	//	//pGameObject = new cPhysicsObject();
//	//}
//	else if (objectType == "light") {
//		pGameObject = new cLightObject(g_vec_pLightObjects);
//	}
//
//	// this should not happen the factory can't create something that it does not know about
//	//if (pGameObject != NULL) {
//	//	this->BuildGameObject(pGameObject, objectType);
//	//}
//
//
//	//return pGameObject;
//}
//
//void cGameObjectFactory::BuildGameObject(iGameObject* pObject, std::string objectType) {
//	if (objectType == "game object") {
//		//((cGameObject*)pObject)->smth = new smth();
//	}
//	//if (objectType == "physics")
//	//{
//	//	//((cPhysicsObject*)pObject)->smth = new smth();
//	//}
//	//else if (objectType == "graphics")
//	//{
//	//	//((cGraphicsObject*)pObject)->smth = new smth();
//	//}
//	else if (objectType == "light") {
//		//((cLightObject*)pObject)->smth = new smth();
//	}
//}