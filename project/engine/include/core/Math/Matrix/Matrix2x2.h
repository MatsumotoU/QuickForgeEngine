#pragma once
#ifndef Matrix2x2_H
#define Matrix2x2_H

class Matrix2x2 final{
public:
	float m[2][2];

public:
	float Get(int row, int col) const {
		return m[row][col];
	}
	void Set(int row, int col, float value) {
		m[row][col] = value;
	}

public:

	/// <summary>
	/// 縺薙・陦悟・縺ｮ霆｢蝨ｰ陦悟・繧呈ｱゅａ繧・
	/// </summary>
	/// <returns>縺薙・陦悟・縺ｮ霆｢蝨ｰ陦悟・</returns>
	[[nodiscard]] Matrix2x2 Transpose() const;

public:

	/// <summary>
	/// 2x2縺ｮ霆｢鄂ｮ陦悟・繧呈ｱゅａ繧・
	/// </summary>
	/// <param name="m">2x2縺ｮ陦悟・</param>
	/// <returns>2x2縺ｮ霆｢鄂ｮ陦悟・</returns>
	[[nodiscard]] static Matrix2x2 Transpose(const Matrix2x2& m);

public:
	/// <summary>
	/// 2縺､縺ｮ2x2陦悟・縺ｮ蜉邂礼ｵ先棡繧呈ｱゅａ繧・
	/// </summary>
	/// <param name="m1">1縺､逶ｮ縺ｮ2x2陦悟・</param>
	/// <param name="m2">2縺､逶ｮ縺ｮ2x2陦悟・</param>
	/// <returns>2縺､縺ｮ2x2陦悟・縺ｮ蜉邂礼ｵ先棡</returns>
	[[nodiscard]] static Matrix2x2 Add(const Matrix2x2& m1, const Matrix2x2& m2);

	/// <summary>
	/// 2縺､縺ｮ2x2陦悟・縺ｮ貂帷ｮ礼ｵ先棡繧呈ｱゅａ繧・
	/// </summary>
	/// <param name="m1">1縺､逶ｮ縺ｮ2x2陦悟・</param>
	/// <param name="m2">2縺､逶ｮ縺ｮ2x2陦悟・</param>
	/// <returns>2縺､縺ｮ2x2陦悟・縺ｮ貂帷ｮ礼ｵ先棡</returns>
	[[nodiscard]] static Matrix2x2 Subtract(const Matrix2x2& m1, const Matrix2x2& m2);

public:
	/// <summary>
	/// 2x2縺ｮ蜊倅ｽ崎｡悟・繧呈ｱゅａ繧・
	/// </summary>
	/// <returns>2x2縺ｮ蜊倅ｽ崎｡悟・</returns>
	[[nodiscard]] static Matrix2x2 MakeIdentity2x2();
};

#endif
