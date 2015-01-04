/* -*- mode: c; tab-width: 4; c-basic-offset: 3; c-file-style: "linux" -*- */
//
// Copyright (c) 2009, Wei Mingzhi <whistler_wmz@users.sf.net>.
// All rights reserved.
//
// This file is part of SDLPAL.
//
// SDLPAL is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

#include "main.h"

static WORD g_wCurEffectSprite = 0;

VOID
PAL_EndingSetEffectSprite(
WORD         wSpriteNum
)
/*++
  Purpose:

  Set the effect sprite of the ending.

  Parameters:

  [IN]  wSpriteNum - the number of the sprite.

  Return value:

  None.

  --*/
{
	g_wCurEffectSprite = wSpriteNum;
}

VOID
PAL_ShowFBP(
WORD         wChunkNum,
WORD         wFade
)
/*++
  Purpose:

  Draw an FBP picture to the screen.

  Parameters:

  [IN]  wChunkNum - number of chunk in fbp.mkf file.

  [IN]  wFade - fading speed of showing the picture.

  Return value:

  None.

  --*/
{
	PAL_LARGE BYTE            buf[320 * 200];
	PAL_LARGE BYTE            bufSprite[320 * 200];
	const int                 rgIndex[6] = {0, 3, 1, 5, 2, 4};
	SDL_Surface              *p;
	int                       i, j, k;
	BYTE                      a, b;

	if (PAL_MKFDecompressChunk(buf, 320 * 200, wChunkNum, gpGlobals->f.fpFBP) <= 0)
	{
		memset(buf, 0, sizeof(buf));
	}

	if (g_wCurEffectSprite != 0)
	{
		PAL_MKFDecompressChunk(bufSprite, 320 * 200, g_wCurEffectSprite, gpGlobals->f.fpMGO);
	}

	if (wFade)
	{
		wFade++;
		wFade *= 10;

		p = SDL_CreateRGBSurface(gpScreen->flags & ~SDL_HWSURFACE, 320, 200, 8,
			gpScreen->format->Rmask, gpScreen->format->Gmask,
			gpScreen->format->Bmask, gpScreen->format->Amask);
#if SDL_VERSION_ATLEAST(2, 0, 0)
		SDL_SetSurfacePalette(p, gpScreen->format->palette);
#else
		SDL_SetPalette(p, SDL_PHYSPAL | SDL_LOGPAL, VIDEO_GetPalette(), 0, 256);
#endif
		PAL_FBPBlitToSurface(buf, p);
		VIDEO_BackupScreen();

		for (i = 0; i < 16; i++)
		{
			for (j = 0; j < 6; j++)
			{
				//
				// Blend the pixels in the 2 buffers, and put the result into the
				// backup buffer
				//
				for (k = rgIndex[j]; k < gpScreen->pitch * gpScreen->h; k += 6)
				{
					a = ((LPBYTE)(p->pixels))[k];
					b = ((LPBYTE)(gpScreenBak->pixels))[k];

					if (i > 0)
					{
						if ((a & 0x0F) > (b & 0x0F))
						{
							b++;
						}
						else if ((a & 0x0F) < (b & 0x0F))
						{
							b--;
						}
					}

					((LPBYTE)(gpScreenBak->pixels))[k] = ((a & 0xF0) | (b & 0x0F));
				}

				SDL_BlitSurface(gpScreenBak, NULL, gpScreen, NULL);

				if (g_wCurEffectSprite != 0)
				{
					int f = SDL_GetTicks() / 150;
					PAL_RLEBlitToSurface(PAL_SpriteGetFrame(bufSprite, f % PAL_SpriteGetNumFrames(bufSprite)),
						gpScreen, PAL_XY(0, 0));
				}

				VIDEO_UpdateScreen(NULL);
				UTIL_Delay(wFade);
			}
		}

		SDL_FreeSurface(p);
	}

	//
	// HACKHACK: to make the ending show correctly
	//
#ifdef PAL_WIN95
	if (wChunkNum != 68)
#else
	if (wChunkNum != 49)
#endif
	{
		PAL_FBPBlitToSurface(buf, gpScreen);
	}

	VIDEO_UpdateScreen(NULL);
}

