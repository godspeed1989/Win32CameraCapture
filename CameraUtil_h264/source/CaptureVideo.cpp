#include "CaptureVideo.h"

SampleGrabberCallback g_sampleGrabberCB;

CaptureVideo::CaptureVideo()
{
	//COM Library Initialize
	if (FAILED(CoInitialize(NULL)))
	{
		return;
	}
	//initialize member variable
	m_nCaptureDeviceNumber = 0;
	m_pDevFilter = NULL;
	m_pCaptureGB = NULL;
	m_pGraphBuilder = NULL;
	m_pMediaControl = NULL;
	m_pMediaEvent = NULL;
	m_pSampGrabber = NULL;
	m_pVideoWindow = NULL;
	m_bConnected = FALSE;
	InitializeEnv();
}

CaptureVideo::~CaptureVideo()
{
	CloseInterface();
	CoUninitialize();
}

HRESULT CaptureVideo::EnumAllDevices()
{
	ICreateDevEnum *pDevEnum;
	IEnumMoniker   *pEnumMon;
	IMoniker	   *pMoniker;
	HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum,NULL,CLSCTX_INPROC_SERVER,
			IID_ICreateDevEnum,(LPVOID*)&pDevEnum);
	if (SUCCEEDED(hr))
	{
		hr = pDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory,&pEnumMon, 0);
		if (hr == S_FALSE)
		{
			hr = VFW_E_NOT_FOUND;
			return hr;
		}
		pEnumMon->Reset();
		ULONG cFetched;
		while(hr=pEnumMon->Next(1,&pMoniker,&cFetched),hr == S_OK)
		{
			IPropertyBag *pProBag;
			hr = pMoniker->BindToStorage(0,0,IID_IPropertyBag,(LPVOID*)&pProBag);
			if (SUCCEEDED(hr))
			{
				VARIANT varTemp;
				varTemp.vt = VT_BSTR;
				hr = pProBag->Read(L"FriendlyName",&varTemp,NULL);
				if (SUCCEEDED(hr))
				{
					StringCchCopy(m_pCapDeviceName[m_nCaptureDeviceNumber],MAX_PATH,varTemp.bstrVal);
					m_nCaptureDeviceNumber++;
					SysFreeString(varTemp.bstrVal);
				}
				pProBag->Release();
			}
			pMoniker->Release();
		}
		pEnumMon->Release();
	}
	return hr;
}

HRESULT CaptureVideo::InitializeEnv()
{
	HRESULT hr;

	//Create the filter graph
	hr = CoCreateInstance(CLSID_FilterGraph,NULL,CLSCTX_INPROC_SERVER,
						  IID_IGraphBuilder,(LPVOID*)&m_pGraphBuilder);
	if(FAILED(hr))
		return hr;

	//Create the capture graph builder
	hr = CoCreateInstance(CLSID_CaptureGraphBuilder2,NULL,CLSCTX_INPROC_SERVER,
						  IID_ICaptureGraphBuilder2,(LPVOID*)&m_pCaptureGB);
	if(FAILED(hr))
		return hr;

	//Obtain interfaces for media control and Video Window
	hr = m_pGraphBuilder->QueryInterface(IID_IMediaControl,(LPVOID*)&m_pMediaControl);
	if(FAILED(hr))
		return hr;

	hr = m_pGraphBuilder->QueryInterface(IID_IVideoWindow,(LPVOID*)&m_pVideoWindow);
	if(FAILED(hr))
		return hr;
	m_pVideoWindow->put_Owner(NULL);

	hr = m_pGraphBuilder->QueryInterface(IID_IMediaEventEx,(LPVOID*)&m_pMediaEvent);
	if(FAILED(hr))
		return hr;

	m_pCaptureGB->SetFiltergraph(m_pGraphBuilder);
	if(FAILED(hr))
		return hr;
	return hr;
}

void CaptureVideo::CloseInterface()
{
	if (m_pMediaControl)
	{
		m_pMediaControl->Stop();
	}
	if(m_pVideoWindow)
	{
		m_pVideoWindow->put_Visible(OAFALSE);
	}

	if(m_pMediaEvent)
	{
		m_pMediaEvent->SetNotifyWindow(NULL,WM_GRAPHNOTIFY,0);
	}
	m_bConnected = FALSE;
	//release interface
	ReleaseInterface(m_pDevFilter);
	ReleaseInterface(m_pCaptureGB);
	ReleaseInterface(m_pGraphBuilder);
	ReleaseInterface(m_pMediaControl);
	ReleaseInterface(m_pMediaEvent);
	ReleaseInterface(m_pSampGrabber);
	ReleaseInterface(m_pVideoWindow);
}

HRESULT CaptureVideo::BindFilter(int deviceID, IBaseFilter **pBaseFilter)
{
	ICreateDevEnum *pDevEnum;
	IEnumMoniker   *pEnumMon;
	IMoniker	   *pMoniker;
	HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum,NULL,CLSCTX_INPROC_SERVER,
		IID_ICreateDevEnum,(LPVOID*)&pDevEnum);
	if (SUCCEEDED(hr))
	{
		hr = pDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory,&pEnumMon, 0);
		if (hr == S_FALSE)
		{
			hr = VFW_E_NOT_FOUND;
			return hr;
		}
		pEnumMon->Reset();
		ULONG cFetched;
		int index = 0;
		hr = pEnumMon->Next(1,&pMoniker,&cFetched);
		while (hr == S_OK && index <= deviceID)
		{
			IPropertyBag *pProBag;
			hr = pMoniker->BindToStorage(0,0,IID_IPropertyBag,(LPVOID*)&pProBag);
			if (SUCCEEDED(hr))
			{
				if (index == deviceID)
				{
					pMoniker->BindToObject(0,0,IID_IBaseFilter,(LPVOID*)pBaseFilter);
				}
			}
			pMoniker->Release();
			index++;
			hr = pEnumMon->Next(1,&pMoniker,&cFetched);
		}
		pEnumMon->Release();
	}
	return hr;
}

