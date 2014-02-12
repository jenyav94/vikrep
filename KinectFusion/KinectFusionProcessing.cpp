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
    m_pVolume(nullptr),
    m_pNuiSensor(nullptr),
    m_depthImageResolution(NUI_IMAGE_RESOLUTION_640x480),
    m_colorImageResolution(NUI_IMAGE_RESOLUTION_640x480),
    m_cDepthImagePixels(0),
    m_cColorImagePixels(0),
    m_hNextDepthFrameEvent(INVALID_HANDLE_VALUE),
    m_pDepthStreamHandle(INVALID_HANDLE_VALUE),
    m_bTranslateResetPoseByMinDepthThreshold(true),
    m_bAutoResetReconstructionWhenLost(false),
    m_bAutoResetReconstructionOnTimeout(true),
    m_cLostFrameCounter(0),
    m_bTrackingFailed(false),
    m_cFrameCounter(0),
    m_fStartTime(0),
    m_cLastDepthFrameTimeStamp(0),
    m_cLastColorFrameTimeStamp(0),
    m_pDepthImagePixelBuffer(nullptr),
    m_pColorCoordinates(nullptr),
    m_pMapper(nullptr),
    m_pDepthFloatImage(nullptr),
    m_pColorImage(nullptr),
    m_pResampledColorImageDepthAligned(nullptr),
    m_bInitializeError(false),
    m_bCaptureColor(true),
    m_cColorIntegrationInterval(2)  // Capturing color has an associated processing cost, so we do not capture every frame here
{
	// Get the depth frame size from the NUI_IMAGE_RESOLUTION enum
    // You can use NUI_IMAGE_RESOLUTION_640x480 or NUI_IMAGE_RESOLUTION_320x240 in this sample
    // Smaller resolutions will be faster in per-frame computations, but show less detail in reconstructions.
    DWORD width = 0, height = 0;
    NuiImageResolutionToSize(m_depthImageResolution, width, height);
    m_cDepthWidth = width;
    m_cDepthHeight = height;
    m_cDepthImagePixels = m_cDepthWidth*m_cDepthHeight;

	NuiImageResolutionToSize(m_colorImageResolution, width, height);
    m_cColorWidth  = width;
    m_cColorHeight = height;
    m_cColorImagePixels = m_cColorWidth*m_cColorHeight;

   
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

   
}

/// <summary>
/// Destructor
/// </summary>
KinectFusionProcessing::~KinectFusionProcessing()
{

	
	// Clean up Kinect Fusion
    SafeRelease(m_pVolume);
	SafeRelease(m_pMapper);

    SAFE_FUSION_RELEASE_IMAGE_FRAME(m_pDepthFloatImage);
	SAFE_FUSION_RELEASE_IMAGE_FRAME(m_pColorImage);
    SAFE_FUSION_RELEASE_IMAGE_FRAME(m_pResampledColorImageDepthAligned);

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

	 if (m_hNextColorFrameEvent != INVALID_HANDLE_VALUE)
    {
        CloseHandle(m_hNextColorFrameEvent);
    }


    // clean up the depth pixel array
   SAFE_DELETE_ARRAY(m_pDepthImagePixelBuffer);

    // Clean up the color pixel arrays
    SAFE_DELETE_ARRAY(m_pColorCoordinates);

    // clean up Direct2D renderer
  //SAFE_DELETE(m_pDrawDepth);

    // done with depth pixel data
   // SAFE_DELETE_ARRAY(m_pDepthRGBX);

    // clean up Direct2D
  //  SafeRelease(m_pD2DFactory);
}