VOID
PAL_ScrollFBP(
WORD         wChunkNum,
WORD         wScrollSpeed,
BOOL         fScrollDown
)
/*++
  Purpose:

  Scroll up an FBP picture to the screen.

  Parameters:

  [IN]  wChunkNum - number of chunk in fbp.mkf file.

  [IN]  wScrollSpeed - scrolling speed of showing the picture.

  [IN]  fScrollDown - TRUE if scroll down, FALSE if scroll up.

  Return value:

  None.

  --*/
{
	SDL_Surface          *p;
	PAL_LARGE BYTE        buf[320 * 200];
	PAL_LARGE BYTE        bufSprite[320 * 200];
	int                   i, l;
	SDL_Rect              rect, dstrect;

	if (PAL_MKFDecompressChunk(buf, 320 * 200, wChunkNum, gpGlobals->f.fpFBP) <= 0)
	{
		return;
	}

	if (g_wCurEffectSprite != 0)
	{
		PAL_MKFDecompressChunk(bufSprite, 320 * 200, g_wCurEffectSprite, gpGlobals->f.fpMGO);
	}

	p = SDL_CreateRGBSurface(gpScreen->flags & ~SDL_HWSURFACE, 320, 200, 8,
		gpScreen->format->Rmask, gpScreen->format->Gmask,
		gpScreen->format->Bmask, gpScreen->format->Amask);

	if (p == NULL)
	{
		return;
	}

#if SDL_VERSION_ATLEAST(2, 0, 0)
	SDL_SetSurfacePalette(p, gpScreen->format->palette);
#else
	SDL_SetPalette(p, SDL_PHYSPAL | SDL_LOGPAL, VIDEO_GetPalette(), 0, 256);
#endif

	VIDEO_BackupScreen();
	PAL_FBPBlitToSurface(buf, p);

	if (wScrollSpeed == 0)
	{
		wScrollSpeed = 1;
	}

	rect.x = 0;
	rect.w = 320;
	dstrect.x = 0;
	dstrect.w = 320;

	for (l = 0; l < 220; l++)
	{
		i = l;
		if (i > 200)
		{
			i = 200;
		}

		if (fScrollDown)
		{
			rect.y = 0;
			dstrect.y = i;
			rect.h = 200 - i;
			dstrect.h = 200 - i;
		}
		else
		{
			rect.y = i;
			dstrect.y = 0;
			rect.h = 200 - i;
			dstrect.h = 200 - i;
		}

		SDL_BlitSurface(gpScreenBak, &rect, gpScreen, &dstrect);

		if (fScrollDown)
		{
			rect.y = 200 - i;
			dstrect.y = 0;
			rect.h = i;
			dstrect.h = i;
		}
		else
		{
			rect.y = 0;
			dstrect.y = 200 - i;
			rect.h = i;
			dstrect.h = i;
		}

		SDL_BlitSurface(p, &rect, gpScreen, &dstrect);

		PAL_ApplyWave(gpScreen);

		if (g_wCurEffectSprite != 0)
		{
			int f = SDL_GetTicks() / 150;
			PAL_RLEBlitToSurface(PAL_SpriteGetFrame(bufSprite, f % PAL_SpriteGetNumFrames(bufSprite)),
				gpScreen, PAL_XY(0, 0));
		}

		VIDEO_UpdateScreen(NULL);

		if (gpGlobals->fNeedToFadeIn)
		{
			PAL_FadeIn(gpGlobals->wNumPalette, gpGlobals->fNightPalette, 1);
			gpGlobals->fNeedToFadeIn = FALSE;
#if SDL_VERSION_ATLEAST(2, 0, 0)
			SDL_SetSurfacePalette(p, gpScreen->format->palette);
#else
			SDL_SetPalette(p, SDL_PHYSPAL | SDL_LOGPAL, VIDEO_GetPalette(), 0, 256);
#endif
		}

		UTIL_Delay(800 / wScrollSpeed);
	}

	SDL_BlitSurface(p, NULL, gpScreen, NULL);
	SDL_FreeSurface(p);
	VIDEO_UpdateScreen(NULL);
}

