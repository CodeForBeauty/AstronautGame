#include "Skeleton.h"

#include "Logging.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

using namespace glender;
using namespace std;


Bone::Bone(const TransformData& initTransform, const int index) : m_index(index), m_posKeys() {
	SetTransform(initTransform);
}

void Bone::AddPosKey(const KeyInfo& info) {
	m_posKeys.push_back(info);
}

void Bone::AddRotKey(const RotKeyInfo& info) {
	m_rotKeys.push_back(info);
}

void Bone::AddScaleKey(const KeyInfo& info) {
	m_scaleKeys.push_back(info);
}

void Bone::UpdateTransform(float time) {
	if (time < 0) {
		time = 0;
	}

	lm::vec3 pos, scale;
	Quaternion rot;
	if (m_posKeys.size() == 0) {
		pos = m_position;
	}
	else if (m_posKeys.size() == 1) {
		pos = m_posKeys[0].Value;
	}
	else {
		int second = 1;
		for (int i = 1; i < m_posKeys.size(); i++) {
			if (m_posKeys[i].Time > time) {
				second = i;
				break;
			}
		}

		float t = (time - m_posKeys[second - 1].Time) / (m_posKeys[second].Time - m_posKeys[second - 1].Time);
		pos = InterpolateVec3(m_posKeys[second - 1].Value, m_posKeys[second].Value, t);
	}

	if (m_rotKeys.size() == 0) {
		rot = m_rotation;
	}
	else if (m_rotKeys.size() == 1) {
		rot = m_rotKeys[0].Value;
	}
	else {
		int second = 1;
		for (int i = 1; i < m_rotKeys.size(); i++) {
			second = i;
			if (m_rotKeys[i].Time > time) {
				break;
			}
		}

		float t = (time - m_rotKeys[second - 1].Time) / (m_rotKeys[second].Time - m_rotKeys[second - 1].Time);
		rot = slerp(m_rotKeys[second - 1].Value, m_rotKeys[second].Value, t);
	}

	if (m_scaleKeys.size() == 0) {
		scale = m_scale;
	}
	else if (m_scaleKeys.size() == 1) {
		scale = m_scaleKeys[0].Value;
	}
	else {
		int second = 1;
		for (int i = 1; i < m_scaleKeys.size(); i++) {
			if (m_scaleKeys[i].Time > time) {
				second = i;
				break;
			}
		}

		float t = (time - m_scaleKeys[second - 1].Time) / (m_scaleKeys[second].Time - m_scaleKeys[second - 1].Time);
		scale = InterpolateVec3(m_scaleKeys[second - 1].Value, m_scaleKeys[second].Value, t);
	}

	SetTransform({ pos, rot, scale });
}

int Bone::GetIndex() const {
	return m_index;
}

void Bone::UpdateMatrix(const lm::mat4& previous) {
	lm::mat4 pos = lm::position3d(m_position);
	lm::mat4 rot = (-m_rotation).GetMatrix();
	rot.w.w = 1.0f;
	lm::mat4 scale = { {m_scale.x, 0.0f, 0.0f, 0.0f},
						{0.0f, m_scale.y, 0.0f, 0.0f},
						{0.0f, 0.0f, m_scale.z, 0.0f},
						{0.0f, 0.0f, 0.0f, 1.0f} };
	BoneMatrix = scale * rot * pos;
	BoneMatrix = BoneMatrix * previous;

	for (Transform* child : Children) {
		Bone* boneCh = static_cast<Bone*>(child);
		boneCh->UpdateMatrix(BoneMatrix);
	}
}

TransformData Bone::InterpolateTransform(const TransformData& a, const TransformData& b, const float t) {
	return { a.Position * (1 - t) + b.Position * t, 
		slerp(a.Rotation, b.Rotation, t),
		a.Scale * (1 - t) + b.Scale * t };
}

lm::vec3 Bone::InterpolateVec3(const lm::vec3& a, const lm::vec3& b, const float t) {
	return a * (1 - t) + b * t;
}

Animation::Animation(const filesystem::path& file, unordered_map<string, BoneData>& bonesMap) : m_currentTime(0), m_bones() {
	LoadFromFile(file, bonesMap);
}

