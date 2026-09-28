#include "SplineMovementSystem.h"

#include "design-patterns/EntityManager.h"
#include "components/SplineMovementComponent.h"
#include "components/SplineMoveToPointComponent.h"
#include "components/TransformHierarchy.h"
#include "EventSystem.h"

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

	float GetSegmentDurationSeconds(const QFE::SCENE::SplineControlPoint& point) {
		constexpr float kMinimumSegmentDurationSeconds = 0.01f;
		return point.secondsToNextPoint < kMinimumSegmentDurationSeconds
			? kMinimumSegmentDurationSeconds
			: point.secondsToNextPoint;
	}

	QFE::MATH::Vector3 GetControlPointTimeTangent(
		const std::vector<QFE::SCENE::SplineControlPoint>& points,
		size_t pointIndex,
		bool loop) {
		const size_t pointCount = points.size();
		const size_t previousIndex = loop
			? (pointIndex + pointCount - 1) % pointCount
			: (pointIndex == 0 ? pointIndex : pointIndex - 1);
		const size_t nextIndex = loop
			? (pointIndex + 1) % pointCount
			: (pointIndex + 1 < pointCount ? pointIndex + 1 : pointIndex);

		if (!loop && pointIndex == 0) {
			return (points[nextIndex].transform.translate - points[pointIndex].transform.translate) /
				GetSegmentDurationSeconds(points[pointIndex]);
		}
		if (!loop && pointIndex + 1 == pointCount) {
			return (points[pointIndex].transform.translate - points[previousIndex].transform.translate) /
				GetSegmentDurationSeconds(points[previousIndex]);
		}

		const float previousDuration = GetSegmentDurationSeconds(points[previousIndex]);
		const float nextDuration = GetSegmentDurationSeconds(points[pointIndex]);
		return (points[nextIndex].transform.translate - points[previousIndex].transform.translate) /
			(previousDuration + nextDuration);
	}

	QFE::MATH::Vector3 EvaluateSplineSegmentPosition(
		const std::vector<QFE::SCENE::SplineControlPoint>& points,
		size_t startIndex,
		size_t endIndex,
		float t,
		bool loop) {
		const float t2 = t * t;
		const float t3 = t2 * t;
		const float segmentDuration = GetSegmentDurationSeconds(points[startIndex]);
		const QFE::MATH::Vector3& p1 = points[startIndex].transform.translate;
		const QFE::MATH::Vector3& p2 = points[endIndex].transform.translate;
		const QFE::MATH::Vector3 tangent1 =
			GetControlPointTimeTangent(points, startIndex, loop) * segmentDuration;
		const QFE::MATH::Vector3 tangent2 =
			GetControlPointTimeTangent(points, endIndex, loop) * segmentDuration;
		const float h00 = 2.0f * t3 - 3.0f * t2 + 1.0f;
		const float h10 = t3 - 2.0f * t2 + t;
		const float h01 = -2.0f * t3 + 3.0f * t2;
		const float h11 = t3 - t2;
		return p1 * h00 + tangent1 * h10 + p2 * h01 + tangent2 * h11;
	}

	QFE::MATH::Vector3 EvaluateSplineSegmentTangent(
		const std::vector<QFE::SCENE::SplineControlPoint>& points,
		size_t startIndex,
		size_t endIndex,
		float t,
		bool loop) {
		const float t2 = t * t;
		const float segmentDuration = GetSegmentDurationSeconds(points[startIndex]);
		const QFE::MATH::Vector3& p1 = points[startIndex].transform.translate;
		const QFE::MATH::Vector3& p2 = points[endIndex].transform.translate;
		const QFE::MATH::Vector3 tangent1 =
			GetControlPointTimeTangent(points, startIndex, loop) * segmentDuration;
		const QFE::MATH::Vector3 tangent2 =
			GetControlPointTimeTangent(points, endIndex, loop) * segmentDuration;
		return p1 * (6.0f * t2 - 6.0f * t) +
			tangent1 * (3.0f * t2 - 4.0f * t + 1.0f) +
			p2 * (-6.0f * t2 + 6.0f * t) +
			tangent2 * (3.0f * t2 - 2.0f * t);
	}

	QFE::MATH::EulerTransform EvaluateSegment(
		const std::vector<QFE::SCENE::SplineControlPoint>& points,
		size_t segmentIndex,
		float t,
		bool loop,
		bool reverseFacing = false) {
		const size_t pointCount = points.size();
		const size_t startIndex = loop ? segmentIndex % pointCount : segmentIndex;
		const size_t endIndex = loop ? (startIndex + 1) % pointCount : startIndex + 1;
		const size_t nextIndex = loop
			? (endIndex + 1) % pointCount
			: (endIndex + 1 < pointCount ? endIndex + 1 : endIndex);
		const auto& p1 = points[startIndex].transform;
		const auto& p2 = points[endIndex].transform;
		const auto& p3 = points[nextIndex].transform;

		QFE::MATH::EulerTransform result;
		result.translate = EvaluateSplineSegmentPosition(
			points, startIndex, endIndex, t, loop);
		// Use the time-aware curve tangent so position and facing remain smooth across knots.
		QFE::MATH::Vector3 facingDirection = EvaluateSplineSegmentTangent(
			points, startIndex, endIndex, t, loop);
		if (facingDirection.LengthSq() <= 1.0e-6f) {
			facingDirection = p2.translate - p1.translate;
		}
		if (facingDirection.LengthSq() <= 1.0e-6f) {
			facingDirection = p3.translate - p1.translate;
		}
		if (facingDirection.LengthSq() <= 1.0e-6f) {
			facingDirection = { 0.0f, 0.0f, 1.0f };
		}
		if (reverseFacing) {
			facingDirection = -facingDirection;
		}
		const QFE::MATH::Vector3 facingRotation = QFE::MATH::Vector3::LookAt(
			QFE::MATH::Vector3::Zero(), facingDirection);
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

	float GetControlPointTimeSeconds(
		const std::vector<QFE::SCENE::SplineControlPoint>& points,
		size_t controlPointIndex) {
		float timeSeconds = 0.0f;
		const size_t segmentCount = (std::min)(controlPointIndex, points.size() - 1);
		for (size_t segment = 0; segment < segmentCount; ++segment) {
			timeSeconds += GetSegmentDurationSeconds(points[segment]);
		}
		return timeSeconds;
	}

	float WrapTimeSeconds(float timeSeconds, float totalDurationSeconds) {
		if (totalDurationSeconds <= 0.0f) {
			return 0.0f;
		}
		float wrappedTime = std::fmod(timeSeconds, totalDurationSeconds);
		if (wrappedTime < 0.0f) {
			wrappedTime += totalDurationSeconds;
		}
		return wrappedTime;
	}

	QFE::MATH::EulerTransform FindTransformAtTime(
		const std::vector<QFE::SCENE::SplineControlPoint>& points,
		float elapsedTimeSeconds,
		bool loop,
		bool reverseFacing = false) {
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
				return EvaluateSegment(points, segment, t, loop, reverseFacing);
			}
			remainingTime -= segmentDuration;
		}
		return EvaluateSegment(points, segmentCount - 1, 1.0f, loop, reverseFacing);
	}
}