VOID
PAL_EndingAnimation(
VOID
)
/*++
  Purpose:

  Show the ending animation.//就是灵儿独自面对合体水魔兽的动画

  Parameters:

  None.

  Return value:

  None.

  --*/
{
	LPBYTE            buf;
	LPBYTE            bufGirl;
	SDL_Surface      *pUpper;
	SDL_Surface      *pLower;
	SDL_Rect          srcrect, dstrect;

	int               yPosGirl = 180;
	int               i;

	buf = (LPBYTE)UTIL_calloc(1, 64000);
	bufGirl = (LPBYTE)UTIL_calloc(1, 6000);

	pUpper = SDL_CreateRGBSurface(gpScreen->flags & ~SDL_HWSURFACE, 320, 200, 8,
		gpScreen->format->Rmask, gpScreen->format->Gmask,
		gpScreen->format->Bmask, gpScreen->format->Amask);

	pLower = SDL_CreateRGBSurface(gpScreen->flags & ~SDL_HWSURFACE, 320, 200, 8,
		gpScreen->format->Rmask, gpScreen->format->Gmask,
		gpScreen->format->Bmask, gpScreen->format->Amask);

#if SDL_VERSION_ATLEAST(2, 0, 0)
	SDL_SetSurfacePalette(pUpper, gpScreen->format->palette);
	SDL_SetSurfacePalette(pLower, gpScreen->format->palette);
#else
	SDL_SetPalette(pUpper, SDL_PHYSPAL | SDL_LOGPAL, VIDEO_GetPalette(), 0, 256);
	SDL_SetPalette(pLower, SDL_PHYSPAL | SDL_LOGPAL, VIDEO_GetPalette(), 0, 256);
#endif

#ifdef PAL_WIN95
	PAL_MKFDecompressChunk(buf, 64000, 69, gpGlobals->f.fpFBP);
	PAL_FBPBlitToSurface(buf, pUpper);

	PAL_MKFDecompressChunk(buf, 64000, 70, gpGlobals->f.fpFBP);
	PAL_FBPBlitToSurface(buf, pLower);
#else
	PAL_MKFDecompressChunk(buf, 64000, 61, gpGlobals->f.fpFBP);
	PAL_FBPBlitToSurface(buf, pUpper);

	PAL_MKFDecompressChunk(buf, 64000, 62, gpGlobals->f.fpFBP);
	PAL_FBPBlitToSurface(buf, pLower);
#endif

	PAL_MKFDecompressChunk(buf, 64000, 571, gpGlobals->f.fpMGO);
	PAL_MKFDecompressChunk(bufGirl, 6000, 572, gpGlobals->f.fpMGO);

	srcrect.x = 0;
	dstrect.x = 0;
	srcrect.w = 320;
	dstrect.w = 320;

	gpGlobals->wScreenWave = 2;

	for (i = 0; i < 400; i++)
	{
		//
		// Draw the background
		//
		srcrect.y = 0;
		srcrect.h = 200 - i / 2;

		dstrect.y = i / 2;
		dstrect.h = 200 - i / 2;

		SDL_BlitSurface(pLower, &srcrect, gpScreen, &dstrect);

		srcrect.y = 200 - i / 2;
		srcrect.h = i / 2;

		dstrect.y = 0;
		dstrect.h = i / 2;

		SDL_BlitSurface(pUpper, &srcrect, gpScreen, &dstrect);

		PAL_ApplyWave(gpScreen);

		//
		// Draw the beast
		//
		PAL_RLEBlitToSurface(PAL_SpriteGetFrame(buf, 0), gpScreen, PAL_XY(0, -400 + i));
		PAL_RLEBlitToSurface(PAL_SpriteGetFrame(buf, 1), gpScreen, PAL_XY(0, -200 + i));

#ifdef PAL_WIN95
		PAL_RLEBlitToSurface(buf + 0x8444, gpScreen, PAL_XY(0, -200 + i));
#else
		PAL_RLEBlitToSurface(PAL_SpriteGetFrame(buf, 1), gpScreen, PAL_XY(0, -200 + i));
#endif

		//
		// Draw the girl
		//
		yPosGirl -= i & 1;
		if (yPosGirl < 80)
		{
			yPosGirl = 80;
		}

		PAL_RLEBlitToSurface(PAL_SpriteGetFrame(bufGirl, (SDL_GetTicks() / 50) % 4),
			gpScreen, PAL_XY(220, yPosGirl));

		//
		// Update the screen
		//
		VIDEO_UpdateScreen(NULL);
		if (gpGlobals->fNeedToFadeIn)
		{
			PAL_FadeIn(gpGlobals->wNumPalette, gpGlobals->fNightPalette, 1);
			gpGlobals->fNeedToFadeIn = FALSE;
#if SDL_VERSION_ATLEAST(2, 0, 0)
			SDL_SetSurfacePalette(pUpper, gpScreen->format->palette);
			SDL_SetSurfacePalette(pLower, gpScreen->format->palette);
#else
			SDL_SetPalette(pUpper, SDL_LOGPAL | SDL_PHYSPAL, VIDEO_GetPalette(), 0, 256);
			SDL_SetPalette(pLower, SDL_LOGPAL | SDL_PHYSPAL, VIDEO_GetPalette(), 0, 256);
#endif
		}

		UTIL_Delay(50);
	}

	gpGlobals->wScreenWave = 0;

	SDL_FreeSurface(pUpper);
	SDL_FreeSurface(pLower);

	free(buf);
	free(bufGirl);
}

