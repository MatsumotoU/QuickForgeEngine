#pragma once

enum AudioMaterialType {
	None = 0,
	Wood,
	Stone,
	Metal,
	Glass,
	Water,
	Max,
};

struct AudioMaterial final {
	AudioMaterialType type_;
};