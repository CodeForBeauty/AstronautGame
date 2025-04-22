#pragma once

#include "Window.h"
#include "Camera.h"
#include "Mesh.h"
#include "Lighting.h"
#include "Skeleton.h"

#include <vector>
#include <unordered_map>
#include <memory>
#include <filesystem>

#include <assimp/scene.h>


namespace glender {
	struct FileLoadData {
	public:
		std::vector<std::shared_ptr<Texture>> Textures;
		std::vector<std::shared_ptr<PbrMaterial>> Materials;
		std::vector<std::shared_ptr<SolidMesh>> SolidMeshes;
		std::vector<std::shared_ptr<SkinnedMesh>> SkinnedMeshes;
	};

	class Scene {
	public:
		Scene(Window* linkedWindow);

		template<class T, typename... TArgs>
		std::shared_ptr<T> SpawnMesh(const TArgs&... args);

		template<class T, typename... TArgs>
		std::shared_ptr<T> SpawnMesh(Transform& parent, const TArgs&... args);

		void RemoveMesh(std::shared_ptr<Mesh> mesh);

		std::shared_ptr<SolidMesh> SpawnCube(Transform& parent, std::shared_ptr<Material> material);
		std::shared_ptr<SolidMesh> SpawnCube(std::shared_ptr<Material> material);

		template<class T, typename... TArgs>
		std::shared_ptr<T> SpawnLight(const TArgs&... args);

		template<class T, typename... TArgs>
		std::shared_ptr<T> CreateMaterial(const TArgs&... args);

		std::shared_ptr<PbrMaterial> CreateTextMaterial(lm::vec4 albedo = { 1.0f, 1.0f, 1.0f, 1.0f }, float specular = 0.5f, 
			float roughness = 0.5f, float metalic = 0.0f);
		std::shared_ptr<PbrMaterial> CreateSolidMaterial(lm::vec4 albedo = { 1.0f, 1.0f, 1.0f, 1.0f }, float specular = 0.5f,
			float roughness = 0.5f, float metalic = 0.0f);
		std::shared_ptr<PbrMaterial> CreateSkinnedMaterial(lm::vec4 albedo = { 1.0f, 1.0f, 1.0f, 1.0f }, float specular = 0.5f,
			float roughness = 0.5f, float metalic = 0.0f);

		std::shared_ptr<Texture> LoadTexture(const std::filesystem::path& path, const TextureDataType dataType = TextureDataUByte, 
			const TextureUse use = TextureUseColor);
		std::shared_ptr<Texture> AddTexture(std::shared_ptr<Texture> texture);

		std::shared_ptr<FontMap> LoadFont(const std::filesystem::path& path, const int2 mapSize);

		void AddMesh(std::shared_ptr<Mesh> mesh);

		FileLoadData LoadFile(const std::filesystem::path& path);

		std::unordered_map<std::filesystem::path, std::shared_ptr<Texture>> SceneTextures;
		std::vector<std::shared_ptr<Material>> SceneMaterials;
		std::vector<std::shared_ptr<Mesh>> SceneMeshes;
		std::vector<std::shared_ptr<Light>> SceneLights;
		std::unordered_map<std::filesystem::path, std::shared_ptr<FontMap>> SceneFonts;

		Camera SceneCamera;

		Entity SceneRoot;

		Window* SceneWindow;
	protected:
	private:
		std::shared_ptr<Texture> TextureFromAssimp(aiMaterial* material, FileLoadData& data, aiTextureType texType);
		std::shared_ptr<PbrMaterial> MaterialFromAssimp(aiMaterial* material, const aiScene* scene, FileLoadData& data);
		std::shared_ptr<SolidMesh> SolidMeshFromAssimp(aiMesh* mesh, const aiScene* scene, FileLoadData& data);
		std::shared_ptr<SkinnedMesh> SkinnedMeshFromAssimp(aiMesh* mesh, const aiScene* scene, FileLoadData& data);
		std::shared_ptr<Mesh> MeshFromAssimp(aiMesh* mesh, const aiScene* scene, FileLoadData& data);

		void LoadMeshDataFromAssimp(aiMesh* assimpMesh, std::shared_ptr<Mesh> mesh);

		void ProcessAssimpNode(Transform* parent, aiNode* node, const aiScene* scene, FileLoadData& data);

		std::filesystem::path m_lastDir;
		std::vector<std::shared_ptr<PbrMaterial>> m_loadedMaterials;
	};

	template<class T, typename... TArgs>
	inline std::shared_ptr<T> Scene::SpawnMesh(const TArgs&... args) {
		std::shared_ptr<T> mesh = std::make_shared<T>(args...);
		SceneMeshes.emplace_back(mesh);

		mesh->EntityTransform.SetParent(&SceneRoot.EntityTransform);

		return mesh;
	}

	template<class T, typename ...TArgs>
	inline std::shared_ptr<T> Scene::SpawnMesh(Transform& parent, const TArgs & ...args) {
		std::shared_ptr<T> mesh = std::make_shared<T>(args...);
		SceneMeshes.emplace_back(mesh);

		mesh->EntityTransform.SetParent(&parent);

		return mesh;
	}

	template<class T, typename... TArgs>
	inline std::shared_ptr<T> Scene::CreateMaterial(const TArgs&... args) {
		std::shared_ptr<T> material = std::make_shared<T>(args...);
		SceneMaterials.emplace_back(material);
		return material;
	}

	template<class T, typename ...TArgs>
	inline std::shared_ptr<T> Scene::SpawnLight(const TArgs & ...args) {
		std::shared_ptr<T> light = std::make_shared<T>(args...);
		SceneLights.emplace_back(light);

		light->EntityTransform.SetParent(&SceneRoot.EntityTransform);

		return light;
	}
}