#include "AnimationDataServices.h"

#include <cstdint>
#include <filesystem>
#include <fstream>

namespace {
	constexpr uint32_t kMaximumStringLength = 1024;
	constexpr uint32_t kMaximumKeyFrames = 100000;

	template <typename T>
	bool Write(std::ofstream& stream, const T& value) {
		stream.write(reinterpret_cast<const char*>(&value), sizeof(T));
		return stream.good();
	}

	template <typename T>
	bool Read(std::ifstream& stream, T& value) {
		stream.read(reinterpret_cast<char*>(&value), sizeof(T));
		return stream.good();
	}
}

bool QFE::ANIMATION::SaveAnimationClip(const AnimationClip& clip, const std::string& filePath) {
	const std::filesystem::path path(filePath);
	std::error_code error;
	if (path.has_parent_path()) std::filesystem::create_directories(path.parent_path(), error);
	std::ofstream stream(path, std::ios::binary | std::ios::trunc);
	if (!stream) return false;

	const uint32_t nameSize = static_cast<uint32_t>(clip.GetName().size());
	if (!Write(stream, nameSize)) return false;
	stream.write(clip.GetName().data(), nameSize);
	const uint8_t loop = clip.IsLoop() ? 1u : 0u;
	if (!Write(stream, loop)) return false;

	const auto& keyFrames = clip.GetKeyFrames();
	const uint32_t keyFrameCount = static_cast<uint32_t>(keyFrames.size());
	if (!Write(stream, keyFrameCount)) return false;
	for (const AnimationKeyFrame& keyFrame : keyFrames) {
		if (!Write(stream, keyFrame.time) ||
			!Write(stream, keyFrame.transform.translate.x) ||
			!Write(stream, keyFrame.transform.translate.y) ||
			!Write(stream, keyFrame.transform.translate.z) ||
			!Write(stream, keyFrame.transform.rotate.x) ||
			!Write(stream, keyFrame.transform.rotate.y) ||
			!Write(stream, keyFrame.transform.rotate.z) ||
			!Write(stream, keyFrame.transform.scale.x) ||
			!Write(stream, keyFrame.transform.scale.y) ||
			!Write(stream, keyFrame.transform.scale.z)) return false;
	}
	return stream.good();
}

bool QFE::ANIMATION::LoadAnimationClip(const std::string& filePath, AnimationClip& clip) {
	std::ifstream stream(filePath, std::ios::binary);
	if (!stream) return false;

	uint32_t nameSize = 0;
	if (!Read(stream, nameSize) || nameSize > kMaximumStringLength) return false;
	std::string name(nameSize, '\0');
	stream.read(name.data(), nameSize);
	uint8_t loop = 0;
	uint32_t keyFrameCount = 0;
	if (!stream.good() || !Read(stream, loop) || !Read(stream, keyFrameCount) ||
		keyFrameCount > kMaximumKeyFrames) return false;

	AnimationClip loaded;
	loaded.SetName(name);
	loaded.SetLoop(loop != 0);
	for (uint32_t index = 0; index < keyFrameCount; ++index) {
		AnimationKeyFrame keyFrame;
		if (!Read(stream, keyFrame.time) ||
			!Read(stream, keyFrame.transform.translate.x) ||
			!Read(stream, keyFrame.transform.translate.y) ||
			!Read(stream, keyFrame.transform.translate.z) ||
			!Read(stream, keyFrame.transform.rotate.x) ||
			!Read(stream, keyFrame.transform.rotate.y) ||
			!Read(stream, keyFrame.transform.rotate.z) ||
			!Read(stream, keyFrame.transform.scale.x) ||
			!Read(stream, keyFrame.transform.scale.y) ||
			!Read(stream, keyFrame.transform.scale.z)) return false;
		loaded.AddKeyFrame(keyFrame);
	}
	clip = std::move(loaded);
	return true;
}

std::string QFE::ANIMATION::ResolveAnimationClipPath(const std::string& clipName) {
	if (clipName.empty()) return {};
	std::filesystem::path path(clipName);
	if (path.extension().empty()) path.replace_extension(".anim");
	if (path.is_absolute() || path.string().starts_with("resources")) return path.generic_string();
	if (path.has_parent_path()) return (std::filesystem::path("resources") / path).generic_string();
	return (std::filesystem::path("resources/animations") / path).generic_string();
}