HRESULT WriteAsciiPlyMeshFile(INuiFusionColorMesh *mesh, bool flipYZ, bool outputColor)
{
    HRESULT hr = S_OK;

    if (NULL == mesh)
    {
        return E_INVALIDARG;
    }

    unsigned int numVertices = mesh->VertexCount();
    unsigned int numTriangleIndices = mesh->TriangleVertexIndexCount();
    unsigned int numTriangles = numVertices / 3;
    unsigned int numColors = mesh->ColorCount();

    if (0 == numVertices || 0 == numTriangleIndices || 0 != numVertices % 3 
        || numVertices != numTriangleIndices  || (outputColor && numVertices != numColors))
    {
        return E_INVALIDARG;
    }

    const Vector3 *vertices = NULL;
    hr = mesh->GetVertices(&vertices);
    if (FAILED(hr))
    {
        return hr;
    }

    const int *triangleIndices = NULL;
    hr = mesh->GetTriangleIndices(&triangleIndices);
    if (FAILED(hr))
    {
        return hr;
    }

    const int *colors = NULL;
    if (outputColor)
    {
        hr = mesh->GetColors(&colors);
        if (FAILED(hr))
        {
            return hr;
        }
    }

    // Open File
    
    
    FILE *meshFile = NULL;
    errno_t err = fopen_s(&meshFile, "MeshedReconstruction.ply", "wt");

    // Could not open file for writing - return
    if (0 != err || NULL == meshFile)
    {
        return E_ACCESSDENIED;
    }

    // Write the header line
    std::string header = "ply\nformat ascii 1.0\ncomment file created by Microsoft Kinect Fusion\n";
    fwrite(header.c_str(), sizeof(char), header.length(), meshFile);

    const unsigned int bufSize = MAX_PATH*3;
    char outStr[bufSize];
    int written = 0;

    if (outputColor)
    {
        // Elements are: x,y,z, r,g,b
        written = sprintf_s(outStr, bufSize, "element vertex %u\nproperty float x\nproperty float y\nproperty float z\nproperty uchar red\nproperty uchar green\nproperty uchar blue\n", numVertices);
        fwrite(outStr, sizeof(char), written, meshFile);
    }
    else
    {
        // Elements are: x,y,z
        written = sprintf_s(outStr, bufSize, "element vertex %u\nproperty float x\nproperty float y\nproperty float z\n", numVertices);
        fwrite(outStr, sizeof(char), written, meshFile);
    }

    written = sprintf_s(outStr, bufSize, "element face %u\nproperty list uchar int vertex_index\nend_header\n", numTriangles);
    fwrite(outStr, sizeof(char), written, meshFile);

    if (flipYZ)
    {
        if (outputColor)
        {
            // Sequentially write the 3 vertices of the triangle, for each triangle
            for (unsigned int t=0, vertexIndex=0; t < numTriangles; ++t, vertexIndex += 3)
            {
                unsigned int color0 = colors[vertexIndex];
                unsigned int color1 = colors[vertexIndex+1];
                unsigned int color2 = colors[vertexIndex+2];

                written = sprintf_s(outStr, bufSize, "%f %f %f %u %u %u\n%f %f %f %u %u %u\n%f %f %f %u %u %u\n", 
                    vertices[vertexIndex].x, -vertices[vertexIndex].y, -vertices[vertexIndex].z, 
                    ((color0 >> 16) & 255), ((color0 >> 8) & 255), (color0 & 255), 
                    vertices[vertexIndex+1].x, -vertices[vertexIndex+1].y, -vertices[vertexIndex+1].z,
                    ((color1 >> 16) & 255), ((color1 >> 8) & 255), (color1 & 255), 
                    vertices[vertexIndex+2].x, -vertices[vertexIndex+2].y, -vertices[vertexIndex+2].z,
                    ((color2 >> 16) & 255), ((color2 >> 8) & 255), (color2 & 255));

                fwrite(outStr, sizeof(char), written, meshFile);
            }
        }
        else
        {
            // Sequentially write the 3 vertices of the triangle, for each triangle
            for (unsigned int t=0, vertexIndex=0; t < numTriangles; ++t, vertexIndex += 3)
            {
                written = sprintf_s(outStr, bufSize, "%f %f %f\n%f %f %f\n%f %f %f\n", 
                    vertices[vertexIndex].x, -vertices[vertexIndex].y, -vertices[vertexIndex].z, 
                    vertices[vertexIndex+1].x, -vertices[vertexIndex+1].y, -vertices[vertexIndex+1].z, 
                    vertices[vertexIndex+2].x, -vertices[vertexIndex+2].y, -vertices[vertexIndex+2].z);
                fwrite(outStr, sizeof(char), written, meshFile);
            }
        }
    }
    else
    {
        if (outputColor)
        {
            // Sequentially write the 3 vertices of the triangle, for each triangle
            for (unsigned int t=0, vertexIndex=0; t < numTriangles; ++t, vertexIndex += 3)
            {
                unsigned int color0 = colors[vertexIndex];
                unsigned int color1 = colors[vertexIndex+1];
                unsigned int color2 = colors[vertexIndex+2];

                written = sprintf_s(outStr, bufSize, "%f %f %f %u %u %u\n%f %f %f %u %u %u\n%f %f %f %u %u %u\n", 
                    vertices[vertexIndex].x, vertices[vertexIndex].y, vertices[vertexIndex].z, 
                    ((color0 >> 16) & 255), ((color0 >> 8) & 255), (color0 & 255), 
                    vertices[vertexIndex+1].x, vertices[vertexIndex+1].y, vertices[vertexIndex+1].z,
                    ((color1 >> 16) & 255), ((color1 >> 8) & 255), (color1 & 255), 
                    vertices[vertexIndex+2].x, vertices[vertexIndex+2].y, vertices[vertexIndex+2].z,
                    ((color2 >> 16) & 255), ((color2 >> 8) & 255), (color2 & 255));

                fwrite(outStr, sizeof(char), written, meshFile);
            }
        }
        else
        {
            // Sequentially write the 3 vertices of the triangle, for each triangle
            for (unsigned int t=0, vertexIndex=0; t < numTriangles; ++t, vertexIndex += 3)
            {
                written = sprintf_s(outStr, bufSize, "%f %f %f\n%f %f %f\n%f %f %f\n", 
                    vertices[vertexIndex].x, vertices[vertexIndex].y, vertices[vertexIndex].z, 
                    vertices[vertexIndex+1].x, vertices[vertexIndex+1].y, vertices[vertexIndex+1].z,
                    vertices[vertexIndex+2].x, vertices[vertexIndex+2].y, vertices[vertexIndex+2].z);
                fwrite(outStr, sizeof(char), written, meshFile);
            }
        }
    }

    // Sequentially write the 3 vertex indices of the triangle face, for each triangle (0-referenced in PLY)
    for (unsigned int t=0, baseIndex=0; t < numTriangles; ++t, baseIndex += 3)
    {
        written = sprintf_s(outStr, bufSize, "3 %u %u %u\n", baseIndex, baseIndex+1, baseIndex+2);
        fwrite(outStr, sizeof(char), written, meshFile);
    }

    fflush(meshFile);
    fclose(meshFile);

    return hr;
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
		HRESULT hr;
		INuiFusionColorMesh *mesh = nullptr;
		
		
		 if (m_pVolume != nullptr)
      {
         hr = m_pVolume->CalculateMesh(1, &mesh);

         // Set the frame counter to 0 to prevent a reset reconstruction call due to large frame 
         // timestamp change after meshing. Also reset frame time for fps counter.
         m_cFrameCounter = 0;
         //m_fFrameCounterStartTime =  m_timer.AbsoluteTime();
      }

		  if (SUCCEEDED(hr))
        {
            // Save mesh
            hr =WriteAsciiPlyMeshFile(mesh, true, true);
		}

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
        hr = m_pNuiSensor->NuiInitialize(NUI_INITIALIZE_FLAG_USES_DEPTH  | NUI_INITIALIZE_FLAG_USES_COLOR); 
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

			 if (SUCCEEDED(hr))
            {
                // Open a color image stream to receive color frames
                hr = m_pNuiSensor->NuiImageStreamOpen(
                    NUI_IMAGE_TYPE_COLOR,
                    m_colorImageResolution,
                    0,
                    2,
                    m_hNextColorFrameEvent,
                    &m_pColorStreamHandle);
            }

            if (SUCCEEDED(hr))
            {
                // Create the coordinate mapper for converting color to depth space
                hr = m_pNuiSensor->NuiGetCoordinateMapper(&m_pMapper);
            }
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
    hr = NuiFusionCreateColorReconstruction(
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

	 // Frames generated from the color input
    hr = NuiFusionCreateImageFrame(NUI_FUSION_IMAGE_TYPE_COLOR, m_cColorWidth, m_cColorHeight, nullptr, &m_pColorImage);
    if (FAILED(hr))
    {
		system("cls");
        cout<<"Failed to initialize Kinect Fusion image."<<endl;
        return hr;
    }

    // Frames generated from the color input aligned to depth - same size as depth
    hr = NuiFusionCreateImageFrame(NUI_FUSION_IMAGE_TYPE_COLOR, m_cDepthWidth, m_cDepthHeight, nullptr, &m_pResampledColorImageDepthAligned);
    if (FAILED(hr))
    {
		system("cls");
        cout<<"Failed to initialize Kinect Fusion image."<<endl;
        return hr;
    }

    
	// Depth pixel array to capture data from Kinect sensor
    m_pDepthImagePixelBuffer = new(std::nothrow) NUI_DEPTH_IMAGE_PIXEL[m_cDepthImagePixels];
    if (nullptr == m_pDepthImagePixelBuffer)
    {
		system("cls");
        cout<<"Failed to initialize Kinect Fusion depth image pixel buffer."<<endl;
        return hr;
    }

	 // Setup color coordinate image for depth to color mapping - this must be the same size as the depth
    m_pColorCoordinates = new(std::nothrow) NUI_COLOR_IMAGE_POINT[m_cDepthImagePixels];
    if (nullptr == m_pColorCoordinates)
    {
		system("cls");
        cout<<"Failed to initialize Kinect Fusion color image pixel buffer."<<endl;
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
/// Get Color data
/// </summary>
/// <param name="imageFrame">The color image frame to copy.</param>
/// <returns>S_OK on success, otherwise failure code</returns>
HRESULT KinectFusionProcessing::CopyColor(NUI_IMAGE_FRAME &imageFrame)
{
    HRESULT hr = S_OK;

    if (nullptr == m_pColorImage)
    {
		system("cls");
        cout<<"Error copying color texture pixels."<<endl;
        return E_FAIL;
    }

    INuiFrameTexture *srcColorTex = imageFrame.pFrameTexture;
    INuiFrameTexture *destColorTex = m_pColorImage->pFrameTexture;

    if (nullptr == srcColorTex || nullptr == destColorTex)
    {
        return E_NOINTERFACE;
    }

    // Lock the frame data to access the color pixels
    NUI_LOCKED_RECT srcLockedRect;

    hr = srcColorTex->LockRect(0, &srcLockedRect, nullptr, 0);

    if (FAILED(hr) || srcLockedRect.Pitch == 0)
    {
		system("cls");
        cout<<"Error getting color texture pixels."<<endl;
        return E_NOINTERFACE;
    }

    // Lock the frame data to access the color pixels
    NUI_LOCKED_RECT destLockedRect;

    hr = destColorTex->LockRect(0, &destLockedRect, nullptr, 0);

    if (FAILED(hr) || destLockedRect.Pitch == 0)
    {
        srcColorTex->UnlockRect(0);
		system("cls");
        cout<<"Error copying color texture pixels."<<endl;
        return E_NOINTERFACE;
    }

    // Copy the color pixels so we can return the image frame
    errno_t err = memcpy_s(
        destLockedRect.pBits, 
        m_cColorImagePixels * cBytesPerPixel,
        srcLockedRect.pBits,
        srcLockedRect.size);

    srcColorTex->UnlockRect(0);
    destColorTex->UnlockRect(0);

    if (0 != err)
    {
		system("cls");
        cout<<"Error copying color texture pixels."<<endl;
        hr = E_FAIL;
    }

    return hr;
}

/// <summary>
/// Process color data received from Kinect
/// </summary>
/// <returns>S_OK for success, or failure code</returns>
HRESULT KinectFusionProcessing::MapColorToDepth()
{
    HRESULT hr;

    if (nullptr == m_pColorImage || nullptr == m_pResampledColorImageDepthAligned 
        || nullptr == m_pDepthImagePixelBuffer || nullptr == m_pColorCoordinates)
    {
        return E_FAIL;
    }

    INuiFrameTexture *srcColorTex = m_pColorImage->pFrameTexture;
    INuiFrameTexture *destColorTex = m_pResampledColorImageDepthAligned->pFrameTexture;

    if (nullptr == srcColorTex || nullptr == destColorTex)
    {
		system("cls");
        cout<<"Error accessing color textures."<<endl;
        return E_NOINTERFACE;
    }

    // Lock the source color frame
    NUI_LOCKED_RECT srcLockedRect;

    // Lock the frame data to access the color pixels
    hr = srcColorTex->LockRect(0, &srcLockedRect, nullptr, 0);

    if (FAILED(hr) || srcLockedRect.Pitch == 0)
    {
		system("cls");
        cout<<"Error accessing color texture pixels."<<endl;
        return  E_FAIL;
    }

    // Lock the destination color frame
    NUI_LOCKED_RECT destLockedRect;

    // Lock the frame data to access the color pixels
    hr = destColorTex->LockRect(0, &destLockedRect, nullptr, 0);

    if (FAILED(hr) || destLockedRect.Pitch == 0)
    {
        srcColorTex->UnlockRect(0);
		system("cls");
        cout<<"Error accessing color texture pixels."<<endl;
        return  E_FAIL;
    }

    int *rawColorData = reinterpret_cast<int*>(srcLockedRect.pBits);
    int *colorDataInDepthFrame = reinterpret_cast<int*>(destLockedRect.pBits);

    // Get the coordinates to convert color to depth space
    hr = m_pMapper->MapDepthFrameToColorFrame(
        m_depthImageResolution, 
        m_cDepthImagePixels, 
        m_pDepthImagePixelBuffer, 
        NUI_IMAGE_TYPE_COLOR, 
        m_colorImageResolution, 
        m_cDepthImagePixels,   // the color coordinates that get set are the same array size as the depth image
        m_pColorCoordinates);

    if (FAILED(hr))
    {
        srcColorTex->UnlockRect(0);
        destColorTex->UnlockRect(0);
        return hr;
    }

    // Loop over each row and column of the destination color image and copy from the source image
    // Note that we could also do this the other way, and convert the depth pixels into the color space, 
    // avoiding black areas in the converted color image and repeated color images in the background.
    // However, then the depth would have radial and tangential distortion like the color camera image,
    // which is not ideal for Kinect Fusion reconstruction.
    Concurrency::parallel_for(0, static_cast<int>(m_cDepthHeight), [&](int y)
    {
        // Horizontal flip the color image as the standard depth image is flipped internally in Kinect Fusion
        // to give a viewpoint as though from behind the Kinect looking forward by default.
        unsigned int destIndex = y * m_cDepthWidth;
        unsigned int flippedDestIndex = destIndex + (m_cDepthWidth-1);

        for (int x = 0; x < m_cDepthWidth; ++x, ++destIndex, --flippedDestIndex)
        {
            // Calculate index into depth array
            int colorInDepthX = m_pColorCoordinates[destIndex].x;
            int colorInDepthY = m_pColorCoordinates[destIndex].y;

            // Make sure the depth pixel maps to a valid point in color space
            // Depth and color images are the same size in this sample, so we use the depth image size here.
            // For a more flexible version, see the KinectFusionExplorer-D2D sample.
            if ( colorInDepthX >= 0 && colorInDepthX < m_cColorWidth 
                && colorInDepthY >= 0 && colorInDepthY < m_cColorHeight 
                && m_pDepthImagePixelBuffer[destIndex].depth != 0)
            {
                // Calculate index into color array- this will perform a horizontal flip as well
                unsigned int sourceColorIndex = colorInDepthX + (colorInDepthY * m_cColorWidth);

                // Copy color pixel
                colorDataInDepthFrame[flippedDestIndex] = rawColorData[sourceColorIndex];
            }
            else
            {
                colorDataInDepthFrame[flippedDestIndex] = 0;
            }
        }
    });

    srcColorTex->UnlockRect(0);
    destColorTex->UnlockRect(0);

    return hr;
}


/// <summary>
/// Perform only depth conversion and camera tracking
/// </summary>
HRESULT KinectFusionProcessing::CameraTrackingOnly()
{
    // Convert the pixels describing extended depth as unsigned short type in millimeters to depth
    // as floating point type in meters.
    HRESULT hr = m_pVolume->DepthToDepthFloatFrame(m_pDepthImagePixelBuffer, m_cDepthImagePixels * sizeof(NUI_DEPTH_IMAGE_PIXEL), m_pDepthFloatImage, m_fMinDepthThreshold, m_fMaxDepthThreshold, m_bMirrorDepthFrame);

    if (FAILED(hr))
    {
		system("cls");
        cout<<"Kinect Fusion NuiFusionDepthToDepthFloatFrame call failed."<<endl;
        return hr;
    }

    HRESULT tracking = m_pVolume->AlignDepthFloatToReconstruction(
        m_pDepthFloatImage,
        NUI_FUSION_DEFAULT_ALIGN_ITERATION_COUNT,
        nullptr,
        nullptr,
        nullptr);

    if (FAILED(tracking))
    {
        m_cLostFrameCounter++;
        m_bTrackingFailed = true;

        if (tracking == E_NUI_FUSION_TRACKING_ERROR)
        {
                system("cls");
                cout<<"Kinect Fusion camera tracking failed! Align the camera to the last tracked position."<<endl;
        }
        else
        {
			system("cls");
            cout<<"Kinect Fusion AlignDepthFloatToReconstruction call failed!"<<endl;
            hr = tracking;
        }
    }
    else
    {
        m_pVolume->GetCurrentWorldToCameraTransform(&m_worldToCameraTransform);
        m_cLostFrameCounter = 0;
        m_bTrackingFailed = false;
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
    bool integrateColor = m_bCaptureColor && m_cFrameCounter % m_cColorIntegrationInterval == 0;

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

    LONGLONG currentDepthFrameTime = imageFrame.liTimeStamp.QuadPart;

    // Release the Kinect camera frame
    m_pNuiSensor->NuiImageStreamReleaseFrame(m_pDepthStreamHandle, &imageFrame);

    if (FAILED(hr))
    {
        return;
    }

	 ////////////////////////////////////////////////////////
    // Get a color frame from Kinect

    LONGLONG currentColorFrameTime = m_cLastColorFrameTimeStamp;

    hr = m_pNuiSensor->NuiImageStreamGetNextFrame(m_pColorStreamHandle, 0, &imageFrame);
    if (FAILED(hr))
    {
        integrateColor = false;
    }
    else
    {
        hr = CopyColor(imageFrame);

        currentColorFrameTime = imageFrame.liTimeStamp.QuadPart;

        // Release the Kinect camera frame
        m_pNuiSensor->NuiImageStreamReleaseFrame(m_pColorStreamHandle, &imageFrame);

        if (FAILED(hr))
        {
            return;
        }
    }

    // Check color and depth frame timestamps to ensure they were captured at the same time
    // If not, we attempt to re-synchronize by getting a new frame from the stream that is behind.
    int timestampDiff = static_cast<int>(abs(currentColorFrameTime - currentDepthFrameTime));

    if (integrateColor && timestampDiff >= cMinTimestampDifferenceForFrameReSync)
    {
        // Get another frame to try and re-sync
        if (currentColorFrameTime - currentDepthFrameTime >= cMinTimestampDifferenceForFrameReSync)
        {
            // Perform camera tracking only from this current depth frame
            if (m_cFrameCounter > 0)
            {
                CameraTrackingOnly();
            }

            // Get another depth frame to try and re-sync as color ahead of depth
            hr = m_pNuiSensor->NuiImageStreamGetNextFrame(m_pDepthStreamHandle, timestampDiff, &imageFrame);
            if (FAILED(hr))
            {
                // Return, having performed camera tracking on the current depth frame
                return;
            }

            hr = CopyExtendedDepth(imageFrame);

            currentDepthFrameTime = imageFrame.liTimeStamp.QuadPart;

            // Release the Kinect camera frame
            m_pNuiSensor->NuiImageStreamReleaseFrame(m_pDepthStreamHandle, &imageFrame);

            if (FAILED(hr))
            {
				system("cls");
                cout<<"Kinect Depth stream NuiImageStreamReleaseFrame call failed."<<endl;
                return;
            }
        }
        else if (currentDepthFrameTime - currentColorFrameTime >= cMinTimestampDifferenceForFrameReSync && WaitForSingleObject(m_hNextColorFrameEvent, 0) != WAIT_TIMEOUT)
        {
            // Get another color frame to try and re-sync as depth ahead of color
            hr = m_pNuiSensor->NuiImageStreamGetNextFrame(m_pColorStreamHandle, 0, &imageFrame);
            if (FAILED(hr))
            {
                integrateColor = false;
            }
            else
            {
                hr = CopyColor(imageFrame);

                currentColorFrameTime = imageFrame.liTimeStamp.QuadPart;

                // Release the Kinect camera frame
                m_pNuiSensor->NuiImageStreamReleaseFrame(m_pColorStreamHandle, &imageFrame);

                if (FAILED(hr))
                {
					system("cls");
                    cout<<"Kinect Color stream NuiImageStreamReleaseFrame call failed."<<endl;
                    integrateColor = false;
                }
            }
        }

        timestampDiff = static_cast<int>(abs(currentColorFrameTime - currentDepthFrameTime));

        // If the difference is still too large, we do not want to integrate color
        if (timestampDiff > cMinTimestampDifferenceForFrameReSync)
        {
            integrateColor = false;
        }
    }

    // To enable playback of a .xed file through Kinect Studio and reset of the reconstruction
    // if the .xed loops, we test for when the frame timestamp has skipped a large number. 
    // Note: this will potentially continually reset live reconstructions on slow machines which
    // cannot process a live frame in less time than the reset threshold. Increase the number of
    // milliseconds in cResetOnTimeStampSkippedMilliseconds if this is a problem.
    if (m_bAutoResetReconstructionOnTimeout && m_cFrameCounter != 0 
        && abs(currentDepthFrameTime - m_cLastDepthFrameTimeStamp) > cResetOnTimeStampSkippedMilliseconds)
    {
        ResetReconstruction();

        if (FAILED(hr))
        {
            return;
        }
    }

    m_cLastDepthFrameTimeStamp = currentDepthFrameTime;
	m_cLastColorFrameTimeStamp = currentColorFrameTime;

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

	if (integrateColor)
    {
        // Map the color frame to the depth
        MapColorToDepth();
    }

    // Perform the camera tracking and update the Kinect Fusion Volume
    // This will create memory on the GPU, upload the image, run camera tracking and integrate the
    // data into the Reconstruction Volume if successful. Note that passing nullptr as the final 
    // parameter will use and update the internal camera pose.
   hr = m_pVolume->ProcessFrame(
        m_pDepthFloatImage, 
        integrateColor ? m_pResampledColorImageDepthAligned : nullptr,
        NUI_FUSION_DEFAULT_ALIGN_ITERATION_COUNT, 
        m_cMaxIntegrationWeight, 
        NUI_FUSION_DEFAULT_COLOR_INTEGRATION_OF_ALL_ANGLES,
        &m_worldToCameraTransform);

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

