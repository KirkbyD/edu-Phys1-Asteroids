#pragma once
#include <fstream>
#include <vector>
#include <string>
#include <sstream>

#include "cModelObject.hpp"
#include "cMesh.hpp"
#include "cVAOManager.hpp"
#include "cModelLoader.hpp"
#include "cShaderManager.hpp"
#include "cLightManager.hpp"
#include "cGameObjectFactory.hpp"
#include "cLightObject.hpp"
#include "cError.hpp"
#include "Globals.hpp"



class cFileManager {
	cFileManager();
	static cFileManager stonFileMngr;

	cError errmngr;
	cGameObjectFactory fact_game_obj;

	std::string _filePath = "data/config/";
	std::string _fileExt = ".ini";

	std::string _graphics_path = "models/";
	std::string _lights_path = "lights/";
	std::string _complex_object_path = "complex/";


	std::string _model_File = "models/models_physics_midterm";
	std::string _lights_File = "lights/lights_physics_midterm";
	std::string _mesh_File = "models/meshes";


	std::string _shaders_File = "shaders/shaders";
	std::string _complexObj_File = "complex/complextest";

	std::string _config_node;
	std::string _lhs, _rhs;
	std::vector<std::string> _vlhs, _vrhs;

	std::ifstream _input;
	std::ofstream _output;
	
	std::stringstream _data_stream;

	std::vector<std::string> _data;
	std::string _tempData;

	std::vector<float> vf;
	float f = 0.0f;

	void LoadData(std::string filename);

	void FormatModelData();
	void FormatLightData();
	void FormatComplexModelData();

	// Converts the string into floats and populates the vf ( vector float ) vector with the values.
	void ConvertStringToFloatArray(std::string cs);

	// Converts the string into a float and stores it in the f ( float ) float with the value.
	void ConvertStringToFloat(std::string cs);

	// Resets the conversion variables.
	void ResetConversionVars();

	// IF NOT FOUND RETURNS 0 ( NULL )
	eShapeTypes DetermineShapeType(std::string type, std::map<std::string, eShapeTypes>& mpShapeTypes);
	std::string GetShapeTypeString(eShapeTypes shapeType);
	bool CheckDecimal(float num);

public:
	static cFileManager* GetFileManager();
	
	std::string GetModelFile();
	std::string GetMeshFile();
	std::string GetLightsFile();
	std::string GetShadersFile();
	std::string GetComplexObjFile();

	void LoadModelsOntoGPU(cModelLoader* pModelLoader, cVAOManager* pVAOManager, const GLuint& shaderProgID, std::string filename, std::map<std::string, cMesh*> &mpMesh);
	void LoadShaders(std::string filename, std::map<std::string, cShaderManager::cShader>& mpVertexShader, std::map<std::string, cShaderManager::cShader>& mpFragmentShader);

	void BuildObjects(std::string filename);

	// SAVES ALL THREE OBJECT TYPES
	bool SaveData();
	bool SaveData(std::string modelsFilename, std::string lightsFilename, std::string complexFilename);
};
