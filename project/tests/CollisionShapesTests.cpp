#include "game/shooting/systems/CollisionShapes.h"

#include <cassert>
#include <cmath>

using namespace QFE::GAMESYSTEM::CollisionShapes;

int main() {
	// 移動側と壁側を同じ形状で用意し、壁を通過する前の接触時刻を確かめる。
	const Shape box{ 1, 0, false, { 0.0f, 0.0f, 0.0f }, 0.0f,
		{ { 1, 0, 0 }, { 0, 1, 0 }, { 0, 0, 1 } }, { 0.5f, 0.5f, 0.5f } };
	Shape wall = box;
	wall.entityId = 2;
	wall.center = { 3.0f, 0.0f, 0.0f };
	float time = -1.0f;
	Vector3 normal{};
	assert(SweepBoxBox(box, { 4.0f, 0.0f, 0.0f }, wall, time, normal));
	assert(std::abs(time - 0.5f) < 1.0e-5f && normal.x < -0.99f);
	assert(!SweepBoxBox(box, { 0.0f, 0.0f, 4.0f }, wall, time, normal));
	Shape touching = box;
	touching.center = { 2.0f, 0.0f, 0.0f };
	assert(SweepBoxBox(touching, { 1.0f, 0.0f, 0.25f }, wall, time, normal));
	assert(time == 0.0f && normal.x < -0.99f);
	assert(!SweepBoxBox(touching, { -1.0f, 0.0f, 0.0f }, wall, time, normal));

	// 回転した OBB と AABB の両方向で、高速移動時にも衝突する。
	const float diagonal = std::sqrt(0.5f);
	Shape rotated = box;
	rotated.axes[0] = { diagonal, 0.0f, diagonal };
	rotated.axes[2] = { -diagonal, 0.0f, diagonal };
	assert(SweepBoxBox(rotated, { 10.0f, 0.0f, 0.0f }, wall, time, normal));
	rotated.center = wall.center;
	assert(SweepBoxBox(box, { 10.0f, 0.0f, 0.0f }, rotated, time, normal));

	// 壁際で箱が回転し、角だけが壁に入ったときも外へ押し戻せる。
	Shape wallAtEdge = box;
	wallAtEdge.center = { 1.0f, 0.0f, 0.0f };
	Shape turned = box;
	turned.axes[0] = { diagonal, 0.0f, diagonal };
	turned.axes[2] = { -diagonal, 0.0f, diagonal };
	float depth = 0.0f;
	assert(BoxPenetration(turned, wallAtEdge, depth, normal));
	assert(depth > 0.2f && normal.x < -0.99f);
	turned.center += normal * (depth + 1.0e-5f);
	assert(!BoxPenetration(turned, wallAtEdge, depth, normal));
	// 押し戻した後も、壁に向かう移動を止められる。
	assert(SweepBoxBox(turned, { 0.1f, 0.0f, 0.1f }, wallAtEdge, time, normal));

	// 最初から重なっている場合は、従来どおり脱出を妨げない。
	wall.center = { 0.25f, 0.0f, 0.0f };
	assert(!SweepBoxBox(box, { -1.0f, 0.0f, 0.0f }, wall, time, normal));
}
