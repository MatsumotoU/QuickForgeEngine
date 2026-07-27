#pragma once
#include <string>
#include <vector>
#include "math/MathInclude.h"

namespace QFE::GRAPHIC {
	struct Node{
		QFE::MATH::Matrix4x4 localMatrix; // ノードのローカル変換行列
		std::string name; // ノードの名前
		QFE::MATH::QuaternionTransform transform; // ノードの変換情報（位置、回転、スケール）
		std::vector<Node> children; // 子ノードの配列
	};;
}