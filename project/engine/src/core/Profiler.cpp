#include "engine/include/core/Profiler.h"
#include "engine/include/core/EngineDefines.h"

namespace QFE {
	void Profiler::Initialize() {
		
	}
	void Profiler::Finalize() {
		
	}
	void Profiler::FrameStart() {
		frameStartTime_ = std::chrono::high_resolution_clock::now();
	}
	void Profiler::FrameEnd() {
		frameEndTime_ = std::chrono::high_resolution_clock::now();
		frameDuration_ = std::chrono::duration_cast<std::chrono::milliseconds>(frameEndTime_ - frameStartTime_);
	}
	void Profiler::ClearScopeProfiles()
	{
		scopeProfiles_.clear();
	}
	void Profiler::RecordScopeProfile(const std::string& scopeName, const std::chrono::milliseconds& duration)
	{
		QFE_LOG("Recorded scope profile: " + scopeName + " - " + std::to_string(duration.count()) + " ms", QFE::LogLevel::EngineInfo);
		scopeProfiles_[scopeName].push_back(duration);
		if (scopeProfiles_[scopeName].size() > maxProfileEntries_) {
			scopeProfiles_[scopeName].pop_front();
		}
	}
	const std::unordered_map<std::string, std::deque<std::chrono::milliseconds>>& Profiler::GetScopeProfiles() const
	{
		return scopeProfiles_;
	}
	std::chrono::milliseconds Profiler::GetFrameDuration() const
	{
		return frameDuration_;
	}
	std::chrono::milliseconds Profiler::GetAverageScopeDuration(const std::string& scopeName) const
	{
		auto it = scopeProfiles_.find(scopeName);
		if (it == scopeProfiles_.end() || it->second.empty()) {
			QFE_LOG("No profile data for scope: " + scopeName, QFE::LogLevel::Warning);
			return std::chrono::milliseconds(0);
		}

		std::chrono::milliseconds totalDuration(0);
		for (const auto& duration : it->second) {
			totalDuration += duration;
		}
		return totalDuration / static_cast<int>(it->second.size());
	}
}