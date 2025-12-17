///////////////////////////////////////////////////////////////////////////////
// scenemanager.cpp
// ============
// manage the preparing and rendering of 3D scenes - textures, materials, lighting
//
//  AUTHOR: Brian Battersby - SNHU Instructor / Computer Science
//	Created for CS-330-Computational Graphics and Visualization, Nov. 1st, 2023
///////////////////////////////////////////////////////////////////////////////
//  STUDENT MODIFICATIONS: LaTisha Burns - SNHU Student / Computer Science
//  Updated for CS-330-Computational Graphics and Visualization, Dec. 12th, 2025
/********************************************************************************/

#include "SceneManager.h"

#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#endif

#include <glm/gtx/transform.hpp>

// declaration of global variables
namespace
{
	const char* g_ModelName = "model";
	const char* g_ColorValueName = "objectColor";
	const char* g_TextureValueName = "objectTexture";
	const char* g_TextureOverlayValueName = "overlayTexture"; //added
	const char* g_UseTextureName = "bUseTexture";
	const char* g_UseLightingName = "bUseLighting";
	const char* g_UseTextureOverlayName = "bUseTextureOverlay"; //added
}

/***********************************************************
 *  SceneManager()
 *
 *  The constructor for the class
 ***********************************************************/
SceneManager::SceneManager(ShaderManager *pShaderManager)
{
	m_pShaderManager = pShaderManager;
	m_basicMeshes = new ShapeMeshes();

	//*** Added from OpenGLSample
		// initialize the texture collection
	for (int i = 0; i < 16; i++)
	{
		m_textureIDs[i].tag = "/0";
		m_textureIDs[i].ID = -1;
	}
	m_loadedTextures = 0;
}

/***********************************************************
 *  ~SceneManager()
 *
 *  The destructor for the class
 ***********************************************************/
SceneManager::~SceneManager()
{
	m_pShaderManager = NULL;
	delete m_basicMeshes;
	m_basicMeshes = NULL;
}

/***********************************************************
 *  CreateGLTexture()
 *
 *  This method is used for loading textures from image files,
 *  configuring the texture mapping parameters in OpenGL,
 *  generating the mipmaps, and loading the read texture into
 *  the next available texture slot in memory.
 ***********************************************************/
bool SceneManager::CreateGLTexture(const char* filename, std::string tag)
{
	int width = 0;
	int height = 0;
	int colorChannels = 0;
	GLuint textureID = 0;

	// indicate to always flip images vertically when loaded
	stbi_set_flip_vertically_on_load(true);

	// try to parse the image data from the specified image file
	unsigned char* image = stbi_load(
		filename,
		&width,
		&height,
		&colorChannels,
		0);

	// if the image was successfully read from the image file
	if (image)
	{
		std::cout << "Successfully loaded image:" << filename << ", width:" << width << ", height:" << height << ", channels:" << colorChannels << std::endl;

		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_2D, textureID);

		// set the texture wrapping parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		// set texture filtering parameters
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// if the loaded image is in RGB format
		if (colorChannels == 3)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
		// if the loaded image is in RGBA format - it supports transparency
		else if (colorChannels == 4)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
		else
		{
			std::cout << "Not implemented to handle image with " << colorChannels << " channels" << std::endl;
			return false;
		}

		// generate the texture mipmaps for mapping textures to lower resolutions
		glGenerateMipmap(GL_TEXTURE_2D);

		// free the image data from local memory
		stbi_image_free(image);
		glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture

		// register the loaded texture and associate it with the special tag string
		m_textureIDs[m_loadedTextures].ID = textureID;
		m_textureIDs[m_loadedTextures].tag = tag;
		m_loadedTextures++;

		return true;
	}

	std::cout << "Could not load image:" << filename << std::endl;

	// Error loading the image
	return false;
}

/***********************************************************
 *  BindGLTextures()
 *
 *  This method is used for binding the loaded textures to
 *  OpenGL texture memory slots.  There are up to 16 slots.
 ***********************************************************/
