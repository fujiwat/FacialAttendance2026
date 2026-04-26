// 標準ライブラリ
#include "pch.h"
#include <string>
#include <vector>
#include <ctime>

// Windows 固有の設定（windows.h の前に置く）
#define WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <windows.h>
#include <sal.h> // _In_, _Out_ 等のアノテーション
#include <Shlobj.h>

#include "OpenCV_without_warning.h"
#include "FaceDetector.h"
#include "MyFunctions.h"
#include "MyConst.h"

// -----------------------------
// Helpers
// -----------------------------

// narrow formatted string (vsnprintf)
static std::string VFormatA(const char* fmt, va_list args)
{
    va_list tmp;
    va_copy(tmp, args);
    int len = vsnprintf(nullptr, 0, fmt, tmp);
    va_end(tmp);
    if (len < 0) return std::string();
    std::vector<char> buf(static_cast<size_t>(len) + 1);
    vsnprintf(buf.data(), buf.size(), fmt, args);
    return std::string(buf.data());
}

// wide formatted string (vswprintf_s)
static std::wstring VFormatW(const wchar_t* fmt, va_list args)
{
    va_list tmp;
    va_copy(tmp, args);
    int len = _vscwprintf(fmt, tmp); // required length (without terminating NUL)
    va_end(tmp);

    if (len < 0) return std::wstring();
    std::vector<wchar_t> buf(static_cast<size_t>(len) + 1);
    vswprintf_s(buf.data(), buf.size(), fmt, args);
    return std::wstring(buf.data());
}

// -----------------------------
// Public functions
// -----------------------------

// MyMessageBoxW: wide-format, wide-args
void MyMessageBoxW(HWND parent, UINT type, const std::wstring& title, const wchar_t* format, ...)
{
    va_list args;
    va_start(args, format);
    std::wstring msg = VFormatW(format, args);
    va_end(args);

    MessageBoxW(parent, msg.c_str(), title.c_str(), type);
}

// MyMessageBoxA: narrow title + narrow printf-style format + variadic args
// Internally formats to a narrow string, converts to UTF-16 (CP_UTF8 by default) and calls MessageBoxW.
void MyMessageBoxA(HWND parent, UINT type, const std::string& titleA, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    std::string msgA = VFormatA(format, args);
    va_end(args);

    // Convert formatted narrow string and title to wide (UTF-8 assumed).
    std::wstring msgW = ToWString(msgA);
    std::wstring titleW = ToWString(titleA);

    MessageBoxW(parent, msgW.c_str(), titleW.c_str(), type);
}

// ShowErrorA: simple single-message helper (UTF-8 std::string)

void ShowErrorA(const std::string& messageUtf8)
{
    std::wstring wmsg = ToWString(messageUtf8);
	std::wstring wtitle = ToWString(APP_NAME_LONG);
    MessageBoxW(NULL, wmsg.c_str(), wtitle.c_str(), MB_OK | MB_ICONERROR);
}

// -----------------------------
// FpsCounter implementation
// -----------------------------

FpsCounter::FpsCounter() noexcept
    : prev_tick_(static_cast<int64_t>(cv::getTickCount()))
    , last_fps_(0.0)
{
}

void FpsCounter::reset() noexcept
{
    prev_tick_ = static_cast<int64_t>(cv::getTickCount());
    last_fps_ = 0.0;
}

double FpsCounter::tick() noexcept
{
    int64_t current = static_cast<int64_t>(cv::getTickCount());
    int64_t delta = current - prev_tick_;
    // protect against zero / negative delta (shouldn't normally happen)
    if (delta <= 0) {
        // update prev_tick_ anyway to avoid repeated zero delta
        prev_tick_ = current;
        return last_fps_ = 0.0;
    }
    double freq = cv::getTickFrequency(); // ticks per second
    double fps = freq / static_cast<double>(delta);
    prev_tick_ = current;
    last_fps_ = fps;
    return fps;
}

double FpsCounter::get() const noexcept
{
    return last_fps_;
}

std::wstring ToWString(const std::string& str) {
    if (str.empty()) return L"";
    int size = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
    std::wstring result(size, 0);
    MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &result[0], size);
    return result;
}

std::string ToString(const std::wstring& wstr) {
    if (wstr.empty()) return "";
    
    int size = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
    
    std::string result(size, 0);
    WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &result[0], size, NULL, NULL);
    return result;
}

std::wstring GetAttendanceCsvPath()
{
    // 1. まず日付文字列 (YYYYMMDD) を取得する
    std::wstring dateStr = L"";
    std::time_t t = std::time(nullptr);
    struct tm tm_info;
    if (localtime_s(&tm_info, &t) == 0)
    {
        wchar_t dateBuf[32];
        wcsftime(dateBuf, sizeof(dateBuf) / sizeof(wchar_t), L"%Y%m%d", &tm_info);
        dateStr = dateBuf;
    }
    // (万が一時計が取得できない等の超例外は、そのまま空文字にしておく)

    // 2. マイドキュメントのパスを取得し、フルパスを組み立てる
    wchar_t szPath[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_PERSONAL, NULL, 0, szPath)))
    {
        std::wstring folderPath = szPath;
        // ★ 直書きをやめて、MyConst.h の定数を使う
        folderPath += L"\\";
        folderPath += wAPP_NAME_SHORT;
        
        // フォルダが存在しないなら作成する
        CreateDirectoryW(folderPath.c_str(), NULL);
        
        return folderPath + L"\\" + dateStr + L".csv";
    }

    // 3. マイドキュメントのパス取得に失敗した場合のフォールバック
    // カレントディレクトリに "attendanceYYYYMMDD.csv" として保存する
    return L"attendance" + dateStr + L".csv";
}

