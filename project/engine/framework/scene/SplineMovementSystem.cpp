#include "SplineMovementSystem.h"

#include "design-patterns/EntityManager.h"
#include "components/AllComponent.h"
#include "components/TransformHierarchy.h"

#include <algorithm>
#include <vector>

namespace {
	constexpr uint32_t kSamplesPerSegment = 16;

	struct SplineSample {
		QFE::MATH::EulerTransform transform;
		QFE::MATH::Vector3 worldPosition;
		float distance = 0.0f;
	};

	QFE::MATH::Vector3 CatmullRom(
		const QFE::MATH::Vector3& p0,
		const QFE::MATH::Vector3& p1,
		const QFE::MATH::Vector3& p2,
		const QFE::MATH::Vector3& p3,
		float t) {
		const float t2 = t * t;
		const float t3 = t2 * t;
		return (p1 * 2.0f + (p2 - p0) * t +
			(p0 * 2.0f - p1 * 5.0f + p2 * 4.0f - p3) * t2 +
			(-p0 + p1 * 3.0f - p2 * 3.0f + p3) * t3) * 0.5f;
	}

	QFE::MATH::EulerTransform EvaluateSegment(
		const std::vector<QFE::MATH::EulerTransform>& points,
		size_t segmentIndex,
		float t) {
		const size_t startIndex = segmentIndex;
		const size_t endIndex = segmentIndex + 1;
		const auto& p1 = points[startIndex];
		const auto& p2 = points[endIndex];
		const auto& p0 = points[startIndex == 0 ? startIndex : startIndex - 1];
		const auto& p3 = points[endIndex + 1 < points.size() ? endIndex + 1 : endIndex];

		QFE::MATH::EulerTransform result;
		result.translate = CatmullRom(p0.translate, p1.translate, p2.translate, p3.translate, t);
		result.rotate = QFE::MATH::Vector3::Lerp(p1.rotate, p2.rotate, t);
		result.scale = QFE::MATH::Vector3::Lerp(p1.scale, p2.scale, t);
		return result;
	}

	QFE::MATH::Vector3 ToWorldPosition(
		const QFE::MATH::Vector3& localPosition,
		const QFE::MATH::Matrix4x4& parentWorldMatrix) {
		return QFE::MATH::Vector4::EulerTransform(
			{ localPosition.x, localPosition.y, localPosition.z, 1.0f },
			parentWorldMatrix).xyz();
	}

	std::vector<SplineSample> BuildSamples(
		const std::vector<QFE::MATH::EulerTransform>& points,
		const QFE::MATH::Matrix4x4& parentWorldMatrix) {
		std::vector<SplineSample> samples;
		if (points.size() < 2) {
			return samples;
		}

		const size_t segmentCount = points.size() - 1;
		samples.reserve(segmentCount * kSamplesPerSegment + 1);
		SplineSample first;
		first.transform = EvaluateSegment(points, 0, 0.0f);
		first.worldPosition = ToWorldPosition(first.transform.translate, parentWorldMatrix);
		samples.push_back(first);

		for (size_t segment = 0; segment < segmentCount; ++segment) {
			for (uint32_t sampleIndex = 1; sampleIndex <= kSamplesPerSegment; ++sampleIndex) {
				const float t = static_cast<float>(sampleIndex) / static_cast<float>(kSamplesPerSegment);
				SplineSample sample;
				sample.transform = EvaluateSegment(points, segment, t);
				sample.worldPosition = ToWorldPosition(sample.transform.translate, parentWorldMatrix);
				sample.distance = samples.back().distance +
					(sample.worldPosition - samples.back().worldPosition).Length();
				samples.push_back(sample);
			}
		}
		return samples;
	}

	QFE::MATH::EulerTransform FindTransformAtDistance(
		const std::vector<SplineSample>& samples,
		float distance) {
		if (distance <= 0.0f || samples.size() < 2) {
			return samples.front().transform;
		}
		if (distance >= samples.back().distance) {
			return samples.back().transform;
		}

		const auto end = std::lower_bound(
			samples.begin() + 1, samples.end(), distance,
			[](const SplineSample& sample, float targetDistance) {
				return sample.distance < targetDistance;
			});
		const SplineSample& next = *end;
		const SplineSample& previous = *(end - 1);
		const float sectionLength = next.distance - previous.distance;
		const float t = sectionLength > 0.0f
			? (distance - previous.distance) / sectionLength
			: 0.0f;

		QFE::MATH::EulerTransform result;
		result.translate = QFE::MATH::Vector3::Lerp(previous.transform.translate, next.transform.translate, t);
		result.rotate = QFE::MATH::Vector3::Lerp(previous.transform.rotate, next.transform.rotate, t);
		result.scale = QFE::MATH::Vector3::Lerp(previous.transform.scale, next.transform.scale, t);
		return result;
	}
}

void QFE::FRAMEWORK::UpdateSplineMovement(
	QFE::EntityManager& entityManager,
	float deltaTime) {
	entityManager.Each<QFE::SCENE::SplineMovementComponent>(
		[&](uint32_t entityId, QFE::SCENE::SplineMovementComponent& spline) {
			if (spline.controlPoints.size() < 2) {
				return;
			}

			const QFE::MATH::Matrix4x4 parentWorldMatrix =
				QFE::SCENE::GetParentWorldMatrix(entityManager, entityId);
			const std::vector<SplineSample> samples = BuildSamples(spline.controlPoints, parentWorldMatrix);
			if (samples.size() < 2) {
				return;
			}

			if (!spline.enabled || !entityManager.HasComponent<QFE::SCENE::TransformComponent>(entityId)) {
				return;
			}

			QFE::SCENE::TransformComponent& transform =
				entityManager.GetComponent<QFE::SCENE::TransformComponent>(entityId);
			if (!spline.movementStarted) {
				spline.distanceAlongPath = 0.0f;
				spline.movementStarted = true;
				transform.transform = samples.front().transform;
			}

			if (!spline.finished && spline.speed > 0.0f && deltaTime > 0.0f) {
				spline.distanceAlongPath += spline.speed * deltaTime;
				if (spline.distanceAlongPath >= samples.back().distance) {
					spline.distanceAlongPath = samples.back().distance;
					spline.finished = true;
				}
			}
			transform.transform = FindTransformAtDistance(samples, spline.distanceAlongPath);
		});
}

void QFE::FRAMEWORK::DrawSplineMovementPaths(
	const QFE::EntityManager& entityManager,
	const QFE::FRAMEWORK::SplineLineDrawer& drawLine) {
	if (!drawLine) {
		return;
	}
	entityManager.Each<QFE::SCENE::SplineMovementComponent>(
		[&](uint32_t entityId, const QFE::SCENE::SplineMovementComponent& spline) {
			if (!spline.drawPath || spline.controlPoints.size() < 2) {
				return;
			}
			const QFE::MATH::Matrix4x4 parentWorldMatrix =
				QFE::SCENE::GetParentWorldMatrix(entityManager, entityId);
			const std::vector<SplineSample> samples = BuildSamples(spline.controlPoints, parentWorldMatrix);
			for (size_t index = 1; index < samples.size(); ++index) {
				drawLine(samples[index - 1].worldPosition, samples[index].worldPosition, spline.pathColor);
			}
		});
}
