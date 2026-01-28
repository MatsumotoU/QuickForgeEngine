#pragma once
#include <string>

namespace QFE {
	class EntityManager;

	class ISceneEntityCommand {
	public:
		ISceneEntityCommand() = delete;
		explicit ISceneEntityCommand(EntityManager& em) : entityManager_(em) {}
		virtual ~ISceneEntityCommand() = default;
		virtual void Execute() = 0;
		virtual void Undo() = 0;

		virtual std::string GetCommandName() const = 0;

	protected:
		EntityManager& entityManager_;
	};
}