#ifdef PAL_WIN95

VOID
PAL_EndingScreen(
VOID
)
{
	RIX_Play(0x1a, TRUE, 0);
	PAL_RNGPlay(gpGlobals->iCurPlayingRNG, 110, 150, 7);
	PAL_RNGPlay(gpGlobals->iCurPlayingRNG, 151, 999, 9);

	PAL_FadeOut(2);

	RIX_Play(0x19, TRUE, 0);

	PAL_ShowFBP(75, 0);
	PAL_FadeIn(5, FALSE, 1);
	PAL_ScrollFBP(74, 0xf, TRUE);

	PAL_FadeOut(1);

	SDL_FillRect(gpScreen, NULL, 0);
	gpGlobals->wNumPalette = 4;
	gpGlobals->fNeedToFadeIn = TRUE;
	PAL_EndingAnimation();

	RIX_Play(0, FALSE, 2);
	PAL_ColorFade(7, 15, FALSE);

	if (!SOUND_PlayCDA(2))
	{
		RIX_Play(0x11, TRUE, 0);
	}

	SDL_FillRect(gpScreen, NULL, 0);
	PAL_SetPalette(0, FALSE);
	PAL_RNGPlay(0xb, 0, 999, 7);

	PAL_FadeOut(2);

	SDL_FillRect(gpScreen, NULL, 0);
	gpGlobals->wNumPalette = 8;
	gpGlobals->fNeedToFadeIn = TRUE;
	PAL_RNGPlay(10, 0, 999, 6);

	PAL_EndingSetEffectSprite(0);
	PAL_ShowFBP(77, 10);

	VIDEO_BackupScreen();

	PAL_EndingSetEffectSprite(0x27b);
	PAL_ShowFBP(76, 7);

	PAL_SetPalette(5, FALSE);
	PAL_ShowFBP(73, 7);
	PAL_ScrollFBP(72, 0xf, TRUE);

	PAL_ShowFBP(71, 7);
	PAL_ShowFBP(68, 7);

	PAL_EndingSetEffectSprite(0);
	PAL_ShowFBP(68, 6);

	PAL_WaitForKey(0);
	RIX_Play(0, FALSE, 1);
	UTIL_Delay(500);

	if (!SOUND_PlayCDA(13))
	{
		RIX_Play(9, TRUE, 0);
	}

	PAL_ScrollFBP(67, 0xf, TRUE);
	PAL_ScrollFBP(66, 0xf, TRUE);
	PAL_ScrollFBP(65, 0xf, TRUE);
	PAL_ScrollFBP(64, 0xf, TRUE);
	PAL_ScrollFBP(63, 0xf, TRUE);
	PAL_ScrollFBP(62, 0xf, TRUE);
	PAL_ScrollFBP(61, 0xf, TRUE);
	PAL_ScrollFBP(60, 0xf, TRUE);
	PAL_ScrollFBP(59, 0xf, TRUE);

	RIX_Play(0, FALSE, 6);
	PAL_FadeOut(3);
}
#endif


