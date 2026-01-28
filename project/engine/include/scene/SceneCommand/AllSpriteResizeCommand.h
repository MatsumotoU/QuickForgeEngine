#pragma once
#include "ISceneEntityCommand.h"
namespace QFE {
	class AllSpriteResizeCommand : public ISceneEntityCommand
	{
	public:
		AllSpriteResizeCommand() = delete;
		explicit AllSpriteResizeCommand(EntityManager& em);
		virtual ~AllSpriteResizeCommand() = default;

		virtual void Execute() override;
		virtual void Undo() override;

		virtual std::string GetCommandName() const override { return "Sprite Resize Command"; }
	};
}