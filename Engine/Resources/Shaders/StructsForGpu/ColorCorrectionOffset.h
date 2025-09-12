#pragma once
struct alignas(16) ColorCorrectionOffset {
    float exposure; // 露出
    float contrast; // コントラスト
    float saturation; // 彩度
    float gamma; // ガンマ
    float hue; // 色相
	float padding[3]; // パディング（16バイト境界に合わせるため）
};