#include "main.h"



#ifdef PAL_HAS_AVI
#include <assert.h>
#include <SDL_syswm.h>

#ifdef _WIN32
#include <vfw.h>
#pragma comment(lib, "vfw32.lib")

int PAL_PlayAVI(const char *szFilename, BOOL fCanJump)
{
	SDL_SysWMinfo wm;
	HWND hw;
	DWORD len, starttime;
	INT prevw, prevh;
	extern SDL_Surface *gpScreenReal;
#if SDL_VERSION_ATLEAST(2, 0, 0)
	extern SDL_Window *gpWindow;
#endif
	INT width, height;

	SDL_VERSION(&wm.version);

#if SDL_VERSION_ATLEAST(2, 0, 0)
	SDL_GetWindowSize(gpWindow, &width, &height);
	SDL_GetWindowWMInfo(gpWindow, &wm);
	hw = MCIWndCreate(wm.info.win.window, (HINSTANCE)GetModuleHandle(NULL),
		WS_CHILD | WS_VISIBLE | MCIWNDF_NOMENU | MCIWNDF_NOPLAYBAR, NULL);
#else
	width = gpScreenReal->w;
	height = gpScreenReal->h;
	SDL_GetWMInfo(&wm);
	hw = MCIWndCreate(wm.window, (HINSTANCE)GetModuleHandle(NULL),
		WS_CHILD | WS_VISIBLE | MCIWNDF_NOMENU | MCIWNDF_NOPLAYBAR, NULL);
#endif

	if (hw == NULL) return -1;

	//unicode时可以使用MultiByteToWideChar
	if (MCIWndOpen(hw, szFilename, 0) != 0)
	{
		MCIWndDestroy(hw);
		return -1;
	}

	if (gpGlobals->wNumMusic != 0)
	{
		PAL_PlayMUS(0, FALSE, 0);
		gpGlobals->wNumMusic = 0;
	}

	SDL_FillRect(gpScreen, NULL, 0);
	VIDEO_UpdateScreen(NULL);

	MCIWndSetZoom(hw, 100);
	MCIWndPlay(hw);

	PAL_ClearKeyState();
	len = MCIWndGetLength(hw) * (1000 / 15);
	starttime = SDL_GetTicks();

	prevw = width;
	prevh = height;

	MoveWindow(hw, 0, 0, width, height, TRUE);

	while (SDL_GetTicks() - starttime < len)
	{
#if SDL_VERSION_ATLEAST(2, 0, 0)
		SDL_GetWindowSize(gpWindow, &width, &height);
#else
		width = gpScreenReal->w;
		height = gpScreenReal->h;
#endif
		if (prevw != width || prevh != height)
		{	//对窗口进行拉伸或者收缩的时候，令视频的大小与之保持一致
			MoveWindow(hw, 0, 0, width, height, FALSE);
			prevw = width;
			prevh = height;
		}
		if (g_InputState.dwKeyPress != 0 && fCanJump == TRUE)
		{
			break;
		}
		UTIL_Delay(500);
	}

	PAL_ClearKeyState();

	MCIWndStop(hw);
	MCIWndDestroy(hw);

	return 0;
}
#else

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>

#pragma comment(lib, "avcodec.lib")
#pragma comment(lib, "avformat.lib")
#pragma comment(lib, "avfilter.lib")
#pragma comment(lib, "avdevice.lib")
#pragma comment(lib, "avutil.lib")
#pragma comment(lib, "postproc.lib")
#pragma comment(lib, "swresample.lib")
#pragma comment(lib, "swscale.lib")

