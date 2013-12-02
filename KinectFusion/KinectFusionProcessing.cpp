#include "KinectFusionProcessing.h"



/// <summary>
/// Set Identity in a Matrix4
/// </summary>
/// <param name="mat">The matrix to set to identity</param>
void SetIdentityMatrix(Matrix4 &mat)
{
    mat.M11 = 1; mat.M12 = 0; mat.M13 = 0; mat.M14 = 0;
    mat.M21 = 0; mat.M22 = 1; mat.M23 = 0; mat.M24 = 0;
    mat.M31 = 0; mat.M32 = 0; mat.M33 = 1; mat.M34 = 0;
    mat.M41 = 0; mat.M42 = 0; mat.M43 = 0; mat.M44 = 1;
}

/// <summary>
/// Constructor
/// </summary>
KinectFusionProcessing::KinectFusionProcessing():
    stopApp(true),
    m_pVolume(nullptr),
    m_pNuiSensor(nullptr),
    m_depthImageResolution(NUI_IMAGE_RESOLUTION_640x480),
    m_cDepthImagePixels(0),
    m_hNextDepthFrameEvent(INVALID_HANDLE_VALUE),
    m_pDepthStreamHandle(INVALID_HANDLE_VALUE),
    m_bMirrorDepthFrame(false),
    m_bTranslateResetPoseByMinDepthThreshold(true),
    m_bAutoResetReconstructionWhenLost(false),
    m_bAutoResetReconstructionOnTimeout(true),
    m_cLostFrameCounter(0),
    m_bTrackingFailed(false),
    m_cFrameCounter(0),
    m_fStartTime(0),
    m_pDepthImagePixelBuffer(nullptr),
    m_pDepthFloatImage(nullptr),
    m_bInitializeError(false)
{
	// Get the depth frame size from the NUI_IMAGE_RESOLUTION enum
    // You can use NUI_IMAGE_RESOLUTION_640x480 or NUI_IMAGE_RESOLUTION_320x240 in this sample
    // Smaller resolutions will be faster in per-frame computations, but show less detail in reconstructions.
    DWORD width = 0, height = 0;
    NuiImageResolutionToSize(m_depthImageResolution, width, height);
    m_cDepthWidth = width;
    m_cDepthHeight = height;
    m_cDepthImagePixels = m_cDepthWidth*m_cDepthHeight;


    // Define a cubic Kinect Fusion reconstruction volume,
    // with the Kinect at the center of the front face and the volume directly in front of Kinect.
    m_reconstructionParams.voxelsPerMeter = 4;// 1000mm / 256vpm = ~3.9mm/voxel    
    m_reconstructionParams.voxelCountX = 128;   // 512 / 256vpm = 2m wide reconstruction
    m_reconstructionParams.voxelCountY = 128;   // Memory = 512*384*512 * 4bytes per voxel
    m_reconstructionParams.voxelCountZ = 128;   // This will require a GPU with at least 512MB

    // These parameters are for optionally clipping the input depth image 
    m_fMinDepthThreshold = NUI_FUSION_DEFAULT_MINIMUM_DEPTH;   // min depth in meters
    m_fMaxDepthThreshold = NUI_FUSION_DEFAULT_MAXIMUM_DEPTH;    // max depth in meters

    // This parameter is the temporal averaging parameter for depth integration into the reconstruction
    m_cMaxIntegrationWeight = NUI_FUSION_DEFAULT_INTEGRATION_WEIGHT;	// Reasonable for static scenes

    // This parameter sets whether GPU or CPU processing is used. Note that the CPU will likely be 
    // too slow for real-time processing.
    m_processorType = NUI_FUSION_RECONSTRUCTION_PROCESSOR_TYPE_AMP;

    // If GPU processing is selected, we can choose the index of the device we would like to
    // use for processing by setting this zero-based index parameter. Note that setting -1 will cause
    // automatic selection of the most suitable device (specifically the DirectX11 compatible device 
    // with largest memory), which is useful in systems with multiple GPUs when only one reconstruction
    // volume is required. Note that the automatic choice will not load balance across multiple 
    // GPUs, hence users should manually select GPU indices when multiple reconstruction volumes 
    // are required, each on a separate device.
    m_deviceIndex = -1;    // automatically choose device index for processing

    SetIdentityMatrix(m_worldToCameraTransform);
    SetIdentityMatrix(m_defaultWorldToVolumeTransform);

    m_cLastDepthFrameTimeStamp.QuadPart = 0;
}

