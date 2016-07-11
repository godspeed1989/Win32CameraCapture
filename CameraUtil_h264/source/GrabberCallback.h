#ifndef __GRABBERCALLBACK_H__
#define __GRABBERCALLBACK_H__
#include "Common.h"

#define x264_grab 1

#if x264_grab
#include "stdint.h"
#if defined ( __cplusplus)
extern "C"
{
#include "x264.h"
};
#else
#include "x264.h"
#endif
#endif

class SampleGrabberCallback : public ISampleGrabberCB
{
public:
	SampleGrabberCallback();
	ULONG STDMETHODCALLTYPE AddRef();
	ULONG STDMETHODCALLTYPE Release();
	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid,void** ppvObject);
	HRESULT STDMETHODCALLTYPE SampleCB(double Time, IMediaSample *pSample);
	HRESULT STDMETHODCALLTYPE BufferCB(double Time, BYTE *pBuffer, long BufferLen);
public:
	long m_lWidth;       //´æ´¢Í¼Æ¬µÄ¿í¶È
	long m_lHeight;		 //´æ´¢Í¼Æ¬µÄ³¤¶È
	int  m_iBitCount;    //the number of bits per pixel (bpp)
	//
	void GrabBMP(const WCHAR *pFileName);
	//
	void StartGrabVideo(const char *pFileName);
	void StopGrabVideo();
private:
	bool m_bGrabBMP;
	const WCHAR *m_pBMPFileName;
	BOOL SaveBitmap(BYTE *pBuffer, long lBufferSize);
#if x264_grab
	bool x264_do;
	int x264_i;
	int x264_iNal;
	x264_nal_t* x264_pNals;
	x264_t* x264_pHandle;
	x264_picture_t* x264_pPic_in;
	x264_picture_t* x264_pPic_out;
	x264_param_t* x264_pParam;
	FILE *x264_fp_dst;
	HANDLE x264_hMutex_encoding;
	void save264_yuv444_init(int w, int h, int bitcnt, const char *filename);
	void save264_yuv444_encode(BYTE *pBuffer);
	void save264_yuv444_destroy();
#endif
};

#endif //__GRABBERCALLBACK_H__
