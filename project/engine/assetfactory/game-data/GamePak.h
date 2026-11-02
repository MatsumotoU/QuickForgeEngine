#pragma once
#include <array>
#include <cstdint>
#include <filesystem>
#include <iosfwd>
#include <string>

namespace QFE::PAK {
    /// @brief qfepak ファイルの先頭に置く識別子。
    inline constexpr std::array<char, 8> Magic{{ 'Q','F','E','P','A','K','\r','\n' }};
    /// @brief この実装が読み書きするパッケージ形式のバージョン。
	inline constexpr std::uint32_t Version = 2;
    /// @brief 一つのパッケージに格納できるファイルの最大数。
    inline constexpr std::uint32_t MaxEntries = 100000;
    /// @brief パッケージ内の UTF-8 パスの最大バイト数。
    inline constexpr std::uint32_t MaxPathBytes = 4096;

    /// @brief プロジェクトの設定、アセット登録情報、assetsとsourcesをパッケージに書き出す。
    /// @param project 編集用プロジェクトのルート。
    /// @param output 出力する .qfepak ファイルのパス。
    /// @param errorText 失敗時に理由を受け取る文字列。
    /// @param previewScene デバッグ実行時に使用する開始シーン。プロジェクト設定は変更しない。
    /// @return 出力に成功した場合は true。
    bool Write(const std::filesystem::path& project, const std::filesystem::path& output,
               std::string& errorText, const std::string& previewScene = {});
    /// @brief パッケージを呼び出し元が作成した空のディレクトリに展開する。
    /// @param pak 読み込む .qfepak ファイルのパス。
    /// @param directory 展開先。呼び出し元がディレクトリの削除を管理する。
    /// @param errorText 失敗時に理由を受け取る文字列。
    /// @return 展開に成功した場合は true。
    bool Extract(const std::filesystem::path& pak, const std::filesystem::path& directory,
                 std::string& errorText);

    /// @brief 整数の下位 bytes バイトをリトルエンディアンで書き込む。
    void WriteNumber(std::ostream& out, std::uint64_t value, unsigned bytes);
    /// @brief パッケージ内の相対パスとして使用できる名前か判定する。
    bool ValidName(const std::string& name);
}
