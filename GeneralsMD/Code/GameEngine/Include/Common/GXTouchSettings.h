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

// GeneralsX @feature Find N5 fork 28/09/2026 Player-tunable touch controls.
//
// The launcher's Touch controls page passes these as startup arguments (-gxLongPressMs,
// -gxDoubleTapDrag, -gxSmartTap, -gxCancelButton, -gxCancelButtonSize; parsed in
// CommandLine.cpp). The gesture state machine (SDL3GameEngine.cpp) reads them, and InGameUI
// draws the on-screen cancel button from the geometry kept here.
//
// It lives in GameEngine rather than next to the touch code in GameEngineDevice because both
// layers need it: CommandLine and InGameUI are GameEngine, the gesture code is the device
// layer, and the device layer may include GameEngine headers but not the reverse.
//
// Client-side only. Nothing here reaches the game logic, the network or a replay: the
// settings change which existing gesture a finger becomes, never what that gesture orders.
#pragma once

#include "Lib/BaseType.h"

namespace GXTouchSettings
{
	// --- Launcher settings --------------------------------------------------------------

	/// How long a finger must rest on the battlefield, without moving, for its release to cancel
	/// the armed ability or pending building, or else clear the selection. Clamped to 300..2000.
	void setLongPressMs( Int ms );
	Int longPressMs();

	/// Tap, then touch again within the double-tap window and drag: a selection box.
	void setDoubleTapDrag( Bool enabled );
	Bool doubleTapDrag();

	/// A tap that would issue an order waits for the double-tap window before it is sent, so
	/// the first tap of a double tap or a double-tap-drag never sends units anywhere.
	void setSmartTap( Bool enabled );
	Bool smartTap();

	/// The on-screen cancel button, and its size step (1 small, 2 medium, 3 large).
	void setCancelButton( Bool enabled );
	Bool cancelButton();
	void setCancelButtonSize( Int level );

	// --- The cancel button on screen, in display pixels ----------------------------------

	/// Enabled, real gameplay, command bar up, no movie. Asked live by both the drawing and the
	/// hit test rather than cached: a flag set while drawing would go stale the moment the game
	/// returns to the menus (which stop drawing the in-game UI), and an invisible button would
	/// then swallow menu taps.
	Bool cancelButtonShown();

	/// Square the button occupies right now. The position is kept as a fraction of the display,
	/// so it survives a change of resolution or Screen Shape, and is clamped fully on screen.
	void cancelButtonRect( Int &x, Int &y, Int &size );
	Bool cancelButtonHit( Int px, Int py );

	/// Centre the button on a point while the player drags it; save when the drag ends.
	void moveCancelButtonTo( Int px, Int py );
	void saveCancelButtonPosition();

	/// Drawn pressed while a finger is on it.
	void setCancelButtonPressed( Bool pressed );
	Bool cancelButtonPressed();
}
