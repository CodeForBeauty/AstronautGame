#include "Scene.h"

#include "Logging.h"

#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <stb/stb_image.h>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

using namespace glender;
using namespace std;


Scene::Scene(Window* linkedWindow) : SceneWindow(linkedWindow),
		SceneCamera(linkedWindow, linkedWindow->GetWindowSize(), 45, 0.8f, 1000.0f) {
	CreateMaterial<PbrMaterial>(lm::vec4{1, 1, 1, 1}, 0.5f, 0.5f, 0.2f);
}

void glender::Scene::RemoveMesh(std::shared_ptr<Mesh> mesh) {
	SceneMeshes.erase(find(SceneMeshes.begin(), SceneMeshes.end(), mesh));
	mesh->EntityTransform.SetParent(nullptr);
}

std::shared_ptr<SolidMesh> glender::Scene::SpawnCube(Transform& parent, std::shared_ptr<Material> material) {
	std::shared_ptr<SolidMesh> mesh = SpawnMesh<SolidMesh>(parent, material);

	mesh->SetGeometry({
						{ {-1,  1,  1}, { 0,  0,  1}, { 1,  0,  0} }, // 0 Front
						{ { 1,  1,  1}, { 0,  0,  1}, { 1,  0,  0} }, // 1
						{ { 1, -1,  1}, { 0,  0,  1}, { 1,  0,  0} }, // 2
						{ {-1, -1,  1}, { 0,  0,  1}, { 1,  0,  0} }, // 3
						
						{ {-1,  1, -1}, { 0,  0, -1}, { 1,  0,  0} }, // 4 Back
						{ { 1,  1, -1}, { 0,  0, -1}, { 1,  0,  0} }, // 5
						{ { 1, -1, -1}, { 0,  0, -1}, { 1,  0,  0} }, // 6
						{ {-1, -1, -1}, { 0,  0, -1}, { 1,  0,  0} }, // 7

						{ { 1, -1, -1}, { 1,  0,  0}, { 0,  1,  0} }, // 8 Right
						{ { 1,  1, -1}, { 1,  0,  0}, { 0,  1,  0} }, // 9
						{ { 1,  1,  1}, { 1,  0,  0}, { 0,  1,  0} }, // 10
						{ { 1, -1,  1}, { 1,  0,  0}, { 0,  1,  0} }, // 11

						{ {-1, -1,  1}, {-1,  0,  0}, { 0,  1,  0} }, // 12 Left
						{ {-1,  1,  1}, {-1,  0,  0}, { 0,  1,  0} }, // 13
						{ {-1,  1, -1}, {-1,  0,  0}, { 0,  1,  0} }, // 14
						{ {-1, -1, -1}, {-1,  0,  0}, { 0,  1,  0} }, // 15

						{ { 1,  1,  1}, { 0,  1,  0}, { 0,  0,  1} }, // 16 Up
						{ { 1,  1, -1}, { 0,  1,  0}, { 0,  0,  1} }, // 17
						{ {-1,  1, -1}, { 0,  1,  0}, { 0,  0,  1} }, // 18
						{ {-1,  1,  1}, { 0,  1,  0}, { 0,  0,  1} }, // 19

						{ { 1, -1, -1}, { 0, -1,  0}, { 0,  0,  1} }, // 20 Down
						{ { 1, -1,  1}, { 0, -1,  0}, { 0,  0,  1} }, // 21
						{ {-1, -1,  1}, { 0, -1,  0}, { 0,  0,  1} }, // 22
						{ {-1, -1, -1}, { 0, -1,  0}, { 0,  0,  1} }, // 23
					}, 
					{
						{ { 0.0f,  1.0f }, { 1,  1,  1} }, // 0 Front
						{ { 1.0f,  1.0f }, { 1,  1,  1} }, // 1
						{ { 1.0f,  0.0f }, { 1,  1,  1} }, // 2
						{ { 0.0f,  0.0f }, { 1,  1,  1} }, // 3
								  
						{ { 0.0f,  1.0f }, { 1,  1,  1} }, // 4 Back
						{ { 1.0f,  1.0f }, { 1,  1,  1} }, // 5
						{ { 1.0f,  0.0f }, { 1,  1,  1} }, // 6
						{ { 0.0f,  0.0f }, { 1,  1,  1} }, // 7
								 
						{ { 0.0f,  1.0f }, { 1,  1,  1} }, // 8 Right
						{ { 1.0f,  1.0f }, { 1,  1,  1} }, // 9
						{ { 1.0f,  0.0f }, { 1,  1,  1} }, // 10
						{ { 0.0f,  0.0f }, { 1,  1,  1} }, // 11
								  
						{ { 0.0f,  1.0f }, { 1,  1,  1} }, // 12 Left
						{ { 1.0f,  1.0f }, { 1,  1,  1} }, // 13
						{ { 1.0f,  0.0f }, { 1,  1,  1} }, // 14
						{ { 0.0f,  0.0f }, { 1,  1,  1} }, // 15
								   	
						{ { 0.0f,  1.0f }, { 1,  1,  1} }, // 16 Up
						{ { 1.0f,  1.0f }, { 1,  1,  1} }, // 17
						{ { 1.0f,  0.0f }, { 1,  1,  1} }, // 18
						{ { 0.0f,  0.0f }, { 1,  1,  1} }, // 19
								   	
						{ { 0.0f,  1.0f }, { 1,  1,  1} }, // 20 Down
						{ { 1.0f,  1.0f }, { 1,  1,  1} }, // 21
						{ { 1.0f,  0.0f }, { 1,  1,  1} }, // 22
						{ { 0.0f,  0.0f }, { 1,  1,  1} }, // 23
					}, 
						{
								2, 1, 0,
								0, 3, 2,

								4, 5, 6,
								6, 7, 4,

								8, 9, 10,
								10, 11, 8,

								12, 13, 14,
								14, 15, 12,

								16, 17, 18,
								18, 19, 16,

								20, 21, 22,
								22, 23, 20

								/*6, 5, 1,
								1, 2, 6,

								3, 0, 4,
								4, 7, 3,

								1, 5, 4,
								4, 0, 1,

								6, 2, 3,
								3, 7, 6 */ });

	return mesh;
}

