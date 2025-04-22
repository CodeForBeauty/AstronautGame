#pragma once

#include "Entity.h"
#include "Mesh.h"

#include <filesystem>
#include <string>
#include <vector>
#include <unordered_map>
#include <map>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>


namespace glender {
	struct KeyInfo {
		lm::vec3 Value;
		float Time;
	};
	struct RotKeyInfo {
		Quaternion Value;
		float Time;
	};

	class Bone : public Transform {
	public:
		Bone(const TransformData& initTransform, const int index);

		void AddPosKey(const KeyInfo& info);
		void AddRotKey(const RotKeyInfo& info);
		void AddScaleKey(const KeyInfo& info);

		void UpdateTransform(float time);

		std::string Name;

		int GetIndex() const;

		lm::mat4 BoneMatrix;
		void UpdateMatrix(const lm::mat4& previous);

	protected:
		std::vector<KeyInfo> m_posKeys;
		std::vector<RotKeyInfo> m_rotKeys;
		std::vector<KeyInfo> m_scaleKeys;
		int m_index;

		TransformData InterpolateTransform(const TransformData& a, const TransformData& b, const float t);
		lm::vec3 InterpolateVec3(const lm::vec3& a, const lm::vec3& b, const float t);
	};

	struct BoneData {
		int Index;
		lm::mat4 BoneMatrix;
	};

	class Animation {
	public:
		Animation(const std::filesystem::path& file, std::unordered_map<std::string, BoneData>& bonesMap);
		Animation(aiAnimation* animation, aiNode* skeleton, std::unordered_map<std::string, BoneData>& bonesMap);

		void LoadFromFile(const std::filesystem::path& file, std::unordered_map<std::string, BoneData>& bonesMap);

		void UpdateMatrices(std::shared_ptr<Material> material, const std::string& uniformName, std::unordered_map<std::string, BoneData>& bonesMap);

		void SetTime(const float time);

	protected:
		float m_animTime;
		float m_currentTime;
		float m_animRate;

		std::vector<std::unique_ptr<Bone>> m_bones;
		lm::mat4 m_rootInverseTransform;

		std::string m_rootBone;

		void PrintBones(const int index, const std::string& tabs);

	private:

		void ProcessAssimpNode(Transform* parent, aiNode* node, std::unordered_map<std::string, BoneData>& bonesMap);
		void ProcessAnimation(aiAnimation* animation, std::unordered_map<std::string, BoneData>& bonesMap);

		void SetRootInverseRootTransform(aiNode* node);
	};

	class SkinnedMesh : public Mesh {
	public:
		SkinnedMesh(std::shared_ptr<Material> material);

		int AddAnimation(std::shared_ptr<Animation> animation);
		int AddAnimation(aiAnimation* anim, aiNode* skeleton);
		int AddAnimation(const std::filesystem::path& file);

		void PlayAnimation(std::shared_ptr<Animation> animation);
		void PlayAnimation(const int index);

		void AddBone(const std::string& name, const BoneData& bone);

		void SetTime(const float time);

		void UpdateBones();

		std::vector<std::shared_ptr<Animation>> Animations;

		std::string SetDebugBone(const int index);

	protected:
		std::unordered_map<std::string, BoneData> m_bonesData;

		std::shared_ptr<Animation> m_currentAnimation;

	private:

	};
}