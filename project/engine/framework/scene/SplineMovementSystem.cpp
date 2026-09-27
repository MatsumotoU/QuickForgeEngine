#include "SplineMovementSystem.h"

#include "design-patterns/EntityManager.h"
#include "components/SplineMovementComponent.h"
#include "components/TransformHierarchy.h"

#include <algorithm>
#include <vector>

namespace {
	constexpr uint32_t kSamplesPerSegment = 16;

	struct SplineSample {
		QFE::MATH::EulerTransform transform;
		QFE::MATH::Vector3 worldPosition;
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
		const std::vector<QFE::SCENE::SplineControlPoint>& points,
		size_t segmentIndex,
		float t) {
		const size_t startIndex = segmentIndex;
		const size_t endIndex = segmentIndex + 1;
		const auto& p1 = points[startIndex].transform;
		const auto& p2 = points[endIndex].transform;
		const auto& p0 = points[startIndex == 0 ? startIndex : startIndex - 1].transform;
		const auto& p3 = points[endIndex + 1 < points.size() ? endIndex + 1 : endIndex].transform;

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
		const std::vector<QFE::SCENE::SplineControlPoint>& points,
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
				samples.push_back(sample);
			}
		}
		return samples;
	}

	float GetSegmentDurationSeconds(const QFE::SCENE::SplineControlPoint& point) {
		constexpr float kMinimumSegmentDurationSeconds = 0.01f;
		return point.secondsToNextPoint < kMinimumSegmentDurationSeconds
			? kMinimumSegmentDurationSeconds
			: point.secondsToNextPoint;
	}

	float GetTotalDurationSeconds(const std::vector<QFE::SCENE::SplineControlPoint>& points) {
		float totalDuration = 0.0f;
		for (size_t segment = 0; segment + 1 < points.size(); ++segment) {
			totalDuration += GetSegmentDurationSeconds(points[segment]);
		}
		return totalDuration;
	}

	QFE::MATH::EulerTransform FindTransformAtTime(
		const std::vector<QFE::SCENE::SplineControlPoint>& points,
		float elapsedTimeSeconds) {
		if (points.size() < 2) {
			return points.front().transform;
		}

		float remainingTime = elapsedTimeSeconds < 0.0f ? 0.0f : elapsedTimeSeconds;
		const size_t segmentCount = points.size() - 1;
		for (size_t segment = 0; segment < segmentCount; ++segment) {
			const float segmentDuration = GetSegmentDurationSeconds(points[segment]);
			if (remainingTime < segmentDuration || segment + 1 == segmentCount) {
				float t = remainingTime / segmentDuration;
				if (t < 0.0f) {
					t = 0.0f;
				} else if (t > 1.0f) {
					t = 1.0f;
				}
				return EvaluateSegment(points, segment, t);
			}
			remainingTime -= segmentDuration;
		}
		return EvaluateSegment(points, segmentCount - 1, 1.0f);
	}
}

void QFE::FRAMEWORK::UpdateSplineMovement(
	QFE::EntityManager& entityManager,
	float deltaTime) {
	entityManager.Each<QFE::SCENE::SplineMovementComponent>(
		[&](uint32_t entityId, QFE::SCENE::SplineMovementComponent& spline) {
			if (!spline.enabled || spline.finished || spline.controlPoints.size() < 2 ||
				!entityManager.HasComponent<QFE::SCENE::TransformComponent>(entityId)) {
				return;
			}

			QFE::SCENE::TransformComponent& transform =
				entityManager.GetComponent<QFE::SCENE::TransformComponent>(entityId);
			if (!spline.movementStarted) {
				spline.elapsedTimeSeconds = 0.0f;
				spline.movementStarted = true;
				transform.transform = EvaluateSegment(spline.controlPoints, 0, 0.0f);
			}

			const float totalDuration = GetTotalDurationSeconds(spline.controlPoints);
			if (!spline.finished && deltaTime > 0.0f) {
				spline.elapsedTimeSeconds += deltaTime;
				if (spline.elapsedTimeSeconds >= totalDuration) {
					spline.elapsedTimeSeconds = totalDuration;
					spline.finished = true;
				}
			}
			transform.transform = FindTransformAtTime(spline.controlPoints, spline.elapsedTimeSeconds);
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
