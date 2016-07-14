#ifndef __CAPTUREVIDEO_H__
#define __CAPTUREVIDEO_H__
#include "Common.h"
#include "GrabberCallback.h"

class CaptureVideo
{
public:
	CaptureVideo();
	~CaptureVideo();
	// 枚举所有视频设备
	static UINT EnumAllDevices()
	{
		int nCaptureDeviceNumber = 0; //Device Count
		TCHAR pCapDeviceName[10][MAX_PATH]; //the Device name
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
				return 0;
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
						StringCchCopy(pCapDeviceName[nCaptureDeviceNumber],MAX_PATH,varTemp.bstrVal);
						wprintf(L"[%d] %s\n", nCaptureDeviceNumber, pCapDeviceName[nCaptureDeviceNumber]);
						nCaptureDeviceNumber++;
						SysFreeString(varTemp.bstrVal);
					}
					pProBag->Release();
				}
				pMoniker->Release();
			}
			pEnumMon->Release();
		}
		return nCaptureDeviceNumber;
	}
	// 打开、关闭设备
	HRESULT OpenDevice(int deviceID);
	void CloseDevice();
	// 开启、停止捕获
	HRESULT StartCapture();
	void StopCapture();
	// 捕获一张图片
	void GrabOneFrame(const WCHAR *pFileName);
	// 开启、停止捕获视频文件
	void StartGrabVideo(const char *pFileName);
	void StopGrabVideo();
public:
	BOOL m_bConnected;
	LONG m_lWidth, m_lHeight;
private:
	SampleGrabberCallback m_sampleGrabberCB;
	HRESULT InitializeEnv();
	HRESULT BindFilter(int deviceID, IBaseFilter **pBaseFilter);
	void CloseInterface();
private:
	IGraphBuilder *m_pGraphBuilder;
	ICaptureGraphBuilder2 *m_pCaptureGB;
	IMediaControl *m_pMediaControl;
	IBaseFilter *m_pDevFilter;
	ISampleGrabber *m_pSampGrabber;  
	IMediaEventEx *m_pMediaEvent;
	IVideoWindow *m_pVideoWindow;
};

#endif  //__CAPTUREVIDEO_H__
