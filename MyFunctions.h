// MyFunctions.h
#pragma once

#include <string>
#include <windows.h>

#pragma region String_Conversion_Utilities
std::wstring ToWString(const std::string& str);	// std::string ("abc") -> std::wstring (L"abc")
std::string ToString(const std::wstring& wstr);	// std::wstring (L"abc") -> std::string ("abc")
#pragma endregion

#pragma region Application_Constants
// exeファイル名から取得した名前を保持するグローバル変数
extern std::wstring g_wAppNameShort;
extern std::string  g_appNameShort;
extern std::wstring g_wAppNameLong;  // 必要に応じてLong版も
extern std::string  g_appNameLong;
#pragma endregion


/// ワイド文字列フォーマットを受け取る MessageBox ラッパー
/// format はワイド文字列（例: L"File: %ls, code=%d"）を想定
void MyMessageBoxW(HWND parent, UINT type, const std::wstring& title, const wchar_t* format, ...);

/// 狭い文字列（std::string / UTF-8 想定）を受け取り内部で整形して MessageBoxW を呼ぶ
/// title は narrow (std::string)、format は printf 形式の narrow フォーマット（const char*）
void MyMessageBoxA(HWND parent, UINT type, const std::string& titleA, const char* format, ...);

/// 単純メッセージ表示用（UTF-8 の std::string をそのまま表示）
void ShowErrorA(const std::string& messageUtf8);

// FpsCounter: encapsulates tick / fps calculation using OpenCV tick counters.
// Usage:
//   FpsCounter fps;         // captures initial tick
//   double fpsValue = fps.tick(); // compute fps since last tick and update internal tick
//   double last = fps.get(); // get last computed fps
class FpsCounter
{
public:
	FpsCounter() noexcept;
	// reset the internal timer to now
	void reset() noexcept;
	// compute FPS based on elapsed ticks since last call, update internal tick, and return fps
	double tick() noexcept;
	// get last computed FPS (0.0 if none)
	double get() const noexcept;

private:
	int64_t prev_tick_;
	double last_fps_;
};

// マイドキュメント内に保存用のディレクトリを作成し、
// その日の日付に基づいたCSVファイルのフルパス (例: C:\Users\name\Documents\FacialAttendance2026\20260425.csv) を返します。
std::wstring GetAttendanceCsvPath();
std::wstring GetEvaluationFolderPath(const std::wstring& categoryFolder);

// ★ 第5引数 long long sampleCount を追加
void SaveEvaluationLatencyCsv(const std::wstring& categoryFolder, const std::wstring& modeName, double val1, double val2, long long sampleCount);

void OpenEvaluationFolder(const std::wstring& categoryFolder);
