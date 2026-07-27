#include "SkinningUtility.h"
using namespace QFE::GRAPHIC;

int32_t QFE::GRAPHIC::CreateJoint(const Node& node, const std::optional<int32_t>& parentIndex, std::vector<Joint>& joints) {
	Joint joint;
	joint.name = node.name;
	joint.localMatrix = node.localMatrix;
	joint.skeletonSpaceMatrix = QFE::MATH::Matrix4x4::MakeIdentity4x4(); // 初期化
	joint.transform = node.transform;
	joint.index = static_cast<int32_t>(joints.size());
	joint.parentIndex = parentIndex;
	joints.push_back(joint);
	for(const Node& childNode : node.children) {
		int32_t childIndex = CreateJoint(childNode, joint.index, joints);
		joints[joint.index].children.push_back(childIndex);
	}
	return joint.index;
}

Skeleton QFE::GRAPHIC::CreateSkeleton(const Node& rootNode) {
	Skeleton skeleton;
	skeleton.rootJointIndex = CreateJoint(rootNode, std::nullopt, skeleton.joints);
	for (const Joint& joint : skeleton.joints) {
		skeleton.jointNameToIndexMap.emplace(joint.name, joint.index);
	}
	return skeleton;
}

void QFE::GRAPHIC::UpdateSkeletonSpaceMatrices(Skeleton& skeleton) {
	for(Joint& joint : skeleton.joints) {
		if(joint.parentIndex.has_value()) {
			const Joint& parentJoint = skeleton.joints[joint.parentIndex.value()];
			joint.skeletonSpaceMatrix = parentJoint.skeletonSpaceMatrix * joint.localMatrix;
		} else {
			joint.skeletonSpaceMatrix = joint.localMatrix;
		}
	}
}