std::shared_ptr<SolidMesh> glender::Scene::SpawnCube(std::shared_ptr<Material> material) {
	return SpawnCube(SceneRoot.EntityTransform, material);
}

shared_ptr<PbrMaterial> Scene::CreateTextMaterial(lm::vec4 albedo, float specular, float roughness, float metalic) {
	return CreateMaterial<PbrMaterial>(albedo, specular, roughness, metalic,
		vector<filesystem::path>{ "shaders/Default.vert", "shaders/Text/TextRender.frag", "shaders/Default.geom" });
}

shared_ptr<PbrMaterial> Scene::CreateSolidMaterial(lm::vec4 albedo, float specular, float roughness, float metalic) {
	return CreateMaterial<PbrMaterial>(albedo, specular, roughness, metalic,
		vector<filesystem::path>{ "shaders/Default.vert", "shaders/Default.frag", "shaders/Default.geom" });
}

shared_ptr<PbrMaterial> Scene::CreateSkinnedMaterial(lm::vec4 albedo, float specular, float roughness, float metalic) {
	return CreateMaterial<PbrMaterial>(albedo, specular, roughness, metalic,
		vector<filesystem::path>{ "shaders/Skinned.vert", "shaders/Default.frag", "shaders/Default.geom" });
}

shared_ptr<Texture> Scene::LoadTexture(const filesystem::path& path, const TextureDataType dataType, const TextureUse use) {
	if (SceneTextures.find(path) != SceneTextures.end()) {
		return SceneTextures[path];
	}
	shared_ptr<Texture2D> texture = make_shared<Texture2D>(path, dataType, use);
	SceneTextures[path] = texture;
	return texture;
}

shared_ptr<Texture> Scene::AddTexture(shared_ptr<Texture> texture) {
	filesystem::path path = texture->GetTexturePath();
	SceneTextures[path] = texture;
	return texture;
}

shared_ptr<FontMap> Scene::LoadFont(const filesystem::path& path, const int2 mapSize) {
	shared_ptr<FontMap> tmp = make_shared<FontMap>(mapSize, path);
	SceneFonts[path] = tmp;
	return tmp;
}

void Scene::AddMesh(shared_ptr<Mesh> mesh) {
	SceneMeshes.push_back(mesh);
}

FileLoadData Scene::LoadFile(const filesystem::path& path) {
	Assimp::Importer importer;
	
	const aiScene* scene = importer.ReadFile(path.string(), aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_FlipUVs |
		aiProcess_CalcTangentSpace | aiShadingMode_PBR_BRDF | aiProcess_FixInfacingNormals |
		aiProcess_FindInvalidData | aiProcess_LimitBoneWeights | aiProcess_ImproveCacheLocality);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		GlenderLog(LogTypeError, format("Failed to load asset: {}, with error: {}", path.string(), importer.GetErrorString()));
		return {};
	}

	m_lastDir = path;
	m_lastDir.remove_filename();

	FileLoadData data = {};

	for (int i = 0; i < scene->mNumTextures; i++) {
		shared_ptr<Texture2D> tex = make_shared<Texture2D>(int2{ 1, 1 });
		aiTexture* aiTex = scene->mTextures[i];

		int width = (int)aiTex->mWidth, height = (int)aiTex->mHeight;
		int imageWidth, imageHeight, imageChannels;
		int size = height == 0 ? width : width * height;
		unsigned char* data = stbi_load_from_memory((unsigned char*)aiTex->pcData, size, &imageWidth, &imageHeight, &imageChannels, 4);
		
		tex->SetData(data, { imageWidth, imageHeight });
		tex->SetTexturePath(m_lastDir / aiTex->mFilename.C_Str());

		AddTexture(tex);

		stbi_image_free(data);
	}

	for (int i = 0; i < scene->mNumMaterials; i++) {
		shared_ptr<PbrMaterial> mat = MaterialFromAssimp(scene->mMaterials[i], scene, data);
		m_loadedMaterials.push_back(mat);
	}

	ProcessAssimpNode(&SceneRoot.EntityTransform, scene->mRootNode, scene, data);
	m_loadedMaterials.clear();
	importer.FreeScene();
	return data;
}

