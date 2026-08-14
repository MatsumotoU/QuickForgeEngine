#pragma once

#include <array>
#include <atomic>
#include <chrono>
#include <cstdio>
#include <random>
#include <string>

namespace QFE {
	/// @brief RFC 4122互換のランダムUUIDを生成する。
	[[nodiscard]] inline std::string GenerateEntityUuid() {
		static std::atomic<uint64_t> sequence{ 0 };
		thread_local std::mt19937_64 randomEngine([] {
			std::random_device randomDevice;
			const uint64_t timeSeed = static_cast<uint64_t>(
				std::chrono::high_resolution_clock::now().time_since_epoch().count());
			std::seed_seq seed{
				randomDevice(), randomDevice(),
				static_cast<uint32_t>(timeSeed),
				static_cast<uint32_t>(timeSeed >> 32)
			};
			return std::mt19937_64(seed);
		}());

		std::array<unsigned char, 16> bytes{};
		const uint64_t first = randomEngine();
		const uint64_t second = randomEngine() ^ sequence.fetch_add(1, std::memory_order_relaxed);
		for (size_t index = 0; index < 8; ++index) {
			bytes[index] = static_cast<unsigned char>(first >> (index * 8));
			bytes[index + 8] = static_cast<unsigned char>(second >> (index * 8));
		}
		bytes[6] = static_cast<unsigned char>((bytes[6] & 0x0f) | 0x40);
		bytes[8] = static_cast<unsigned char>((bytes[8] & 0x3f) | 0x80);

		char uuid[37]{};
		std::snprintf(
			uuid, sizeof(uuid),
			"%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
			bytes[0], bytes[1], bytes[2], bytes[3],
			bytes[4], bytes[5], bytes[6], bytes[7],
			bytes[8], bytes[9], bytes[10], bytes[11],
			bytes[12], bytes[13], bytes[14], bytes[15]);
		return uuid;
	}
}
