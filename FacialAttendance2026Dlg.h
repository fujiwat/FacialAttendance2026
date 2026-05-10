#pragma once
#include "FaceDetector.h"
#include "FaceIdentifier.h" 
#include "AdaptiveScreenLight.h"
#include <atomic>
#include <mutex>
#include <thread>
#include <afxcontrolbars.h> 
#include <deque>   
#include <condition_variable>                

#define WM_UPDATE_SCREEN_LIGHT (WM_APP + 1)

/**
 * @brief Describes the active mode of the screen adaptive light.
 */
enum class ScreenLightMode
{
    None = 0,    /**< Screen light is completely turned off. */
    Slider = 1,  /**< Screen light brightness is manually controlled via a slider. */
    Auto = 2,    /**< Screen light dynamically adjusts based on ambient measurements. */
};

/**
 * @brief Holds image arrays and evaluation metrics for a single verified face occurrence.
 */
struct FaceBufferItem {
    cv::Mat face112;             /**< The 112x112 normalized and cropped facial image. */
    cv::Mat rawFrame;            /**< The original frame bounding the face area securely. */
    std::vector<float> faceData; /**< Detected 15-point landmark and bounding box data. */

    double sharpness = 0.0;      /**< Evaluated clarity parameter validating focus. */
    double confidence = 0.0;     /**< Deep neural network confidence metric. */
    double contrast = 0.0;       /**< Image contrast level evaluating tonal spectrum limits. */
    double totalScore = 0.0;     /**< Aggregate heuristic ranking metric isolating optimal profiles. */

    ULONGLONG timestamp = 0;     /**< System tick count highlighting exact timeline persistence. */
};

/**
 * @brief The main graphical user interface dialog mapping visual controls to underlying algorithms.
 */
class CFacialAttendance2026Dlg : public CDialogEx
{
public:
    /**
     * @brief Constructs the main dialog object and initializes window pointers.
     * @param pParent Pointer to the parent window, if any. Defaults to nullptr.
     */
    CFacialAttendance2026Dlg(CWnd* pParent = nullptr);

#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_FACIALATTENDANCE2026_DIALOG };
#endif

protected:
    /**
     * @brief Links dialog controls with actual member variables via DDX mapping.
     * @param pDX Pointer to the data exchange validation context structure.
     */
    virtual void DoDataExchange(CDataExchange* pDX);

protected:
    HICON m_hIcon;

    /**
     * @brief Overrides initial dialog creation establishing baseline GUI parameters.
     * @return TRUE if initialized accurately.
     */
    virtual BOOL OnInitDialog();

    /**
     * @brief Clears resources safely avoiding memory leaks.
     */
    virtual void OnCancel();

    /**
     * @brief Intercepts command instructions addressing standard window title bar actions.
     * @param nID Command identifier natively routed.
     * @param lParam Extra contextual argument references.
     */
    afx_msg void OnSysCommand(UINT nID, LPARAM lParam);

    /**
     * @brief Forces standard visual refreshing drawing overlaps natively.
     */
    afx_msg void OnPaint();

    /**
     * @brief Formats dragging operations.
     * @return Defined handle referencing primary icon mappings.
     */
    afx_msg HCURSOR OnQueryDragIcon();

    /**
     * @brief Processes persistent threaded UI timers prompting canvas invalidations continuously.
     * @param nIDEvent Tick identifier matching initialization states natively.
     */
    afx_msg void OnTimer(UINT_PTR nIDEvent);

    /**
     * @brief Evaluates visual triggers swapping connected webcam data pointers.
     */
    afx_msg void OnCbnSelchangeComboCamera();

    /**
     * @brief Handles modifications to the Face Detector drop down selection.
     */
    afx_msg void OnCbnSelchangeComboFaceDetector();

    /**
     * @brief Handles modifications to the Face Identifier drop down selection.
     */
    afx_msg void OnCbnSelchangeComboFaceIdentifier();

    /**
     * @brief Handles dialog close events.
     */
    afx_msg void OnBnClickedClose();

