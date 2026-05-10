#pragma once

#include <string>
#include <windows.h>

#pragma region String_Conversion_Utilities

/**
 * @brief Converts a standard narrow string to a wide string.
 * @param str The standard narrow string to convert.
 * @return The converted wide string.
 */
std::wstring ToWString(const std::string& str);

/**
 * @brief Converts a wide string to a standard narrow string.
 * @param wstr The wide string to convert.
 * @return The converted standard narrow string.
 */
std::string ToString(const std::wstring& wstr);

/**
 * @brief Converts a typical string into Title Case.
 * @param str The string to apply the Title Case transformation to.
 */
void ConvertToTitleCase(CString& str);

#pragma endregion

#pragma region Application_Constants

extern std::wstring g_wAppNameShort;
extern std::string  g_appNameShort;
extern std::wstring g_wAppNameLong;
extern std::string  g_appNameLong;

#pragma endregion

/**
 * @brief A message box wrapper that accepts wide string formatting.
 * @param parent Handle to the owner window.
 * @param type The contents and behavior of the dialog box.
 * @param title The dialog box title.
 * @param format The formatting wide string.
 * @param ... Additional arguments for formatting.
 */
void MyMessageBoxW(HWND parent, UINT type, const std::wstring& title, const wchar_t* format, ...);

/**
 * @brief A message box wrapper that accepts narrow string (UTF-8) formatting.
 * @param parent Handle to the owner window.
 * @param type The contents and behavior of the dialog box.
 * @param titleA The dialog box title in a narrow string format.
 * @param format The formatting narrow string.
 * @param ... Additional arguments for formatting.
 */
void MyMessageBoxA(HWND parent, UINT type, const std::string& titleA, const char* format, ...);

/**
 * @brief Displays an error message inside a message box using a UTF-8 string.
 * @param messageUtf8 The error message to display.
 */
void ShowErrorA(const std::string& messageUtf8);

/**
 * @brief Encapsulates tick and FPS calculation using OpenCV tick tools.
 */
class FpsCounter
{
public:
    /**
     * @brief Constructor that initializes the FPS counter.
     */
    FpsCounter() noexcept;

    /**
     * @brief Resets the internal timer to the current tick.
     */
    void reset() noexcept;

    /**
     * @brief Computes FPS based on elapsed ticks since the last call.
     * @return The newly computed frames per second value.
     */
    double tick() noexcept;

    /**
     * @brief Retrieves the last computed FPS value.
     * @return The last computed FPS, or 0.0 if not yet computed.
     */
    double get() const noexcept;

private:
    int64_t prev_tick_;
    double last_fps_;
};

/**
 * @brief Generates and returns the full path for the attendance CSV file based on today's date.
 * Creates the required directory structure inside the My Documents folder if it does not exist.
 * @return The full path for the CSV file.
 */
std::wstring GetAttendanceCsvPath();

/**
 * @brief Retrieves the folder path for the specified evaluation category.
 * @param categoryFolder The name of the category folder.
 * @return The absolute path to the evaluation folder.
 */
std::wstring GetEvaluationFolderPath(const std::wstring& categoryFolder);

/**
 * @brief Saves evaluation latency data to a CSV file.
 * @param categoryFolder The specific category folder.
 * @param modeName The operation mode name.
 * @param val1 The first evaluation value.
 * @param val2 The second evaluation value.
 * @param sampleCount The total number of evaluation samples.
 */
void SaveEvaluationLatencyCsv(const std::wstring& categoryFolder, const std::wstring& modeName, double val1, double val2, long long sampleCount);

/**
 * @brief Opens the folder designated for the specified evaluation category.
 * @param categoryFolder The name of the category folder to open.
 */
void OpenEvaluationFolder(const std::wstring& categoryFolder);