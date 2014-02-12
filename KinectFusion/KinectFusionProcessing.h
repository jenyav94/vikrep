#pragma once

// For timing calls
#include "Time.h"

#include "stdafx.h"
#include <NuiApi.h>
#include <NuiKinectFusionApi.h>
#include<iostream>
#include "ppl.h"
using namespace std;




class KinectFusionProcessing
{

	static const int            cBytesPerPixel = 4; // for depth float and int-per-pixel raycast images
    static const int            cResetOnTimeStampSkippedMilliseconds = 2000;
    static const int            cResetOnNumberOfLostFrames = 100;
    static const int            cStatusMessageMaxLen = MAX_PATH*2;
    static const int            cTimeDisplayInterval = 10;
	static const int			cMinTimestampDifferenceForFrameReSync = 17;


public:

	KinectFusionProcessing(void);
	~KinectFusionProcessing(void);

	

	 /// <summary>
    /// Creates the main window and begins processing
    /// </summary>
    /// <param name="hInstance"></param>
    /// <param name="nCmdShow"></param>
    void                         Run(void);

	bool                         stopApp;


	 // Current Kinect
    INuiSensor*                 m_pNuiSensor;

	NUI_IMAGE_RESOLUTION        m_depthImageResolution;
	NUI_IMAGE_RESOLUTION        m_colorImageResolution;

    int                         m_cDepthWidth;
    int                         m_cDepthHeight;
    int                         m_cDepthImagePixels;

	int                         m_cColorWidth;
    int                         m_cColorHeight;
    int                         m_cColorImagePixels;

    HANDLE                      m_pDepthStreamHandle;
    HANDLE                      m_hNextDepthFrameEvent;

	HANDLE                      m_pColorStreamHandle;
    HANDLE                      m_hNextColorFrameEvent;

	LONGLONG                    m_cLastDepthFrameTimeStamp;
    LONGLONG                    m_cLastColorFrameTimeStamp;

	
	/// <summary>
    /// Main processing function
    /// </summary>
    void                        Update();

	/// <summary>
    /// Create the first connected Kinect found 
    /// </summary>
    /// <returns>S_OK on success, otherwise failure code</returns>
    HRESULT                     CreateFirstConnected();

	 /// <summary>
    /// Initialize Kinect Fusion volume and images for processing
    /// </summary>
    /// <returns>S_OK on success, otherwise failure code</returns>
    HRESULT                     InitializeKinectFusion();

	 /// <summary>
    /// Copy the extended depth data out of a Kinect image frame
    /// </summary>
    /// <returns>S_OK on success, otherwise failure code</returns>
    HRESULT                     CopyExtendedDepth(NUI_IMAGE_FRAME &imageFrame);

	 /// <summary>
    /// Copy the color data out of a Kinect image frame
    /// </summary>
    /// <returns>S_OK on success, otherwise failure code</returns>
    HRESULT                     CopyColor(NUI_IMAGE_FRAME &imageFrame);

    /// <summary>
    /// Adjust color to the same space as depth
    /// </summary>
    /// <returns>S_OK on success, otherwise failure code</returns>
    HRESULT                     MapColorToDepth();

	 /// <summary>
     /// Save Mesh to disk.
     /// </summary>
     /// <param name="mesh">The mesh to save.</param>
     /// <returns>indicates success or failure</returns>
    HRESULT KinectFusionProcessing::SaveMeshFile(INuiFusionColorMesh* pMesh);

    /// <summary>
    /// Handle new depth data
    /// </summary>
    void                        ProcessDepth();

	/// <summary>
	/// Perform only depth conversion and camera tracking
	/// </summary>
	HRESULT						CameraTrackingOnly();

    /// <summary>
    /// Reset the reconstruction camera pose and clear the volume.
    /// </summary>
    /// <returns>S_OK on success, otherwise failure code</returns>
    HRESULT                     ResetReconstruction();

	


	 /// <summary>
    /// The Kinect Fusion Reconstruction Volume
    /// </summary>
    INuiFusionColorReconstruction*   m_pVolume;

    /// <summary>
    /// The Kinect Fusion Volume Parameters
    /// </summary>
    NUI_FUSION_RECONSTRUCTION_PARAMETERS m_reconstructionParams;

    /// <summary>
    // The Kinect Fusion Camera Transform
    /// </summary>
    Matrix4                     m_worldToCameraTransform;

    /// <summary>
    // The default Kinect Fusion World to Volume Transform
    /// </summary>
    Matrix4                     m_defaultWorldToVolumeTransform;

    /// <summary>
    /// Frames from the depth input
    /// </summary>
    NUI_DEPTH_IMAGE_PIXEL*      m_pDepthImagePixelBuffer;
    NUI_FUSION_IMAGE_FRAME*     m_pDepthFloatImage;

	 /// <summary>
    /// For mapping depth to color
    /// </summary>
    NUI_FUSION_IMAGE_FRAME*     m_pColorImage;
    NUI_FUSION_IMAGE_FRAME*     m_pResampledColorImageDepthAligned;
    NUI_COLOR_IMAGE_POINT*      m_pColorCoordinates;
    float                       m_colorToDepthDivisor;
    float                       m_oneOverDepthDivisor;
    INuiCoordinateMapper*       m_pMapper;
    bool                        m_bCaptureColor;
    unsigned int                m_cColorIntegrationInterval;

	/// <summary>
    /// Camera Tracking parameters
    /// </summary>
    int                         m_cLostFrameCounter;
    bool                        m_bTrackingFailed;

	 /// <summary>
    /// Parameter to turn automatic reset of the reconstruction when camera tracking is lost on or off.
    /// Set to true in the constructor to enable auto reset on cResetOnNumberOfLostFrames lost frames,
    /// or set false to never automatically reset.
    /// </summary>
    bool                        m_bAutoResetReconstructionWhenLost;

    /// <summary>
    /// Parameter to enable automatic reset of the reconstruction when there is a large
    /// difference in timestamp between subsequent frames. This should usually be set true as 
    /// default to enable recorded .xed files to generate a reconstruction reset on looping of
    /// the playback or scrubbing, however, for debug purposes, it can be set false to prevent
    /// automatic reset on timeouts.
    /// </summary>
    bool                        m_bAutoResetReconstructionOnTimeout;

	 /// <summary>
    /// Processing parameters
    /// </summary>
    int                         m_deviceIndex;
    NUI_FUSION_RECONSTRUCTION_PROCESSOR_TYPE m_processorType;
    bool                        m_bInitializeError;
    float                       m_fMinDepthThreshold;
    float                       m_fMaxDepthThreshold;
    bool                        m_bMirrorDepthFrame;
    unsigned short              m_cMaxIntegrationWeight;
    int                         m_cFrameCounter;
    double                      m_fStartTime;
    Timing1::Time               m_timer;

	/// <summary>
    /// Parameter to translate the reconstruction based on the minimum depth setting. When set to
    /// false, the reconstruction volume +Z axis starts at the camera lens and extends into the scene.
    /// Setting this true in the constructor will move the volume forward along +Z away from the
    /// camera by the minimum depth threshold to enable capture of very small reconstruction volumes
    /// by setting a non-identity camera transformation in the ResetReconstruction call.
    /// Small volumes should be shifted, as the Kinect hardware has a minimum sensing limit of ~0.35m,
    /// inside which no valid depth is returned, hence it is difficult to initialize and track robustly  
    /// when the majority of a small volume is inside this distance.
    /// </summary>
    bool                        m_bTranslateResetPoseByMinDepthThreshold;
};

