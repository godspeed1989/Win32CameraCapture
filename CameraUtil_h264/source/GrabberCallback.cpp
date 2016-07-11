#include "GrabberCallback.h"
#include <stdlib.h>

SampleGrabberCallback::SampleGrabberCallback()
{
#if x264_grab
	x264_do = false;
	x264_i = 0;
#endif
}

ULONG STDMETHODCALLTYPE SampleGrabberCallback::AddRef()
{
	return 1;
}

ULONG STDMETHODCALLTYPE SampleGrabberCallback::Release()
{
	return 2;
}

HRESULT STDMETHODCALLTYPE SampleGrabberCallback::QueryInterface(REFIID riid,void** ppvObject)
{
	if (NULL == ppvObject) return E_POINTER;
	if (riid == __uuidof(IUnknown))
	{
		*ppvObject = static_cast<IUnknown*>(this);
		return S_OK;
	}
	if (riid == IID_ISampleGrabberCB)
	{
		*ppvObject = static_cast<ISampleGrabberCB*>(this);
		return S_OK;
	}
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE SampleGrabberCallback::SampleCB(double Time, IMediaSample *pSample)
{
	return E_NOTIMPL;
}

HRESULT STDMETHODCALLTYPE SampleGrabberCallback::BufferCB(double Time, BYTE *pBuffer, long BufferLen)
{
	if(!pBuffer)
		return E_POINTER;

#if x264_grab
	if (x264_do == true)
	{
		save264_yuv444_encode(pBuffer);
	}
#endif
	if (m_bGrabBMP == true)
	{
		SaveBitmap(pBuffer, BufferLen);
		m_bGrabBMP = false;
	}

	return S_OK;
}

void SampleGrabberCallback::GrabBMP(const WCHAR *pFileName)
{
	m_bGrabBMP = true;
	m_pBMPFileName = pFileName;
}

BOOL SampleGrabberCallback::SaveBitmap(BYTE *pBuffer, long lBufferSize)
{
	HANDLE hf = CreateFile(m_pBMPFileName, GENERIC_WRITE,FILE_SHARE_WRITE,NULL,CREATE_ALWAYS,0,NULL);
	if(hf == INVALID_HANDLE_VALUE)
	{
		return E_FAIL;
	}

	BITMAPFILEHEADER bfh;  //Set bitmap header
	ZeroMemory(&bfh,sizeof(bfh));
	bfh.bfType = 'MB';
	bfh.bfSize = sizeof(bfh) + lBufferSize + sizeof(BITMAPFILEHEADER);
	bfh.bfOffBits = sizeof(BITMAPFILEHEADER)+sizeof(BITMAPFILEHEADER);
	// Write the file header.
	DWORD dwWritten = 0;
	WriteFile( hf, &bfh, sizeof( bfh ), &dwWritten, NULL );
	// Write the file Format
	BITMAPINFOHEADER bih;
	ZeroMemory(&bih,sizeof(bih));
	bih.biSize = sizeof( bih );
	bih.biWidth = m_lWidth;
	bih.biHeight = m_lHeight;
	bih.biPlanes = 1;
	bih.biBitCount = m_iBitCount;  //Specifies the number of bits per pixel (bpp)
	WriteFile( hf, &bih, sizeof( bih ), &dwWritten, NULL );
	//Write the file Data
	WriteFile( hf, pBuffer, lBufferSize, &dwWritten, NULL );
	CloseHandle( hf );

	return 0;
}

void SampleGrabberCallback::StartGrabVideo(const char *pFileName)
{
#if x264_grab
	save264_yuv444_init(m_lWidth, m_lHeight, m_iBitCount, pFileName);
	x264_do = true;
	x264_i = 0;
#endif
}

void SampleGrabberCallback::StopGrabVideo()
{
#if x264_grab
	x264_do = false;
	save264_yuv444_destroy();
#endif
}

#if x264_grab

void SampleGrabberCallback::save264_yuv444_init(int w, int h, int bitcnt, const char *filename)
{
	x264_pPic_in = (x264_picture_t*)malloc(sizeof(x264_picture_t));
	x264_pPic_out = (x264_picture_t*)malloc(sizeof(x264_picture_t));
	x264_pParam = (x264_param_t*)malloc(sizeof(x264_param_t));

	x264_param_default(x264_pParam);
	x264_pParam->i_width  = w; 
	x264_pParam->i_height = h;
	x264_pParam->i_keyint_max = 3; // 在此间隔设置IDR关键帧
	x264_pParam->rc.i_lookahead = 1;
	x264_pParam->rc.b_mb_tree = 0;
	x264_pParam->i_sync_lookahead = 1;
	x264_pParam->i_bframe = 0;
	x264_pParam->b_vfr_input = 0;
	
	if (bitcnt == 32)
		x264_pParam->i_csp = X264_CSP_BGRA;
	else if (bitcnt == 24)
		x264_pParam->i_csp = X264_CSP_BGR;
	else
		assert(0);

	x264_pHandle = x264_encoder_open(x264_pParam);
	x264_picture_alloc(x264_pPic_in, x264_pParam->i_csp, x264_pParam->i_width, x264_pParam->i_height);
	x264_picture_init(x264_pPic_out);

	x264_fp_dst = fopen(filename, "wb");
	x264_hMutex_encoding = CreateMutex(NULL, FALSE, NULL);
}

void SampleGrabberCallback::save264_yuv444_encode(BYTE *pBuffer)
{
	int len, ret;
	WaitForSingleObject(x264_hMutex_encoding, INFINITE);
	
	if (x264_pParam->i_csp == X264_CSP_BGRA)
		len = 4 * x264_pParam->i_width * x264_pParam->i_height;
	else if (x264_pParam->i_csp == X264_CSP_BGR)
		len = 3 * x264_pParam->i_width * x264_pParam->i_height;
	memcpy(x264_pPic_in->img.plane[0], pBuffer, len);

	// encode frame
	x264_pPic_in->i_pts = x264_i++;
	x264_encoder_encode(x264_pHandle, &x264_pNals, &x264_iNal, x264_pPic_in, x264_pPic_out);
	for (int j = 0; j < x264_iNal; ++j)
	{
		fwrite(x264_pNals[j].p_payload, 1, x264_pNals[j].i_payload, x264_fp_dst);
		fflush(x264_fp_dst);
		printf("|");
	}
	printf("%d.", x264_i);
	ReleaseMutex(x264_hMutex_encoding);
}

void SampleGrabberCallback::save264_yuv444_destroy()
{
	int ret;
	WaitForSingleObject(x264_hMutex_encoding, INFINITE);
	// flush encoder
	printf("x\n");
	while(1)
	{
		ret = x264_encoder_encode(x264_pHandle, &x264_pNals, &x264_iNal, NULL, x264_pPic_out);
		if (ret == 0)
			break;
		for (int j = 0; j < x264_iNal; ++j)
		{
			fwrite(x264_pNals[j].p_payload, 1, x264_pNals[j].i_payload, x264_fp_dst);
			fflush(x264_fp_dst);
		}
	}

	x264_picture_clean(x264_pPic_in);
	x264_encoder_close(x264_pHandle);
	free(x264_pPic_in);
	free(x264_pPic_out);
	free(x264_pParam);
	
	fclose(x264_fp_dst);
	CloseHandle(x264_hMutex_encoding);
}

#endif