/// <summary>
/// Destructor
/// </summary>
KinectFusionProcessing::~KinectFusionProcessing()
{

	/*INuiFusionColorMesh *mesh = nullptr;
    HRESULT hr = m_processor.CalculateMesh(&mesh);
	if (m_pVolume != nullptr)
    {
        hr = m_pVolume->CalculateMesh(1, ppMesh);
	}*/
	
	// Clean up Kinect Fusion
    SafeRelease(m_pVolume);

    SAFE_FUSION_RELEASE_IMAGE_FRAME(m_pDepthFloatImage);

    // Clean up Kinect
    if (m_pNuiSensor)
    {
        m_pNuiSensor->NuiShutdown();
        m_pNuiSensor->Release();
    }

    if (m_hNextDepthFrameEvent != INVALID_HANDLE_VALUE)
    {
        CloseHandle(m_hNextDepthFrameEvent);
    }

    // clean up the depth pixel array
   // SAFE_DELETE_ARRAY(m_pDepthImagePixelBuffer);

    // clean up Direct2D renderer
  //  SAFE_DELETE(m_pDrawDepth);

    // done with depth pixel data
  //  SAFE_DELETE_ARRAY(m_pDepthRGBX);

    // clean up Direct2D
  //  SafeRelease(m_pD2DFactory);
}

void KinectFusionProcessing::Run()
{
   

    const int eventCount = 1;
    HANDLE hEvents[eventCount];

    // Main message loop
    while (stopApp)
    {
		if(_kbhit()){
			if(_getch()==27)
		      stopApp=false;
		}
		
        hEvents[0] = m_hNextDepthFrameEvent;

        // Check to see if we have either a message (by passing in QS_ALLINPUT)
        // Or a Kinect event (hEvents)
        // Update() will check for Kinect events individually, in case more than one are signalled
        MsgWaitForMultipleObjects(eventCount, hEvents, FALSE, INFINITE, QS_ALLINPUT);

        // Explicitly check the Kinect frame event since MsgWaitForMultipleObjects
        // can return for other reasons even though it is signaled.
        Update();

       
    }

	if(!stopApp){
		
		 UINT sourceOriginX=0;
         UINT sourceOriginY=0;
         UINT sourceOriginZ=0;
         UINT destinationResolutionX= (int)m_reconstructionParams.voxelCountX;//* m_reconstructionParams.voxelsPerMeter;
         UINT destinationResolutionY=(int)m_reconstructionParams.voxelCountY;//* m_reconstructionParams.voxelsPerMeter;
         UINT destinationResolutionZ=(int)m_reconstructionParams.voxelCountZ;//* m_reconstructionParams.voxelsPerMeter;
         UINT voxelStep=1;
         UINT cbVolumeBlock=(int)destinationResolutionX*destinationResolutionY*destinationResolutionZ*sizeof(SHORT);
         SHORT *pVolumeBlock=new SHORT[destinationResolutionX*destinationResolutionY*destinationResolutionZ];
		 int sizeofMas=(int)destinationResolutionX*destinationResolutionY*destinationResolutionZ;

		 for(int i=0; i<sizeofMas;++i){
			pVolumeBlock[i]=0;
		}


		 
	    m_pVolume->ExportVolumeBlock(sourceOriginX,sourceOriginY,sourceOriginZ,destinationResolutionX,destinationResolutionY,destinationResolutionZ,voxelStep,cbVolumeBlock,pVolumeBlock);
		
		FILE *file;
		file=fopen("out.txt","w");
		cout<<endl<<sizeofMas;
		for(int i=0; i<sizeofMas;++i){
		if(pVolumeBlock[i]>0)
		 fprintf(file,"%d  ",pVolumeBlock[i]);
		}
		fclose(file);
	    delete[] pVolumeBlock;
	}

    
}

/// <summary>
/// Main processing function
/// </summary>
void KinectFusionProcessing::Update()
{
    if (nullptr == m_pNuiSensor)
    {
        return;
    }

    if ( WAIT_OBJECT_0 == WaitForSingleObject(m_hNextDepthFrameEvent, 0) )
    {
        ProcessDepth();
    }
}