void SceneManager::BindGLTextures()
{
	for (int i = 0; i < m_loadedTextures; i++)
	{
		// bind textures on corresponding texture units
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, m_textureIDs[i].ID);
	}
}

/***********************************************************
 *  DestroyGLTextures()
 *
 *  This method is used for freeing the memory in all the
 *  used texture memory slots.
 ***********************************************************/
void SceneManager::DestroyGLTextures()
{
	for (int i = 0; i < m_loadedTextures; i++)
	{
		glGenTextures(1, &m_textureIDs[i].ID);
	}
}

/***********************************************************
 *  FindTextureID()
 *
 *  This method is used for getting an ID for the previously
 *  loaded texture bitmap associated with the passed in tag.
 ***********************************************************/
int SceneManager::FindTextureID(std::string tag)
{
	int textureID = -1;
	int index = 0;
	bool bFound = false;

	while ((index < m_loadedTextures) && (bFound == false))
	{
		if (m_textureIDs[index].tag.compare(tag) == 0)
		{
			textureID = m_textureIDs[index].ID;
			bFound = true;
		}
		else
			index++;
	}

	return(textureID);
}

/***********************************************************
 *  FindTextureSlot()
 *
 *  This method is used for getting a slot index for the previously
 *  loaded texture bitmap associated with the passed in tag.
 ***********************************************************/
int SceneManager::FindTextureSlot(std::string tag)
{
	int textureSlot = -1;
	int index = 0;
	bool bFound = false;

	while ((index < m_loadedTextures) && (bFound == false))
	{
		if (m_textureIDs[index].tag.compare(tag) == 0)
		{
			textureSlot = index;
			bFound = true;
		}
		else
			index++;
	}

	return(textureSlot);
}

/***********************************************************
 *  FindMaterial()
 *
 *  This method is used for getting a material from the previously
 *  defined materials list that is associated with the passed in tag.
 ***********************************************************/
bool SceneManager::FindMaterial(std::string tag, OBJECT_MATERIAL& material)
{
	if (m_objectMaterials.size() == 0)
	{
		return(false);
	}

	int index = 0;
	bool bFound = false;
	while ((index < m_objectMaterials.size()) && (bFound == false))
	{
		if (m_objectMaterials[index].tag.compare(tag) == 0)
		{
			bFound = true;
			material.diffuseColor = m_objectMaterials[index].diffuseColor;
			material.specularColor = m_objectMaterials[index].specularColor;
			material.shininess = m_objectMaterials[index].shininess;
		}
		else
		{
			index++;
		}
	}

	return(true);
}

/***********************************************************
 *  SetTransformations()
 *
 *  This method is used for setting the transform buffer
 *  using the passed in transformation values.
 ***********************************************************/
void SceneManager::SetTransformations(
	glm::vec3 scaleXYZ,
	float XrotationDegrees,
	float YrotationDegrees,
	float ZrotationDegrees,
	glm::vec3 positionXYZ)
{
	// variables for this method
	glm::mat4 modelView;
	glm::mat4 scale;
	glm::mat4 rotationX;
	glm::mat4 rotationY;
	glm::mat4 rotationZ;
	glm::mat4 translation;

	// set the scale value in the transform buffer
	scale = glm::scale(scaleXYZ);
	// set the rotation values in the transform buffer
	rotationX = glm::rotate(glm::radians(XrotationDegrees), glm::vec3(1.0f, 0.0f, 0.0f));
	rotationY = glm::rotate(glm::radians(YrotationDegrees), glm::vec3(0.0f, 1.0f, 0.0f));
	rotationZ = glm::rotate(glm::radians(ZrotationDegrees), glm::vec3(0.0f, 0.0f, 1.0f));
	// set the translation value in the transform buffer
	translation = glm::translate(positionXYZ);

	modelView = translation * rotationZ * rotationY * rotationX * scale;

	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setMat4Value(g_ModelName, modelView);
	}
}

