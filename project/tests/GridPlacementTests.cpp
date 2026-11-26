#include "editor/window/GridPlacement.h"
#include <cmath>

int main() {
	using namespace QFE::EDITOR;
	using namespace QFE::MATH;
	GridPlacement grid;
	grid.enabled = true;
	grid.spacing = 0.5f;
	grid.layer = 1;
	grid.plane = GridPlane::XY;
	Vector3 position{};
	const auto identity = Matrix4x4::MakeIdentity4x4();
	if (!IntersectGrid(grid, identity, 0.76f, 0.25f, position) ||
		position.x != 0.5f || position.y != 0.5f || position.z != 0.5f) return 1;
	grid.layer = -1;
	if (IntersectGrid(grid, identity, 0.5f, 0.5f, position)) return 2;
	grid.plane = GridPlane::XZ;
	grid.layer = 1;
	if (IntersectGrid(grid, identity, 0.5f, 0.5f, position)) return 3;
	if (!IntersectGrid(grid, Matrix4x4::MakeRotateXMatrix(1.5707963f), 0.5f, 0.5f, position) ||
		std::fabs(position.y - 0.5f) > 0.0001f) return 4;
	grid.plane = GridPlane::YZ;
	grid.layer = -1;
	if (!IntersectGrid(grid, Matrix4x4::MakeRotateYMatrix(1.5707963f), 0.5f, 0.5f, position) ||
		std::fabs(position.x + 0.5f) > 0.0001f) return 5;
	const auto snapped = SnapGridPosition({ -0.76f, 1.24f, 0.26f }, 0.5f);
	if (snapped.x != -1.0f || snapped.y != 1.0f || snapped.z != 0.5f) return 6;
	grid.spacing = 0.0f;
	if (IntersectGrid(grid, identity, 0.5f, 0.5f, position)) return 7;
	return 0;
}
