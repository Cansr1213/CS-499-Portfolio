// CS-499 Milestone Two Enhancement:
// Added documentation, improved consistency, helper functions, and defensive checks.
///////////////////////////////////////////////////////////////////////////////
//CS-499 Milestone Three: Enhancement Two: Algorithms and Data Structures
// Rafael V. Canseco
// 11/23/2025
// scenemanager.cpp
// ============
// manage the preparing and rendering of 3D scenes - textures, materials, lighting
//
//  AUTHOR: Brian Battersby - SNHU Instructor / Computer Science
//  Created for CS-330-Computational Graphics and Visualization, Nov. 1st, 2023
//  Updated by: Rafael Canseco (CS-499)
//
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

    if (m_basicMeshes != NULL)
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
    int  width = 0;
    int  height = 0;
    int  colorChannels = 0;
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
        std::cout << "Successfully loaded image:" << filename
            << ", width:" << width
            << ", height:" << height
            << ", channels:" << colorChannels << std::endl;

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
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height,
                0, GL_RGB, GL_UNSIGNED_BYTE, image);
        }
        // if the loaded image is in RGBA format - it supports transparency
        else if (colorChannels == 4)
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height,
                0, GL_RGBA, GL_UNSIGNED_BYTE, image);
        }
        else
        {
            std::cout << "Not implemented to handle image with "
                << colorChannels << " channels" << std::endl;
            stbi_image_free(image);
            glBindTexture(GL_TEXTURE_2D, 0);
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
  

       

		// Cs-499 Enhancement: maintain fast lookup tables for textures
		int slotIndex = m_loadedTextures;   // slot we just used 
        m_textureIdLookup[tag] = textureID;     //tag -> GL texture ID
		m_textureSlotLookup[tag] = slotIndex;      //tag -> texture slot

        // Now that everything is registered, advance the count 
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
        if (m_textureIDs[i].ID != static_cast<GLuint>(-1))
        {
            glDeleteTextures(1, &m_textureIDs[i].ID); // CS-499 fix: delete, not generate
            m_textureIDs[i].ID = static_cast<GLuint>(-1);
        }
    }
    m_loadedTextures = 0;
}

/***********************************************************
 *  FindTextureID()
 *
 *  This method is used for getting an ID for the previously
 *  loaded texture bitmap associated with the passed in tag.
 ***********************************************************/
int SceneManager::FindTextureID(std::string tag) {

	// CS-499 Enhancement: 0(1) average lookup using an unordered_map
    auto it = m_textureIdLookup.find(tag);
    if (it != m_textureIdLookup.end()) {
        return static_cast<int>(it->second);

    }
	// Fallback to legacy method if not found in cache
    int textureID = -1;
    int index = 0;
    bool bFound = false;

    while ((index < m_loadedTextures) && (bFound == false)) {
        if (m_textureIDs[index].tag.compare(tag) == 0) {
            textureID = m_textureIDs[index].ID;
            bFound = true;

        }
        else {
            index++;

        }
    }
    return textureID;

}

/***********************************************************
 *  FindTextureSlot()
 *
 *  This method is used for getting a slot index for the previously
 *  loaded texture bitmap associated with the passed in tag.
 ***********************************************************/
int SceneManager::FindTextureSlot(std::string tag) {
	// CS-499 Enhancement: 0(1) average lookup using an unordered_map

    auto it = m_textureSlotLookup.find(tag);
    if (it != m_textureSlotLookup.end()) {
        return it->second;

    }

	// Fallback to legacy linear search if not found in cache

    int textureSlot = -1;
    int index = 0;
    bool bFound = false;

    while ((index < m_loadedTextures) && (bFound == false)) {
        if (m_textureIDs[index].tag.compare(tag) == 0) {
            textureSlot = index;
            bFound = true;

        }
        else {
            index++;

        }
    }
    return textureSlot;

}