HRESULT CaptureVideo::OpenDevice(int deviceID)
{
	HRESULT hr;
	IBaseFilter *pSampleGrabberFilter;

	if (m_bConnected)
	{
		CloseInterface();
		InitializeEnv();
	}

	// create grabber filter instance
	hr = CoCreateInstance(CLSID_SampleGrabber,NULL,CLSCTX_INPROC_SERVER,
						  IID_IBaseFilter, (LPVOID*)&pSampleGrabberFilter);
	if(FAILED(hr))
		return hr;

	// bind source device
	hr = BindFilter(deviceID, &m_pDevFilter);
	if (FAILED(hr))
		return hr;

	// add src filter
	hr = m_pGraphBuilder->AddFilter(m_pDevFilter, L"Video Filter");
	if (FAILED(hr))
		return hr;
	// add grabber filter and query interface
	hr = m_pGraphBuilder->AddFilter(pSampleGrabberFilter,L"Sample Grabber");
	if (FAILED(hr))
		return hr;
	hr = pSampleGrabberFilter->QueryInterface(IID_ISampleGrabber,(LPVOID*)&m_pSampGrabber);
	if(FAILED(hr))
		return hr;

	// find the current bit depth
	HDC hdc=GetDC(NULL);
	int iBitDepth=GetDeviceCaps(hdc, BITSPIXEL);
	g_sampleGrabberCB.m_iBitCount = iBitDepth;
	ReleaseDC(NULL,hdc);

	// set the media type for grabber filter
	AM_MEDIA_TYPE mediaType;
	ZeroMemory(&mediaType,sizeof(AM_MEDIA_TYPE));
	mediaType.majortype = MEDIATYPE_Video;
	switch(iBitDepth)
	{
	case  8:
		mediaType.subtype=MEDIASUBTYPE_RGB8;
		break;
	case 16:
		mediaType.subtype=MEDIASUBTYPE_RGB555;
		break;
	case 24:
		mediaType.subtype=MEDIASUBTYPE_RGB24;
		break;
	case 32:
		mediaType.subtype=MEDIASUBTYPE_RGB32;
		break;
	default:
		return E_FAIL;
	}
	mediaType.formattype = FORMAT_VideoInfo;
	hr = m_pSampGrabber->SetMediaType(&mediaType);

	// connect source filter to grabber filter
	hr = m_pCaptureGB->RenderStream(&PIN_CATEGORY_PREVIEW,&MEDIATYPE_Video,
		m_pDevFilter,pSampleGrabberFilter,NULL);
	if(FAILED(hr))
		return hr;

	// get connected media type
	hr = m_pSampGrabber->GetConnectedMediaType(&mediaType);
	if(FAILED(hr))
		return hr;
	VIDEOINFOHEADER * vih = (VIDEOINFOHEADER*) mediaType.pbFormat;
	m_lWidth = vih->bmiHeader.biWidth;
	m_lHeight = vih->bmiHeader.biHeight;
	g_sampleGrabberCB.m_lWidth = vih->bmiHeader.biWidth;
	g_sampleGrabberCB.m_lHeight = vih->bmiHeader.biHeight;
	
	// configure grabber filter
	hr = m_pSampGrabber->SetOneShot(FALSE);
	if (FAILED(hr))
		return hr;
	hr = m_pSampGrabber->SetBufferSamples(TRUE);
	if (FAILED(hr))
		return hr;

	// Use the BufferCB callback method
	hr = m_pSampGrabber->SetCallback(&g_sampleGrabberCB, 1);

	// release resource
	if (mediaType.cbFormat != 0)
	{
		CoTaskMemFree((PVOID)mediaType.pbFormat);
		mediaType.cbFormat = 0;
		mediaType.pbFormat = NULL;
	}
	if (mediaType.pUnk != NULL)
	{
		mediaType.pUnk->Release();
		mediaType.pUnk = NULL;
	}
	m_bConnected = TRUE;
	return hr;
}

void CaptureVideo::CloseDevice()
{
	CloseInterface();
	InitializeEnv();
}

HRESULT CaptureVideo::StartCapture()
{
	HRESULT hr;
	if (m_bConnected == FALSE)
		return (HRESULT)-1;
	hr = m_pMediaControl->Run();
	if(FAILED(hr))
		return hr;
	m_pVideoWindow->put_Visible(OAFALSE);
	return hr;
}

void CaptureVideo::StopCapture()
{
	if (m_bConnected == FALSE)
		return;
	if (m_pMediaControl)
	{
		m_pMediaControl->Stop();
	}
	if(m_pVideoWindow)
	{
		m_pVideoWindow->put_Visible(OAFALSE);
	}
}

void CaptureVideo::GrabOneFrame(const WCHAR *pFileName)
{
	if (m_bConnected == FALSE)
		return;
	g_sampleGrabberCB.GrabBMP(pFileName);
}

void CaptureVideo::StartGrabVideo(const char *pFileName)
{
	if (m_bConnected == FALSE)
		return;
	g_sampleGrabberCB.StartGrabVideo(pFileName);
}

void CaptureVideo::StopGrabVideo()
{
	if (m_bConnected == FALSE)
		return;
	g_sampleGrabberCB.StopGrabVideo();
}
