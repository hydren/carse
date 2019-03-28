/*
 * engine_sound.hpp
 *
 *  Created on: 12 de abr de 2017
 *      Author: carlosfaruolo
 */

#ifndef AUTOMOTIVE_ENGINE_SOUND_HPP_
#define AUTOMOTIVE_ENGINE_SOUND_HPP_
#include <ciso646>

#include "fgeal/fgeal.hpp"

#include <vector>

// stores data needed to simulate engine sound
struct EngineSoundProfile
{
	struct RangeProfile
	{
		short startRpm;  // the initial RPM of this range
		short depictedRpm;  // the RPM depicted by this range's sound when played at normal playback speed/pitch
		std::string soundFilename;
	};

	// controls whether the ranges should be pitched according to the current RPM or not at all
	bool allowRpmPitching;

	// information about each sound for each range
	std::vector<RangeProfile> ranges;
};

class EngineSoundSimulator
{
	// the engine sound profile
	EngineSoundProfile profile;

	// a set of actual sound data. each index in this corresponds to each index on the profile's ranges
	std::vector<fgeal::Sound*> soundData;

	// the maximum RPM expected to be simulated. this must be the maximum value expected to be passed to EngineSoundSimulator::updateSound().
	short simulatedMaximumRpm;

	// the overall volume of this simulator. this must be a decimal in the range [0,1].
	float volume;

	//calculates engine sound pitch for given RPM and max RPM
	float calculatePitch(float rpmDiff);

	public:
	// changes the current profile.
	void setProfile(const EngineSoundProfile& profile, short simulatedMaximumRpm);

	// effectively loads sound data from files, as specified in the profile (any previously loaded data will be freed if it isn't shared)
	void loadAssetsData();

	// free all sound data loaded by this simulator (if not shared by other objects)
	void freeAssetsData();

	// gets the range index for the given RPM value
	unsigned getRangeIndex(float currentRpm);

	// use this for debug purposes
	std::vector<fgeal::Sound*>& getSoundData();

	// begins playing the simulated engine sound. initially, the idle engine sound is played.
	void play();

	// updates the engine sound simulation to play the desired engine RPM. if no sound is being played, the simulator begins playing.
	void update(float currentRpm);

	// stops all currently playing sounds from this simulator
	void halt();

	// sets the overall volume of this simulator (parameter 'volume' must be a decimal in the range [0,1])
	void setVolume(float volume);

	// returns the overall volume of this simulator
	float getVolume() const;

	~EngineSoundSimulator();
};

#endif /* AUTOMOTIVE_ENGINE_SOUND_HPP_ */