int PAL_PlayAVI(const char *szFilename, BOOL canJump)
{
	// tutorial02.c
	// A pedagogical video player that will stream through every video frame as fast as it can.
	//
	// This tutorial was written by Stephen Dranger (dranger@gmail.com).
	//
	// Code based on FFplay, Copyright (c) 2003 Fabrice Bellard,
	// and a tutorial by Martin Bohme (boehme@inb.uni-luebeckREMOVETHIS.de)
	// Tested on Gentoo, CVS version 5/01/07 compiled with GCC 4.1.1
	//
	// Use the Makefile to build all examples.
	//
	// Run using
	// tutorial02 myvideofile.mpg
	//
	// to play the video stream on your screen.
	extern SDL_Surface *gpScreenReal;
	extern SDL_Window *gpWindow;
	int width, height;

	AVFormatContext *pFormatCtx = NULL;
	int             i, videoStream;
	AVCodecContext  *pCodecCtx = NULL;
	AVCodec         *pCodec = NULL;
	AVFrame         *pFrame = NULL;
	AVPacket        packet;
	int             frameFinished;
	//float           aspect_ratio;

	AVDictionary    *optionsDict = NULL;
	struct SwsContext *sws_ctx = NULL;
	//SDL_CreateTexture();
	SDL_Texture    *bmp = NULL;

	SDL_Rect        rect;
	SDL_Event       event;

	SDL_Renderer *renderer;

	// Register all formats and codecs
	av_register_all();

	// Open video file
	if (avformat_open_input(&pFormatCtx, szFilename, NULL, NULL) != 0)
		return -1; // Couldn't open file

	// Retrieve stream information
	if (avformat_find_stream_info(pFormatCtx, NULL) < 0)
		return -1; // Couldn't find stream information

	// Dump information about file onto standard error
	av_dump_format(pFormatCtx, 0, szFilename, 0);

	// Find the first video stream
	videoStream = -1;
	for (i = 0; i < pFormatCtx->nb_streams; i++)
	if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
	{
		videoStream = i;
		break;
	}
	if (videoStream == -1)
		return -1; // Didn't find a video stream

	// Get a pointer to the codec context for the video stream
	pCodecCtx = pFormatCtx->streams[videoStream]->codec;


	SDL_GetWindowSize(gpWindow, &width, &height);
	//pCodecCtx->height = height;
	//pCodecCtx->width = width;

	// Find the decoder for the video stream
	pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
	if (pCodec == NULL)
	{
		fprintf(stderr, "Unsupported codec!\n");
		return -1; // Codec not found
	}

	// Open codec
	if (avcodec_open2(pCodecCtx, pCodec, &optionsDict) < 0)
		return -1; // Could not open codec

	// Allocate video frame
	pFrame = avcodec_alloc_frame();

	AVFrame* pFrameYUV = avcodec_alloc_frame();
	if (pFrameYUV == NULL)
		return -1;

	// Make a screen to put our videe
	//#ifndef __DARWIN__
	//    screen = SDL_SetVideoMode(pCodecCtx->width, pCodecCtx->height, 0, 0);
	//#else
	//    screen = SDL_SetVideoMode(pCodecCtx->width, pCodecCtx->height, 24, 0);
	//#endif
	//    SDL_WM_SetCaption("My Game Window", "game");
	//    SDL_Surface *screen = SDL_SetVideoMode(640, 480, 0, SDL_FULLSCREEN | SDL_OPENGL);
	// 	screen = SDL_CreateWindow("My Game Window",
	// 		SDL_WINDOWPOS_UNDEFINED,
	// 		SDL_WINDOWPOS_UNDEFINED,
	// 		pCodecCtx->width, pCodecCtx->height,
	// 		SDL_WINDOW_FULLSCREEN | SDL_WINDOW_OPENGL);
	renderer = SDL_CreateRenderer(gpWindow, -1, 0);


	if (!gpWindow)
	{
		fprintf(stderr, "SDL: could not set video mode - exiting\n");
		exit(1);
	}

	// Allocate a place to put our YUV image on that screen
	//    bmp = SDL_CreateYUVOverlay(pCodecCtx->width,
	//                               pCodecCtx->height,
	//                               SDL_YV12_OVERLAY,
	//                               screen);
	bmp = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_YV12, SDL_TEXTUREACCESS_STREAMING, pCodecCtx->width, pCodecCtx->height);
	//SDL_SetTextureBlendMode(bmp,SDL_BLENDMODE_BLEND );

	sws_ctx =
		sws_getContext
		(
		pCodecCtx->width,
		pCodecCtx->height,
		pCodecCtx->pix_fmt,
		pCodecCtx->width,
		pCodecCtx->height,
		PIX_FMT_YUV420P,
		SWS_BILINEAR,
		NULL,
		NULL,
		NULL
		);

	int numBytes = avpicture_get_size(PIX_FMT_YUV420P, pCodecCtx->width,
		pCodecCtx->height);
	uint8_t* buffer = (uint8_t *)av_malloc(numBytes*sizeof(uint8_t));

	avpicture_fill((AVPicture *)pFrameYUV, buffer, PIX_FMT_YUV420P,
		pCodecCtx->width, pCodecCtx->height);

	// Read frames and save first five frames to disk
	i = 0;

	rect.x = 0;
	rect.y = 0;
	rect.w = pCodecCtx->width;
	rect.h = pCodecCtx->height;

	while (av_read_frame(pFormatCtx, &packet) >= 0)
	{
		// Is this a packet from the video stream?
		if (packet.stream_index == videoStream)
		{
			// Decode video frame
			avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished,
				&packet);

			// Did we get a video frame?
			if (frameFinished)
			{

				sws_scale
					(
					sws_ctx,
					(uint8_t const * const *)pFrame->data,
					pFrame->linesize,
					0,
					pCodecCtx->height,
					pFrameYUV->data,
					pFrameYUV->linesize
					);
				////iPitch 计算yuv一行数据占的字节数
				SDL_UpdateYUVTexture(bmp, &rect,
					pFrameYUV->data[0], pFrameYUV->linesize[0],
					pFrameYUV->data[1], pFrameYUV->linesize[1],
					pFrameYUV->data[2], pFrameYUV->linesize[2]);
				SDL_RenderClear(renderer);
				SDL_RenderCopy(renderer, bmp, &rect, &rect);
				SDL_RenderPresent(renderer);
			}
			SDL_Delay(50);
			//Sleep(500);
		}

		// Free the packet that was allocated by av_read_frame
		av_free_packet(&packet);

		if (g_InputState.dwKeyPress != 0)
		{
			break;
		} 
		UTIL_Delay(500);
	}
	PAL_ClearKeyState();
	SDL_DestroyTexture(bmp);

	// Free the YUV frame
	av_free(pFrame);
	av_free(pFrameYUV);
	// Close the codec
	avcodec_close(pCodecCtx);

	// Close the video file
	avformat_close_input(&pFormatCtx);

	return 0;
}
#endif


#endif