/// <summary>
/// Create the first connected Kinect found 
/// </summary>
/// <returns>indicates success or failure</returns>
HRESULT KinectFusionProcessing::CreateFirstConnected()
{
    INuiSensor * pNuiSensor;
    HRESULT hr;

    int iSensorCount = 0;
    hr = NuiGetSensorCount(&iSensorCount);
    if (FAILED(hr))
    {
		system("cls");
        cout<<"No ready Kinect found!"<<endl;
        return hr;
    }

    // Look at each Kinect sensor
    for (int i = 0; i < iSensorCount; ++i)
    {
        // Create the sensor so we can check status, if we can't create it, move on to the next
        hr = NuiCreateSensorByIndex(i, &pNuiSensor);
        if (FAILED(hr))
        {
            continue;
        }

        // Get the status of the sensor, and if connected, then we can initialize it
        hr = pNuiSensor->NuiStatus();
        if (S_OK == hr)
        {
            m_pNuiSensor = pNuiSensor;
            break;
        }

        // This sensor wasn't OK, so release it since we're not using it
        pNuiSensor->Release();
    }

    if (nullptr != m_pNuiSensor)
    {
        // Initialize the Kinect and specify that we'll be using depth
        hr = m_pNuiSensor->NuiInitialize(NUI_INITIALIZE_FLAG_USES_DEPTH); 
        if (SUCCEEDED(hr))
        {
            // Create an event that will be signaled when depth data is available
            m_hNextDepthFrameEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);

            // Open a depth image stream to receive depth frames
            hr = m_pNuiSensor->NuiImageStreamOpen(
                NUI_IMAGE_TYPE_DEPTH,
                m_depthImageResolution,
                0,
                2,
                m_hNextDepthFrameEvent,
                &m_pDepthStreamHandle);
        }

       
    }

    if (nullptr == m_pNuiSensor || FAILED(hr))
    {
		system("cls");
        cout<<"No ready Kinect found!"<<endl;
        return E_FAIL;
    }

    return hr;
}


/// <summary>
/// Initialize Kinect Fusion volume and images for processing
/// </summary>
/// <returns>S_OK on success, otherwise failure code</returns>
HRESULT KinectFusionProcessing::InitializeKinectFusion()
{
    HRESULT hr = S_OK;

    // Check to ensure suitable DirectX11 compatible hardware exists before initializing Kinect Fusion
    WCHAR description[MAX_PATH];
    WCHAR instancePath[MAX_PATH];
    UINT memorySize = 0;

    if (FAILED(hr = NuiFusionGetDeviceInfo(
        m_processorType, 
        m_deviceIndex, 
        &description[0], 
        ARRAYSIZE(description), 
        &instancePath[0],
        ARRAYSIZE(instancePath), 
        &memorySize)))
    {
        if (hr ==  E_NUI_BADINDEX)
        {
            // This error code is returned either when the device index is out of range for the processor 
            // type or there is no DirectX11 capable device installed. As we set -1 (auto-select default) 
            // for the device index in the parameters, this indicates that there is no DirectX11 capable 
            // device. The options for users in this case are to either install a DirectX11 capable device
            // (see documentation for recommended GPUs) or to switch to non-real-time CPU based 
            // reconstruction by changing the processor type to NUI_FUSION_RECONSTRUCTION_PROCESSOR_TYPE_CPU.
			system("cls");
            cout<<"No DirectX11 device detected, or invalid device index - Kinect Fusion requires a DirectX11 device for GPU-based reconstruction."<<endl;
        }
        else
        {
			system("cls");
            cout<<"Failed in call to NuiFusionGetDeviceInfo."<<endl;
        }
        return hr;
    }

    // Create the Kinect Fusion Reconstruction Volume
    hr = NuiFusionCreateReconstruction(
        &m_reconstructionParams,
        m_processorType, m_deviceIndex,
        &m_worldToCameraTransform,
        &m_pVolume);

    if (FAILED(hr))
    {
        if (E_NUI_GPU_FAIL == hr)
        {
            system("cls");
            cout<<"Device "<< m_deviceIndex<<" not able to run Kinect Fusion, or error initializing."<<endl;
            
        }
        else if (E_NUI_GPU_OUTOFMEMORY == hr)
        {
            system("cls");
            cout<<"Device "<<m_deviceIndex<<" out of memory error initializing reconstruction - try a smaller reconstruction volume."<<endl;
            
        }
        else if (NUI_FUSION_RECONSTRUCTION_PROCESSOR_TYPE_CPU != m_processorType)
        {
            system("cls");
            cout<<"Failed to initialize Kinect Fusion reconstruction volume on device"<<m_deviceIndex<<endl;
            
        }
        else
        {   system("cls");
            cout<<"Failed to initialize Kinect Fusion reconstruction volume on CPU."<<endl;
        }

        return hr;
    }

    // Save the default world to volume transformation to be optionally used in ResetReconstruction
    hr = m_pVolume->GetCurrentWorldToVolumeTransform(&m_defaultWorldToVolumeTransform);
    if (FAILED(hr))
    {
		system("cls");
        cout<<"Failed in call to GetCurrentWorldToVolumeTransform."<<endl;
        return hr;
    }

    if (m_bTranslateResetPoseByMinDepthThreshold)
    {
        // This call will set the world-volume transformation
        hr = ResetReconstruction();
        if (FAILED(hr))
        {
            return hr;
        }
    }

    // Frames generated from the depth input
    hr = NuiFusionCreateImageFrame(NUI_FUSION_IMAGE_TYPE_FLOAT, m_cDepthWidth, m_cDepthHeight, nullptr, &m_pDepthFloatImage);
    if (FAILED(hr))
    {
		system("cls");
        cout<<"Failed to initialize Kinect Fusion image."<<endl;
        return hr;
    }

    

    m_pDepthImagePixelBuffer = new(std::nothrow) NUI_DEPTH_IMAGE_PIXEL[m_cDepthImagePixels];
    if (nullptr == m_pDepthImagePixelBuffer)
    {
		system("cls");
        cout<<"Failed to initialize Kinect Fusion depth image pixel buffer."<<endl;
        return hr;
    }

    m_fStartTime = m_timer.AbsoluteTime();

   

    return hr;
}