/***********************************************************
 *  SetShaderColor()
 *
 *  This method is used for setting the passed in color
 *  into the shader for the next draw command
 ***********************************************************/
void SceneManager::SetShaderColor(
	float redColorValue,
	float greenColorValue,
	float blueColorValue,
	float alphaValue)
{
	// variables for this method
	glm::vec4 currentColor;

	currentColor.r = redColorValue;
	currentColor.g = greenColorValue;
	currentColor.b = blueColorValue;
	currentColor.a = alphaValue;

	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setIntValue(g_UseTextureName, false);
		m_pShaderManager->setVec4Value(g_ColorValueName, currentColor);
	}
}

/***********************************************************
 *  SetShaderTexture()
 *
 *  This method is used for setting the texture data
 *  associated with the passed in ID into the shader.
 ***********************************************************/
void SceneManager::SetShaderTexture(
	std::string textureTag)
{
	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setIntValue(g_UseTextureName, true);

		int textureID = -1;
		textureID = FindTextureSlot(textureTag);
		m_pShaderManager->setSampler2DValue(g_TextureValueName, textureID);
	}
}

//***Added

/***********************************************************
 *  SetShaderTextureOverlay()
 *
 *  This method is used for setting the texture data
 *  associated with the passed in ID into the shader.
 ***********************************************************/
void SceneManager::SetShaderTextureOverlay(
	std::string textureTag)
{
	if (NULL != m_pShaderManager)
	{
		if (textureTag.size() > 0)
		{
			m_pShaderManager->setIntValue(g_UseTextureOverlayName, true);

			int textureID = -1;
			textureID = FindTextureSlot(textureTag);
			m_pShaderManager->setSampler2DValue(g_TextureOverlayValueName, textureID);
		}
		else
		{
			m_pShaderManager->setIntValue(g_UseTextureOverlayName, false);
		}
	}
}



/***********************************************************
 *  SetTextureUVScale()
 *
 *  This method is used for setting the texture UV scale
 *  values into the shader.
 ***********************************************************/
void SceneManager::SetTextureUVScale(float u, float v)
{
	if (NULL != m_pShaderManager)
	{
		m_pShaderManager->setVec2Value("UVscale", glm::vec2(u, v));
	}
}



/***********************************************************
 *  SetShaderMaterial()
 *
 *  This method is used for passing the material values
 *  into the shader.
 ***********************************************************/
void SceneManager::SetShaderMaterial(
	std::string materialTag)
{
	if (m_objectMaterials.size() > 0)
	{
		OBJECT_MATERIAL material;
		bool bReturn = false;

		bReturn = FindMaterial(materialTag, material);
		if (bReturn == true)
		{
			m_pShaderManager->setVec3Value("material.diffuseColor", material.diffuseColor);
			m_pShaderManager->setVec3Value("material.specularColor", material.specularColor);
			m_pShaderManager->setFloatValue("material.shininess", material.shininess);
		}
	}
}

/**************************************************************/
/*** STUDENTS CAN MODIFY the code in the methods BELOW for  ***/
/*** preparing and rendering their own 3D replicated scenes.***/
/*** Please refer to the code in the OpenGL sample project  ***/
/*** for assistance.                                        ***/
/**************************************************************/

//***Added from OpenGLSample
/***********************************************************
 *  LoadSceneTextures()
 *
 *  This method is used for preparing the 3D scene by loading
 *  the shapes, textures in memory to support the 3D scene
 *  rendering
 ***********************************************************/