// ★追加: 評価CSVの保存関数
void SaveEvaluationLatencyCsv(int mode, double avgLatencyMs, double equivalentFps)
{
    // マイドキュメントのパスを取得
    wchar_t szPath[MAX_PATH];
    if (FAILED(SHGetFolderPathW(NULL, CSIDL_PERSONAL, NULL, 0, szPath))) {
        return;
    }

    std::wstring basePath = szPath;
    basePath += L"\\";
    basePath += wAPP_NAME_SHORT;
    CreateDirectoryW(basePath.c_str(), NULL);
    
    basePath += L"\\";
    basePath += wEVALUATION_FOLDER_NAME;
    CreateDirectoryW(basePath.c_str(), NULL);

    basePath += L"\\";
    basePath += wFACE_DETECTION_LATENCY_FOLDER_NAME;
    CreateDirectoryW(basePath.c_str(), NULL);

    // ★ここから修正: モードとビルド環境からのファイル名・アルゴリズム名決定
    std::wstring fileName;
    std::wstring algoName;

    // --- ビルド構成を示す文字列を追加 ---
#ifdef _DEBUG
    std::wstring buildType = L"(Debug)";
#else
    std::wstring buildType = L"(Release)";
#endif

    if (mode == 0) {
        fileName = L"HaarCascades.csv";
        algoName = L"Haar Cascades" + buildType; // Haar Cascades(Debug) 等になる
    }
    else if (mode == 1) {
        fileName = L"YuNet.csv";
        algoName = L"YuNet" + buildType;         // YuNet(Release) 等になる
    }
    else {
        fileName = L"Both.csv";
        algoName = L"Both" + buildType;          // Both(Debug) 等になる
    }

    std::wstring fullPath = basePath + L"\\" + fileName;

    // 日付と時刻の取得 (YYYY/MM/DD HH:mm:ss)
    std::time_t t = std::time(nullptr);
    struct tm tm_info;
    wchar_t dateBuf[64] = L""; // 少し大きめに確保
    if (localtime_s(&tm_info, &t) == 0) {
        // ★修正: 時刻 (HH:M:SS) のフォーマットを追加
        wcsftime(dateBuf, sizeof(dateBuf) / sizeof(wchar_t), L"%Y/%m/%d %H:%M:%S", &tm_info);
    }

    // ファイル新規作成かどうかのチェック
    bool isNewFile = true;
    FILE* checkFp = nullptr;
    if (_wfopen_s(&checkFp, fullPath.c_str(), L"rb") == 0 && checkFp != nullptr) {
        fseek(checkFp, 0, SEEK_END);
        if (ftell(checkFp) > 0) isNewFile = false;
        fclose(checkFp);
    }

    // ★書き込み（バイナリモード "ab" で確実にUTF-8を出力する）
    FILE* fp = nullptr;
    if (_wfopen_s(&fp, fullPath.c_str(), L"ab") == 0 && fp != nullptr) {
        if (isNewFile) {
            // BOM (EF BB BF) の書き込み
            unsigned char bom[] = { 0xEF, 0xBB, 0xBF };
            fwrite(bom, 1, sizeof(bom), fp);
            
            // ヘッダー行
            std::string header = "DateTime (YYYY/MM/DD HH:mm:ss),Algorithm,Average Time per Frame (ms),Equivalent FPS (FPS)\n";
            fwrite(header.c_str(), 1, header.length(), fp);
        }

        // データ行（一度ワイド文字でフォーマットしてからToStringでUTF-8に変換）
        wchar_t lineBuf[512];
        swprintf(lineBuf, sizeof(lineBuf)/sizeof(wchar_t), L"%ls,%ls,%.1f,%.1f\n", dateBuf, algoName.c_str(), avgLatencyMs, equivalentFps);
        
        std::string utf8Line = ToString(lineBuf); // MyFunctions.cpp上部に定義済みのUTF8変換関数
        fwrite(utf8Line.c_str(), 1, utf8Line.length(), fp);
        
        fclose(fp);
    }
}

// ★変更: 引数を受け取る
void OpenEvaluationFolder(const std::wstring& subFolderName)
{
    wchar_t szPath[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_PERSONAL, NULL, 0, szPath))) {
        std::wstring basePath = szPath;
        
        // 1. 直下のアプリフォルダを作成
        basePath += L"\\";
        basePath += wAPP_NAME_SHORT;
        CreateDirectoryW(basePath.c_str(), NULL);
        
        // 2. Evaluation フォルダを作成
        basePath += L"\\";
        basePath += wEVALUATION_FOLDER_NAME;
        CreateDirectoryW(basePath.c_str(), NULL);

        // 3. 引数で渡されたサブフォルダ (FaceDetectionLatency 等) を作成
        basePath += L"\\";
        basePath += subFolderName; // ★ ここで引数を使う
        CreateDirectoryW(basePath.c_str(), NULL);

        // エクスプローラでフォルダを開く
        ShellExecuteW(NULL, L"open", basePath.c_str(), NULL, NULL, SW_SHOWDEFAULT);
    }
}