shared_ptr<Texture> Scene::TextureFromAssimp(aiMaterial* material, FileLoadData& data, aiTextureType texType) {
	aiString str;
	if (material->GetTextureCount(texType) > 0) {
		material->GetTexture(texType, 0, &str);
		shared_ptr<Texture> tex = LoadTexture(m_lastDir / str.C_Str());
		data.Textures.push_back(tex);
		return tex;
	}
	return nullptr;
}

shared_ptr<PbrMaterial> Scene::MaterialFromAssimp(aiMaterial* material, const aiScene* scene, FileLoadData& data) {

	shared_ptr<PbrMaterial> mat = CreateMaterial<PbrMaterial>();
	aiColor4D col;
	if (aiGetMaterialColor(material, AI_MATKEY_COLOR_DIFFUSE, &col)) {
		mat->SetAlbedo({col.r, col.g, col.b, col.a});
	}
	if (aiGetMaterialColor(material, AI_MATKEY_COLOR_SPECULAR, &col)) {
		mat->SetSpecular(col.r);
	}
	float val = 0;
	if (aiGetMaterialFloat(material, AI_MATKEY_ROUGHNESS_FACTOR, &val)) {
		mat->SetRoughness(val);
	}
	if (aiGetMaterialFloat(material, AI_MATKEY_METALLIC_FACTOR, &val)) {
		mat->SetMetalic(val);
	}
	
	mat->SetTexture(TextureAlbedo, TextureFromAssimp(material, data, aiTextureType_DIFFUSE));
	mat->SetTexture(TextureNormal, TextureFromAssimp(material, data, aiTextureType_NORMALS));
	mat->SetTexture(TextureSpecular, TextureFromAssimp(material, data, aiTextureType_SPECULAR));
	mat->SetTexture(TextureRoughness, TextureFromAssimp(material, data, aiTextureType_DIFFUSE_ROUGHNESS));
	mat->SetTexture(TextureMetalic, TextureFromAssimp(material, data, aiTextureType_METALNESS));
	mat->SetTexture(TextureAO, TextureFromAssimp(material, data, aiTextureType_AMBIENT_OCCLUSION));

	data.Materials.push_back(mat);

	return mat;
}

shared_ptr<SolidMesh> Scene::SolidMeshFromAssimp(aiMesh* mesh, const aiScene* scene, FileLoadData& data) {
	shared_ptr<Material> mat = SceneMaterials[0];
	if (mesh->mMaterialIndex >= 0) {
		mat = m_loadedMaterials[mesh->mMaterialIndex];
	}
	
	shared_ptr<SolidMesh> mesh1 = SpawnMesh<SolidMesh>(mat);

	LoadMeshDataFromAssimp(mesh, mesh1);

	data.SolidMeshes.push_back(mesh1);
	return mesh1;
}

