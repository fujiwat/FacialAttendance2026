#include "pch.h"         
#include "framework.h"   

#include <fstream>
#include <string>
#include <vector>
#include <ctime>

#include <Shlobj.h>
#include <sal.h> 

#include "OpenCV_without_warning.h"
#include "FaceDetector.h"
#include "MyFunctions.h"
#include "MyConst.h"
#include "CMyMsgBox.h"

std::wstring g_wAppNameShort;
std::string  g_appNameShort;
std::wstring g_wAppNameLong;
std::string  g_appNameLong;

/**
 * @brief Formats a narrow string using a variable argument list.
 * @param fmt The formatting string.
 * @param args The variable argument list.
 * @return The formatted narrow string.
 */
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

/**
 * @brief Formats a wide string using a variable argument list.
 * @param fmt The formatting wide string.
 * @param args The variable argument list.
 * @return The formatted wide string.
 */
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

void ConvertToTitleCase(CString& str)
{
    if (str.IsEmpty()) return;

    str.MakeLower();

    bool bNewWord = true;
    for (int i = 0; i < str.GetLength(); i++)
    {
        if (iswspace(str[i]))
        {
            bNewWord = true;
        }
        else if (bNewWord)
        {
            str.SetAt(i, (wchar_t)towupper(str[i]));
            bNewWord = false;
        }
    }
}

/**
 * @brief Retrieves the current date and time as a formatted wide string.
 * @param format The format descriptor string.
 * @return The formatted date and time string.
 */
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

/**
 * @brief Retrieves the path to the user's Documents folder.
 * @return The Documents folder path.
 */
static std::wstring GetDocumentsPath()
{
    wchar_t szPath[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_PERSONAL, NULL, 0, szPath))) {
        return std::wstring(szPath);
    }
    return L"";
}

/**
 * @brief Retrieves or creates the application's specific folder path.
 * @return The application folder path.
 */
std::wstring GetAppFolderPath()
{
    std::wstring docsPath = GetDocumentsPath();
    if (docsPath.empty()) return L"";

    wchar_t szExePath[MAX_PATH];
    GetModuleFileNameW(NULL, szExePath, MAX_PATH);
    std::wstring exeFullPath(szExePath);

    size_t pos = exeFullPath.find_last_of(L"\\/");
    std::wstring exeFileName = (pos == std::wstring::npos) ? exeFullPath : exeFullPath.substr(pos + 1);

    size_t extPos = exeFileName.find_last_of(L".");
    if (extPos != std::wstring::npos) {
        exeFileName = exeFileName.substr(0, extPos);
    }

    std::wstring appPath = docsPath + L"\\" + exeFileName;
    CreateDirectoryW(appPath.c_str(), NULL);
    return appPath;
}

/**
 * @brief Retrieves or creates the path for a specific evaluation subfolder.
 * @param subFolderName The name of the evaluation subfolder.
 * @return The full path to the required evaluation folder.
 */
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

/**
 * @brief Evaluates configuration conditions to retrieve algorithm detection info.
 * @param modeName The chosen mode name.
 * @param outFileName Holds the output file name string.
 * @param outAlgoName Holds the resulting algorithm name string.
 */
static void GetDetectionAlgorithmInfo(const std::wstring& modeName, std::wstring& outFileName, std::wstring& outAlgoName)
{
#ifdef _DEBUG
    std::wstring buildType = L"(Debug)";
#else
    std::wstring buildType = L"(Release)";
#endif

    outFileName = modeName + L".csv";

    if (modeName == L"HaarCascades") {
        outAlgoName = L"Haar Cascades" + buildType;
    }
    else {
        outAlgoName = modeName + buildType;
    }
}

/**
 * @brief Evaluates configuration conditions to retrieve algorithm identification info.
 * @param modeName The chosen mode name.
 * @param outFileName Holds the output file name string.
 * @param outAlgoName Holds the resulting algorithm name string.
 */
static void GetIdentificationAlgorithmInfo(const std::wstring& modeName, std::wstring& outFileName, std::wstring& outAlgoName)
{
#ifdef _DEBUG
    std::wstring buildType = L"(Debug)";
#else
    std::wstring buildType = L"(Release)";
#endif

    outFileName = modeName + L".csv";
    outAlgoName = modeName + buildType;
}

/**
 * @brief Determines whether a given file path points to an empty or non-existent file.
 * @param filePath Target file path.
 * @return True if the file is missing or empty, false otherwise.
 */
static bool IsFileEmptyOrNew(const std::wstring& filePath)
{
    FILE* checkFp = nullptr;
    if (_wfopen_s(&checkFp, filePath.c_str(), L"rb") == 0 && checkFp != nullptr) {
        fseek(checkFp, 0, SEEK_END);
        long size = ftell(checkFp);
        fclose(checkFp);
        return size == 0;
    }
    return true;
}

/**
 * @brief Appends evaluated metrics into the specified CSV file.
 * @param filePath The destination CSV file path.
 * @param dateStr Date formatted string.
 * @param algoName Evaluated algorithm's name.
 * @param avgLatencyMs Registered averaged latency.
 * @param equivalentFps Equivalent frames per second value.
 */
