#include "SplineMovementSystem.h"

#include "design-patterns/EntityManager.h"
#include "components/SplineMovementComponent.h"
#include "components/TransformHierarchy.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace {
	constexpr uint32_t kSamplesPerSegment = 16;

	struct SplineSample {
		QFE::MATH::EulerTransform transform;
		QFE::MATH::Vector3 worldPosition;
	};

	struct SplinePreviewState {
		float elapsedTimeSeconds = 0.0f;
		std::chrono::steady_clock::time_point lastUpdated;
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

	QFE::MATH::Vector3 CatmullRomTangent(
		const QFE::MATH::Vector3& p0,
		const QFE::MATH::Vector3& p1,
		const QFE::MATH::Vector3& p2,
		const QFE::MATH::Vector3& p3,
		float t) {
		const float t2 = t * t;
		return ((p2 - p0) +
			(p0 * 2.0f - p1 * 5.0f + p2 * 4.0f - p3) * (2.0f * t) +
			(-p0 + p1 * 3.0f - p2 * 3.0f + p3) * (3.0f * t2)) * 0.5f;
	}

	QFE::MATH::EulerTransform EvaluateSegment(
		const std::vector<QFE::SCENE::SplineControlPoint>& points,
		size_t segmentIndex,
		float t,
		bool loop) {
		const size_t pointCount = points.size();
		const size_t startIndex = loop ? segmentIndex % pointCount : segmentIndex;
		const size_t endIndex = loop ? (startIndex + 1) % pointCount : startIndex + 1;
		const size_t previousIndex = loop
			? (startIndex + pointCount - 1) % pointCount
			: (startIndex == 0 ? startIndex : startIndex - 1);
		const size_t nextIndex = loop
			? (endIndex + 1) % pointCount
			: (endIndex + 1 < pointCount ? endIndex + 1 : endIndex);
		const auto& p0 = points[previousIndex].transform;
		const auto& p1 = points[startIndex].transform;
		const auto& p2 = points[endIndex].transform;
		const auto& p3 = points[nextIndex].transform;

		QFE::MATH::EulerTransform result;
		result.translate = CatmullRom(p0.translate, p1.translate, p2.translate, p3.translate, t);
		QFE::MATH::Vector3 tangent = CatmullRomTangent(
			p0.translate, p1.translate, p2.translate, p3.translate, t);
		if (tangent.LengthSq() <= 1.0e-6f) {
			tangent = t >= 1.0f
				? p3.translate - p2.translate
				: p2.translate - p1.translate;
		}
		if (tangent.LengthSq() <= 1.0e-6f) {
			tangent = p3.translate - p1.translate;
		}
		if (tangent.LengthSq() <= 1.0e-6f) {
			tangent = { 0.0f, 0.0f, 1.0f };
		}
		const QFE::MATH::Vector3 facingRotation = QFE::MATH::Vector3::LookAt(
			QFE::MATH::Vector3::Zero(), tangent);
		const QFE::MATH::Vector3 rotationOffset = QFE::MATH::Vector3::Lerp(
			p1.rotate, p2.rotate, t);
		result.rotate = facingRotation + rotationOffset;
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
		const QFE::MATH::Matrix4x4& parentWorldMatrix,
		bool loop) {
		std::vector<SplineSample> samples;
		if (points.size() < 2) {
			return samples;
		}

		const size_t segmentCount = loop ? points.size() : points.size() - 1;
		samples.reserve(segmentCount * kSamplesPerSegment + 1);
		SplineSample first;
		first.transform = EvaluateSegment(points, 0, 0.0f, loop);
		first.worldPosition = ToWorldPosition(first.transform.translate, parentWorldMatrix);
		samples.push_back(first);

		for (size_t segment = 0; segment < segmentCount; ++segment) {
			for (uint32_t sampleIndex = 1; sampleIndex <= kSamplesPerSegment; ++sampleIndex) {
				const float t = static_cast<float>(sampleIndex) / static_cast<float>(kSamplesPerSegment);
				SplineSample sample;
				sample.transform = EvaluateSegment(points, segment, t, loop);
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

	float GetTotalDurationSeconds(
		const std::vector<QFE::SCENE::SplineControlPoint>& points,
		bool loop) {
		float totalDuration = 0.0f;
		const size_t segmentCount = loop ? points.size() : points.size() - 1;
		for (size_t segment = 0; segment < segmentCount; ++segment) {
			totalDuration += GetSegmentDurationSeconds(points[segment]);
		}
		return totalDuration;
	}

	QFE::MATH::EulerTransform FindTransformAtTime(
		const std::vector<QFE::SCENE::SplineControlPoint>& points,
		float elapsedTimeSeconds,
		bool loop) {
		if (points.size() < 2) {
			return points.front().transform;
		}

		float remainingTime = elapsedTimeSeconds < 0.0f ? 0.0f : elapsedTimeSeconds;
		const size_t segmentCount = loop ? points.size() : points.size() - 1;
		if (loop) {
			const float totalDuration = GetTotalDurationSeconds(points, true);
			if (totalDuration > 0.0f) {
				remainingTime = std::fmod(remainingTime, totalDuration);
			}
		}
		for (size_t segment = 0; segment < segmentCount; ++segment) {
			const float segmentDuration = GetSegmentDurationSeconds(points[segment]);
			if (remainingTime < segmentDuration || (!loop && segment + 1 == segmentCount)) {
				float t = remainingTime / segmentDuration;
				if (t < 0.0f) {
					t = 0.0f;
				} else if (t > 1.0f) {
					t = 1.0f;
				}
				return EvaluateSegment(points, segment, t, loop);
			}
			remainingTime -= segmentDuration;
		}
		return EvaluateSegment(points, segmentCount - 1, 1.0f, loop);
	}
}

void QFE::FRAMEWORK::UpdateSplineMovement(
	QFE::EntityManager& entityManager,
	float deltaTime) {
	entityManager.Each<QFE::SCENE::SplineMovementComponent>(
		[&](uint32_t entityId, QFE::SCENE::SplineMovementComponent& spline) {
			if (!spline.enabled || spline.paused || (spline.finished && !spline.loop) ||
				spline.controlPoints.size() < 2 ||
				!entityManager.HasComponent<QFE::SCENE::TransformComponent>(entityId)) {
				return;
			}

			QFE::SCENE::TransformComponent& transform =
				entityManager.GetComponent<QFE::SCENE::TransformComponent>(entityId);
			if (!spline.movementStarted) {
				spline.elapsedTimeSeconds = 0.0f;
				spline.movementStarted = true;
				const QFE::MATH::EulerTransform startTransform = EvaluateSegment(
					spline.controlPoints, 0, 0.0f, spline.loop);
				transform.transform.translate = startTransform.translate;
				if (spline.syncRotationToPath) {
					transform.transform.rotate = startTransform.rotate;
				}
				transform.transform.scale = startTransform.scale;
			}

			const float totalDuration = GetTotalDurationSeconds(spline.controlPoints, spline.loop);
			if (spline.loop) {
				spline.finished = false;
				if (deltaTime > 0.0f && totalDuration > 0.0f) {
					spline.elapsedTimeSeconds = std::fmod(
						spline.elapsedTimeSeconds + deltaTime, totalDuration);
				}
			} else if (!spline.finished && deltaTime > 0.0f) {
				spline.elapsedTimeSeconds += deltaTime;
				if (spline.elapsedTimeSeconds >= totalDuration) {
					spline.elapsedTimeSeconds = totalDuration;
					spline.finished = true;
				}
			}
			const QFE::MATH::EulerTransform splineTransform = FindTransformAtTime(
				spline.controlPoints, spline.elapsedTimeSeconds, spline.loop);
			transform.transform.translate = splineTransform.translate;
			if (spline.syncRotationToPath) {
				transform.transform.rotate = splineTransform.rotate;
			}
			transform.transform.scale = splineTransform.scale;
		});
}

void QFE::FRAMEWORK::DrawSplineMovementPaths(
	const QFE::EntityManager& entityManager,
	const QFE::FRAMEWORK::SplineLineDrawer& drawLine) {
	if (!drawLine) {
		return;
	}
	using Clock = std::chrono::steady_clock;
	static const QFE::EntityManager* previewEntityManager = nullptr;
	static std::unordered_map<uint32_t, SplinePreviewState> previewStates;
	if (previewEntityManager != &entityManager) {
		previewEntityManager = &entityManager;
		previewStates.clear();
	}
	const Clock::time_point now = Clock::now();
	std::unordered_set<uint32_t> activePreviewEntities;
	entityManager.Each<QFE::SCENE::SplineMovementComponent>(
		[&](uint32_t entityId, const QFE::SCENE::SplineMovementComponent& spline) {
			if (!spline.drawPath || spline.controlPoints.size() < 2) {
				previewStates.erase(entityId);
				return;
			}
			activePreviewEntities.insert(entityId);
			const QFE::MATH::Matrix4x4 parentWorldMatrix =
				QFE::SCENE::GetParentWorldMatrix(entityManager, entityId);
			const std::vector<SplineSample> samples = BuildSamples(
				spline.controlPoints, parentWorldMatrix, spline.loop);

			const float totalDuration = GetTotalDurationSeconds(
				spline.controlPoints, spline.loop);
			auto [previewStateIt, inserted] = previewStates.try_emplace(entityId);
			SplinePreviewState& previewState = previewStateIt->second;
			if (inserted) {
				previewState.lastUpdated = now;
			} else {
				previewState.elapsedTimeSeconds += std::chrono::duration<float>(
					now - previewState.lastUpdated).count();
				previewState.lastUpdated = now;
			}
			const float previewTime = totalDuration > 0.0f
				? std::fmod(previewState.elapsedTimeSeconds, totalDuration)
				: 0.0f;
			const QFE::MATH::EulerTransform previewTransform = FindTransformAtTime(
				spline.controlPoints, previewTime, spline.loop);
			const QFE::MATH::Vector3 previewPosition = ToWorldPosition(
				previewTransform.translate, parentWorldMatrix);

			// Draw a small wireframe triangle facing along the spline tangent.
			const float pitch = previewTransform.rotate.x;
			const float yaw = previewTransform.rotate.y;
			const QFE::MATH::Vector3 localForward = QFE::MATH::Vector3{
				std::cos(pitch) * std::sin(yaw),
				-std::sin(pitch),
				std::cos(pitch) * std::cos(yaw)}.Normalize();
			QFE::MATH::Vector3 worldForward = (
				ToWorldPosition(previewTransform.translate + localForward, parentWorldMatrix) -
				previewPosition).Normalize();
			if (worldForward.LengthSq() <= 1.0e-6f) {
				worldForward = { 0.0f, 0.0f, 1.0f };
			}
			QFE::MATH::Vector3 worldSide = QFE::MATH::Vector3::Cross(
				{ 0.0f, 1.0f, 0.0f }, worldForward);
			if (worldSide.LengthSq() <= 1.0e-6f) {
				worldSide = QFE::MATH::Vector3::Cross(
					{ 1.0f, 0.0f, 0.0f }, worldForward);
			}
			if (worldSide.LengthSq() <= 1.0e-6f) {
				worldSide = QFE::MATH::Vector3::Cross(
					{ 0.0f, 0.0f, 1.0f }, worldForward);
			}
			worldSide = worldSide.Normalize();
			constexpr float kPreviewTriangleTipLength = 0.45f;
			constexpr float kPreviewTriangleBaseLength = 0.25f;
			constexpr float kPreviewTriangleHalfWidth = 0.22f;
			const QFE::MATH::Vector3 triangleTip =
				previewPosition + worldForward * kPreviewTriangleTipLength;
			const QFE::MATH::Vector3 triangleBaseCenter =
				previewPosition - worldForward * kPreviewTriangleBaseLength;
			const QFE::MATH::Vector3 triangleBaseLeft =
				triangleBaseCenter - worldSide * kPreviewTriangleHalfWidth;
			const QFE::MATH::Vector3 triangleBaseRight =
				triangleBaseCenter + worldSide * kPreviewTriangleHalfWidth;
			const QFE::MATH::Vector4 previewColor = { 1.0f, 0.8f, 0.1f, 1.0f };
			drawLine(triangleTip, triangleBaseLeft, previewColor);
			drawLine(triangleBaseLeft, triangleBaseRight, previewColor);
			drawLine(triangleBaseRight, triangleTip, previewColor);

			for (size_t index = 1; index < samples.size(); ++index) {
				drawLine(samples[index - 1].worldPosition, samples[index].worldPosition, spline.pathColor);
			}
		});
	for (auto stateIt = previewStates.begin(); stateIt != previewStates.end();) {
		if (activePreviewEntities.find(stateIt->first) == activePreviewEntities.end()) {
			stateIt = previewStates.erase(stateIt);
		} else {
			++stateIt;
		}
	}
}
