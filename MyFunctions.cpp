#include "pch.h"         // ★必ず一番最初！
#include "framework.h"   // ★ここで MFC (Windows.hやwinsock2など) を正しく読み込ませる

#include <fstream>
#include <string>
#include <vector>
#include <ctime>

// ※ここで #define WIN32_LEAN_AND_MEAN や <windows.h> を直接書かないのがポイントです
#include <Shlobj.h>
#include <sal.h> // _In_, _Out_ 等のアノテーション

#include "OpenCV_without_warning.h"
#include "FaceDetector.h"
#include "MyFunctions.h"
#include "MyConst.h"

// Defining the variables to allocate memory.
// They naturally start as empty strings ("") and will be populated in InitInstance().
std::wstring g_wAppNameShort;
std::string  g_appNameShort;
std::wstring g_wAppNameLong;
std::string  g_appNameLong;

// -----------------------------
// Helpers (String Formatting)
// -----------------------------

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

static std::wstring VFormatW(const wchar_t* fmt, va_list args)
{
    va_list tmp;
    va_copy(tmp, args);
    int len = _vscwprintf(fmt, tmp);
    va_end(tmp);

    if (len < 0) return std::wstring();
    std::vector<wchar_t> buf(static_cast<size_t>(len) + 1);
    vswprintf_s(buf.data(), buf.size(), fmt, args);
    return std::wstring(buf.data());
}

// -----------------------------
// Helpers (Path & File IO)
// -----------------------------

// 日時文字列を取得
static std::wstring GetCurrentDateTimeString(const wchar_t* format)
{
    std::time_t t = std::time(nullptr);
    struct tm tm_info;
    if (localtime_s(&tm_info, &t) == 0)
    {
        wchar_t dateBuf[64];
        wcsftime(dateBuf, sizeof(dateBuf) / sizeof(wchar_t), format, &tm_info);
        return std::wstring(dateBuf);
    }
    return L"";
}

// マイドキュメントのパスを取得
static std::wstring GetDocumentsPath()
{
    wchar_t szPath[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_PERSONAL, NULL, 0, szPath))) {
        return std::wstring(szPath);
    }
    return L"";
}

// アプリケーション用フォルダパスを取得・作成
static std::wstring GetAppFolderPath()
{
    std::wstring docsPath = GetDocumentsPath();
    if (docsPath.empty()) return L"";

    // 自プロセスのEXEファイルパスを取得
    wchar_t szExePath[MAX_PATH];
    GetModuleFileNameW(NULL, szExePath, MAX_PATH);
    std::wstring exeFullPath(szExePath);

    // フォルダパス部分を取り除いてファイル名のみ抽出
    size_t pos = exeFullPath.find_last_of(L"\\/");
    std::wstring exeFileName = (pos == std::wstring::npos) ? exeFullPath : exeFullPath.substr(pos + 1);

    // 拡張子(.exeなど)を取り除く
    size_t extPos = exeFileName.find_last_of(L".");
    if (extPos != std::wstring::npos) {
        exeFileName = exeFileName.substr(0, extPos);
    }

    // マイドキュメント配下にEXE名でフォルダを作成
    std::wstring appPath = docsPath + L"\\" + exeFileName;
    CreateDirectoryW(appPath.c_str(), NULL);
    return appPath;
}

// 評価用サブフォルダパスを取得・作成
static std::wstring GetEvaluationFolderPath(const std::wstring& subFolderName)
{
#ifdef _DEBUG
    std::wstring buildType = L"(Debug)";
#else
    std::wstring buildType = L"(Release)";
#endif

    std::wstring appPath = GetAppFolderPath();
    if (appPath.empty()) return L"";

    std::wstring evalPath = appPath + L"\\" + std::wstring(wEVALUATION_FOLDER_NAME);
    CreateDirectoryW(evalPath.c_str(), NULL);

    std::wstring targetPath = evalPath + L"\\" + subFolderName;
    CreateDirectoryW(targetPath.c_str(), NULL);

    return targetPath;
}

