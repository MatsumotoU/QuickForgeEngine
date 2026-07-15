#include "engine/include/assets/Animator/SkeletonAnimation.h"
#include "engine/include/core/Math/Matrix/Matrix4x4.h"

void QFE::ANIMATION::ApplyAnimation(QFE::SKELETON::Skeleton& skeleton, const AnimClip& animClip, float time) {
	if (skeleton.rootIndex < 0 || skeleton.joints.empty()) {
		return;
	}

	if (animClip.GetKeyFrameCount() == 0) {
		return;
	}

	const EulerTransform transform = animClip.GetTransformAtTime(time);
	QFE::SKELETON::Joint& rootJoint = skeleton.joints.at(static_cast<size_t>(skeleton.rootIndex));
	rootJoint.localMatrix = QFE::Matrix4x4::MakeAffineMatrix(transform);

	QFE::SKELETON::Update(skeleton);
}