Animation::Animation(aiAnimation* animation, aiNode* skeleton, unordered_map<string, BoneData>& bonesMap) 
		: m_currentTime(0), m_bones() {
	m_bones.resize(bonesMap.size());

	m_rootBone = skeleton->mName.C_Str();

	SetRootInverseRootTransform(skeleton);

	ProcessAssimpNode(nullptr, skeleton, bonesMap);

	ProcessAnimation(animation, bonesMap);
}

void Animation::LoadFromFile(const filesystem::path& file, unordered_map<string, BoneData>& bonesMap) {
	m_bones.clear();

	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(file.string(), aiProcess_PopulateArmatureData);

	if (!scene || !scene->mRootNode) {
		GlenderLog(LogTypeError, format("Failed to load animation: {}, with error: {}", file.string(), importer.GetErrorString()));
	}
	
	aiNode* skeleton = scene->mRootNode;
	m_bones.resize(bonesMap.size());
	m_rootBone = skeleton->mName.C_Str();

	SetRootInverseRootTransform(skeleton);

	ProcessAssimpNode(nullptr, skeleton, bonesMap);

	if (!scene->HasAnimations()) {
		importer.FreeScene();
		return;
	}
	
	ProcessAnimation(scene->mAnimations[0], bonesMap);
	
	importer.FreeScene();
}

void Animation::UpdateMatrices(shared_ptr<Material> material, const string& uniformName, unordered_map<string, BoneData>& bonesMap) {

	for (int i = 0; i < m_bones.size(); i++) {
		if (!m_bones[i]) {
			continue;
		}
		
		material->SetUniform((uniformName + "[" + to_string(i) + "]").c_str(),
			bonesMap[m_bones[i]->Name].BoneMatrix * m_bones[i]->WorldMatrix * m_rootInverseTransform);
	}
}

void Animation::SetTime(float time) {
	time *= m_animRate;
	time = fmod(time, m_animTime);
	m_currentTime = time;

	for (int i = 0; i < m_bones.size(); i++) {
		if (!m_bones[i]) {
			continue;
		}
		m_bones[i]->UpdateTransform(time);
	}
}

void Animation::PrintBones(const int index, const string& tabs) {
	cout << tabs << m_bones[index]->Name << " pos: " << m_bones[index]->GetPosition() << 
		" rot: " << m_bones[index]->GetRotation() << " scale: " << m_bones[index]->GetScale() << endl;

	for (Transform* child : m_bones[index]->Children) {
		Bone* boneCh = static_cast<Bone*>(child);

		PrintBones(boneCh->GetIndex(), tabs + "|  ");
	}
}

void Animation::ProcessAssimpNode(Transform* parent, aiNode* node, unordered_map<string, BoneData>& bonesMap) {
	aiVector3D pos, scale;
	aiQuaternion rot;
	node->mTransformation.Decompose(scale, rot, pos);
	
	TransformData initTransform = {
		{ pos.x, pos.y, pos.z },
		{ -rot.x, -rot.y, -rot.z, rot.w },
		{ scale.x, scale.y, scale.z }
	};
	
	Transform* current = parent;
	int index = 0;
	if (bonesMap.find(node->mName.C_Str()) != bonesMap.end()) {
		index = bonesMap[node->mName.C_Str()].Index;

		while (index >= m_bones.size()) {
			m_bones.push_back(make_unique<Bone>(TransformData{}, m_bones.size()));
		}
		
		m_bones[index] = make_unique<Bone>(initTransform, index);
	}
	else {
		index = m_bones.size();

		m_bones.push_back(make_unique<Bone>(initTransform, m_bones.size()));

		bonesMap[node->mName.C_Str()] = { index };
	}

	m_bones[index]->SetParent(parent);

	m_bones[index]->Name = node->mName.C_Str();

	current = m_bones[index].get();
	
	for (int i = 0; i < node->mNumChildren; i++) {
		ProcessAssimpNode(current, node->mChildren[i], bonesMap);
	}
}

