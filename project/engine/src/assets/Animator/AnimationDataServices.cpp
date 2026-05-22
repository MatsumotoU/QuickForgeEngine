#include "engine/include/assets/Animator/AnimationDataServices.h"
#include "engine/include/core/EngineDefines.h"

#include <fstream>
#include <cstdint>

using namespace QFE;

void QFE::ANIMATION::SaveAnimClipToAnimFile(const AnimClip& animClip, const std::string& filePath)
{
	// ファイルをバイナリモードで開く
	std::ofstream outFile(filePath, std::ios::binary);
	if (!outFile) {
		QFE_REPORT_SYSTEM_ERROR("Failed to open file for writing: " + filePath, SystemError::Abort);
		return;
	}

	// クリップ名
	const std::string& clipName = animClip.GetName();
	uint32_t nameSize = static_cast<uint32_t>(clipName.size());
	outFile.write(reinterpret_cast<const char*>(&nameSize), sizeof(nameSize));
	outFile.write(clipName.data(), nameSize);

	// ループフラグ
	uint8_t isLoop = animClip.IsLoop() ? 1 : 0;
	outFile.write(reinterpret_cast<const char*>(&isLoop), sizeof(isLoop));

	// キーフレーム数
	const std::vector<KeyFrame> keyframes = animClip.GetKeyFrames();
	uint32_t keyframeCount = static_cast<uint32_t>(keyframes.size());
	outFile.write(reinterpret_cast<const char*>(&keyframeCount), sizeof(keyframeCount));

	// 時間とTransformのペアをファイルに保存する
	for (const KeyFrame& keyframe : keyframes) {
		outFile.write(reinterpret_cast<const char*>(&keyframe.time), sizeof(keyframe.time));

		outFile.write(reinterpret_cast<const char*>(&keyframe.transform.translate.x), sizeof(keyframe.transform.translate.x));
		outFile.write(reinterpret_cast<const char*>(&keyframe.transform.translate.y), sizeof(keyframe.transform.translate.y));
		outFile.write(reinterpret_cast<const char*>(&keyframe.transform.translate.z), sizeof(keyframe.transform.translate.z));

		outFile.write(reinterpret_cast<const char*>(&keyframe.transform.rotate.x), sizeof(keyframe.transform.rotate.x));
		outFile.write(reinterpret_cast<const char*>(&keyframe.transform.rotate.y), sizeof(keyframe.transform.rotate.y));
		outFile.write(reinterpret_cast<const char*>(&keyframe.transform.rotate.z), sizeof(keyframe.transform.rotate.z));

		outFile.write(reinterpret_cast<const char*>(&keyframe.transform.scale.x), sizeof(keyframe.transform.scale.x));
		outFile.write(reinterpret_cast<const char*>(&keyframe.transform.scale.y), sizeof(keyframe.transform.scale.y));
		outFile.write(reinterpret_cast<const char*>(&keyframe.transform.scale.z), sizeof(keyframe.transform.scale.z));
	}

	if (!outFile) {
		QFE_REPORT_SYSTEM_ERROR("Failed to write animation data: " + filePath, SystemError::Abort);
	}
}

AnimClip QFE::ANIMATION::LoadAnimClipFromAnimFile(const std::string& filePath)
{
	AnimClip animClip;

	std::ifstream inFile(filePath, std::ios::binary);
	if (!inFile) {
		QFE_REPORT_SYSTEM_ERROR("Failed to open file for reading: " + filePath, SystemError::Abort);
		return animClip;
	}

	// クリップ名
	uint32_t nameSize = 0;
	inFile.read(reinterpret_cast<char*>(&nameSize), sizeof(nameSize));
	if (!inFile) {
		QFE_REPORT_SYSTEM_ERROR("Failed to read clip name size: " + filePath, SystemError::Abort);
		return animClip;
	}

	std::string clipName(nameSize, '\0');
	inFile.read(&clipName[0], nameSize);
	if (!inFile) {
		QFE_REPORT_SYSTEM_ERROR("Failed to read clip name: " + filePath, SystemError::Abort);
		return animClip;
	}
	animClip.SetName(clipName);

	// ループフラグ
	uint8_t isLoop = 0;
	inFile.read(reinterpret_cast<char*>(&isLoop), sizeof(isLoop));
	if (!inFile) {
		QFE_REPORT_SYSTEM_ERROR("Failed to read loop flag: " + filePath, SystemError::Abort);
		return animClip;
	}
	animClip.SetLoop(isLoop != 0);

	// キーフレーム数
	uint32_t keyframeCount = 0;
	inFile.read(reinterpret_cast<char*>(&keyframeCount), sizeof(keyframeCount));
	if (!inFile) {
		QFE_REPORT_SYSTEM_ERROR("Failed to read keyframe count: " + filePath, SystemError::Abort);
		return animClip;
	}

	for (uint32_t i = 0; i < keyframeCount; ++i) {
		KeyFrame keyframe{};

		inFile.read(reinterpret_cast<char*>(&keyframe.time), sizeof(keyframe.time));

		inFile.read(reinterpret_cast<char*>(&keyframe.transform.translate.x), sizeof(keyframe.transform.translate.x));
		inFile.read(reinterpret_cast<char*>(&keyframe.transform.translate.y), sizeof(keyframe.transform.translate.y));
		inFile.read(reinterpret_cast<char*>(&keyframe.transform.translate.z), sizeof(keyframe.transform.translate.z));

		inFile.read(reinterpret_cast<char*>(&keyframe.transform.rotate.x), sizeof(keyframe.transform.rotate.x));
		inFile.read(reinterpret_cast<char*>(&keyframe.transform.rotate.y), sizeof(keyframe.transform.rotate.y));
		inFile.read(reinterpret_cast<char*>(&keyframe.transform.rotate.z), sizeof(keyframe.transform.rotate.z));

		inFile.read(reinterpret_cast<char*>(&keyframe.transform.scale.x), sizeof(keyframe.transform.scale.x));
		inFile.read(reinterpret_cast<char*>(&keyframe.transform.scale.y), sizeof(keyframe.transform.scale.y));
		inFile.read(reinterpret_cast<char*>(&keyframe.transform.scale.z), sizeof(keyframe.transform.scale.z));

		if (!inFile) {
			QFE_REPORT_SYSTEM_ERROR("Failed to read keyframe data: " + filePath, SystemError::Abort);
			return AnimClip();
		}

		animClip.AddKeyFrame(keyframe);
	}

	return animClip;
}
