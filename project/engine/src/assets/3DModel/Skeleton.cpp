#include "engine/include/assets/3DModel/Skeleton.h"
#include <assimp/scene.h>

#include "engine/include/assets/3DModel/Skeleton.h"
#include <assimp/scene.h>

using namespace QFE::SKELETON;

namespace {

	QFE::Matrix4x4 ConvertAssimpMatrix(const aiMatrix4x4& matrix) {
		QFE::Matrix4x4 result{};
		result.m[0][0] = matrix.a1; result.m[0][1] = matrix.a2; result.m[0][2] = matrix.a3; result.m[0][3] = matrix.a4;
		result.m[1][0] = matrix.b1; result.m[1][1] = matrix.b2; result.m[1][2] = matrix.b3; result.m[1][3] = matrix.b4;
		result.m[2][0] = matrix.c1; result.m[2][1] = matrix.c2; result.m[2][2] = matrix.c3; result.m[2][3] = matrix.c4;
		result.m[3][0] = matrix.d1; result.m[3][1] = matrix.d2; result.m[3][2] = matrix.d3; result.m[3][3] = matrix.d4;
		return result;
	}

	void UpdateJointRecursive(Skeleton& skeleton, int32_t jointIndex) {
		Joint& joint = skeleton.joints.at(static_cast<size_t>(jointIndex));
		if (joint.parentIndex >= 0) {
			const Joint& parent = skeleton.joints.at(static_cast<size_t>(joint.parentIndex));
			joint.globalMatrix = QFE::Matrix4x4::Multiply(parent.globalMatrix, joint.localMatrix);
		} else {
			joint.globalMatrix = joint.localMatrix;
		}

		for (int32_t childIndex : joint.children) {
			UpdateJointRecursive(skeleton, childIndex);
		}
	}

}

Skeleton QFE::SKELETON::CreateSkeleton(const aiNode* rootNode) {
	Skeleton skeleton;
	if (!rootNode) {
		return skeleton;
	}

	skeleton.rootIndex = CreateJoint(rootNode, -1, skeleton);
	Update(skeleton);
	return skeleton;
}

int32_t QFE::SKELETON::CreateJoint(const aiNode* node, int32_t parentIndex, Skeleton& skeleton) {
	if (!node) {
		return -1;
	}

	Joint joint;
	joint.name = node->mName.C_Str();
	joint.parentIndex = parentIndex;
	joint.localMatrix = ConvertAssimpMatrix(node->mTransformation);

	const int32_t jointIndex = static_cast<int32_t>(skeleton.joints.size());
	skeleton.joints.push_back(joint);
	skeleton.jointIndexMap[joint.name] = jointIndex;

	for (uint32_t i = 0; i < node->mNumChildren; ++i) {
		int32_t childIndex = CreateJoint(node->mChildren[i], jointIndex, skeleton);
		if (childIndex >= 0) {
			skeleton.joints[static_cast<size_t>(jointIndex)].children.push_back(childIndex);
		}
	}

	return jointIndex;
}

void QFE::SKELETON::Update(Skeleton& skeleton) {
	if (skeleton.rootIndex < 0 || skeleton.joints.empty()) {
		return;
	}

	UpdateJointRecursive(skeleton, skeleton.rootIndex);
}

namespace {

	QFE::Matrix4x4 ConvertAssimpMatrix(const aiMatrix4x4& matrix) {
		QFE::Matrix4x4 result{};
		result.m[0][0] = matrix.a1; result.m[0][1] = matrix.a2; result.m[0][2] = matrix.a3; result.m[0][3] = matrix.a4;
		result.m[1][0] = matrix.b1; result.m[1][1] = matrix.b2; result.m[1][2] = matrix.b3; result.m[1][3] = matrix.b4;
		result.m[2][0] = matrix.c1; result.m[2][1] = matrix.c2; result.m[2][2] = matrix.c3; result.m[2][3] = matrix.c4;
		result.m[3][0] = matrix.d1; result.m[3][1] = matrix.d2; result.m[3][2] = matrix.d3; result.m[3][3] = matrix.d4;
		return result;
	}

	void UpdateJointRecursive(Skeleton& skeleton, int32_t jointIndex) {
		Joint& joint = skeleton.joints.at(static_cast<size_t>(jointIndex));
		if (joint.parentIndex >= 0) {
			const Joint& parent = skeleton.joints.at(static_cast<size_t>(joint.parentIndex));
			joint.globalMatrix = QFE::Matrix4x4::Multiply(parent.globalMatrix, joint.localMatrix);
		} else {
			joint.globalMatrix = joint.localMatrix;
		}

		for (int32_t childIndex : joint.children) {
			UpdateJointRecursive(skeleton, childIndex);
		}
	}

}

Skeleton QFE::SKELETON::CreateSkeleton(const aiNode* rootNode) {
	Skeleton skeleton;
	if (!rootNode) {
		return skeleton;
	}

	skeleton.rootIndex = CreateJoint(rootNode, -1, skeleton);
	Update(skeleton);
	return skeleton;
}

int32_t QFE::SKELETON::CreateJoint(const aiNode* node, int32_t parentIndex, Skeleton& skeleton) {
	if (!node) {
		return -1;
	}

	Joint joint;
	joint.name = node->mName.C_Str();
	joint.parentIndex = parentIndex;
	joint.localMatrix = ConvertAssimpMatrix(node->mTransformation);

	const int32_t jointIndex = static_cast<int32_t>(skeleton.joints.size());
	skeleton.joints.push_back(joint);
	skeleton.jointIndexMap[joint.name] = jointIndex;

	for (uint32_t i = 0; i < node->mNumChildren; ++i) {
		int32_t childIndex = CreateJoint(node->mChildren[i], jointIndex, skeleton);
		if (childIndex >= 0) {
			skeleton.joints[static_cast<size_t>(jointIndex)].children.push_back(childIndex);
		}
	}

	return jointIndex;
}

void QFE::SKELETON::Update(Skeleton& skeleton) {
	if (skeleton.rootIndex < 0 || skeleton.joints.empty()) {
		return;
	}

	UpdateJointRecursive(skeleton, skeleton.rootIndex);
}