    /**
     * @brief Handles window activation cycles ensuring correct top level drawing behavior natively.
     * @param nState Status reflecting activation transitions securely.
     * @param pWndOther Related opposing windows logically mapped.
     * @param bMinimized Identifies minification state variables dynamically.
     */
    afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);

    /**
     * @brief Traps inter-thread logic dynamically casting projected light arrays onto canvases.
     * @param wParam Custom defined RGB message block.
     * @param lParam Unused argument trailing.
     * @return Zero declaring successfully processed routines natively.
     */
    afx_msg LRESULT OnUpdateScreenLight(WPARAM wParam, LPARAM lParam);

    /**
     * @brief Reconstitutes screen light modes immediately post startup cycles appropriately natively.
     * @param wParam Reserved empty integer.
     * @param lParam Reserved empty integer.
     * @return Zero validating completion logic perfectly.
     */
    afx_msg LRESULT OnInitScreenLight(WPARAM wParam, LPARAM lParam);

    /**
     * @brief Activates manual light configuration overriding dynamic inferences internally securely.
     */
    afx_msg void OnRadioSlSlider();

    /**
     * @brief Reactivates dynamic ambient illumination evaluating lighting constraints effectively.
     */
    afx_msg void OnRadioSlAuto();

    /**
     * @brief Powers down light processing layers saving background allocations.
     */
    afx_msg void OnRadioSlNone();

    /**
     * @brief Extracts adjusted interface ranges pushing brightness factors consistently.
     * @param nSBCode Slider movement action code.
     * @param nPos Tracking offset limits.
     * @param pScrollBar Generic GUI control pointers.
     */
    afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);

    /**
     * @brief Triggers formal identification enrollment wrapping outputs.
     */
    afx_msg void OnBnClickedButtonConfirm();

    /**
     * @brief Evaluates matched buffer data ceasing active capture streams validating best profiles.
     */
    afx_msg void OnBnClickedButtonPhotoOk();

    /**
     * @brief Reopens camera loops flushing residual metrics.
     */
    afx_msg void OnBnClickedButtonCameraOn();

    /**
     * @brief Contextualizes application main menu clicks preparing underlying actions.
     * @param pPopupMenu Menu pointer.
     * @param nIndex Tracking numerical list.
     * @param bSysMenu Checks if the target is the system menu.
     */
    afx_msg void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);

    /**
     * @brief Formally signals cancellation commands enforcing graceful logical termination securely.
     */
    afx_msg void OnFileExit();

    /**
     * @brief Safely processes external tracking instructions routing list visualization requests.
     */
    afx_msg void OnFileShowattendancelist();

    /**
     * @brief Evaluates file references isolating target detection storage accurately.
     */
    afx_msg void OnFacedetectionShowfolder();

    /**
     * @brief Prompts system commands navigating detection telemetry folders intuitively.
     */
    afx_msg void OnEvaluationFacedetection32783();

    /**
     * @brief Spawns explorer structures pointing towards recorded validation evaluations natively.
     */
    afx_msg void OnEvaluationFaceidentification32784();

    /**
     * @brief Routes menu parameters triggering modal configuration structures correctly.
     */
    afx_msg void OnFileSettings();

    /**
     * @brief Selects all text inside the Name Combo Box.
     */
    afx_msg void OnCbnSetfocusComboName();

    /**
     * @brief Selects all text inside the User ID input edit control.
     */
    afx_msg void OnEnSetfocusEditId();

    /**
     * @brief Selects all text inside the Comment input edit control.
     */
    afx_msg void OnEnSetfocusEditComment();

    DECLARE_MESSAGE_MAP()