/// <summary>
/// Copy the extended depth data out of a Kinect image frame
/// </summary>
/// <param name="imageFrame">The extended depth image frame to copy.</param>
/// <returns>S_OK on success, otherwise failure code</returns>
HRESULT KinectFusionProcessing::CopyExtendedDepth(NUI_IMAGE_FRAME &imageFrame)
{
    HRESULT hr = S_OK;

    if (nullptr == m_pDepthImagePixelBuffer)
    {
		system("cls");
        cout<<"Error depth image pixel buffer is nullptr."<<endl;
        return E_FAIL;
    }

    INuiFrameTexture *extendedDepthTex = nullptr;

    // Extract the extended depth in NUI_DEPTH_IMAGE_PIXEL format from the frame
    BOOL nearModeOperational = FALSE;
    hr = m_pNuiSensor->NuiImageFrameGetDepthImagePixelFrameTexture(m_pDepthStreamHandle, &imageFrame, &nearModeOperational, &extendedDepthTex);
    if (FAILED(hr))
    {
		system("cls");
        cout<<"Error getting extended depth texture."<<endl;
        return hr;
    }

    NUI_LOCKED_RECT extendedDepthLockedRect;

    // Lock the frame data to access the un-clamped NUI_DEPTH_IMAGE_PIXELs
    hr = extendedDepthTex->LockRect(0, &extendedDepthLockedRect, nullptr, 0);

    if (FAILED(hr) || extendedDepthLockedRect.Pitch == 0)
    {
		system("cls");
        cout<<"Error getting extended depth texture pixels."<<endl;
        return hr;
    }

    // Copy the depth pixels so we can return the image frame
    errno_t err = memcpy_s(m_pDepthImagePixelBuffer, m_cDepthImagePixels * sizeof(NUI_DEPTH_IMAGE_PIXEL), extendedDepthLockedRect.pBits, extendedDepthTex->BufferLen());

    extendedDepthTex->UnlockRect(0);

    if(0 != err)
    {
		system("cls");
        cout<<"Error copying extended depth texture pixels."<<endl;
        return hr;
    }

    return hr;
}


