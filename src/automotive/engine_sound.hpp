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

	std::vector<RangeProfile> ranges;
	bool treatLastRangeAsRedline;

	EngineSoundProfile();

	// load a sound profile from properties. use 'forceCustomOnly' only when loading built-in profiles.
	static EngineSoundProfile loadFromProperties(const util::Properties& prop, short maxRpm, bool forceCustomOnly=false);
};

class EngineSoundSimulator
{
	EngineSoundProfile profile;
	std::vector<fgeal::Sound*> sound;

	public:
	void setProfile(const EngineSoundProfile& profile);
	void updateSound(float currentRpm);

	~EngineSoundSimulator();
};

#endif /* AUTOMOTIVE_ENGINE_SOUND_HPP_ */