void SceneManager::LoadSceneTextures()
{
	bool bReturn = false;

	// Added Wood texture to make plane look like a table
	bReturn = CreateGLTexture(
		"textures/wood_cherry_seamless.jpg",
		"Wood Table");

	bReturn = CreateGLTexture(
		"textures/ERainbowOverlay2.png",
		"Cylinder Overlay");

	bReturn = CreateGLTexture(
		"textures/VaseStripes2.png",
		"Stripes2");

	bReturn = CreateGLTexture(
		"textures/wood_black_seamless.jpg",
		"Black Wood");

	bReturn = CreateGLTexture(
		"textures/transparent.png",
		"transparent");

	bReturn = CreateGLTexture(
		"textures/GoldLeaves.png",
		"Gold Leaves");

	bReturn = CreateGLTexture(
		"textures/GoldLeavesSides.png",
		"Gold Leaves2");

	bReturn = CreateGLTexture(
		"textures/CandleHolder.png",
		"Candle Holder");

	bReturn = CreateGLTexture(
		"textures/WetGlass.jpg",
		"Wet Glass");

	bReturn = CreateGLTexture(
		"textures/pumpkin_texture3.jpg",
		"Pumpkin3");

	bReturn = CreateGLTexture(
		"textures/Pumpkinbark.jpg",
		"Stem");

	bReturn = CreateGLTexture(
		"textures/bricks_weathered_seamless2.jpg",
		"backdrop2");


}

//***Added
/***********************************************************
 *  DefineObjectMaterials()
 *
 *  This method is used for configuring the various material
 *  settings for all of the objects within the 3D scene.
 ***********************************************************/
void SceneManager::DefineObjectMaterials()
{
	// Metal
	OBJECT_MATERIAL metalMaterial;
	metalMaterial.diffuseColor = glm::vec3(0.2f, 0.2f, 0.2f);
	metalMaterial.specularColor = glm::vec3(0.7f, 0.7f, 0.7f);
	metalMaterial.shininess = 42.0;
	metalMaterial.tag = "metal";

	m_objectMaterials.push_back(metalMaterial);

	// Wood
	OBJECT_MATERIAL woodMaterial;
	woodMaterial.diffuseColor = glm::vec3(0.6f, 0.35f, 0.2f);
	woodMaterial.specularColor = glm::vec3(0.2f, 0.2f, 0.2f);
	woodMaterial.shininess = 8.0f;
	woodMaterial.tag = "wood";

	m_objectMaterials.push_back(woodMaterial);

	//Glass
	OBJECT_MATERIAL glassMaterial;
	glassMaterial.diffuseColor = glm::vec3(0.4f, 0.4f, 0.5f);
	glassMaterial.specularColor = glm::vec3(1.0f, 1.0f, 1.0f);
	glassMaterial.shininess = 95.0;
	glassMaterial.tag = "glass";

	m_objectMaterials.push_back(glassMaterial);

	//Gold
	OBJECT_MATERIAL goldMaterial;
	goldMaterial.diffuseColor = glm::vec3(0.3f, 0.3f, 0.2f);
	goldMaterial.specularColor = glm::vec3(0.6f, 0.5f, 0.4f);
	goldMaterial.shininess = 22.0;
	goldMaterial.tag = "gold";

	m_objectMaterials.push_back(goldMaterial);

	//Tile
	OBJECT_MATERIAL tileMaterial;
	tileMaterial.diffuseColor = glm::vec3(0.3f, 0.2f, 0.1f);
	tileMaterial.specularColor = glm::vec3(0.4f, 0.5f, 0.6f);
	tileMaterial.shininess = 25.0;
	tileMaterial.tag = "tile";

	m_objectMaterials.push_back(tileMaterial);

	OBJECT_MATERIAL backdropMaterial;
	backdropMaterial.diffuseColor = glm::vec3(0.4f, 0.4f, 0.4f);
	backdropMaterial.specularColor = glm::vec3(0.0f, 0.0f, 0.0f);
	backdropMaterial.shininess = .02;
	backdropMaterial.tag = "backdrop";

	m_objectMaterials.push_back(backdropMaterial);


}

/***********************************************************
 *  SetupSceneLights()
 *
 *  This method is called to add and configure the light
 *  sources for the 3D scene.  There are up to 4 light sources.
 ***********************************************************/
