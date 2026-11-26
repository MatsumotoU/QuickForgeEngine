#include "framework/application/WindowsQuickForgeEngineSystems.h"
// 機能をunique_ptrをつかって管理するために必要なヘッダーファイル
#include "window/interface/IGameWindowManager.h"
#include "graphics/interface/IGraphicEngine.h"
#include "gui/interface/IGuiManager.h"
#include "input/interface/InputInterface.h"
#include "scene/SceneManager.h"
#include "audio/AudioEngine.h"

QFE::FRAMEWORK::WindowsQuickForgeEngineSystems::WindowsQuickForgeEngineSystems() = default;
QFE::FRAMEWORK::WindowsQuickForgeEngineSystems::~WindowsQuickForgeEngineSystems() = default;
