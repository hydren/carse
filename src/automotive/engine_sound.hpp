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
#include "util/properties.hpp"

#include <vector>
#include <utility>

// stores data needed to simulate engine sound
struct EngineSoundProfile
{
	struct RangeProfile
	{
		short rpm;
		std::string filename;
	};

	// information about each sound for each range
	std::vector<RangeProfile> ranges;

	// read-only. the intended max rpm for this engine sound profile
	short intendedMaxRpm;

	// if true, treat the last range as a redline range, in other words, this sound won't be pitched at all.
	bool treatLastRangeAsRedline;

	EngineSoundProfile();

	// returns true if the given properties requests a preset profile instead of specifying a custom profile.
	static bool requestsPresetProfile(const util::Properties& prop);

	// peeks the given properties object and returns the specified sound definition.
	static std::string getSoundDefinitionFromProperties(const util::Properties& prop);

	// load a custom sound profile from properties. if a non-custom (preset) profile is specified, a std::logic_error is thrown.
	static EngineSoundProfile loadFromProperties(const util::Properties& prop);
};

class EngineSoundSimulator
{
	// the engine sound profile
	EngineSoundProfile profile;

	// a set of actual sound data. each index in this corresponds to each index on the profile's ranges
	std::vector<fgeal::Sound*> soundData;

	public:
	void setProfile(const EngineSoundProfile& profile);

	unsigned getCurrentRangeIndex(float currentRpm);

	// use this for debug purposes
	std::vector<fgeal::Sound*>& getSoundData();

	// plays an idle engine sound
	void playIdle();

	// updates the current playing sound. if no sound is being played, it begins playing.
	void updateSound(float currentRpm);

	void haltSound();

	~EngineSoundSimulator();
};

#endif /* AUTOMOTIVE_ENGINE_SOUND_HPP_ */