void SceneManager::SetupSceneLights()
{
	// this line of code is NEEDED for telling the shaders to render 
	// the 3D scene with custom lighting - to use the default rendered 
	// lighting then comment out the following line
	m_pShaderManager->setBoolValue(g_UseLightingName, true);

	// directional light to emulate sunlight coming into scene
	m_pShaderManager->setVec3Value("directionalLight.direction", -0.1f, -0.3f, -0.2f);
	m_pShaderManager->setVec3Value("directionalLight.ambient", 0.5f, 0.5f, 0.5f);
	m_pShaderManager->setVec3Value("directionalLight.diffuse", 0.25f, 0.25f, 0.30f);
	m_pShaderManager->setVec3Value("directionalLight.specular", 0.05f, 0.05f, 0.05f);
	m_pShaderManager->setBoolValue("directionalLight.bActive", true);


	///****************************************************************/
	//light 0 - above the scene
	m_pShaderManager->setVec3Value("pointLights[0].position", 0.0f, 5.0f, 0.0f);
	m_pShaderManager->setVec3Value("pointLights[0].ambient", 0.15f, 0.15f, 0.15f);
	m_pShaderManager->setVec3Value("pointLights[0].diffuse", 0.5f, 0.5f, 0.5f);
	m_pShaderManager->setVec3Value("pointLights[0].specular", 0.5f, 0.5f, 0.5f);
	m_pShaderManager->setBoolValue("pointLights[0].bActive", true);
	
	//point light 1
	m_pShaderManager->setVec3Value("pointLights[1].position", -3.0f, 7.0f, -3.0f);
	m_pShaderManager->setVec3Value("pointLights[1].ambient", 0.05f, 0.05f, 0.05f);
	m_pShaderManager->setVec3Value("pointLights[1].diffuse", 0.4f, 0.3f, 0.4f);
	m_pShaderManager->setVec3Value("pointLights[1].specular", 0.2f, 0.2f, 0.2f);
	m_pShaderManager->setBoolValue("pointLights[1].bActive", true); 

	//point light 2
	m_pShaderManager->setVec3Value("pointLights[2].position", -3.0f, 7.0f, 3.0f);
	m_pShaderManager->setVec3Value("pointLights[2].ambient", 0.05f, 0.05f, 0.05f);
	m_pShaderManager->setVec3Value("pointLights[2].diffuse", 0.4f, 0.3f, 0.4f);
	m_pShaderManager->setVec3Value("pointLights[2].specular", 0.2f, 0.3f, 0.2f);
	m_pShaderManager->setBoolValue("pointLights[2].bActive", true); 
	
	//point light 3
	m_pShaderManager->setVec3Value("pointLights[3].position", 0.0f, 2.0f, -7.0f);
	m_pShaderManager->setVec3Value("pointLights[3].ambient", 0.05f, 0.05f, 0.05f);
	m_pShaderManager->setVec3Value("pointLights[3].diffuse", 0.5f, 0.5f, .5f);
	m_pShaderManager->setVec3Value("pointLights[3].specular", 0.1f, 0.1f, 0.1f);
	m_pShaderManager->setBoolValue("pointLights[3].bActive", true);

	//point light 4
	m_pShaderManager->setVec3Value("pointLights[4].position", 3.2f, 6.0f, 4.0f);
	m_pShaderManager->setVec3Value("pointLights[4].ambient", 0.05f, 0.05f, 0.05f);
	m_pShaderManager->setVec3Value("pointLights[4].diffuse", 0.5f, 0.5f, 0.5f);
	m_pShaderManager->setVec3Value("pointLights[4].specular", 0.1f, 0.1f, 0.1f);
	m_pShaderManager->setBoolValue("pointLights[4].bActive", true);
	
	
	m_pShaderManager->setVec3Value("spotLight.ambient", 0.3f, 0.3f, 0.3f);
	m_pShaderManager->setVec3Value("spotLight.diffuse", 0.8f, 0.8f, 0.8f);
	m_pShaderManager->setVec3Value("spotLight.specular", 0.4f, 0.4f, 0.4f);
	m_pShaderManager->setFloatValue("spotLight.constant", 1.0f);
	m_pShaderManager->setFloatValue("spotLight.linear", 0.09f);
	m_pShaderManager->setFloatValue("spotLight.quadratic", 0.032f);
	m_pShaderManager->setFloatValue("spotLight.cutOff", glm::cos(glm::radians(42.5f)));
	m_pShaderManager->setFloatValue("spotLight.outerCutOff", glm::cos(glm::radians(48.0f)));
	m_pShaderManager->setBoolValue("spotLight.bActive", true);

}
/***********************************************************
 *  PrepareScene()
 *
 *  This method is used for preparing the 3D scene by loading
 *  the shapes, textures in memory to support the 3D scene 
 *  rendering
 ***********************************************************/
