///////////////////////////////////////////////////////////////////////////////
// scenemanager.cpp
// ============
// manage the preparing and rendering of 3D scenes - textures, materials, lighting
//
//  AUTHOR: Brian Battersby - SNHU Instructor / Computer Science
//	Created for CS-330-Computational Graphics and Visualization, Nov. 1st, 2023
///////////////////////////////////////////////////////////////////////////////

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
	const char* g_UseTextureName = "bUseTexture";
	const char* g_UseLightingName = "bUseLighting";
}

/***********************************************************
 *  SceneManager()
 *
 *  The constructor for the class
 ***********************************************************/
SceneManager::SceneManager(ShaderManager* pShaderManager)
{
	m_pShaderManager = pShaderManager;
	// create the shape meshes object
	m_basicMeshes = new ShapeMeshes();

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
	// free the allocated objects
	m_pShaderManager = NULL;
	if (NULL != m_basicMeshes)
	{
		delete m_basicMeshes;
		m_basicMeshes = NULL;
	}

	// free the allocated OpenGL textures
	DestroyGLTextures();
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

void SceneManager::DefineObjectMaterials()
{
	// Define material properties for a polished silver appearance
	OBJECT_MATERIAL silverMaterial;
	silverMaterial.diffuseColor = glm::vec3(0.7f, 0.7f, 0.7f); // Bright gray for the main silver color
	silverMaterial.specularColor = glm::vec3(0.9f, 0.9f, 0.9f); // Reflective shine for highlights
	silverMaterial.shininess = 64.0; // High shininess for polished metal appearance
	silverMaterial.tag = "metal"; // Tag for identification in the scene

	m_objectMaterials.push_back(silverMaterial); // Add the material to the materials list

	// Define material properties for a rich wood texture
	OBJECT_MATERIAL woodMaterial;
	woodMaterial.diffuseColor = glm::vec3(0.3f, 0.2f, 0.1f); // Dark brown for the wood's natural tone
	woodMaterial.specularColor = glm::vec3(0.1f, 0.1f, 0.1f); // Subtle highlights to simulate matte wood
	woodMaterial.shininess = 16.0; // Low shininess for a natural finish
	woodMaterial.tag = "wood"; // Tag for identification in the scene

	m_objectMaterials.push_back(woodMaterial); // Add the material to the materials list

	// Define material properties for tinted glass
	OBJECT_MATERIAL glassMaterial;
	glassMaterial.diffuseColor = glm::vec3(0.2f, 0.2f, 0.3f); // Slightly tinted dark glass color
	glassMaterial.specularColor = glm::vec3(1.0f, 1.0f, 1.0f); // High reflectivity for a glassy appearance
	glassMaterial.shininess = 128.0; // Very high shininess to simulate reflective glass
	glassMaterial.tag = "glass"; // Tag for identification in the scene

	m_objectMaterials.push_back(glassMaterial); // Add the material to the materials list

	// Define material properties for a simple plate
	OBJECT_MATERIAL plateMaterial;
	plateMaterial.diffuseColor = glm::vec3(0.5f, 0.5f, 0.5f); // Neutral gray for the plate's base color
	plateMaterial.specularColor = glm::vec3(0.3f, 0.3f, 0.3f); // Moderate highlights
	plateMaterial.shininess = 24.0; // Mild reflectiveness for a subtle glossy finish
	plateMaterial.tag = "plate"; // Tag for identification in the scene

	m_objectMaterials.push_back(plateMaterial); // Add the material to the materials list

	// Define material properties for a backdrop or background plane
	OBJECT_MATERIAL backdropMaterial;
	backdropMaterial.diffuseColor = glm::vec3(0.8f, 0.8f, 0.9f); // Light grayish-blue for the backdrop
	backdropMaterial.specularColor = glm::vec3(0.2f, 0.2f, 0.2f); // Minimal reflectivity for a matte look
	backdropMaterial.shininess = 8.0; // Very low shininess for a soft finish
	backdropMaterial.tag = "backdrop"; // Tag for identification in the scene

	m_objectMaterials.push_back(backdropMaterial); // Add the material to the materials list

	// Define material properties for coffee-like liquid
	OBJECT_MATERIAL coffeeMaterial;
	coffeeMaterial.diffuseColor = glm::vec3(0.4f, 0.25f, 0.1f); // Rich brown color resembling coffee
	coffeeMaterial.specularColor = glm::vec3(0.5f, 0.3f, 0.2f); // Subtle warm highlights
	coffeeMaterial.shininess = 30.0; // Moderate shininess for a liquid-like gloss
	coffeeMaterial.tag = "liquid"; // Tag for identification in the scene

	m_objectMaterials.push_back(coffeeMaterial); // Add the material to the materials list

	// Define material properties for the cover of a book
	OBJECT_MATERIAL coverMaterial;
	coverMaterial.diffuseColor = glm::vec3(1.0f, 1.0f, 1.0f); // Pure white color for the book's cover
	coverMaterial.specularColor = glm::vec3(0.9f, 0.9f, 0.9f); // High specular for a glossy white finish
	coverMaterial.shininess = 64.0; // Increased shininess for a polished look
	coverMaterial.tag = "cover"; // Tag for identification in the scene

	m_objectMaterials.push_back(coverMaterial); // Add the material to the materials list

	// Define material properties for the spine of a book
	OBJECT_MATERIAL spineMaterial;
	spineMaterial.diffuseColor = glm::vec3(0.3f, 0.15f, 0.15f); // Darker red for the spine
	spineMaterial.specularColor = glm::vec3(0.4f, 0.2f, 0.2f); // Subtle highlights
	spineMaterial.shininess = 20.0; // Low shininess
	spineMaterial.tag = "spine"; // Tag for identification in the scene

	m_objectMaterials.push_back(spineMaterial); // Add the material to the materials list

	// Define material properties for the pages of a book
	OBJECT_MATERIAL pagesMaterial;
	pagesMaterial.diffuseColor = glm::vec3(0.9f, 0.9f, 0.85f); // Cream color to represent book pages
	pagesMaterial.specularColor = glm::vec3(0.2f, 0.2f, 0.2f); // Minimal reflectivity
	pagesMaterial.shininess = 5.0; // Very low shininess for a matte appearance
	pagesMaterial.tag = "pages"; // Tag for identification in the scene

	m_objectMaterials.push_back(pagesMaterial); // Add the material to the materials list


	// Leaf Material
	OBJECT_MATERIAL leafMaterial;
	leafMaterial.diffuseColor = glm::vec3(0.2f, 0.6f, 0.2f); // Green color for leaves
	leafMaterial.specularColor = glm::vec3(0.1f, 0.3f, 0.1f); // Low reflectivity
	leafMaterial.shininess = 6.0f; // Matte-like finish for leaves
	leafMaterial.tag = "leaf"; // Tag to identify leaf material in the scene

	m_objectMaterials.push_back(leafMaterial); // Add material to the list

	// Soil Material
	OBJECT_MATERIAL soilMaterial;
	soilMaterial.diffuseColor = glm::vec3(0.2f, 0.1f, 0.0f); // Dark brown color for soil
	soilMaterial.specularColor = glm::vec3(0.05f, 0.02f, 0.01f); // Very low reflectivity
	soilMaterial.shininess = 4.0f; // Matte finish for soil
	soilMaterial.tag = "soil"; // Tag to identify soil material in the scene

	m_objectMaterials.push_back(soilMaterial); // Add material to the list

	// Clay Material
	OBJECT_MATERIAL clayMaterial;
	clayMaterial.diffuseColor = glm::vec3(0.8f, 0.5f, 0.3f); // Terracotta clay color
	clayMaterial.specularColor = glm::vec3(0.2f, 0.1f, 0.05f); // Subtle reflectivity for clay
	clayMaterial.shininess = 16.0f; // Slight shine to simulate ceramic finish
	clayMaterial.tag = "clay"; // Tag to identify clay material in the scene

	m_objectMaterials.push_back(clayMaterial); // Add material to the list

}


/***********************************************************
 * SetupSceneLights()
 * Define lighting for the scene
 ***********************************************************/
void SceneManager::SetupSceneLights()
{
	// Enable lighting in the shader
	m_pShaderManager->setBoolValue(g_UseLightingName, true);

	// Main directional light (simulating sunlight)
	m_pShaderManager->setVec3Value("directionalLight.direction", -0.5f, -1.0f, -0.3f); // Steeper angle for stronger shadows
	m_pShaderManager->setVec3Value("directionalLight.ambient", 0.2f, 0.2f, 0.2f); // Balanced ambient light
	m_pShaderManager->setVec3Value("directionalLight.diffuse", 0.7f, 0.7f, 0.6f); // Soft diffuse light for natural illumination
	m_pShaderManager->setVec3Value("directionalLight.specular", 0.5f, 0.5f, 0.5f); // Subtle highlights for objects
	m_pShaderManager->setBoolValue("directionalLight.bActive", true);

	// Point light 1 (illuminating the left side of the scene)
	m_pShaderManager->setVec3Value("pointLights[0].position", -3.0f, 5.0f, 2.0f); // Positioned to light the left objects
	m_pShaderManager->setVec3Value("pointLights[0].ambient", 0.1f, 0.1f, 0.1f);   // Gentle ambient light
	m_pShaderManager->setVec3Value("pointLights[0].diffuse", 0.6f, 0.6f, 0.5f);   // Warm diffuse light
	m_pShaderManager->setVec3Value("pointLights[0].specular", 0.7f, 0.7f, 0.6f);  // Bright specular reflections
	m_pShaderManager->setFloatValue("pointLights[0].constant", 1.0f);
	m_pShaderManager->setFloatValue("pointLights[0].linear", 0.09f);
	m_pShaderManager->setFloatValue("pointLights[0].quadratic", 0.032f);
	m_pShaderManager->setBoolValue("pointLights[0].bActive", true);

	// Point light 2 (illuminating the right side of the scene)
	m_pShaderManager->setVec3Value("pointLights[1].position", 4.0f, 5.0f, -2.0f); // Positioned to light the right objects
	m_pShaderManager->setVec3Value("pointLights[1].ambient", 0.1f, 0.1f, 0.1f);   // Gentle ambient light
	m_pShaderManager->setVec3Value("pointLights[1].diffuse", 0.5f, 0.5f, 0.6f);   // Cool diffuse light
	m_pShaderManager->setVec3Value("pointLights[1].specular", 0.6f, 0.6f, 0.7f);  // Sharp highlights
	m_pShaderManager->setFloatValue("pointLights[1].constant", 1.0f);
	m_pShaderManager->setFloatValue("pointLights[1].linear", 0.09f);
	m_pShaderManager->setFloatValue("pointLights[1].quadratic", 0.032f);
	m_pShaderManager->setBoolValue("pointLights[1].bActive", true);

	// Spotlight (focused on the teapot or central object)
	m_pShaderManager->setVec3Value("spotLight.position", 0.0f, 6.0f, 0.0f); // Overhead spotlight
	m_pShaderManager->setVec3Value("spotLight.direction", 0.0f, -1.0f, 0.0f); // Downward focus
	m_pShaderManager->setVec3Value("spotLight.ambient", 0.05f, 0.05f, 0.05f); // Subtle ambient light
	m_pShaderManager->setVec3Value("spotLight.diffuse", 0.7f, 0.7f, 0.6f);    // Reduced diffuse light for better focus
	m_pShaderManager->setVec3Value("spotLight.specular", 0.8f, 0.8f, 0.7f);   // Strong highlights for emphasis
	m_pShaderManager->setFloatValue("spotLight.constant", 1.0f);
	m_pShaderManager->setFloatValue("spotLight.linear", 0.07f);
	m_pShaderManager->setFloatValue("spotLight.quadratic", 0.017f);
	m_pShaderManager->setFloatValue("spotLight.cutOff", glm::cos(glm::radians(15.0f))); // Narrower focus
	m_pShaderManager->setFloatValue("spotLight.outerCutOff", glm::cos(glm::radians(25.0f))); // Softer edge
	m_pShaderManager->setBoolValue("spotLight.bActive", true);
}

/***********************************************************
 * LoadScenceTextures()
 * Load and bind textures for the scene
 ***********************************************************/
void SceneManager::LoadSceneTextures() //
{
	bool bReturn = false;
	//Loads in texture images froi
	CreateGLTexture("textures/ceramic.png", "teapot");
	CreateGLTexture("textures/woodtable.png", "table");
	CreateGLTexture("textures/backdrop.png", "background");
	CreateGLTexture("textures/woodroundtable.jpg", "roundtable");
	CreateGLTexture("textures/coffeecup.png", "cup");
	CreateGLTexture("textures/book.jpg", "book");
	CreateGLTexture("textures/Coffeeliquid.png", "coffee");
	CreateGLTexture("textures/metal.png", "handle");
	CreateGLTexture("textures/pages.png", "pages");
	CreateGLTexture("textures/bookspine.png", "spine");
	CreateGLTexture("textures/glass.png", "glasshandle");
	CreateGLTexture("textures/soiltexture.png", "soiltexture");
	CreateGLTexture("textures/leaftexture.JPG", "leaftexture");



	BindGLTextures();
}

/***********************************************************
 * PrepareScene()
 * Prepare shapes, textures, and materials
 ***********************************************************/
void SceneManager::PrepareScene()
{	// Loads the the texture image files for the textures applied
	LoadSceneTextures();
	//Define the materials that will be used for the objects
	DefineObjectMaterials();
	//This add and defile the light sources for the 3D scnce
	SetupSceneLights();

	m_basicMeshes->LoadPlaneMesh();
	m_basicMeshes->LoadCylinderMesh();
	m_basicMeshes->LoadTaperedCylinderMesh();
	m_basicMeshes->LoadTorusMesh();
	m_basicMeshes->LoadSphereMesh();
	m_basicMeshes->LoadConeMesh();
	m_basicMeshes->LoadBoxMesh();
	m_basicMeshes->LoadPyramid3Mesh();
	
}

/***********************************************************
 *  RenderScene()
 *
 *  This method is used for rendering the 3D scene by
 *  transforming and drawing the basic 3D shapes
 ***********************************************************/
void SceneManager::RenderScene()
{
	RenderTable();
	RenderBackdrop();
	RenderPercolator();
	RenderCoffeeCup();
	RenderBook();
	RenderTray();
	RenderFlowerPot();


}

/***********************************************************
 * RenderTable()
 * Render the table with its material and texture
 ***********************************************************/
void SceneManager::RenderTable()
{
	// Render the Round Table
	glm::vec3 scaleXYZ = glm::vec3(15.0f, 0.0f, 15.0f); // Scaled to make it round (same x and z for circular base)
	glm::vec3 positionXYZ = glm::vec3(0.0f, 0.0f, 0.0f); // Positioned at the origin
	SetTransformations(scaleXYZ, 0.0f, 0.0f, 0.0f, positionXYZ);

	SetShaderMaterial("plate"); // Material for the tabletop
	SetShaderTexture("roundtable"); // Texture for the tabletop
	SetTextureUVScale(1.0f, 1.0f); // Default texture scaling

	m_basicMeshes->DrawCylinderMesh(); // Render a cylinder to represent the round table
}

/***********************************************************
 * RenderBackdrop()
 * Render the backdrop with its material and texture
 ***********************************************************/
void SceneManager::RenderBackdrop()
{	// Render the Backdrop of the window
	glm::vec3 scaleXYZ = glm::vec3(20.0f, 1.0f, 20.0f);
	glm::vec3 positionXYZ = glm::vec3(0.0f, 20.0f, -10.0f);
	SetTransformations(scaleXYZ, 90.0f, 0.0f, 0.0f, positionXYZ);

	SetShaderMaterial("backdrop");
	SetShaderTexture("background");
	SetTextureUVScale(1.0f, 1.0f);

	m_basicMeshes->DrawPlaneMesh();
}

/***********************************************************
 * RenderTeapot()
 * Render the teapot with its material and texture
 ***********************************************************/
void SceneManager::RenderPercolator()
{
	// Left offset for the entire percolator
	float leftOffset = -2.5f; // Move the pot 2 units to the left

	// Render the body (narrower tapered cylinder for a pot-like shape)
	glm::vec3 scaleXYZ = glm::vec3(1.2f, 3.0f, 1.2f); // Narrower, slightly tapered shape
	glm::vec3 positionXYZ = glm::vec3(leftOffset, 0.0f, 0.0f); // Shifted left
	SetTransformations(scaleXYZ, 0.0f, 0.0f, 0.0f, positionXYZ);
	SetShaderMaterial("glass");
	SetShaderTexture("teapot");
	SetTextureUVScale(1.0f, 1.0f);
	m_basicMeshes->DrawTaperedCylinderMesh(1.0f, 1.2f, 3.0f);

	// Render the spout (tapered and angled outward)
	scaleXYZ = glm::vec3(0.4f, 1.9f, 0.4f); // Long, slightly wider spout
	positionXYZ = glm::vec3(leftOffset + 0.9f, 0.4f, 0.0f); // Adjusted for left offset
	SetTransformations(scaleXYZ, 30.0f, 90.0f, 0.0f, positionXYZ);
	SetShaderMaterial("glass");
	SetShaderTexture("teapot");
	m_basicMeshes->DrawTaperedCylinderMesh();

	// Render the handle (vertical torus handle)
	scaleXYZ = glm::vec3(0.6f, 0.8f, 0.2f); // Tall handle for ergonomic grip
	positionXYZ = glm::vec3(leftOffset - 0.9f, 1.8f, 0.0f); // Adjusted for left offset
	SetTransformations(scaleXYZ, 0.0f, 0.0f, 90.0f, positionXYZ);
	SetShaderMaterial("glass");
	SetShaderTexture("teapot");
	m_basicMeshes->DrawTorusMesh();

	// Render the lid (domed cylinder for sealing)
	scaleXYZ = glm::vec3(0.6f, 0.1f, 0.80f); // Slightly domed lid
	positionXYZ = glm::vec3(leftOffset, 3.0f, 0.0f); // Adjusted for left offset
	SetTransformations(scaleXYZ, 0.0f, 0.0f, 0.0f, positionXYZ);
	SetShaderMaterial("glass");
	SetShaderTexture("teapot");
	m_basicMeshes->DrawCylinderMesh();

	// Render the lid vent or knob (small sphere for decoration or functional vent)
	scaleXYZ = glm::vec3(0.2f, 0.1f, 0.2f); // Small and subtle
	positionXYZ = glm::vec3(leftOffset, 3.1f, 0.0f); // Adjusted for left offset
	SetTransformations(scaleXYZ, 0.0f, 0.0f, 0.0f, positionXYZ);
	SetShaderMaterial("wood");
	m_basicMeshes->DrawCylinderMesh();


}


void SceneManager::RenderCoffeeCup()
{
	// Render the coffee cup body (cylinder for the main cup)
	glm::vec3 scaleXYZ = glm::vec3(1.1f, 1.0f, 1.2f); // Dimensions for the cup body
	glm::vec3 positionXYZ = glm::vec3(0.5f, 0.0f, 1.0f); // Positioned to the side of the teapot
	SetTransformations(scaleXYZ, 0.0f, 0.0f, 0.0f, positionXYZ);
	SetShaderMaterial("glass");
	SetShaderTexture("cup");
	SetTextureUVScale(1.0f, 1.0f);
	m_basicMeshes->DrawCylinderMesh();

	// Render the coffee cup handle (a torus, offset to the side)
	scaleXYZ = glm::vec3(0.5f, 0.3f, 0.2f); // Dimensions for the handle
	positionXYZ = glm::vec3(1.5f, 0.5f, 1.0f); // Positioned on the side of the cup
	SetTransformations(scaleXYZ, 0.0f, 0.0f, 1.0f, positionXYZ);
	SetShaderMaterial("glass");
	SetShaderTexture("glasshandle");
	m_basicMeshes->DrawTorusMesh();

	// Render the coffee liquid (a scaled, flat cylinder inside the cup)
	scaleXYZ = glm::vec3(1.1f, 0.01f, 1.2f); // Slightly smaller than the cup's interior
	positionXYZ = glm::vec3(0.5f, 1.0f, 1.0f); // Positioned at the top of the cup body
	SetTransformations(scaleXYZ, 0.0f, 0.0f, 0.0f, positionXYZ);
	SetShaderMaterial("liquid");
	SetShaderTexture("coffee");
	SetTextureUVScale(1.0f, 1.0f);
	m_basicMeshes->DrawCylinderMesh();
}
void SceneManager::RenderBook()
{
	float gap = 0.02f;       // Small gap between books
	float tableHeight = 0.2f; // Height of the table
	float diagonalOffset = 0.5f; // Amount to shift diagonally (x and z)

	// First book (bottom book) - Resting on the table
	glm::vec3 scaleXYZ = glm::vec3(3.5f, 0.3f, 2.5f); // Thinner cover dimensions
	glm::vec3 positionXYZ = glm::vec3(-6.0f + diagonalOffset, tableHeight, 5.0f + diagonalOffset); // Diagonal shift
	SetTransformations(scaleXYZ, 0.0f, 0.0f, 0.0f, positionXYZ);
	SetShaderMaterial("cover");
	SetShaderTexture("book");
	SetTextureUVScale(1.0f, 1.0f);
	m_basicMeshes->DrawBoxMesh(); // Render the book cover (first book)

	// Book pages for the first book
	scaleXYZ = glm::vec3(3.53f, 0.23f, 2.3f); // Slightly smaller and thinner than the cover
	positionXYZ = glm::vec3(-6.0f + diagonalOffset, tableHeight + 0.01f, 4.89f + diagonalOffset); // Diagonal shift
	SetTransformations(scaleXYZ, 0.0f, 0.0f, 0.0f, positionXYZ);
	SetShaderMaterial("pages");
	SetShaderTexture("pages");
	SetTextureUVScale(1.0f, 1.0f);
	m_basicMeshes->DrawBoxMesh(); // Render the book pages

	// Second book (middle book) - Stacked on the first book
	scaleXYZ = glm::vec3(3.5f, 0.3f, 2.5f); // Same dimensions as the first book
	positionXYZ = glm::vec3(-6.0f + diagonalOffset, tableHeight + 0.3f + gap, 5.0f + diagonalOffset); // Diagonal shift
	SetTransformations(scaleXYZ, 0.0f, 0.0f, 0.0f, positionXYZ);
	SetShaderMaterial("cover");
	SetShaderTexture("book");
	SetTextureUVScale(1.0f, 1.0f);
	m_basicMeshes->DrawBoxMesh(); // Render the book cover (second book)

	// Book pages for the second book
	scaleXYZ = glm::vec3(3.53f, 0.23f, 2.3f); // Same dimensions as the first book's pages
	positionXYZ = glm::vec3(-6.0f + diagonalOffset, tableHeight + 0.33f + gap, 4.89f + diagonalOffset); // Diagonal shift
	SetTransformations(scaleXYZ, 0.0f, 0.0f, 0.0f, positionXYZ);
	SetShaderMaterial("pages");
	SetShaderTexture("pages");
	SetTextureUVScale(1.0f, 1.0f);
	m_basicMeshes->DrawBoxMesh(); // Render the book pages

	// Third book (top book) - Stacked on the second book
	scaleXYZ = glm::vec3(3.5f, 0.3f, 2.5f); // Same dimensions as the first book
	positionXYZ = glm::vec3(-6.0f + diagonalOffset, tableHeight + 0.6f + 2 * gap, 5.0f + diagonalOffset); // Diagonal shift
	SetTransformations(scaleXYZ, 0.0f, 0.0f, 0.0f, positionXYZ);
	SetShaderMaterial("cover");
	SetShaderTexture("book");
	SetTextureUVScale(1.0f, 1.0f);
	m_basicMeshes->DrawBoxMesh(); // Render the book cover (third book)

	// Book pages for the third book
	scaleXYZ = glm::vec3(3.53f, 0.23f, 2.3f); // Same dimensions as the first book's pages
	positionXYZ = glm::vec3(-6.0f + diagonalOffset, tableHeight + 0.61f + 2 * gap, 4.89f + diagonalOffset); // Diagonal shift
	SetTransformations(scaleXYZ, 0.0f, 0.0f, 0.0f, positionXYZ);
	SetShaderMaterial("pages");
	SetShaderTexture("pages");
	SetTextureUVScale(1.0f, 1.0f);
	m_basicMeshes->DrawBoxMesh(); // Render the book pages
}


void SceneManager::RenderTray()
{
	// Tray Base (Flat rectangular surface)
	glm::vec3 scaleXYZ = glm::vec3(8.0f, 0.05f, 5.0f); // Thin, wide base
	glm::vec3 positionXYZ = glm::vec3(0.0f, 0.05f, 0.0f); // Slightly above ground
	SetTransformations(scaleXYZ, 0.0f, 0.0f, 0.0f, positionXYZ);
	SetShaderMaterial("wood");
	SetShaderTexture("table");
	SetTextureUVScale(1.0f, 1.0f);
	m_basicMeshes->DrawBoxMesh(); // Render tray base

	float edgeHeight = 0.3f;
	float edgeThickness = 0.1f;

	// Front Edge (aligned along the Z-axis)
	scaleXYZ = glm::vec3(8.1f, edgeHeight, edgeThickness); // Slightly wider than the base
	positionXYZ = glm::vec3(0.0f, edgeHeight / 2.0f + 0.05f, -2.55f); // Offset correctly to align with front
	SetTransformations(scaleXYZ, 0.0f, 0.0f, 0.0f, positionXYZ);
	m_basicMeshes->DrawBoxMesh();

	// Back Edge
	scaleXYZ = glm::vec3(8.1f, edgeHeight, edgeThickness); // Slightly wider than the base
	positionXYZ = glm::vec3(0.0f, edgeHeight / 2.0f + 0.05f, 2.55f); // Aligned with back
	SetTransformations(scaleXYZ, 0.0f, 0.0f, 0.0f, positionXYZ);
	m_basicMeshes->DrawBoxMesh();

	// Left Edge (aligned along the X-axis)
	scaleXYZ = glm::vec3(edgeThickness, edgeHeight, 5.1f); // Slightly longer to match base
	positionXYZ = glm::vec3(-4.05f, edgeHeight / 2.0f + 0.05f, 0.0f); // Left edge alignment
	SetTransformations(scaleXYZ, 0.0f, 0.0f, 0.0f, positionXYZ);
	m_basicMeshes->DrawBoxMesh();

	// Right Edge
	scaleXYZ = glm::vec3(edgeThickness, edgeHeight, 5.1f); // Slightly longer to match base
	positionXYZ = glm::vec3(4.05f, edgeHeight / 2.0f + 0.05f, 0.0f); // Right edge alignment
	SetTransformations(scaleXYZ, 0.0f, 0.0f, 0.0f, positionXYZ);
	m_basicMeshes->DrawBoxMesh();
}
void SceneManager::RenderFlowerPot()
{
	// Render the small flower pot body (main cylinder)
	glm::vec3 scaleXYZ = glm::vec3(0.8f, 0.6f, 0.8f); // Smaller dimensions for the pot body
	glm::vec3 positionXYZ = glm::vec3(-7.0f, 0.0f, 0.0f); // Move position to -7.0f
	SetTransformations(scaleXYZ, 0.0f, 0.0f, 0.0f, positionXYZ);
	SetShaderMaterial("glass"); // Clay material for realism
	SetShaderTexture("teapot");
	SetTextureUVScale(1.0f, 1.0f);
	m_basicMeshes->DrawCylinderMesh(); // Main body of the pot

	// Render the soil inside the small pot (a flat cylinder)
	scaleXYZ = glm::vec3(0.7f, 0.05f, 0.7f); // Slightly smaller and flatter than the pot
	positionXYZ = glm::vec3(-7.0f, 0.56f, 0.0f); // Positioned inside the pot
	SetTransformations(scaleXYZ, 0.0f, 0.0f, 0.0f, positionXYZ);
	SetShaderMaterial("soil");
	SetShaderTexture("soiltexture");
	SetTextureUVScale(1.0f, 1.0f);
	m_basicMeshes->DrawCylinderMesh();

	// Render the plant as a perfect sphere
	scaleXYZ = glm::vec3(0.5f, 0.5f, 0.5f); // Uniform scale for a round plant
	positionXYZ = glm::vec3(-7.0f, 0.8f, 0.0f); // Positioned on top of the pot
	SetTransformations(scaleXYZ, 0.0f, 0.0f, 0.0f, positionXYZ); // Apply transformations
	SetShaderMaterial("leaf"); // Green material for the plant
	SetShaderTexture("leaftexture"); // Texture to simulate leaves on the sphere
	SetTextureUVScale(1.0f, 1.0f); // Example: doubles horizontal repeats

	m_basicMeshes->DrawSphereMesh(); // Render a sphere to represent the plant
}
