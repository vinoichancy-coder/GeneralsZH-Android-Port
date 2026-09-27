/*
**	Command & Conquer Generals Zero Hour(tm)
**	Copyright 2025 Electronic Arts Inc.
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
**
**	This program is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

// GeneralsX @feature Find N5 fork 28/09/2026 Screen shape: the game's aspect ratio on screens
// far from the 4:3 its interface was laid out for.
//
// Every .wnd layout is scaled separately in X and Y from its creation resolution
// (GameWindowManagerScript.cpp), so the interface takes the shape of the screen: on a
// 2616x1140 foldable cover screen (2.29:1) buttons come out 1.72x too wide, on its
// 2480x2248 inner screen (1.10:1) 0.83x too narrow. The launcher's Screen Shape setting passes
// -gxScreenShape 16:9 or 4:3; the game then renders at the largest rectangle of that shape
// that fits the window, and the existing pillarbox path (DX8Wrapper::Pillarbox_Setup, with its
// touch remapping) centres it with black bars. Without the argument ("fill") nothing changes.
//
// The value travels in the GX_SCREEN_SHAPE environment variable because it is needed twice at
// different points of startup: by SDL3Main before GameMain() to pick the resolution, and by
// W3DDisplay when it builds the Options menu's resolution list. Client-side only: the logic
// never sees it.
#pragma once

#include <cmath>
#include <cstdlib>
#include <cstring>

namespace GXScreenShape
{
	static const char *const ENV_NAME = "GX_SCREEN_SHAPE";

	// Accepts "4:3", "16:9" or "fill"; anything else is treated as "fill".
	inline void set( const char *value )
	{
		const bool shaped = value != nullptr && ( strcmp( value, "4:3" ) == 0 || strcmp( value, "16:9" ) == 0 );
#if defined(_WIN32)
		// An empty value removes the variable with the MSVC runtime.
		_putenv_s( ENV_NAME, shaped ? value : "" );
#else
		if ( shaped )
			setenv( ENV_NAME, value, 1 );
		else
			unsetenv( ENV_NAME );
#endif
	}

	// Width / height of the chosen shape, or 0 for "fill" (use the screen's own shape).
	inline float targetAspect()
	{
		const char *value = getenv( ENV_NAME );
		if ( value == nullptr )
			return 0.0f;
		if ( strcmp( value, "4:3" ) == 0 )
			return 4.0f / 3.0f;
		if ( strcmp( value, "16:9" ) == 0 )
			return 16.0f / 9.0f;
		return 0.0f;
	}

	// True when two sizes have the same shape to within 2 % -- the tolerance absorbs the
	// even-width rounding and the percentage steps of the Options menu's resolution list.
	inline bool sameShape( int w1, int h1, int w2, int h2 )
	{
		if ( w1 <= 0 || h1 <= 0 || w2 <= 0 || h2 <= 0 )
			return false;
		const float a1 = (float)w1 / (float)h1;
		const float a2 = (float)w2 / (float)h2;
		return std::fabs( a1 - a2 ) <= 0.02f * a2;
	}

	// The largest rectangle of the chosen shape that fits in screenW x screenH, with even
	// sides. Returns the screen size itself for "fill", or when the screen is already that
	// shape (a pillarbox of a few pixels would only cost an extra blit).
	inline void fit( int screenW, int screenH, int &outW, int &outH )
	{
		outW = screenW;
		outH = screenH;
		const float target = targetAspect();
		if ( target <= 0.0f || screenW <= 0 || screenH <= 0 )
			return;
		const float screenAspect = (float)screenW / (float)screenH;
		if ( std::fabs( screenAspect - target ) <= 0.01f * target )
			return;
		if ( screenAspect > target )
			outW = (int)( (float)screenH * target + 0.5f );  // wider than the shape: bars left and right
		else
			outH = (int)( (float)screenW / target + 0.5f );  // taller than the shape: bars top and bottom
		outW &= ~1;
		outH &= ~1;
	}
}