VOID
PAL_NEW_EndingScreen1(
VOID
)
/*++
Purpose:

显示结尾处的消灭合体水魔兽的动画，对应95/98版的第4、5个avi视频

Parameters:

None.

Return value:

None.

--*/
{
	//1128 0043 0019 0000 0000 设置播放音乐 25
	PAL_PlayMUS(0x19, TRUE, 0);

	//1129 008B 0005 0000 0000 改变调色板 05
	PAL_SetPalette(5, FALSE);

	//112A 00A4 0044 0000 000F 卷动FBP图片 68 00 15
	PAL_ScrollFBP(68, 0xf, TRUE);

	//112B 0050 0000 0000 0000 全屏淡出
	PAL_FadeOut(1);

	SDL_FillRect(gpScreen, NULL, 0);
	//112C 008B 0004 0000 0000 改变调色板 04
	PAL_SetPalette(4, FALSE);

	//112D 0096 0000 0000 0000 播放结束动画
	PAL_EndingAnimation();


	gpGlobals->wNumPalette = 4;
	//112E 0077 0001 0000 0000 停止播放音乐 01
	PAL_PlayMUS(0, FALSE, 1);

	//112F 008C 000F 0007 0000 屏幕逐渐褪色 15 07 00
	PAL_ColorFade(7, 15, FALSE);

	//1131 00A3 0002 0011 0000 播放CD音轨 02 若失败 则播放RIX 17
	if (!SOUND_PlayCDA(2))
	{
		RIX_Play(0x11, TRUE, 0);
	}

	SDL_FillRect(gpScreen, NULL, 0);

	//1132 0036 000B 0000 0000 设置动画 11
	//1133 0037 0000 0001 0000 播放动画 00 01 00
	PAL_RNGPlay(0xb, 0, 1, 7);

	//1134 008B 0000 0000 0000 改变调色板 00
	PAL_SetPalette(0, FALSE);

	//1135 0037 0002 0000 0007 播放动画 02 00 07
	PAL_RNGPlay(0xb, 2, 999, 7);

	//1136 0050 0003 0000 0000 全屏淡出
	PAL_FadeOut(3);

	SDL_FillRect(gpScreen, NULL, 0);
	//1137 008B 0008 0000 0000 改变调色板 08
	PAL_SetPalette(8, FALSE);

	//1138 0036 000A 0000 0000 设置动画 10
	//1139 0037 0000 0000 0006 播放动画 00 00 06
	PAL_RNGPlay(10, 0, 999, 6);

	//113A 0076 0047 000A 0000 显示FBP图像 71 10
	PAL_EndingSetEffectSprite(0);
	PAL_ShowFBP(71, 10);

	//113B 00A6 0000 0000 0000 备份画面
	VIDEO_BackupScreen();

	//113C 00A5 0046 027B 0007 淡入FBP图片 70 635 07
	PAL_EndingSetEffectSprite(0x27b);
	PAL_ShowFBP(70, 7);

	//113D 008B 0005 0000 0000 改变调色板 05
	PAL_SetPalette(5, FALSE);

	//113E 00A5 0043 FFFF 0007 淡入FBP图片 67 65535 07
	PAL_ShowFBP(67, 7);

	//113F 00A4 0042 FFFF 0007 卷动FBP图片 66 65535 07
	PAL_ScrollFBP(66, 0xf, TRUE);

	//1140 00A5 0041 FFFF 0007 淡入FBP图片 65 65535 07
	PAL_ShowFBP(65, 7);
}

VOID
PAL_NEW_EndingScreen2(
VOID
)
/*++
Purpose:

显示结尾处的人物介绍，对应95/98版的第六个avi视频

Parameters:

None.

Return value:

None.

--*/
{
	PAL_SetPalette(5, FALSE);//不加这句有可能会出现白屏的情况

	//1141 00A5 0031 FFFF 0007 淡入FBP图片 49 65535 07
	PAL_EndingSetEffectSprite(0x27b);
	PAL_ShowFBP(49, 7);

	//1142 0076 0031 0006 0000 显示FBP图像 49 06
	PAL_EndingSetEffectSprite(0);
	PAL_ShowFBP(49, 6);

	//1143 004D 0000 0000 0000 等待按键
	PAL_WaitForKey(0);

	//1144 0077 0001 0000 0000 停止播放音乐 01
	PAL_PlayMUS(0, FALSE, 0.5);
	UTIL_Delay(500);

	//1145 0043 0009 0003 0000 设置播放音乐 09
	if (!SOUND_PlayCDA(13))
	{
		PAL_PlayMUS(9, TRUE, 0);
	}

	//1146 00A4 0030 0000 0010 卷动FBP图片 48 00 16
	int i = 48;
	PAL_ScrollFBP(i--, 0xf, TRUE);
	PAL_ScrollFBP(i--, 0xf, TRUE);
	PAL_ScrollFBP(i--, 0xf, TRUE);
	PAL_ScrollFBP(i--, 0xf, TRUE);
	PAL_ScrollFBP(i--, 0xf, TRUE);
	PAL_ScrollFBP(i--, 0xf, TRUE);
	PAL_ScrollFBP(i--, 0xf, TRUE);
	PAL_ScrollFBP(i--, 0xf, TRUE);
	PAL_ScrollFBP(i--, 0xf, TRUE);

	//1150 0077 0001 0000 0000 停止播放音乐 01
	PAL_PlayMUS(0, FALSE, 3);

	//1151 0050 0004 0000 0000 全屏淡出
	PAL_FadeOut(3);
}