/// <summary>
/// Handle new depth data and perform Kinect Fusion processing
/// </summary>
void KinectFusionProcessing::ProcessDepth()
{
    if (m_bInitializeError)
    {
        return;
    }

    HRESULT hr = S_OK;
    NUI_IMAGE_FRAME imageFrame;

    ////////////////////////////////////////////////////////
    // Get an extended depth frame from Kinect

    hr = m_pNuiSensor->NuiImageStreamGetNextFrame(m_pDepthStreamHandle, 0, &imageFrame);
    if (FAILED(hr))
    {
		system("cls");
        cout<<"Kinect NuiImageStreamGetNextFrame call failed."<<endl;
        return;
    }

    hr = CopyExtendedDepth(imageFrame);

    LARGE_INTEGER currentDepthFrameTime = imageFrame.liTimeStamp;

    // Release the Kinect camera frame
    m_pNuiSensor->NuiImageStreamReleaseFrame(m_pDepthStreamHandle, &imageFrame);

    if (FAILED(hr))
    {
        return;
    }

    // To enable playback of a .xed file through Kinect Studio and reset of the reconstruction
    // if the .xed loops, we test for when the frame timestamp has skipped a large number. 
    // Note: this will potentially continually reset live reconstructions on slow machines which
    // cannot process a live frame in less time than the reset threshold. Increase the number of
    // milliseconds in cResetOnTimeStampSkippedMilliseconds if this is a problem.
    if (m_bAutoResetReconstructionOnTimeout && m_cFrameCounter != 0 
        && abs(currentDepthFrameTime.QuadPart - m_cLastDepthFrameTimeStamp.QuadPart) > cResetOnTimeStampSkippedMilliseconds)
    {
        ResetReconstruction();

        if (FAILED(hr))
        {
            return;
        }
    }

    m_cLastDepthFrameTimeStamp = currentDepthFrameTime;

    // Return if the volume is not initialized
    if (nullptr == m_pVolume)
    {
		system("cls");
        cout<<"Kinect Fusion reconstruction volume not initialized. Please try reducing volume size or restarting."<<endl;
        return;
    }

    ////////////////////////////////////////////////////////
    // Depth to DepthFloat

    // Convert the pixels describing extended depth as unsigned short type in millimeters to depth
    // as floating point type in meters.
    hr = m_pVolume->DepthToDepthFloatFrame(m_pDepthImagePixelBuffer, m_cDepthImagePixels * sizeof(NUI_DEPTH_IMAGE_PIXEL), m_pDepthFloatImage, m_fMinDepthThreshold, m_fMaxDepthThreshold, m_bMirrorDepthFrame);

    if (FAILED(hr))
    {
		system("cls");
        cout<<"Kinect Fusion NuiFusionDepthToDepthFloatFrame call failed."<<endl;
        return;
    }

    ////////////////////////////////////////////////////////
    // ProcessFrame

    // Perform the camera tracking and update the Kinect Fusion Volume
    // This will create memory on the GPU, upload the image, run camera tracking and integrate the
    // data into the Reconstruction Volume if successful. Note that passing nullptr as the final 
    // parameter will use and update the internal camera pose.
    hr = m_pVolume->ProcessFrame(m_pDepthFloatImage, NUI_FUSION_DEFAULT_ALIGN_ITERATION_COUNT, m_cMaxIntegrationWeight, &m_worldToCameraTransform);

    // Test to see if camera tracking failed. 
    // If it did fail, no data integration or raycast for reference points and normals will have taken 
    //  place, and the internal camera pose will be unchanged.
    if (FAILED(hr))
    {
        if (hr == E_NUI_FUSION_TRACKING_ERROR)
        {
            m_cLostFrameCounter++;
            m_bTrackingFailed = true;
			system("cls");
            cout<<"Kinect Fusion camera tracking failed! Align the camera to the last tracked position. "<<endl;
        }
        else
        {
			system("cls");
            cout<<"Kinect Fusion ProcessFrame call failed!"<<endl;
            return;
        }
    }
    else
    {
        Matrix4 calculatedCameraPose;
        hr = m_pVolume->GetCurrentWorldToCameraTransform(&calculatedCameraPose);

        if (SUCCEEDED(hr))
        {
            // Set the pose
            m_worldToCameraTransform = calculatedCameraPose;
            m_cLostFrameCounter = 0;
            m_bTrackingFailed = false;
        }
    }

    if (m_bAutoResetReconstructionWhenLost && m_bTrackingFailed && m_cLostFrameCounter >= cResetOnNumberOfLostFrames)
    {
        // Automatically clear volume and reset tracking if tracking fails
        hr = ResetReconstruction();

        if (FAILED(hr))
        {
            return;
        }

        // Set bad tracking message
		system("cls");
        cout<<"Kinect Fusion camera tracking failed, automatically reset volume."<<endl;
    }

    

    ////////////////////////////////////////////////////////
    // Periodically Display Fps

    // Update frame counter
   m_cFrameCounter++;

    // Display fps count approximately every cTimeDisplayInterval seconds
    double elapsed = m_timer.AbsoluteTime() - m_fStartTime;
    if ((int)elapsed >= cTimeDisplayInterval)
    {
        double fps = (double)m_cFrameCounter / elapsed;
    
        // Update status display
        if (!m_bTrackingFailed)
        {
            system("cls");
            cout<<"Fps:"<< fps;
            
        }

        m_cFrameCounter = 0;
        m_fStartTime = m_timer.AbsoluteTime();
    }
}