void QFE::FRAMEWORK::UpdateSplineMovement(
	QFE::EntityManager& entityManager,
	float deltaTime) {
	entityManager.Each<QFE::SCENE::SplineMovementComponent>(
		[&](uint32_t entityId, QFE::SCENE::SplineMovementComponent& spline) {
			if (!spline.enabled || spline.paused ||
				spline.controlPoints.size() < 2 ||
				!entityManager.HasComponent<QFE::SCENE::TransformComponent>(entityId)) {
				return;
			}

			QFE::SCENE::SplineMoveToPointComponent* moveToPoint =
				entityManager.HasComponent<QFE::SCENE::SplineMoveToPointComponent>(entityId)
				? &entityManager.GetComponent<QFE::SCENE::SplineMoveToPointComponent>(entityId)
				: nullptr;
			QFE::SCENE::TransformComponent& transform =
				entityManager.GetComponent<QFE::SCENE::TransformComponent>(entityId);
			if (!spline.movementStarted) {
				spline.elapsedTimeSeconds = 0.0f;
				spline.elapsedAfterEndSeconds = 0.0f;
				spline.movementStarted = true;
				spline.finished = false;
				spline.endEventTriggered = false;
				const QFE::MATH::EulerTransform startTransform = EvaluateSegment(
					spline.controlPoints, 0, 0.0f, spline.loop);
				transform.transform.translate = startTransform.translate;
				if (spline.syncRotationToPath) {
					transform.transform.rotate = startTransform.rotate;
				}
				transform.transform.scale = startTransform.scale;
			}

			const float totalDuration = GetTotalDurationSeconds(spline.controlPoints, spline.loop);
			if (moveToPoint != nullptr && moveToPoint->requestMoveToControlPoint) {
				moveToPoint->requestMoveToControlPoint = false;
				const size_t destinationIndex = (std::min)(
					static_cast<size_t>(moveToPoint->targetControlPointIndex),
					spline.controlPoints.size() - 1);
				moveToPoint->requestedTargetTimeSeconds = GetControlPointTimeSeconds(
					spline.controlPoints, destinationIndex);

				const float currentTime = spline.loop
					? WrapTimeSeconds(spline.elapsedTimeSeconds, totalDuration)
					: spline.elapsedTimeSeconds;
				float timeToDestination = moveToPoint->requestedTargetTimeSeconds - currentTime;
				if (spline.loop) {
					float forwardTime = timeToDestination;
					if (forwardTime < 0.0f) {
						forwardTime += totalDuration;
					}
					const float backwardTime = forwardTime - totalDuration;
					if (forwardTime <= -backwardTime) {
						moveToPoint->requestedTravelDirection = 1;
						moveToPoint->requestedTravelTimeRemainingSeconds = forwardTime;
					} else {
						moveToPoint->requestedTravelDirection = -1;
						moveToPoint->requestedTravelTimeRemainingSeconds = -backwardTime;
					}
				} else {
					moveToPoint->requestedTravelDirection = timeToDestination < 0.0f ? -1 : 1;
					moveToPoint->requestedTravelTimeRemainingSeconds = std::abs(timeToDestination);
				}

				moveToPoint->movingToRequestedPoint =
					moveToPoint->requestedTravelTimeRemainingSeconds > 1.0e-6f;
				moveToPoint->stoppedAtRequestedPoint = !moveToPoint->movingToRequestedPoint;
				spline.finished = !spline.loop && !moveToPoint->movingToRequestedPoint &&
					destinationIndex + 1 == spline.controlPoints.size();
				spline.elapsedAfterEndSeconds = 0.0f;
				spline.endEventTriggered = false;
			}

			bool reachedEndThisFrame = false;
			float timePastEndThisFrame = 0.0f;
			if (spline.loop) {
				spline.finished = false;
				if (moveToPoint != nullptr && moveToPoint->movingToRequestedPoint) {
					const float timeStep = (std::min)(
						(std::max)(0.0f, deltaTime),
						moveToPoint->requestedTravelTimeRemainingSeconds);
					spline.elapsedTimeSeconds = WrapTimeSeconds(
						spline.elapsedTimeSeconds +
							static_cast<float>(moveToPoint->requestedTravelDirection) * timeStep,
						totalDuration);
					moveToPoint->requestedTravelTimeRemainingSeconds -= timeStep;
					if (moveToPoint->requestedTravelTimeRemainingSeconds <= 1.0e-6f) {
						spline.elapsedTimeSeconds = WrapTimeSeconds(
							moveToPoint->requestedTargetTimeSeconds, totalDuration);
						moveToPoint->requestedTravelTimeRemainingSeconds = 0.0f;
						moveToPoint->movingToRequestedPoint = false;
						moveToPoint->stoppedAtRequestedPoint = true;
					}
				} else if (spline.autoPlay &&
					!(moveToPoint != nullptr && moveToPoint->stoppedAtRequestedPoint) &&
					deltaTime > 0.0f && totalDuration > 0.0f) {
					spline.elapsedTimeSeconds = WrapTimeSeconds(
						spline.elapsedTimeSeconds + deltaTime, totalDuration);
				}
			} else {
				if (moveToPoint != nullptr && moveToPoint->movingToRequestedPoint) {
					const float timeStep = (std::min)(
						(std::max)(0.0f, deltaTime),
						moveToPoint->requestedTravelTimeRemainingSeconds);
					spline.elapsedTimeSeconds +=
						static_cast<float>(moveToPoint->requestedTravelDirection) * timeStep;
					moveToPoint->requestedTravelTimeRemainingSeconds -= timeStep;
					if (moveToPoint->requestedTravelTimeRemainingSeconds <= 1.0e-6f) {
						spline.elapsedTimeSeconds = moveToPoint->requestedTargetTimeSeconds;
						moveToPoint->requestedTravelTimeRemainingSeconds = 0.0f;
						moveToPoint->movingToRequestedPoint = false;
						const bool reachedSplineEnd = moveToPoint->requestedTargetTimeSeconds >= totalDuration;
						spline.finished = reachedSplineEnd;
						moveToPoint->stoppedAtRequestedPoint = !reachedSplineEnd;
						if (reachedSplineEnd) {
							reachedEndThisFrame = true;
							timePastEndThisFrame = (std::max)(0.0f, deltaTime - timeStep);
						}
					}
				} else if (spline.autoPlay &&
					!(moveToPoint != nullptr && moveToPoint->stoppedAtRequestedPoint) &&
					!spline.finished && deltaTime > 0.0f) {
					const float nextElapsedTime = spline.elapsedTimeSeconds + deltaTime;
					if (nextElapsedTime >= totalDuration) {
						timePastEndThisFrame = nextElapsedTime - totalDuration;
						spline.elapsedTimeSeconds = totalDuration;
						spline.finished = true;
						reachedEndThisFrame = true;
					} else {
						spline.elapsedTimeSeconds = nextElapsedTime;
					}
				}
				if (spline.finished) {
					if (reachedEndThisFrame) {
						spline.elapsedAfterEndSeconds += timePastEndThisFrame;
					} else if (!spline.endEventTriggered) {
						spline.elapsedAfterEndSeconds += (std::max)(0.0f, deltaTime);
					}
					if (spline.triggerEventAfterEnd && !spline.endEventTriggered &&
						spline.elapsedAfterEndSeconds >= (std::max)(0.0f, spline.eventDelayAfterEndSeconds)) {
						spline.endEventTriggered = QFE::FRAMEWORK::PlayEvent(entityManager, entityId);
					}
				}
			}
			const QFE::MATH::EulerTransform splineTransform = FindTransformAtTime(
				spline.controlPoints,
				spline.elapsedTimeSeconds,
				spline.loop,
				moveToPoint != nullptr && moveToPoint->movingToRequestedPoint &&
				moveToPoint->requestedTravelDirection < 0);
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
