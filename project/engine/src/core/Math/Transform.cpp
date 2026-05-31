#include "engine/include/core/Math/Transform.h"

using namespace QFE;

void EulerTransform::FromMatrix(const Matrix4x4& mat) {
    // 平行移動
    translate.x = mat.m[3][0];
    translate.y = mat.m[3][1];
    translate.z = mat.m[3][2];

    // スケール
    scale.x = std::sqrt(mat.m[0][0] * mat.m[0][0] + mat.m[0][1] * mat.m[0][1] + mat.m[0][2] * mat.m[0][2]);
    scale.y = std::sqrt(mat.m[1][0] * mat.m[1][0] + mat.m[1][1] * mat.m[1][1] + mat.m[1][2] * mat.m[1][2]);
    scale.z = std::sqrt(mat.m[2][0] * mat.m[2][0] + mat.m[2][1] * mat.m[2][1] + mat.m[2][2] * mat.m[2][2]);

    // 回転（スケールを除去した3x3部分からXYZオイラー角を取得）
    Matrix4x4 rotMat;
    for (int i = 0; i < 3; ++i) {
        rotMat.m[0][i] = mat.m[0][i] / scale.x;
        rotMat.m[1][i] = mat.m[1][i] / scale.y;
        rotMat.m[2][i] = mat.m[2][i] / scale.z;
    }
    rotMat.m[3][0] = rotMat.m[3][1] = rotMat.m[3][2] = 0.0f;
    rotMat.m[0][3] = rotMat.m[1][3] = rotMat.m[2][3] = 0.0f;
    rotMat.m[3][3] = 1.0f;

    // XYZ順オイラー角
    rotate.y = std::asin(-rotMat.m[2][0]);
    if (std::cos(rotate.y) > 1e-6) {
        rotate.x = std::atan2(rotMat.m[2][1], rotMat.m[2][2]);
        rotate.z = std::atan2(rotMat.m[1][0], rotMat.m[0][0]);
    }
    else {
        rotate.x = std::atan2(-rotMat.m[1][2], rotMat.m[1][1]);
        rotate.z = 0.0f;
    }
}