/// <summary>
/// Reset the reconstruction camera pose and clear the volume.
/// </summary>
/// <returns>S_OK on success, otherwise failure code</returns>
HRESULT KinectFusionProcessing::ResetReconstruction()
{
    if (nullptr == m_pVolume)
    {
        return E_FAIL;
    }

    HRESULT hr = S_OK;

    SetIdentityMatrix(m_worldToCameraTransform);

    // Translate the reconstruction volume location away from the world origin by an amount equal
    // to the minimum depth threshold. This ensures that some depth signal falls inside the volume.
    // If set false, the default world origin is set to the center of the front face of the 
    // volume, which has the effect of locating the volume directly in front of the initial camera
    // position with the +Z axis into the volume along the initial camera direction of view.
    if (m_bTranslateResetPoseByMinDepthThreshold)
    {
        Matrix4 worldToVolumeTransform = m_defaultWorldToVolumeTransform;

        // Translate the volume in the Z axis by the minDepthThreshold distance
        float minDist = (m_fMinDepthThreshold < m_fMaxDepthThreshold) ? m_fMinDepthThreshold : m_fMaxDepthThreshold;
        worldToVolumeTransform.M43 -= (minDist * m_reconstructionParams.voxelsPerMeter);

        hr = m_pVolume->ResetReconstruction(&m_worldToCameraTransform, &worldToVolumeTransform);
    }
    else
    {
        hr = m_pVolume->ResetReconstruction(&m_worldToCameraTransform, nullptr);
    }

    m_cLostFrameCounter = 0;
    m_cFrameCounter = 0;
    m_fStartTime = m_timer.AbsoluteTime();

    if (SUCCEEDED(hr))
    {
        m_bTrackingFailed = false;
		system("cls");
        cout<<"Reconstruction has been reset."<<endl;
    }
    else
    {
		system("cls");
        cout<<"Failed to reset reconstruction."<<endl;
    }

    return hr;
}

/// <summary>
/// Save Mesh to disk.
/// </summary>
/// <param name="mesh">The mesh to save.</param>
/// <returns>indicates success or failure</returns>
/*HRESULT KinectFusionProcessing::SaveMeshFile(INuiFusionColorMesh* pMesh)
{
    HRESULT hr = S_OK;

    if (nullptr == pMesh)
    {
        return E_INVALIDARG;
    }

    CComPtr<IFileSaveDialog> pSaveDlg;

    // Create the file save dialog object.
    hr = pSaveDlg.CoCreateInstance(__uuidof(FileSaveDialog));

    if (FAILED(hr))
    {
        return hr;
    }

    // Set the dialog title
    hr = pSaveDlg->SetTitle(L"Save Kinect Fusion Mesh");
    if (SUCCEEDED(hr))
    {
        // Set the button text
        hr = pSaveDlg->SetOkButtonLabel (L"Save");
        if (SUCCEEDED(hr))
        {
           
            if (Obj == saveMeshType)
            {
                hr = pSaveDlg->SetFileName(L"MeshedReconstruction.obj");
            }
            

            if (SUCCEEDED(hr))
            {
                // Set the file type extension
                if (Obj == saveMeshType)
                {
                    hr = pSaveDlg->SetDefaultExtension(L"obj");
                }
                

                if (SUCCEEDED(hr))
                {
                    // Set the file type filters
                    if (Obj == saveMeshType)
                    {
                        COMDLG_FILTERSPEC allPossibleFileTypes[] = {
                            { L"Obj mesh files", L"*.obj" },
                            { L"All files", L"*.*" }
                        };

                        hr = pSaveDlg->SetFileTypes(
                            ARRAYSIZE(allPossibleFileTypes),
                            allPossibleFileTypes );
                    }
                    

                    if (SUCCEEDED(hr))
                    {
                        // Show the file selection box
                        hr = pSaveDlg->Show(m_hWnd);

                        // Save the mesh to the chosen file.
                        if (SUCCEEDED(hr))
                        {
                            CComPtr<IShellItem> pItem;
                            hr = pSaveDlg->GetResult(&pItem);

                            if (SUCCEEDED(hr))
                            {
                                LPOLESTR pwsz = nullptr;
                                hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pwsz);

                                 if (Obj == saveMeshType)
                                    {
                                        hr = WriteAsciiObjMeshFile(pMesh, pwsz);
                                    }
                                   

                                    CoTaskMemFree(pwsz);
                                }
                            }
                        }
                    }
                }
            }
        }
    

    return hr;
}*/