/***********************************************************
 *  FindMaterial()
 *
 *  This method is used for getting a material from the previously
 *  defined materials list that is associated with the passed in tag.
 ***********************************************************/
bool SceneManager::FindMaterial(std::string tag, OBJECT_MATERIAL& material) {

	// CS-499 Enhancement: 0(1) average lookup using a hash map
    auto it = m_materialLookup.find(tag);
    if (it != m_materialLookup.end()) {
        material = it->second;
        return true;
	}

	// Fallback to legacy linear search if map is not populated
    if (m_objectMaterials.empty()) {
        return false;

    }
    int index = 0;
    bool bFound = false;
    while ((index < static_cast<int>(m_objectMaterials.size())) && (bFound == false)) {
        if (m_objectMaterials[index].tag.compare(tag) == 0) {
            bFound = true;
            material = m_objectMaterials[index];

        }
        else {
            index++;

        }
    }
    return bFound;
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

    if (m_pShaderManager != NULL)
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
    glm::vec4 currentColor;
    currentColor.r = redColorValue;
    currentColor.g = greenColorValue;
    currentColor.b = blueColorValue;
    currentColor.a = alphaValue;

    if (m_pShaderManager != NULL)
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
void SceneManager::SetShaderTexture(std::string textureTag)
{
    if (m_pShaderManager != NULL)
    {
        m_pShaderManager->setIntValue(g_UseTextureName, true);

        int textureSlot = FindTextureSlot(textureTag);
        m_pShaderManager->setSampler2DValue(g_TextureValueName, textureSlot);
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
    if (m_pShaderManager != NULL)
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
void SceneManager::SetShaderMaterial(std::string materialTag)
{
    if (m_pShaderManager == NULL || m_objectMaterials.empty())
        return;

    OBJECT_MATERIAL material;
    bool bReturn = FindMaterial(materialTag, material);
    if (bReturn)
    {
        m_pShaderManager->setVec3Value("material.diffuseColor", material.diffuseColor);
        m_pShaderManager->setVec3Value("material.specularColor", material.specularColor);
        m_pShaderManager->setFloatValue("material.shininess", material.shininess);
    }
}

void SceneManager::DefineObjectMaterials()
{
    // Define material properties for a polished silver appearance
    OBJECT_MATERIAL silverMaterial;
    silverMaterial.diffuseColor = glm::vec3(0.7f, 0.7f, 0.7f);
    silverMaterial.specularColor = glm::vec3(0.9f, 0.9f, 0.9f);
    silverMaterial.shininess = 64.0f;
    silverMaterial.tag = "metal";
    m_objectMaterials.push_back(silverMaterial);

    // Define material properties for a rich wood texture
    OBJECT_MATERIAL woodMaterial;
    woodMaterial.diffuseColor = glm::vec3(0.3f, 0.2f, 0.1f);
    woodMaterial.specularColor = glm::vec3(0.1f, 0.1f, 0.1f);
    woodMaterial.shininess = 16.0f;
    woodMaterial.tag = "wood";
    m_objectMaterials.push_back(woodMaterial);

    // Define material properties for tinted glass
    OBJECT_MATERIAL glassMaterial;
    glassMaterial.diffuseColor = glm::vec3(0.2f, 0.2f, 0.3f);
    glassMaterial.specularColor = glm::vec3(1.0f, 1.0f, 1.0f);
    glassMaterial.shininess = 128.0f;
    glassMaterial.tag = "glass";
    m_objectMaterials.push_back(glassMaterial);

    // Define material properties for a simple plate
    OBJECT_MATERIAL plateMaterial;
    plateMaterial.diffuseColor = glm::vec3(0.5f, 0.5f, 0.5f);
    plateMaterial.specularColor = glm::vec3(0.3f, 0.3f, 0.3f);
    plateMaterial.shininess = 24.0f;
    plateMaterial.tag = "plate";
    m_objectMaterials.push_back(plateMaterial);

    // Define material properties for a backdrop or background plane
    OBJECT_MATERIAL backdropMaterial;
    backdropMaterial.diffuseColor = glm::vec3(0.8f, 0.8f, 0.9f);
    backdropMaterial.specularColor = glm::vec3(0.2f, 0.2f, 0.2f);
    backdropMaterial.shininess = 8.0f;
    backdropMaterial.tag = "backdrop";
    m_objectMaterials.push_back(backdropMaterial);

    // Define material properties for coffee-like liquid
    OBJECT_MATERIAL coffeeMaterial;
    coffeeMaterial.diffuseColor = glm::vec3(0.4f, 0.25f, 0.1f);
    coffeeMaterial.specularColor = glm::vec3(0.5f, 0.3f, 0.2f);
    coffeeMaterial.shininess = 30.0f;
    coffeeMaterial.tag = "liquid";
    m_objectMaterials.push_back(coffeeMaterial);

    // Define material properties for the cover of a book
    OBJECT_MATERIAL coverMaterial;
    coverMaterial.diffuseColor = glm::vec3(1.0f, 1.0f, 1.0f);
    coverMaterial.specularColor = glm::vec3(0.9f, 0.9f, 0.9f);
    coverMaterial.shininess = 64.0f;
    coverMaterial.tag = "cover";
    m_objectMaterials.push_back(coverMaterial);

    // Define material properties for the spine of a book
    OBJECT_MATERIAL spineMaterial;
    spineMaterial.diffuseColor = glm::vec3(0.3f, 0.15f, 0.15f);
    spineMaterial.specularColor = glm::vec3(0.4f, 0.2f, 0.2f);
    spineMaterial.shininess = 20.0f;
    spineMaterial.tag = "spine";
    m_objectMaterials.push_back(spineMaterial);

    // Define material properties for the pages of a book
    OBJECT_MATERIAL pagesMaterial;
    pagesMaterial.diffuseColor = glm::vec3(0.9f, 0.9f, 0.85f);
    pagesMaterial.specularColor = glm::vec3(0.2f, 0.2f, 0.2f);
    pagesMaterial.shininess = 5.0f;
    pagesMaterial.tag = "pages";
    m_objectMaterials.push_back(pagesMaterial);

    // Leaf Material
    OBJECT_MATERIAL leafMaterial;
    leafMaterial.diffuseColor = glm::vec3(0.2f, 0.6f, 0.2f);
    leafMaterial.specularColor = glm::vec3(0.1f, 0.3f, 0.1f);
    leafMaterial.shininess = 6.0f;
    leafMaterial.tag = "leaf";
    m_objectMaterials.push_back(leafMaterial);

    // Soil Material
    OBJECT_MATERIAL soilMaterial;
    soilMaterial.diffuseColor = glm::vec3(0.2f, 0.1f, 0.0f);
    soilMaterial.specularColor = glm::vec3(0.05f, 0.02f, 0.01f);
    soilMaterial.shininess = 4.0f;
    soilMaterial.tag = "soil";
    m_objectMaterials.push_back(soilMaterial);

    // Clay Material
    OBJECT_MATERIAL clayMaterial;
    clayMaterial.diffuseColor = glm::vec3(0.8f, 0.5f, 0.3f);
    clayMaterial.specularColor = glm::vec3(0.2f, 0.1f, 0.05f);
    clayMaterial.shininess = 16.0f;
    clayMaterial.tag = "clay";
    m_objectMaterials.push_back(clayMaterial);

	// CS-499 Enhancement: build fast lookup map for materials
    m_materialLookup.clear();
    for (const auto& mat : m_objectMaterials) {
        m_materialLookup[mat.tag] = mat;
	}
}

/***********************************************************
 * SetupSceneLights()
 * Define lighting for the scene
 ***********************************************************/
void SceneManager::SetupSceneLights()
{
    if (m_pShaderManager == NULL)
        return;

    // Enable lighting in the shader
    m_pShaderManager->setBoolValue(g_UseLightingName, true);

    // Main directional light (simulating sunlight)
    m_pShaderManager->setVec3Value("directionalLight.direction", -0.5f, -1.0f, -0.3f);
    m_pShaderManager->setVec3Value("directionalLight.ambient", 0.2f, 0.2f, 0.2f);
    m_pShaderManager->setVec3Value("directionalLight.diffuse", 0.7f, 0.7f, 0.6f);
    m_pShaderManager->setVec3Value("directionalLight.specular", 0.5f, 0.5f, 0.5f);
    m_pShaderManager->setBoolValue("directionalLight.bActive", true);

    // Point light 1
    m_pShaderManager->setVec3Value("pointLights[0].position", -3.0f, 5.0f, 2.0f);
    m_pShaderManager->setVec3Value("pointLights[0].ambient", 0.1f, 0.1f, 0.1f);
    m_pShaderManager->setVec3Value("pointLights[0].diffuse", 0.6f, 0.6f, 0.5f);
    m_pShaderManager->setVec3Value("pointLights[0].specular", 0.7f, 0.7f, 0.6f);
    m_pShaderManager->setFloatValue("pointLights[0].constant", 1.0f);
    m_pShaderManager->setFloatValue("pointLights[0].linear", 0.09f);
    m_pShaderManager->setFloatValue("pointLights[0].quadratic", 0.032f);
    m_pShaderManager->setBoolValue("pointLights[0].bActive", true);

    // Point light 2
    m_pShaderManager->setVec3Value("pointLights[1].position", 4.0f, 5.0f, -2.0f);
    m_pShaderManager->setVec3Value("pointLights[1].ambient", 0.1f, 0.1f, 0.1f);
    m_pShaderManager->setVec3Value("pointLights[1].diffuse", 0.5f, 0.5f, 0.6f);
    m_pShaderManager->setVec3Value("pointLights[1].specular", 0.6f, 0.6f, 0.7f);
    m_pShaderManager->setFloatValue("pointLights[1].constant", 1.0f);
    m_pShaderManager->setFloatValue("pointLights[1].linear", 0.09f);
    m_pShaderManager->setFloatValue("pointLights[1].quadratic", 0.032f);
    m_pShaderManager->setBoolValue("pointLights[1].bActive", true);

    // Spotlight (overhead)
    m_pShaderManager->setVec3Value("spotLight.position", 0.0f, 6.0f, 0.0f);
    m_pShaderManager->setVec3Value("spotLight.direction", 0.0f, -1.0f, 0.0f);
    m_pShaderManager->setVec3Value("spotLight.ambient", 0.05f, 0.05f, 0.05f);
    m_pShaderManager->setVec3Value("spotLight.diffuse", 0.7f, 0.7f, 0.6f);
    m_pShaderManager->setVec3Value("spotLight.specular", 0.8f, 0.8f, 0.7f);
    m_pShaderManager->setFloatValue("spotLight.constant", 1.0f);
    m_pShaderManager->setFloatValue("spotLight.linear", 0.07f);
    m_pShaderManager->setFloatValue("spotLight.quadratic", 0.017f);
    m_pShaderManager->setFloatValue("spotLight.cutOff", glm::cos(glm::radians(15.0f)));
    m_pShaderManager->setFloatValue("spotLight.outerCutOff", glm::cos(glm::radians(25.0f)));
    m_pShaderManager->setBoolValue("spotLight.bActive", true);
}

/***********************************************************
 * LoadSceneTextures()
 * Load and bind textures for the scene
 ***********************************************************/
void SceneManager::LoadSceneTextures()
{
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
{
    LoadSceneTextures();
    DefineObjectMaterials();
    SetupSceneLights();

    if (m_basicMeshes == NULL)
        return;

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
    if (m_basicMeshes == NULL)
    {
        std::cerr << "ERROR: m_basicMeshes is null in RenderScene()." << std::endl;
        return;
    }

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
    glm::vec3 scaleXYZ = glm::vec3(15.0f, 0.0f, 15.0f);
    glm::vec3 positionXYZ = glm::vec3(0.0f, 0.0f, 0.0f);
    SetTransformations(scaleXYZ, 0.0f, 0.0f, 0.0f, positionXYZ);

    SetShaderMaterial("plate");
    SetShaderTexture("roundtable");
    SetTextureUVScale(1.0f, 1.0f);

    m_basicMeshes->DrawCylinderMesh();
}

/***********************************************************
 * RenderBackdrop()
 * Render the backdrop with its material and texture
 ***********************************************************/
void SceneManager::RenderBackdrop()
{
    glm::vec3 scaleXYZ = glm::vec3(20.0f, 1.0f, 20.0f);
    glm::vec3 positionXYZ = glm::vec3(0.0f, 20.0f, -10.0f);
    SetTransformations(scaleXYZ, 90.0f, 0.0f, 0.0f, positionXYZ);

    SetShaderMaterial("backdrop");
    SetShaderTexture("background");
    SetTextureUVScale(1.0f, 1.0f);

    m_basicMeshes->DrawPlaneMesh();
}

/***********************************************************
 * RenderPercolator()
 * Render the teapot/percolator with its material and texture
 ***********************************************************/
void SceneManager::RenderPercolator()
{
    const float leftOffset = -2.5f;

    // Body
    glm::vec3 scaleXYZ = glm::vec3(1.2f, 3.0f, 1.2f);
    glm::vec3 positionXYZ = glm::vec3(leftOffset, 0.0f, 0.0f);
    SetTransformations(scaleXYZ, 0.0f, 0.0f, 0.0f, positionXYZ);
    SetShaderMaterial("glass");
    SetShaderTexture("teapot");
    SetTextureUVScale(1.0f, 1.0f);
    m_basicMeshes->DrawTaperedCylinderMesh(1.0f, 1.2f, 3.0f);

    // Spout
    scaleXYZ = glm::vec3(0.4f, 1.9f, 0.4f);
    positionXYZ = glm::vec3(leftOffset + 0.9f, 0.4f, 0.0f);
    SetTransformations(scaleXYZ, 30.0f, 90.0f, 0.0f, positionXYZ);
    SetShaderMaterial("glass");
    SetShaderTexture("teapot");
    m_basicMeshes->DrawTaperedCylinderMesh();

    // Handle
    scaleXYZ = glm::vec3(0.6f, 0.8f, 0.2f);
    positionXYZ = glm::vec3(leftOffset - 0.9f, 1.8f, 0.0f);
    SetTransformations(scaleXYZ, 0.0f, 0.0f, 90.0f, positionXYZ);
    SetShaderMaterial("glass");
    SetShaderTexture("teapot");
    m_basicMeshes->DrawTorusMesh();

    // Lid
    scaleXYZ = glm::vec3(0.6f, 0.1f, 0.80f);
    positionXYZ = glm::vec3(leftOffset, 3.0f, 0.0f);
    SetTransformations(scaleXYZ, 0.0f, 0.0f, 0.0f, positionXYZ);
    SetShaderMaterial("glass");
    SetShaderTexture("teapot");
    m_basicMeshes->DrawCylinderMesh();

    // Knob
    scaleXYZ = glm::vec3(0.2f, 0.1f, 0.2f);
    positionXYZ = glm::vec3(leftOffset, 3.1f, 0.0f);
    SetTransformations(scaleXYZ, 0.0f, 0.0f, 0.0f, positionXYZ);
    SetShaderMaterial("wood");
    m_basicMeshes->DrawCylinderMesh();
}

/***********************************************************
 * RenderCoffeeCup()
 ***********************************************************/
void SceneManager::RenderCoffeeCup()
{
    // Cup body
    glm::vec3 scaleXYZ = glm::vec3(1.1f, 1.0f, 1.2f);
    glm::vec3 positionXYZ = glm::vec3(0.5f, 0.0f, 1.0f);
    SetTransformations(scaleXYZ, 0.0f, 0.0f, 0.0f, positionXYZ);
    SetShaderMaterial("glass");
    SetShaderTexture("cup");
    SetTextureUVScale(1.0f, 1.0f);
    m_basicMeshes->DrawCylinderMesh();

    // Handle
    scaleXYZ = glm::vec3(0.5f, 0.3f, 0.2f);
    positionXYZ = glm::vec3(1.5f, 0.5f, 1.0f);
    SetTransformations(scaleXYZ, 0.0f, 0.0f, 1.0f, positionXYZ);
    SetShaderMaterial("glass");
    SetShaderTexture("glasshandle");
    m_basicMeshes->DrawTorusMesh();

    // Coffee liquid
    scaleXYZ = glm::vec3(1.1f, 0.01f, 1.2f);
    positionXYZ = glm::vec3(0.5f, 1.0f, 1.0f);
    SetTransformations(scaleXYZ, 0.0f, 0.0f, 0.0f, positionXYZ);
    SetShaderMaterial("liquid");
    SetShaderTexture("coffee");
    SetTextureUVScale(1.0f, 1.0f);
    m_basicMeshes->DrawCylinderMesh();
}

/***********************************************************
 * Helper: RenderBookSection()
 * CS-499 Enhancement - removes duplicated boilerplate
 ***********************************************************/
void SceneManager::RenderBookSection(
    const glm::vec3& scaleXYZ,
    const glm::vec3& positionXYZ,
    const std::string& materialTag,
    const std::string& textureTag)
{
    SetTransformations(scaleXYZ, 0.0f, 0.0f, 0.0f, positionXYZ);
    SetShaderMaterial(materialTag);
    SetShaderTexture(textureTag);
    SetTextureUVScale(1.0f, 1.0f);
    m_basicMeshes->DrawBoxMesh();
}



/***********************************************************
 * RenderBook()
 ***********************************************************/
void SceneManager::RenderBook()
{
    float gap = 0.02f;   // Small gap between books
    float tableHeight = 0.2f;    // Height of the table
    float diagonalOffset = 0.5f;  // Amount to shift diagonally (x and z)

    // First book (bottom)
    RenderBookSection(
        glm::vec3(3.5f, 0.3f, 2.5f),
        glm::vec3(-6.0f + diagonalOffset, tableHeight, 5.0f + diagonalOffset),
        "cover",
        "book");

    RenderBookSection(
        glm::vec3(3.53f, 0.23f, 2.3f),
        glm::vec3(-6.0f + diagonalOffset, tableHeight + 0.01f, 4.89f + diagonalOffset),
        "pages",
        "pages");

    // Second book (middle)
    RenderBookSection(
        glm::vec3(3.5f, 0.3f, 2.5f),
        glm::vec3(-6.0f + diagonalOffset, tableHeight + 0.3f + gap, 5.0f + diagonalOffset),
        "cover",
        "book");

    RenderBookSection(
        glm::vec3(3.53f, 0.23f, 2.3f),
        glm::vec3(-6.0f + diagonalOffset, tableHeight + 0.33f + gap, 4.89f + diagonalOffset),
        "pages",
        "pages");

    // Third book (top)
    RenderBookSection(
        glm::vec3(3.5f, 0.3f, 2.5f),
        glm::vec3(-6.0f + diagonalOffset, tableHeight + 0.6f + 2 * gap, 5.0f + diagonalOffset),
        "cover",
        "book");

    RenderBookSection(
        glm::vec3(3.53f, 0.23f, 2.3f),
        glm::vec3(-6.0f + diagonalOffset, tableHeight + 0.61f + 2 * gap, 4.89f + diagonalOffset),
        "pages",
        "pages");
}

/***********************************************************
 * RenderTray()
 ***********************************************************/
void SceneManager::RenderTray()
{
    // Tray Base
    glm::vec3 scaleXYZ = glm::vec3(8.0f, 0.05f, 5.0f);
    glm::vec3 positionXYZ = glm::vec3(0.0f, 0.05f, 0.0f);
    SetTransformations(scaleXYZ, 0.0f, 0.0f, 0.0f, positionXYZ);
    SetShaderMaterial("wood");
    SetShaderTexture("table");
    SetTextureUVScale(1.0f, 1.0f);
    m_basicMeshes->DrawBoxMesh();

    float edgeHeight = 0.3f;
    float edgeThickness = 0.1f;

    // Front edge
    scaleXYZ = glm::vec3(8.1f, edgeHeight, edgeThickness);
    positionXYZ = glm::vec3(0.0f, edgeHeight / 2.0f + 0.05f, -2.55f);
    SetTransformations(scaleXYZ, 0.0f, 0.0f, 0.0f, positionXYZ);
    m_basicMeshes->DrawBoxMesh();

    // Back edge
    scaleXYZ = glm::vec3(8.1f, edgeHeight, edgeThickness);
    positionXYZ = glm::vec3(0.0f, edgeHeight / 2.0f + 0.05f, 2.55f);
    SetTransformations(scaleXYZ, 0.0f, 0.0f, 0.0f, positionXYZ);
    m_basicMeshes->DrawBoxMesh();

    // Left edge
    scaleXYZ = glm::vec3(edgeThickness, edgeHeight, 5.1f);
    positionXYZ = glm::vec3(-4.05f, edgeHeight / 2.0f + 0.05f, 0.0f);
    SetTransformations(scaleXYZ, 0.0f, 0.0f, 0.0f, positionXYZ);
    m_basicMeshes->DrawBoxMesh();

    // Right edge
    scaleXYZ = glm::vec3(edgeThickness, edgeHeight, 5.1f);
    positionXYZ = glm::vec3(4.05f, edgeHeight / 2.0f + 0.05f, 0.0f);
    SetTransformations(scaleXYZ, 0.0f, 0.0f, 0.0f, positionXYZ);
    m_basicMeshes->DrawBoxMesh();
}

/***********************************************************
 * RenderFlowerPot()
 ***********************************************************/
void SceneManager::RenderFlowerPot()
{
    // Pot body
    glm::vec3 scaleXYZ = glm::vec3(0.8f, 0.6f, 0.8f);
    glm::vec3 positionXYZ = glm::vec3(-7.0f, 0.0f, 0.0f);
    SetTransformations(scaleXYZ, 0.0f, 0.0f, 0.0f, positionXYZ);
    SetShaderMaterial("glass");   // using glass material for stylized look
    SetShaderTexture("teapot");
    SetTextureUVScale(1.0f, 1.0f);
    m_basicMeshes->DrawCylinderMesh();

    // Soil
    scaleXYZ = glm::vec3(0.7f, 0.05f, 0.7f);
    positionXYZ = glm::vec3(-7.0f, 0.56f, 0.0f);
    SetTransformations(scaleXYZ, 0.0f, 0.0f, 0.0f, positionXYZ);
    SetShaderMaterial("soil");
    SetShaderTexture("soiltexture");
    SetTextureUVScale(1.0f, 1.0f);
    m_basicMeshes->DrawCylinderMesh();

    // Plant sphere
    scaleXYZ = glm::vec3(0.5f, 0.5f, 0.5f);
    positionXYZ = glm::vec3(-7.0f, 0.8f, 0.0f);
    SetTransformations(scaleXYZ, 0.0f, 0.0f, 0.0f, positionXYZ);
    SetShaderMaterial("leaf");
    SetShaderTexture("leaftexture");
    SetTextureUVScale(1.0f, 1.0f);
    m_basicMeshes->DrawSphereMesh();
}