void Animation::ProcessAnimation(aiAnimation* animation, unordered_map<string, BoneData>& bonesMap) {
	m_animTime = (float)animation->mDuration;
	m_animRate = (float)animation->mTicksPerSecond;

	for (int i = 0; i < animation->mNumChannels; i++) {
		aiNodeAnim* anim = animation->mChannels[i];

		int index = m_bones.size();
		if (bonesMap.find(anim->mNodeName.C_Str()) != bonesMap.end()) {
			index = bonesMap[anim->mNodeName.C_Str()].Index;
		}

		if (index >= m_bones.size()) {
			continue;
		}

		KeyInfo info = {};

		for (int i = 0; i < anim->mNumPositionKeys; i++) {
			aiVector3D pos = anim->mPositionKeys[i].mValue;
			m_bones[index]->AddPosKey({ { pos.x, pos.y, pos.z }, (float)anim->mPositionKeys[i].mTime });
		}

		for (int i = 0; i < anim->mNumRotationKeys; i++) {
			aiQuaternion rot = anim->mRotationKeys[i].mValue;

			m_bones[index]->AddRotKey({ { -rot.x, -rot.y, -rot.z, rot.w }, (float)anim->mRotationKeys[i].mTime });
		}

		for (unsigned int i = 0; i < anim->mNumScalingKeys; i++) {
			aiVector3D scale = anim->mScalingKeys[i].mValue;
			m_bones[index]->AddScaleKey({ { scale.x, scale.y, scale.z }, (float)anim->mScalingKeys[i].mTime });
		}
	}
}

void Animation::SetRootInverseRootTransform(aiNode* node) {
	aiMatrix4x4t inverse = node->mTransformation.Inverse();
	
	m_rootInverseTransform = {
		{inverse.a1, inverse.b1, inverse.c1, inverse.d1},
		{inverse.a2, inverse.b2, inverse.c2, inverse.d2},
		{inverse.a3, inverse.b3, inverse.c3, inverse.d3},
		{inverse.a4, inverse.b4, inverse.c4, inverse.d4},
	};
}

SkinnedMesh::SkinnedMesh(shared_ptr<Material> material) : Mesh(material) {
	vector<int4> indices(5);
	AddAttribute<int4>("BoneIndices", indices, 4, AttribInt);
	vector<lm::vec4> weights(5);
	AddAttribute<lm::vec4>("BoneWeights", weights, 4, AttribFloat);

	for (int i = 0; i < 150; i++) {
		material->SetUniform(format("u_Bones[{}]", i).c_str(), {
			{1, 0, 0, 0},
			{0, 1, 0, 0},
			{0, 0, 1, 0},
			{0, 0, 0, 1},
			});
	}
}

int SkinnedMesh::AddAnimation(shared_ptr<Animation> animation) {
	Animations.push_back(animation);

	return Animations.size() - 1;
}

int SkinnedMesh::AddAnimation(aiAnimation* anim, aiNode* skeleton) {
	shared_ptr<Animation> animation = make_shared<Animation>(anim, skeleton, m_bonesData);

	Animations.push_back(animation);

	return Animations.size() - 1;
}

int SkinnedMesh::AddAnimation(const filesystem::path& file) {
	shared_ptr<Animation> animation = make_shared<Animation>(file, m_bonesData);

	Animations.push_back(animation);

	return Animations.size() - 1;
}

void SkinnedMesh::PlayAnimation(shared_ptr<Animation> animation) {
	AddAnimation(animation);
	m_currentAnimation = animation;

	UpdateBones();
}

void SkinnedMesh::PlayAnimation(int index) {
	if (index >= Animations.size()) {
		GlenderLog(LogTypeWarning, "Trying to play animation outside of Animations list.");
		return;
	}
	m_currentAnimation = *next(Animations.begin(), index);

	UpdateBones();
}

void SkinnedMesh::AddBone(const string& name, const BoneData& bone) {
	m_bonesData[name] = bone;
}

void SkinnedMesh::SetTime(const float time) {
	if (!m_currentAnimation) {
		return;;
	}
	m_currentAnimation->SetTime(time);

	UpdateBones();
}

void SkinnedMesh::UpdateBones() {
	if (m_currentAnimation) {
		m_currentAnimation->UpdateMatrices(m_material, "u_Bones", m_bonesData);
	}
}

string SkinnedMesh::SetDebugBone(const int index) {
	m_material->SetUniform("u_DebugBone", index);

	for (auto it = m_bonesData.begin(); it != m_bonesData.end(); ++it) {
		if (it->second.Index == index) {
			return it->first;
		}
	}
	return "";
}
