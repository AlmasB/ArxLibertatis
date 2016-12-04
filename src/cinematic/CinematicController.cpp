/*
 * Copyright 2011-2012 Arx Libertatis Team (see the AUTHORS file)
 *
 * This file is part of Arx Libertatis.
 *
 * Arx Libertatis is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Arx Libertatis is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Arx Libertatis.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "cinematic/CinematicController.h"

#include "core/Application.h"
#include "core/GameTime.h"

#include "io/log/Logger.h"
#include "io/resource/PakReader.h"
#include "cinematic/CinematicLoad.h"

#include "cinematic/Cinematic.h"
#include "cinematic/CinematicKeyframer.h"
#include "cinematic/CinematicSound.h"

#include "game/Camera.h"
#include "game/Player.h"

#include "gui/Speech.h"

#include "graphics/Renderer.h"
#include "input/Input.h"

#include "window/RenderWindow.h"

enum CinematicState {
	Cinematic_Stopped,
	Cinematic_StartRequested,
	Cinematic_Started
};
static CinematicState PLAY_LOADED_CINEMATIC = Cinematic_Stopped;

static bool CINE_PRELOAD = false;
static std::string WILL_LAUNCH_CINE;
static std::string LAST_LAUNCHED_CINE;

Cinematic			*ControlCinematique=NULL;	// 2D Cinematic Controller

void cinematicInit() {
	const Vec2i & size = mainApp->getWindow()->getSize();
	ControlCinematique = new Cinematic(size);
}

void cinematicDestroy() {
	delete ControlCinematique, ControlCinematique = NULL;
}

void cinematicPrepare(std::string name, bool preload) {

	WILL_LAUNCH_CINE = name;
	CINE_PRELOAD = preload;
}

void cinematicRequestStart() {

	PLAY_LOADED_CINEMATIC = Cinematic_StartRequested;
}

void cinematicKill() {
	if(ControlCinematique && ControlCinematique->projectload) {
		ControlCinematique->projectload = false;
		ControlCinematique->OneTimeSceneReInit();
		ControlCinematique->DeleteDeviceObjects();
		PLAY_LOADED_CINEMATIC = Cinematic_Stopped;
		CINE_PRELOAD = false;
	}
}

Vec3f ePos;

void cinematicLaunchWaiting() {

	// A cinematic is waiting to be played...
	if(WILL_LAUNCH_CINE.empty()) {
		return;
	}

	LogDebug("LaunchWaitingCine " << CINE_PRELOAD);

	if(ACTIVECAM) {
		ePos = ACTIVECAM->orgTrans.pos;
	}

	cinematicKill();

	res::path cinematic = res::path("graph/interface/illustrations") / WILL_LAUNCH_CINE;

	if(resources->getFile(cinematic)) {

		ControlCinematique->OneTimeSceneReInit();

		if(loadCinematic(ControlCinematique, cinematic)) {

			if(CINE_PRELOAD) {
				LogDebug("only preloaded cinematic");
				PLAY_LOADED_CINEMATIC = Cinematic_Stopped;
			} else {
				LogDebug("starting cinematic");
				PLAY_LOADED_CINEMATIC = Cinematic_StartRequested;
				arxtime.pause();
			}

			LAST_LAUNCHED_CINE = WILL_LAUNCH_CINE;
		} else {
			LogWarning << "Error loading cinematic " << cinematic;
		}

	} else {
		LogWarning << "Could not find cinematic " << cinematic;
	}

	WILL_LAUNCH_CINE.clear();
}

bool cinematicIsStopped() {
	return PLAY_LOADED_CINEMATIC == Cinematic_Stopped;
}


bool isInCinematic() {
	return PLAY_LOADED_CINEMATIC != Cinematic_Stopped
			&& ControlCinematique
			&& ControlCinematique->projectload;
}

// Manages Currently playing 2D cinematic
void cinematicRender() {

	PlatformDuration diff = g_platformTime.lastFrameDuration();

	if(PLAY_LOADED_CINEMATIC == Cinematic_StartRequested) {
		LogDebug("really starting cinematic now");
		diff = PlatformDuration_ZERO;
		PLAY_LOADED_CINEMATIC = Cinematic_Started;
	}

	PlayTrack(ControlCinematique);
	ControlCinematique->InitDeviceObjects();
	GRenderer->SetRenderState(Renderer::AlphaBlending, true);

	ControlCinematique->Render(diff);

	// end the animation
	if(   !ControlCinematique->key
	   || GInput->isKeyPressedNowUnPressed(Keyboard::Key_Escape)
	) {
		ControlCinematique->projectload=false;
		StopSoundKeyFramer();
		ControlCinematique->OneTimeSceneReInit();
		ControlCinematique->DeleteDeviceObjects();
		arxtime.resume();
		PLAY_LOADED_CINEMATIC = Cinematic_Stopped;

		bool bWasBlocked = false;
		if(BLOCK_PLAYER_CONTROLS) {
			bWasBlocked = true;
		}

		// !! avant le cine end
		if(ACTIVECAM) {
			arx_assert(isallfinite(ePos));
			ACTIVECAM->orgTrans.pos = ePos;
		}

		if(bWasBlocked) {
			BLOCK_PLAYER_CONTROLS = true;
		}

		ARX_SPEECH_Reset();
		SendMsgToAllIO(SM_CINE_END, LAST_LAUNCHED_CINE);
	}
}
