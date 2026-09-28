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

// GeneralsX @feature Find N5 fork 28/09/2026 See Common/GXTouchSettings.h.

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/GXTouchSettings.h"
#include "Common/GXSafeArea.h"
#include "Common/NameKeyGenerator.h"
#include "GameClient/Display.h"
#include "GameClient/GameWindowManager.h"
#include "GameClient/Shell.h"
#include "GameLogic/GameLogic.h"

#include <cstdio>
#include <cstdlib>

namespace
{
	const Int LONG_PRESS_DEFAULT_MS = 600;
	const Int LONG_PRESS_MIN_MS = 300;
	const Int LONG_PRESS_MAX_MS = 2000;

	Int s_longPressMs = LONG_PRESS_DEFAULT_MS;
	Bool s_doubleTapDrag = TRUE;
	Bool s_smartTap = TRUE;
	Bool s_cancelButton = TRUE;
	Int s_cancelButtonSize = 2;

	Bool s_cancelPressed = FALSE;

	// Button centre as a fraction of the display. Negative until known: the default depends
	// on the display size and the safe area, neither of which exists when the arguments are
	// parsed, so it is resolved on first use.
	Real s_centerX = -1.0f;
	Real s_centerY = -1.0f;
	Bool s_positionLoaded = FALSE;

	const char *const POSITION_FILE = "TouchButtons.ini";

	// Side of the square as a fraction of the display height. A share of the height rather than
	// a pixel count keeps the button the same physical size across Screen Shapes and on either
	// screen of a foldable.
	Real sizeFraction()
	{
		switch (s_cancelButtonSize)
		{
			case 1: return 0.085f;
			case 3: return 0.14f;
			default: return 0.11f;
		}
	}

	// Next to Options.ini: the user-data dir survives a change of game folder and a reinstall
	// of the game data, and the launcher's "Reset position" deletes this file there.
	void positionFilePath( char *out, size_t size )
	{
		const char *dir = getenv("GENERALSX_USERDATA_DIR");
		if (dir != nullptr && dir[0] != '\0')
			snprintf(out, size, "%s/%s", dir, POSITION_FILE);
		else
			snprintf(out, size, "%s", POSITION_FILE);
	}

	void loadPositionOnce()
	{
		if (s_positionLoaded)
			return;
		s_positionLoaded = TRUE;

		char path[512];
		positionFilePath(path, sizeof(path));
		FILE *fp = fopen(path, "r");
		if (fp == nullptr)
			return;

		char line[128];
		while (fgets(line, sizeof(line), fp) != nullptr)
		{
			float fx = 0.0f, fy = 0.0f;
			if (sscanf(line, " CancelButton = %f %f", &fx, &fy) == 2 &&
					fx > 0.0f && fx < 1.0f && fy > 0.0f && fy < 1.0f)
			{
				s_centerX = fx;
				s_centerY = fy;
				break;
			}
		}
		fclose(fp);
	}

	Int clampInt( Int v, Int lo, Int hi )
	{
		if (hi < lo)
			return lo;
		return v < lo ? lo : (v > hi ? hi : v);
	}
}

