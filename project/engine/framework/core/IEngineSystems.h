#pragma once
namespace QFE {
	namespace GRAPHIC {
		class IGraphicEngine;
	}
	namespace INPUT {
		class InputInterface;
	}

	namespace FRAMEWORK {
		class IEngineSystems {
			public:
			virtual ~IEngineSystems() = default;
			virtual GRAPHIC::IGraphicEngine* GetGraphicEngine() = 0;
			virtual INPUT::InputInterface* GetInputInterface() = 0;
		};
	}
}