static void GetDetectionAlgorithmInfo(const std::wstring& modeName, std::wstring& outFileName, std::wstring& outAlgoName)
{
#ifdef _DEBUG
    std::wstring buildType = L"(Debug)";
#else
    std::wstring buildType = L"(Release)";
#endif

    // ファイル名は "YuNet.csv" のようにする
    outFileName = modeName + L".csv";

    // アルゴリズム名は "YuNet(Release)" または "Haar Cascades(Release)" のようにする
    if (modeName == L"HaarCascades") {
        outAlgoName = L"Haar Cascades" + buildType; // Haar CascadesだけはCSV内でスペースを入れたい場合
    }
    else {
        outAlgoName = modeName + buildType;
    }
}

// Face Identification 用: ビルド環境とモードからファイル名・アルゴリズム名を決定
static void GetIdentificationAlgorithmInfo(const std::wstring& modeName, std::wstring& outFileName, std::wstring& outAlgoName)
{
#ifdef _DEBUG
    std::wstring buildType = L"(Debug)";
#else
    std::wstring buildType = L"(Release)";
#endif

    // ファイル名は "SFace.csv" のようにする
    outFileName = modeName + L".csv";

    // アルゴリズム名は "SFace(Release)" のようにする
    outAlgoName = modeName + buildType;
}

// ファイルが新規か空か判定
static bool IsFileEmptyOrNew(const std::wstring& filePath)
{
    FILE* checkFp = nullptr;
    if (_wfopen_s(&checkFp, filePath.c_str(), L"rb") == 0 && checkFp != nullptr) {
        fseek(checkFp, 0, SEEK_END);
        long size = ftell(checkFp);
        fclose(checkFp);
        return size == 0;
    }
    return true; // 開けない場合は新規作成として扱う
}

// 評価結果をCSVに追記保存
static void AppendEvaluationDataToCsv(const std::wstring& filePath, const std::wstring& dateStr, const std::wstring& algoName, double avgLatencyMs, double equivalentFps)
{
    bool isNewFile = IsFileEmptyOrNew(filePath);

    FILE* fp = nullptr;
    if (_wfopen_s(&fp, filePath.c_str(), L"ab") == 0 && fp != nullptr) {
        if (isNewFile) {
            // BOM の書き込み
            unsigned char bom[] = { 0xEF, 0xBB, 0xBF };
            fwrite(bom, 1, sizeof(bom), fp);
            // ヘッダー行
            std::string header = "DateTime (YYYY/MM/DD HH:mm:ss),Algorithm,Average Time per Frame (ms),Equivalent FPS (FPS)\n";
            fwrite(header.c_str(), 1, header.length(), fp);
        }

        wchar_t lineBuf[512];
        swprintf(lineBuf, sizeof(lineBuf) / sizeof(wchar_t), L"%ls,%ls,%.1f,%.1f\n", dateStr.c_str(), algoName.c_str(), avgLatencyMs, equivalentFps);

        std::string utf8Line = ToString(lineBuf);
        fwrite(utf8Line.c_str(), 1, utf8Line.length(), fp);

        fclose(fp);
    }
}

// -----------------------------
// Public functions
// -----------------------------

void MyMessageBoxW(HWND parent, UINT type, const std::wstring& title, const wchar_t* format, ...)
{
    va_list args;
    va_start(args, format);
    std::wstring msg = VFormatW(format, args);
    va_end(args);

    MessageBoxW(parent, msg.c_str(), title.c_str(), type);
}

void MyMessageBoxA(HWND parent, UINT type, const std::string& titleA, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    std::string msgA = VFormatA(format, args);
    va_end(args);

    std::wstring msgW = ToWString(msgA);
    std::wstring titleW = ToWString(titleA);

    MessageBoxW(parent, msgW.c_str(), titleW.c_str(), type);
}

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
    if (delta <= 0) {
        prev_tick_ = current;
        return last_fps_ = 0.0;
    }
    double freq = cv::getTickFrequency();
    double fps = freq / static_cast<double>(delta);
    prev_tick_ = current;
    last_fps_ = fps;
    return fps;
}