static void AppendEvaluationDataToCsv(const std::wstring& filePath, const std::wstring& dateStr, const std::wstring& algoName, double avgLatencyMs, double equivalentFps)
{
    bool isNewFile = IsFileEmptyOrNew(filePath);

    FILE* fp = nullptr;
    if (_wfopen_s(&fp, filePath.c_str(), L"ab") == 0 && fp != nullptr) {
        if (isNewFile) {
            unsigned char bom[] = { 0xEF, 0xBB, 0xBF };
            fwrite(bom, 1, sizeof(bom), fp);
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

int MyMessageBoxW(HWND parent, UINT type, const std::wstring& title, const wchar_t* format, ...)
{
    va_list args;
    va_start(args, format);
    std::wstring msg = VFormatW(format, args);
    va_end(args);

    CMyMsgBox dlg(CWnd::FromHandle(parent));

    dlg.m_strMessage = msg.c_str();
    dlg.m_strTitle = title.c_str();
    dlg.m_nType = type; 

    return (int)dlg.DoModal();
}

int MyMessageBoxA(HWND parent, UINT type, const std::string& titleA, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    std::string msgA = VFormatA(format, args);
    va_end(args);

    std::wstring msgW = ToWString(msgA);
    std::wstring titleW = ToWString(titleA);

    CMyMsgBox dlg(CWnd::FromHandle(parent));
    dlg.m_strMessage = msgW.c_str();
    dlg.m_strTitle = titleW.c_str();

    return (int)dlg.DoModal();
}

void ShowErrorA(const std::string& messageUtf8)
{
    std::wstring wmsg = ToWString(messageUtf8);
    std::wstring wtitle = ToWString(APP_NAME_LONG);

    MyMessageBoxW(NULL, MB_OK | MB_ICONERROR, wtitle, L"%s", wmsg.c_str());
}

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

//std::wstring ToWString(const std::string& str) {
//    if (str.empty()) return L"";
//    int size = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
//    std::wstring result(size, 0);
//    MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &result[0], size);
//    return result;
//}
//
//std::string ToString(const std::wstring& wstr) {
//    if (wstr.empty()) return "";
//    int size = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0);
//    std::string result(size, 0);
//    WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &result[0], size, NULL, NULL);
//    return result;
//}

std::wstring ToWString(const std::string& str)
{
    if (str.empty()) return L"";
    // Pass -1 to include the null terminator and get required buffer size
    int size = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);
    if (size == 0) return L"";
    std::wstring result(static_cast<size_t>(size), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, &result[0], size);
    // Remove the trailing L'\0' null terminator
    if (!result.empty() && result.back() == L'\0') result.pop_back();
    return result;
}

std::string ToString(const std::wstring& wstr)
{
    if (wstr.empty()) return std::string();
    // Pass -1 to include the null terminator and get required buffer size
    int size = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, NULL, 0, NULL, NULL);
    if (size == 0) return std::string();
    std::string result(static_cast<size_t>(size), '\0');
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &result[0], size, NULL, NULL);
    // Remove the trailing '\0' null terminator
    if (!result.empty() && result.back() == '\0') result.pop_back();
    return result;
}


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
    std::wstring folderPath = GetEvaluationFolderPath(categoryFolder);
    if (folderPath.empty()) return;

    std::wstring algoFileName;
    std::wstring outAlgoNameStr;

    std::string headerLine = "DateTime(YYYY/MM/DD HH:mm:ss), Algorithm, Val1, Val2, Samples\n";

    if (categoryFolder == wFACE_DETECTION_LATENCY_FOLDER_NAME) {
        GetDetectionAlgorithmInfo(modeName, algoFileName, outAlgoNameStr);
        headerLine = "DateTime(YYYY/MM/DD HH:mm:ss), Algorithm, Average Latency (ms), Equivalent FPS, Samples\n";
    }
    else if (categoryFolder == wFACE_IDENTIFICATION_LATENCY_FOLDER_NAME) {
        GetIdentificationAlgorithmInfo(modeName, algoFileName, outAlgoNameStr);
        headerLine = "DateTime(YYYY/MM/DD HH:mm:ss), Algorithm, Average Extraction Time (ms), Average Matching Time (ms), Samples\n";
    }
    else {
        return;
    }

    std::string algoName(outAlgoNameStr.length(), ' ');
    for (size_t i = 0; i < outAlgoNameStr.length(); ++i) {
        algoName[i] = static_cast<char>(outAlgoNameStr[i]);
    }

    std::wstring filePath = folderPath + L"\\" + algoFileName;

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

    char dateBuf[64] = { 0 };
    std::time_t t = std::time(nullptr);
    struct tm tm_info;
    if (localtime_s(&tm_info, &t) == 0) {
        strftime(dateBuf, sizeof(dateBuf), "%Y/%m/%d %H:%M:%S", &tm_info);
    }
    std::string dateStr(dateBuf);

    std::ofstream file(filePath, std::ios::app);
    if (file.is_open()) {
        if (needsHeader) {
            file << headerLine;
        }
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
