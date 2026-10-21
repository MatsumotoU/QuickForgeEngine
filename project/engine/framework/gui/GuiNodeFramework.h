#pragma once

#include <cstddef>
#include <functional>
#include <vector>

#include <imgui_node_editor.h>

namespace QFE::FRAMEWORK {
	/// @brief ImGui Node Editor を使ったノードキャンバスの共通フレームワーク。
	///
	/// NodeEditorFramework はエディタのライフサイクル、ノード/リンクの描画、
	/// 接続と削除イベントの問い合わせを担当します。アプリケーション固有の
	/// ノード内容や接続ルールは std::function のコールバックで受け取ります。
	class NodeEditorFramework final {
	public:
		using NodeId = ax::NodeEditor::NodeId;
		using PinId = ax::NodeEditor::PinId;
		using LinkId = ax::NodeEditor::LinkId;
		using PinKind = ax::NodeEditor::PinKind;

		struct LinkCreateEvent {
			PinId firstPinId;
			PinId secondPinId;
		};

		using NodeDrawCallback = std::function<void(NodeEditorFramework&)>;

		/// @brief ノードの論理ID、初期位置、ノード内部の描画処理。
		///
		/// positionInitialized が false の場合、position を初期位置として
		/// Node Editor に設定します。描画後の位置は position に反映されます。
		struct Node {
			NodeId id;
			ImVec2 position = ImVec2(0.0f, 0.0f);
			bool positionInitialized = false;
			NodeDrawCallback draw;
		};

		/// @brief 既存リンクの描画情報。
		struct Link {
			LinkId id;
			PinId startPinId;
			PinId endPinId;
			ImVec4 color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
			float thickness = 1.0f;
		};

		/// @brief Node Editorの操作をアプリケーションへ渡すコールバック。
		struct Callbacks {
			/// @brief 新しいリンクを許可するか判定します。未設定なら許可します。
			std::function<bool(PinId, PinId)> canCreateLink;
			/// @brief 新しいリンクが確定した後に呼ばれます。
			std::function<void(PinId, PinId)> onLinkCreated;
			/// @brief ユーザーがリンクを削除した後に呼ばれます。
			std::function<void(LinkId)> onLinkDeleted;
			/// @brief ユーザーがノードを削除した後に呼ばれます。
			std::function<void(NodeId)> onNodeDeleted;
			/// @brief 選択状態が変化したときに呼ばれます。
			/// @param selectedNodeIds 選択中のノードID
			/// @param selectedLinkIds 選択中のリンクID
			std::function<void(const std::vector<NodeId>&,
				const std::vector<LinkId>&)> onSelectionChanged;
		};

		NodeEditorFramework() = default;
		NodeEditorFramework(const NodeEditorFramework&) = delete;
		NodeEditorFramework& operator=(const NodeEditorFramework&) = delete;
		NodeEditorFramework(NodeEditorFramework&&) = delete;
		NodeEditorFramework& operator=(NodeEditorFramework&&) = delete;
		~NodeEditorFramework();

		/// @brief Node Editorのコンテキストを作成します。
		/// @param config Node Editorの設定。SettingsFileなどを指定できます。
		void Initialize(const ax::NodeEditor::Config& config);

		/// @brief 設定ファイルを使わずにNode Editorを初期化します。
		void Initialize();

		/// @brief Node Editorのコンテキストを破棄します。
		void Shutdown();

		bool IsInitialized() const;

		void SetCallbacks(Callbacks callbacks);
		const Callbacks& GetCallbacks() const;

		/// @brief 現在のImGuiウィンドウ内にNode Editorキャンバスを描画します。
		///
		/// 呼び出し側はこの関数を ImGui::Begin/End の間で呼びます。
		/// nodes と links は呼び出し側が所有し、コールバック内で更新します。
		void Draw(const char* editorId, const ImVec2& canvasSize,
			std::vector<Node>& nodes, const std::vector<Link>& links);

		/// @brief NodeDrawCallbackから利用するピン描画API。
		void BeginPin(PinId pinId, PinKind kind);
		void PinRect(const ImVec2& minimum, const ImVec2& maximum);
		void PinPivotRect(const ImVec2& minimum, const ImVec2& maximum);
		void PinPivotSize(const ImVec2& size);
		void PinPivotScale(const ImVec2& scale);
		void PinPivotAlignment(const ImVec2& alignment);
		void EndPin();

		void SetNodePosition(NodeId nodeId, const ImVec2& position);
		ImVec2 GetNodePosition(NodeId nodeId) const;

		ax::NodeEditor::EditorContext* GetEditorContext() const;

	private:
		struct PendingEvents {
			std::vector<LinkCreateEvent> createdLinks;
			std::vector<LinkId> deletedLinks;
			std::vector<NodeId> deletedNodes;
		};

		void DrawNode(Node& node);
		void HandleNewLinks(PendingEvents& pendingEvents);
		void HandleDeletedItems(PendingEvents& pendingEvents);
		void NotifyPendingEvents(const PendingEvents& pendingEvents);
		void NotifySelectionChanged();

		ax::NodeEditor::EditorContext* editorContext_ = nullptr;
		Callbacks callbacks_;
	};
}