namespace GXTouchSettings
{

void setLongPressMs( Int ms )
{
	s_longPressMs = clampInt(ms, LONG_PRESS_MIN_MS, LONG_PRESS_MAX_MS);
	fprintf(stderr, "INFO: touch long press %d ms\n", (int)s_longPressMs);
}

Int longPressMs() { return s_longPressMs; }

void setDoubleTapDrag( Bool enabled )
{
	s_doubleTapDrag = enabled;
	fprintf(stderr, "INFO: touch double-tap-drag selection %s\n", enabled ? "on" : "off");
}

Bool doubleTapDrag() { return s_doubleTapDrag; }

void setSmartTap( Bool enabled )
{
	s_smartTap = enabled;
	fprintf(stderr, "INFO: touch smart tap delay %s\n", enabled ? "on" : "off");
}

Bool smartTap() { return s_smartTap; }

void setCancelButton( Bool enabled )
{
	s_cancelButton = enabled;
	fprintf(stderr, "INFO: touch cancel button %s\n", enabled ? "on" : "off");
}

Bool cancelButton() { return s_cancelButton; }

void setCancelButtonSize( Int level )
{
	s_cancelButtonSize = clampInt(level, 1, 3);
}

Bool cancelButtonShown()
{
	if (!s_cancelButton || TheDisplay == nullptr || TheDisplay->isMoviePlaying())
		return FALSE;

	// The same test ControlBar::update uses for the group panel: a match being played, not the
	// shell's backdrop battle, and past its first frame.
	if (TheShell != nullptr && TheShell->isShellActive())
		return FALSE;
	if (TheGameLogic == nullptr || !TheGameLogic->isInInteractiveGame() || TheGameLogic->getFrame() == 0)
		return FALSE;

	// Hidden with the command bar: scripted cinematics and the victory screen hide it, and a
	// cancel button over a cutscene has nothing to cancel.
	if (TheWindowManager == nullptr || TheNameKeyGenerator == nullptr)
		return FALSE;
	static NameKeyType s_controlBarId = NAMEKEY_INVALID;
	if (s_controlBarId == NAMEKEY_INVALID)
		s_controlBarId = TheNameKeyGenerator->nameToKey("ControlBar.wnd:ControlBarParent");
	GameWindow *bar =TheWindowManager->winGetWindowFromId(nullptr, s_controlBarId);
	return bar != nullptr && !bar->winIsHidden();
}

void cancelButtonRect( Int &x, Int &y, Int &size )
{
	x = y = size = 0;
	if (TheDisplay == nullptr)
		return;

	const Int w = TheDisplay->getWidth();
	const Int h = TheDisplay->getHeight();
	if (w <= 0 || h <= 0)
		return;

	size = (Int)(sizeFraction() * (Real)h + 0.5f);

	loadPositionOnce();
	if (s_centerX < 0.0f || s_centerY < 0.0f)
	{
		// Default: the right edge, a little above the middle -- clear of the match timer at
		// the top right and of the command bar along the bottom, and where a right thumb rests.
		s_centerX = ((Real)w - (Real)GXSafeArea::rightPx() - (Real)size * 0.75f) / (Real)w;
		s_centerY = 0.45f;
	}

	const Int margin = 4;
	const Int cx = (Int)(s_centerX * (Real)w + 0.5f);
	const Int cy = (Int)(s_centerY * (Real)h + 0.5f);
	x = clampInt(cx - size / 2, margin, w - size - margin);
	y = clampInt(cy - size / 2, margin, h - size - margin);
}

Bool cancelButtonHit( Int px, Int py )
{
	Int x, y, size;
	cancelButtonRect(x, y, size);
	if (size <= 0)
		return FALSE;
	// A finger is less precise than a pointer: accept a touch slightly outside the drawn square.
	const Int slop = size / 6;
	return px >= x - slop && px <= x + size + slop && py >= y - slop && py <= y + size + slop;
}

void moveCancelButtonTo( Int px, Int py )
{
	if (TheDisplay == nullptr)
		return;
	const Int w = TheDisplay->getWidth();
	const Int h = TheDisplay->getHeight();
	if (w <= 0 || h <= 0)
		return;

	loadPositionOnce();
	s_centerX = (Real)px / (Real)w;
	s_centerY = (Real)py / (Real)h;

	// Store the clamped centre, so what is saved is what is drawn.
	Int x, y, size;
	cancelButtonRect(x, y, size);
	s_centerX = ((Real)x + (Real)size * 0.5f) / (Real)w;
	s_centerY = ((Real)y + (Real)size * 0.5f) / (Real)h;
}

void saveCancelButtonPosition()
{
	if (s_centerX < 0.0f || s_centerY < 0.0f)
		return;

	char path[512];
	positionFilePath(path, sizeof(path));
	FILE *fp = fopen(path, "w");
	if (fp == nullptr)
	{
		fprintf(stderr, "WARNING: could not save the cancel button position to %s\n", path);
		return;
	}
	fprintf(fp, "CancelButton = %.4f %.4f\n", (double)s_centerX, (double)s_centerY);
	fclose(fp);
	fprintf(stderr, "INFO: cancel button moved to %.4f %.4f\n", (double)s_centerX, (double)s_centerY);
}

void setCancelButtonPressed( Bool pressed ) { s_cancelPressed = pressed; }

Bool cancelButtonPressed() { return s_cancelPressed; }

}