void SceneManager::PrepareScene()
{
	// only one instance of a particular mesh needs to be
	// loaded in memory no matter how many times it is drawn
	// in the rendered 3D scene

	m_basicMeshes->LoadPlaneMesh();
	//Shapes Added
	m_basicMeshes->LoadBoxMesh(); 
	m_basicMeshes->LoadSphereMesh();
	m_basicMeshes->LoadCylinderMesh();
	m_basicMeshes->LoadTorusMesh();
	m_basicMeshes->LoadTaperedCylinderMesh();

	// load the texture image files for the textures applied
	// to objects in the 3D scene
	LoadSceneTextures();
	BindGLTextures();

	// define the materials that will be used for the objects
	// in the 3D scene
	DefineObjectMaterials();

	// add and defile the light sources for the 3D scene
	SetupSceneLights();
}

/***********************************************************
 *  RenderScene()
 *
 *  This method is used for rendering the 3D scene by 
 *  transforming and drawing the basic 3D shapes
 ***********************************************************/
void SceneManager::RenderScene()
{
	// declare the variables for the transformations
	glm::vec3 scaleXYZ;
	float XrotationDegrees = 0.0f;
	float YrotationDegrees = 0.0f;
	float ZrotationDegrees = 0.0f;
	glm::vec3 positionXYZ;

	/*** Set needed transformations before drawing the basic mesh.  ***/
	/*** This same ordering of code should be used for transforming ***/
	/*** and drawing all the basic 3D shapes.						***/
	/******************************************************************/
	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(30.0f, 1.0f, 10.0f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(0.0f, -0.5f, 0.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	// Bottom plane for the scene – represents the coffee table surface
	//*** Added texture to plane
	SetShaderTexture("Wood Table");
	SetTextureUVScale(1.0, 1.0);

	//material
	SetShaderMaterial("wood");

	// draw the mesh with transformation values
	m_basicMeshes->DrawBoxMesh();

	/****************************************************************/

	//***Backdrop - added 12/12
	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(20.0f, 1.0f, 10.0f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 90.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(0.0f, 10.0f, -5.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	//texture
	SetShaderTexture("backdrop2");
	SetTextureUVScale(1.0, 1.0);

	//material
	SetShaderMaterial("backdrop");

	// draw the mesh with transformation values - this plane is used for the backdrop
	m_basicMeshes->DrawPlaneMesh();


	/****************************************************************/
	//***Objects Start Here
	
	// ***Glass Candle Holder
	//Base of Candle
	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(1.3f, 1.35f, 0.7f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 90.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(6.0f, 0.2f, 0.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	//texture
	SetShaderTexture("Wet Glass");
	SetTextureUVScale(0.2f, 0.2f);
	//material
	SetShaderMaterial("glass");
	// draw the mesh with transformation values
	m_basicMeshes->DrawTorusMesh();


	// Body of Candle
	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(1.4f, 1.6f, 1.4f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = -10.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(6.0f, 0.3f, 0.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	//***Complex Texturing Technique - Overlay option
	//base texture for the cylinder 
	SetShaderTexture("Candle Holder");

	//overlay texture for cylinder
	SetShaderTextureOverlay("Cylinder Overlay");

	//uv scale for both textures 
	SetTextureUVScale(2.0f, 1.0f);

	//material
	SetShaderMaterial("glass");

	//apply the texture and overlay to the sides only
	m_basicMeshes->DrawCylinderMesh(false, false, true);

	//disable the texture overlay
	SetShaderTextureOverlay("");


	// Rounded Top/Dome of candle holder
	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(1.37f, 1.37f, 1.37f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 15.0f;
	YrotationDegrees = 20.0f;
	ZrotationDegrees = 90.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(6.0f, 2.0f, 0.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	//SetShaderColor(0.467, 0.533, 0.600, 1);
	SetShaderTexture("Candle Holder");
	SetTextureUVScale(0.8f, 0.8);
	//material
	SetShaderMaterial("glass");

	// draw the mesh with transformation values
	m_basicMeshes->DrawSphereMesh();


	// Candle Holder knob
	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(0.3f, 0.5f, 0.3f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(6.0f, 3.5f, 0.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	//texture
	SetShaderTexture("Candle Holder");
	SetTextureUVScale(0.8f, 0.8f);

	//material
	SetShaderMaterial("glass");

	// draw the mesh with transformation values
	m_basicMeshes->DrawSphereMesh();



	/****************************************************************/
	//*** Vase
	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(1.2f, 7.0f, 1.2f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(2.0f, 0.2f, -0.8f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	//sides, stripes
	SetShaderTexture("Stripes2");
	SetTextureUVScale(1.0f, 1.0f);

	//material
	SetShaderMaterial("glass");
	m_basicMeshes->DrawCylinderMesh(false, false, true); //sides only

	//top, transparent
	SetShaderTexture("transparent"); //created transparent texture for glass
	SetTextureUVScale(1.0f, 1.0f);
	m_basicMeshes->DrawCylinderMesh(true, false, false); //top only

	//bottom, dark base
	SetShaderTexture("Black Wood");
	SetTextureUVScale(1.0f, 1.0f);

	//material
	SetShaderMaterial("wood");
	m_basicMeshes->DrawCylinderMesh(false, true, false); //bottom only


	/****************************************************************/
	//*** Picture Frame
	
	//picture inside Frame
	// ***Complex Texturing Technique - Applying different colors to each side of the box
	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(4.0f, 5.0f, 0.1f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = -20.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(-2.0f, 2.5f, 0.0f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	//front of picture = light tan to represent blank picture
	SetShaderColor(0.95f, 0.90f, 0.80f, 1.0f);
	m_basicMeshes->DrawBoxMeshSide(ShapeMeshes::box_front);

	//other sides of picture = gold like the frame so they are not visble from side/top views
	SetShaderColor(0.65f, 0.45f, 0.20f, 1.0f);
	m_basicMeshes->DrawBoxMeshSide(ShapeMeshes::box_back);
	m_basicMeshes->DrawBoxMeshSide(ShapeMeshes::box_left);
	m_basicMeshes->DrawBoxMeshSide(ShapeMeshes::box_right);
	m_basicMeshes->DrawBoxMeshSide(ShapeMeshes::box_top);
	m_basicMeshes->DrawBoxMeshSide(ShapeMeshes::box_bottom);

	//material
	SetShaderMaterial("wood");


	//Back of Picture frame
	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(4.0f, 5.0f, 0.1f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = -20.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(-2.0f, 2.5f, -0.1f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);


	//gold color for back of frame
	SetShaderColor(0.65f, 0.45f, 0.20f, 1.0f);
	SetShaderMaterial("wood");
	m_basicMeshes->DrawBoxMesh();

	//top frame piece
	scaleXYZ = glm::vec3(4.1f, 0.7f, 0.15f);
	XrotationDegrees = -20.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(-2.0f, 4.6f, -0.7f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	//texture
	SetShaderTexture("Gold Leaves");
	SetTextureUVScale(0.9f, 0.3f);
	SetShaderMaterial("gold");
	m_basicMeshes->DrawBoxMesh();

	//bottom frame piece
	scaleXYZ = glm::vec3(4.1f, 0.7f, 0.15f);
	XrotationDegrees = 20.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 180.0f;
	positionXYZ = glm::vec3(-2.0f, 0.5f, 0.8f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	//texture
	SetShaderTexture("Gold Leaves");
	SetTextureUVScale(0.9f, 0.3f);
	SetShaderMaterial("gold");
	m_basicMeshes->DrawBoxMesh();

	//left frame piece
	scaleXYZ = glm::vec3(0.7f, 3.8f, 0.10f);   
	XrotationDegrees = 20.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 180.0f;
	positionXYZ = glm::vec3(-3.7f, 2.6f, 0.05f);   
	
	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	//texture
	SetShaderTexture("Gold Leaves2");
	SetTextureUVScale(0.3f, 0.9f);
	SetShaderMaterial("gold");
	m_basicMeshes->DrawBoxMesh();


	//right frame piece
	scaleXYZ = glm::vec3(0.7f, 3.8f, 0.10f);
	XrotationDegrees = -20.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 0.0f;
	positionXYZ = glm::vec3(-0.3f, 2.6f, 0.05f);  
	
	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	//texture
	SetShaderTexture("Gold Leaves2");
	SetTextureUVScale(0.3f, 0.9f);
	SetShaderMaterial("gold");
	m_basicMeshes->DrawBoxMesh();

	//back stand piece
	scaleXYZ = glm::vec3(0.7f, 3.0f, 0.1f);
	XrotationDegrees = 30.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = 00.0f;
	positionXYZ = glm::vec3(-1.5f, 1.5f, -1.0f);
	
	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	//texture
	SetShaderColor(0.65f, 0.45f, 0.20f, 1.0f);

	//material
	SetShaderMaterial("wood");

	m_basicMeshes->DrawBoxMesh();

	/****************************************************************/
	//***Pumpkin
	//Base of Pumpkin
	// set the XYZ scale for the mesh
	scaleXYZ = glm::vec3(2.1f, 1.8f, 1.5f);

	// set the XYZ rotation for the mesh
	XrotationDegrees = 0.0f;
	YrotationDegrees = 0.0f;
	ZrotationDegrees = -35.0f;

	// set the XYZ position for the mesh
	positionXYZ = glm::vec3(-7.0f, 1.4f, -0.3f);

	// set the transformations into memory to be used on the drawn meshes
	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	//texture
	SetShaderTexture("Pumpkin3");  //closest I could get to getting a pumpkin texture to wrap :(
	SetTextureUVScale(1.0f, 1.0f);
	
	//material
	SetShaderMaterial("glass");

	// draw the mesh with transformation values
	m_basicMeshes->DrawSphereMesh();


	//Pumpkin stem  
	scaleXYZ = glm::vec3(0.5f, 0.7f, 0.5f);
	XrotationDegrees = 0.0f;    
	YrotationDegrees = 0.0f;
	ZrotationDegrees = -10.0f;

	//sit on top of the pumpkin body
	positionXYZ = glm::vec3(-7.1f, 3.1f, -0.3f);

	SetTransformations(
		scaleXYZ,
		XrotationDegrees,
		YrotationDegrees,
		ZrotationDegrees,
		positionXYZ);

	//texture
	SetShaderTexture("Stem");
	SetTextureUVScale(1.0f, 1.0f);

	//material
	SetShaderMaterial("wood");

	// draw the mesh with transformation values
	m_basicMeshes->DrawTaperedCylinderMesh();

}
