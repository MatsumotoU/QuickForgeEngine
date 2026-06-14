#pragma once
namespace QFE::GRAPHIC::INTERNAL {
	class IRenderCommand {
	public:
		virtual ~IRenderCommand() = default;
		/// @brief コマンドリストに描画コマンドを発行
		virtual void Execute() = 0;
	};
}