private:
    int m_faceDetectionMode = 2;
    int m_faceIdentificationMode = 2;
    CComboBox    m_comboCamera;
    CComboBox    m_comboFaceDetector;
    CComboBox    m_comboFaceIdentifier;
    CSliderCtrl  m_sliderScreenLight;
    CComboBox    m_comboName;
    CEdit        m_editID;
    CDateTimeCtrl m_editTime;
    CEdit        m_editTimeOrg;
    CEdit        m_editComment;
    int          m_currentCameraIdx;
    bool         m_shouldChangeCamera;
    bool         m_mirrorView;

    FaceDetector m_detector;
    FaceIdentifier m_identifier;
    std::mutex   m_frameMutex;
    cv::Mat      m_lastFrame;
    UINT_PTR     m_timerId;
    CFont        m_fontBold;
    CFont        m_fontRegular;
    CFont        m_fontFixed;
    CFont        m_fontFixedSmall;
    CFont        m_fontFixedList;
    FpsCounter   m_fpsCounter;

    std::deque<FaceBufferItem> m_faceRingBuffer;
    std::mutex m_bufferMutex;
    FaceBufferItem             m_bestItem;
    cv::Point m_lastTargetCenter{ -1, -1 };
    std::atomic<bool> m_bCapturing;
    std::atomic<int>  m_discardFrames{ 0 };

    std::mutex m_pauseMutex;
    std::condition_variable m_pauseCV;

    AdaptiveScreenLight            m_screenLight;
    std::atomic<ScreenLightMode>   m_screenLightMode;
    int                            m_sliderValue;

    void* m_pvBits;
    int    m_bmpWidth;
    int    m_bmpHeight;

    /**
     * @brief Evaluates generated tracking elements writing buffered OpenCV layers natively inside visual canvas windows.
     */
    void    UpdateFrame();

    /**
     * @brief Encapsulates arrays constructing compatible visual block RGB matrix objects.
     * @param mat Encoded color matrix metric.
     * @return Fully formatted DIB structure mapping properly.
     */
    HBITMAP CreateBitmapFromMat(const cv::Mat& mat);

    /**
     * @brief Updates internal UI components forcing combo-boxes aligning to identity.
     * @param name Text descriptor containing resolved identification formats safely.
     */
    void UpdateIdentificationFields(CString name);

    /**
     * @brief Wraps generated recording logs isolating data towards filesystem outputs.
     * @param strTime Resolved local logging limits structurally applied.
     * @param strName Processed identity alias cleanly.
     * @param strID Tracking numerical ID logic validated efficiently.
     * @param strComment Descriptive notes wrapped seamlessly.
     */
    void SaveAttendanceToCsv(const CString& strTime, const CString& strName, const CString& strID, const CString& strComment);

    /**
     * @brief Translates selected definitions enforcing standard lighting logic visually natively.
     * @param mode Evaluated limit selecting target active tracking modes.
     */
    void    ApplyScreenLightMode(ScreenLightMode mode);

    /**
     * @brief Applies manual logic updates isolating tracking values.
     * @param value Scaled metrics safely applied visually.
     */
    void    ApplySliderValue(int value);

    /**
     * @brief Formats integer representations generating RGB objects clearly.
     * @param value Intensity mappings properly bound natively.
     * @return Formatted Color format evaluating output directly.
     */
    static COLORREF SliderToColor(int value);

    /**
     * @brief Initial configuration iterating video elements.
     */
    void InitializeCameraList();

    /**
     * @brief Instantiates array combo strings initializing detection configurations locally.
     */
    void InitializeFaceDetector();

    /**
     * @brief Wraps logic pushing options matching identity matrices visually efficiently natively.
     */
    void InitializeFaceIdentifier();

    /**
     * @brief Preps secondary threading processes fetching real-time frames continuously.
     */
    void InitializeWorkerThread();

    /**
     * @brief Formats structural bounds resizing visual typography limits internally naturally.
     */
    void InitializeFontsAndUI();

    /**
     * @brief Limits input entries applying specialized formatting configurations tracking safely.
     */
    void InitializeInputFields();

    /**
     * @brief Handles modifications modifying UI states correctly appropriately seamlessly reliably.
     * @param bEnable Setting flag enabling or disabling inputs.
     */
    void EnableInputFields(BOOL bEnable);

    /**
     * @brief Binds table column interfaces natively organically explicitly structurally stably thoughtfully.
     */
    void InitializeListControl();

    /**
     * @brief Retrieves ambient definitions gracefully conceptually securely implicitly elegantly logically cleanly.
     */
    void InitializeScreenLightSettings();

    /**
     * @brief Reconstitutes visual configurations elegantly reliably cleanly.
     */
    void InitializeMenuSettings();

    std::thread m_workerThread;
    std::atomic<bool> m_bStopThread{ false };
    std::atomic<bool> m_bShowWarning{ false };
    std::atomic<double> m_currentFps{ 0.0 };

    std::atomic<uint64_t> m_totalFrames{ 0 };
    std::atomic<double>   m_totalLatencyMs{ 0.0 };

    /**
     * @brief Saves accumulated timing stats accurately writing records sequentially formatting logs.
     * @param currentMethod Internal logical condition explicitly mapping algorithms selectively visually safely.
     */
    void FlushAndResetDetectionData(FaceDetectionMethod currentMethod);

    /**
     * @brief Slices input streams resolving resolution distortions generating gray frames.
     * @param inOutFrame Captured reference frame.
     * @param outResizedFrame Matched evaluated resolution block.
     * @param outDisplayFrame Corrected adjusted output.
     */
    void ProcessCameraFrame(cv::Mat& inOutFrame, cv::Mat& outResizedFrame, cv::Mat& outDisplayFrame);

    /**
     * @brief Asserts detection boundaries filtering out faces not residing near center using Haar cascades.
     * @param displayFrame Underlying image matrix evaluated.
     * @param cx Target axis X defining frame orientation target.
     * @param cy Target axis Y aligning visual emphasis priorities.
     * @param minDistance2 Evaluated square spacing variable iteratively comparing.
     * @param outBestFaceData Bounding structures matched against center limits accurately.
     * @return True when eligible profiles evaluate correctly without errors.
     */
    bool DetectAndGetBestHaarFace(cv::Mat& displayFrame, int cx, int cy, int& minDistance2, std::vector<float>& outBestFaceData);

    /**
     * @brief Asserts detection boundaries filtering out faces using the YuNet neural network architecture.
     * @param displayFrame Full scaled visualization matrix mapped to the screen.
     * @param resizedFrame Normalized structural resolution block.
     * @param cx Screen center X coordinate.
     * @param cy Screen center Y coordinate.
     * @param minDistance2 By-reference square distance matching the closest face dynamically.
     * @param outFaces Extracted neural network matrix output representing multiple profiles.
     * @param outBestFaceData Formatted structured output isolating the physically closest face.
     * @return True if at least one valid face is detected.
     */
    bool DetectAndGetBestYunetFace(cv::Mat& displayFrame, const cv::Mat& resizedFrame, int cx, int cy, int& minDistance2, cv::Mat& outFaces, std::vector<float>& outBestFaceData);

    /**
     * @brief Encapsulates dual-mode detection logic delegating calls explicitly based on user configuration.
     * @param displayFrame BGR structural array outputting strictly to canvas components.
     * @param resizedFrame Evaluated frame matching neural network expected limits.
     * @param outFaces Aggregate extracted raw matrices inherently handled logically.
     * @param outBestFaceData Floating vector containing bounding layouts seamlessly correctly safely.
     * @return True if the process successfully highlights tracking structures natively.
     */
    bool PerformFaceDetection(cv::Mat& displayFrame, cv::Mat& resizedFrame, cv::Mat& outFaces, std::vector<float>& outBestFaceData);

    /**
     * @brief Increments processing counters safely accumulating latency definitions sequentially.
     * @param fps Live evaluating metric defining current tracking outputs smoothly.
     */
    void UpdateFpsAndLatency(double fps);

    /**
     * @brief Asserts lighting configuration bridging captured contexts into dynamic brightness bounds.
     * @param resizedFrame Main tracking area mapping physical light bounds globally.
     * @param faces Found face areas limiting evaluation explicitly logically.
     */
    void UpdateScreenLightUsingFaces(const cv::Mat& resizedFrame, const cv::Mat& faces);

    /**
     * @brief Estimates bounding areas rendering scaled contexts accurately.
     * @param srcW Native camera width.
     * @param srcH Native camera height.
     * @param dstW Physical canvas rendering width.
     * @param dstH Physical canvas rendering height.
     * @param drawX Exported offset horizontally.
     * @param drawW Exported scaled width.
     * @param drawH Exported scaled height.
     */
    void CalculateDrawArea(int srcW, int srcH, int dstW, int dstH, int& drawX, int& drawW, int& drawH) const;

    /**
     * @brief Commits diagnostic frame rates strings visibly validating ongoing computational states reliably.
     * @param memDC Target drawing interface device contexts.
     * @param x Horizontal corner offset reliably mapped.
     * @param y Vertical corner logically mapped.
     */
    void DrawFpsText(CDC& memDC, int x, int y);

    /**
     * @brief Constructs framing overlays assisting human alignment accurately cleanly.
     * @param memDC Interface mapping active windows logically.
     * @param x Baseline offset defining horizontal spans.
     * @param y Height baseline correctly predictably efficiently effectively reliably.
     * @param w Bounding extent tracking completely.
     * @param h Vertical tracking properly.
     */
    void DrawGuideFrame(CDC& memDC, int x, int y, int w, int h);

    /**
     * @brief Draws a warning message on the given device context.
     * @param memDC Render device context.
     * @param x Horizontal origin.
     * @param y Vertical baseline.
     * @param w Width of the drawing area.
     */
    void DrawWarningMessage(CDC& memDC, int x, int y, int w);

    /**
     * @brief Draws a cv::Mat image onto a target static UI control.
     * @param nID The resource ID of the static control.
     * @param mat The cv::Mat image to be drawn.
     */
    void DrawMatToStatic(int nID, const cv::Mat& mat);

    /**
     * @brief Calculates the sharpness of an image using the variance of its Laplacian.
     * @param img The input image matrix.
     * @return A double indicating the sharpness score.
     */
    double CalculateSharpness(const cv::Mat& img);

    /**
     * @brief Calculates the contrast of an image using pixel intensity standard deviation.
     * @param img The input image matrix.
     * @return A double representing the calculated contrast.
     */
    double CalculateContrast(const cv::Mat& img);

    /**
     * @brief Computes a composite quality score combining sharpness, confidence, and contrast.
     * @param sharpness Calculated sharpness metric.
     * @param faceConfidence Detection probability natively gathered from the AI model.
     * @param contrast Calculated image contrast metric.
     * @return The final weighted quality score.
     */
    double CalculateBestFaceScore(double sharpness, double faceConfidence, double contrast);

    /**
     * @brief Clears the displayed faces and evaluation scores from the UI.
     */
    void ClearFaceUI();

    /**
     * @brief Iterates through the face ring buffer to locate the highest and lowest scoring faces.
     * @param outBestFace Output matrix for the highest scoring face.
     * @param outWorstFace Output matrix for the lowest scoring face.
     * @param outMaxScore Output for the maximum score found.
     * @param outMinScore Output for the minimum score found.
     */
    void FindBestAndWorstFaces(cv::Mat& outBestFace, cv::Mat& outWorstFace, double& outMaxScore, double& outMinScore);

    /**
     * @brief Updates the UI with the best and worst valid face buffers and attempts identity recognition.
     * @param bestFace The highest scoring face matrix.
     * @param worstFace The lowest scoring face matrix.
     * @param maxScore The score of the best face.
     * @param minScore The score of the worst face.
     */
    void ApplyFaceIdentificationResults(const cv::Mat& bestFace, const cv::Mat& worstFace, double maxScore, double minScore);

    /**
     * @brief Checks if the target face has moved significantly and resets the buffer if so.
     * @param cx Target face center X coordinate.
     * @param cy Target face center Y coordinate.
     * @param faceWidth Width of the detected face.
     * @return True if a target jump was detected and the buffer was reset.
     */
    bool CheckAndResetTargetJump(int cx, int cy, int faceWidth);

    /**
     * @brief Calculates a safe square bounding box within the given frame dimensions.
     * @param cx Center X coordinate.
     * @param cy Center Y coordinate.
     * @param sideLength Desired length of the square's sides.
     * @param maxWidth Maximum allowed width.
     * @param maxHeight Maximum allowed height.
     * @return A cv::Rect representing the calculated safe square area.
     */
    cv::Rect GetSafeSquareRect(int cx, int cy, int sideLength, int maxWidth, int maxHeight);

    /**
     * @brief Calculates a padded bounding box and shifts facial landmark coordinates accordingly.
     * @param cx Center X coordinate.
     * @param cy Center Y coordinate.
     * @param padSide Side length of the padded area.
     * @param maxWidth Frame width boundary.
     * @param maxHeight Frame height boundary.
     * @param originalData Original landmark coordinates.
     * @param outShiftedData Adjusted landmark coordinates relative to the new padded rect.
     * @return A cv::Rect representing the padded bounding box.
     */
    cv::Rect GetPaddedRectAndShiftLandmarks(int cx, int cy, int padSide, int maxWidth, int maxHeight, const std::vector<float>& originalData, std::vector<float>& outShiftedData);

    /**
     * @brief Adds a processed face item to the ring buffer and removes expired entries.
     * @param item FaceBufferItem containing metadata and images for the detected face.
     */
    void AddItemToRingBuffer(const FaceBufferItem& item);

    /**
     * @brief Processes the detected face, calculates quality scores, and adds it to the tracking buffer.
     * @param displayFrame Frame used for display output.
     * @param resizedFrame Normalized face frame used for processing.
     * @param bestFaceData Coordinates and metrics of the most prominent detected face.
     */
    void ProcessAndBufferDetectedFace(const cv::Mat& displayFrame, const cv::Mat& resizedFrame, const std::vector<float>& bestFaceData);
};