shared_ptr<SkinnedMesh> Scene::SkinnedMeshFromAssimp(aiMesh* mesh, const aiScene* scene, FileLoadData& data) {
	shared_ptr<Material> mat = SceneMaterials[0];
	if (mesh->mMaterialIndex >= 0) {
		mat = m_loadedMaterials[mesh->mMaterialIndex];

		mat->GetShader(VertexShader)->UpdateText(filesystem::path("shaders/Skinned.vert"));

		mat->Recompile();
	}
	else {
		shared_ptr<PbrMaterial> mat = CreateMaterial<PbrMaterial>(lm::vec4{}, 0.5f, 0.5f, 0.5f,
			vector<filesystem::path> { "shaders/Skinned.vert", "shaders/Default.frag", "shaders/Default.geom" });
			//vector<filesystem::path> { "shaders/Debug/Skinned.vert", "shaders/Debug/Skinned.frag", "shaders/Debug/Skinned.geom" });
	}

	shared_ptr<SkinnedMesh> mesh1 = SpawnMesh<SkinnedMesh>(mat);

	LoadMeshDataFromAssimp(mesh, mesh1);

	vector<lm::vec4> weights(mesh->mNumVertices, { 0, 0, 0, 0 });
	vector<int4> indices(mesh->mNumVertices, { -1, -1, -1, -1 });

	aiNode* armature = scene->mRootNode;

	for (int i = 0; i < mesh->mNumBones; i++) {
		aiBone* bone = mesh->mBones[i];
		if (!armature) {
			armature = bone->mArmature;
		}
		
		for (int weight = 0; weight < bone->mNumWeights; weight++) {
			int vertIndex = bone->mWeights[weight].mVertexId;
			for (int x = 0; x < 4; x++) {
				if (weights[vertIndex][x] <= 0) {
					weights[vertIndex][x] = bone->mWeights[weight].mWeight;
					(&indices[vertIndex].x)[x] = i;
					break;
				}
			}
		}
		aiMatrix4x4t offset = bone->mOffsetMatrix;
		mesh1->AddBone(bone->mName.C_Str(), { i,
			{
				{offset.a1, offset.b1, offset.c1, offset.d1},
				{offset.a2, offset.b2, offset.c2, offset.d2},
				{offset.a3, offset.b3, offset.c3, offset.d3},
				{offset.a4, offset.b4, offset.c4, offset.d4},
			}
			});
	}

	mesh1->UpdateAttribute("BoneIndices", indices);
	mesh1->UpdateAttribute("BoneWeights", weights);

	mesh1->UpdateBones();
	
	if (armature) {
		for (int i = 0; i < scene->mNumAnimations; i++) {
			mesh1->AddAnimation(scene->mAnimations[i], armature);
		}
	}

	data.SkinnedMeshes.push_back(mesh1);
	return mesh1;
}

shared_ptr<Mesh> Scene::MeshFromAssimp(aiMesh* mesh, const aiScene* scene, FileLoadData& data) {
	if (mesh->HasBones()) {
		return SkinnedMeshFromAssimp(mesh, scene, data);
	}
	return SolidMeshFromAssimp(mesh, scene, data);
}

void Scene::LoadMeshDataFromAssimp(aiMesh* assimpMesh, shared_ptr<Mesh> mesh) {
	vector<Vertex> verticesDyn;
	vector<VertexStatic> verticesSt;
	vector<unsigned int> indices;

	for (unsigned int i = 0; i < assimpMesh->mNumVertices; i++) {
		Vertex vertex;
		VertexStatic vertexSt;
		vertex.pos.x = assimpMesh->mVertices[i].x;
		vertex.pos.y = assimpMesh->mVertices[i].y;
		vertex.pos.z = assimpMesh->mVertices[i].z;

		vertex.normal.x = assimpMesh->mNormals[i].x;
		vertex.normal.y = assimpMesh->mNormals[i].y;
		vertex.normal.z = assimpMesh->mNormals[i].z;

		if (assimpMesh->mTangents) {
			vertex.tangent.x = assimpMesh->mTangents[i].x;
			vertex.tangent.y = assimpMesh->mTangents[i].y;
			vertex.tangent.z = assimpMesh->mTangents[i].z;
		}
		else {
			vertex.tangent = {};
		}

		if (assimpMesh->mTextureCoords[0]) {
			vertexSt.uv.x = assimpMesh->mTextureCoords[0][i].x;
			vertexSt.uv.y = assimpMesh->mTextureCoords[0][i].y;
		}
		else {
			vertexSt.uv = { 0.0f, 0.0f };
		}

		verticesDyn.push_back(vertex);
		verticesSt.push_back(vertexSt);
	}

	for (unsigned int i = 0; i < assimpMesh->mNumFaces; i++) {
		aiFace face = assimpMesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; j++) {
			indices.push_back(face.mIndices[j]);
		}
	}

	mesh->SetGeometry(verticesDyn, verticesSt, indices);

	mesh->EntityTransform.SetName(assimpMesh->mName.C_Str());
}

void Scene::ProcessAssimpNode(Transform* parent, aiNode* node, const aiScene* scene, FileLoadData& data) {
	Transform* _this = parent;
	for (unsigned int i = 0; i < node->mNumMeshes; i++) {
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		
		aiVector3t<float> pos, rot, scale;
		node->mTransformation.Decompose(scale, rot, pos);
		
		shared_ptr<Mesh> m = MeshFromAssimp(mesh, scene, data);

		m->EntityTransform.SetPosition({ pos.x, pos.y, pos.z });
		m->EntityTransform.SetRotation({ rot.x, rot.y, rot.z });
		m->EntityTransform.SetScale({ scale.x, scale.y, scale.z });

		m->EntityTransform.SetParent(parent);

		_this = &m->EntityTransform;
	}
	
	for (unsigned int i = 0; i < node->mNumChildren; i++) {
		ProcessAssimpNode(_this, node->mChildren[i], scene, data);
	}
}