double FpsCounter::get() const noexcept
{
    return last_fps_;
}

// -----------------------------
// String Conversion
// -----------------------------

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

// -----------------------------
// Business Logic Functions
// -----------------------------

std::wstring GetAttendanceCsvPath()
{
    std::wstring dateStr = GetCurrentDateTimeString(L"%Y%m%d");
    std::wstring appPath = GetAppFolderPath();

    if (!appPath.empty() && !dateStr.empty()) {
        return appPath + L"\\" + dateStr + L".csv";
    }

    return L"attendance" + dateStr + L".csv";
}

void SaveEvaluationLatencyCsv(const std::wstring& categoryFolder, const std::wstring& modeName, double val1, double val2, long long sampleCount)
{
    // 指定されたカテゴリ名で Evaluation フォルダ直下にフォルダを取得/作成
    std::wstring folderPath = GetEvaluationFolderPath(categoryFolder);
    if (folderPath.empty()) return;

    std::wstring algoFileName;
    std::wstring outAlgoNameStr;

    // カテゴリごとにアルゴリズム名、ファイル名、ヘッダ行を自動設定
    // ★ 右端に ", Samples" を追加
    std::string headerLine = "DateTime(YYYY/MM/DD HH:mm:ss), Algorithm, Val1, Val2, Samples\n";

    // ★ 定数定数と比較するように変更
    if (categoryFolder == wFACE_DETECTION_LATENCY_FOLDER_NAME) {
        GetDetectionAlgorithmInfo(modeName, algoFileName, outAlgoNameStr);
        headerLine = "DateTime(YYYY/MM/DD HH:mm:ss), Algorithm, Average Latency (ms), Equivalent FPS, Samples\n";
    }
    else if (categoryFolder == wFACE_IDENTIFICATION_LATENCY_FOLDER_NAME) {
        GetIdentificationAlgorithmInfo(modeName, algoFileName, outAlgoNameStr);
        headerLine = "DateTime(YYYY/MM/DD HH:mm:ss), Algorithm, Average Extraction Time (ms), Average Matching Time (ms), Samples\n";
    }
    else {
        return; // 知らないカテゴリなら何もしない
    }

    // std::wstring を CSV出力用に std::string (ASCII想定) に変換
        // std::wstring を CSV出力用に std::string (ASCII想定) に変換 (キャストを明示して警告を消す)
    std::string algoName(outAlgoNameStr.length(), ' ');
    for (size_t i = 0; i < outAlgoNameStr.length(); ++i) {
        algoName[i] = static_cast<char>(outAlgoNameStr[i]);
    }

    std::wstring filePath = folderPath + L"\\" + algoFileName;

    // ファイルが存在し、かつ空でないかをチェック
    bool needsHeader = false;
    {
        std::ifstream checkFile(filePath);
        if (!checkFile.is_open()) {
            needsHeader = true;
        }
        else {
            checkFile.seekg(0, std::ios::end);
            if (checkFile.tellg() == 0) needsHeader = true;
        }
    }

    // 現在の日時を取得
    char dateBuf[64] = { 0 };
    std::time_t t = std::time(nullptr);
    struct tm tm_info;
    if (localtime_s(&tm_info, &t) == 0) {
        strftime(dateBuf, sizeof(dateBuf), "%Y/%m/%d %H:%M:%S", &tm_info);
    }
    std::string dateStr(dateBuf);

    // CSVへの追記書き込み
    std::ofstream file(filePath, std::ios::app);
    if (file.is_open()) {
        if (needsHeader) {
            file << headerLine;
        }
        // ★ 最後に ", sampleCount" を追加
        file << dateStr << "," << algoName << "," << val1 << "," << val2 << "," << sampleCount << "\n";
    }
}

void OpenEvaluationFolder(const std::wstring& subFolderName)
{
    std::wstring targetPath = GetEvaluationFolderPath(subFolderName);
    if (!targetPath.empty()) {
        ShellExecuteW(NULL, L"open", targetPath.c_str(), NULL, NULL, SW_SHOWDEFAULT);
    }
}
