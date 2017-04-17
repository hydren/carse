/*
 * engine_sound.cpp
 *
 *  Created on: 12 de abr de 2017
 *      Author: carlosfaruolo
 */

#include "engine_sound.hpp"

#include "futil/string/more_operators.hpp"

#include <stdexcept>

using util::Properties;
using std::string;
using fgeal::Sound;

const string
	KEY_SOUND = "sound",
	KEY_SOUND_RELINE_LAST = "sound_redline_last",
	KEY_ENGINE_MAX_RPM = "engine_maximum_rpm",
	KEY_SOUND_PREFIX = "sound";

EngineSoundProfile::EngineSoundProfile()
: ranges(), intendedMaxRpm(7000), treatLastRangeAsRedline(false)
{}

//static
bool EngineSoundProfile::requestsPresetProfile(const util::Properties& prop)
{
	return prop.containsKey(KEY_SOUND) and prop.get(KEY_SOUND) != "custom" and prop.get(KEY_SOUND) != "no";
}

//static
EngineSoundProfile EngineSoundProfile::loadFromProperties(const Properties& prop)
{
	EngineSoundProfile profile;

	if(prop.containsKey(KEY_ENGINE_MAX_RPM))
	{
		short specifiedMaxRpm = atoi(prop.get(KEY_ENGINE_MAX_RPM).c_str());
		if(specifiedMaxRpm > 0)
			profile.intendedMaxRpm = specifiedMaxRpm;
	}

	if(prop.containsKey(KEY_SOUND))
	{
		if(prop.get(KEY_SOUND) == "no")
		{
			profile.ranges.clear();
		}
		else if(prop.get(KEY_SOUND) == "custom")
		{
			if(prop.containsKey(KEY_SOUND_RELINE_LAST) and prop.get(KEY_SOUND_RELINE_LAST) == "true")
				profile.treatLastRangeAsRedline = true;

			int i = 0;
			string key = KEY_SOUND_PREFIX + i;
			while(prop.containsKey(key))
			{
				string filename = prop.get(key);

				// now try to read _rpm property
				key += "_rpm";
				short rpm = -1;
				if(prop.containsKey(key))
					rpm = atoi(prop.get(key).c_str());

				// if rpm < 0, either rpm wasn't specified, or was intentionally left -1 (or other negative number)
				if(rpm < 0)
				{
					if(i == 0) rpm = 0;
					else       rpm = (intendedMaxRpm - profile.ranges.rbegin()->rpm)/2;
				}

				// save filename for given rpm
				profile.ranges[rpm] = filename;
				i += 1;
				key = KEY_SOUND_PREFIX + i;
			}
		}
		else
		{
			// todo create engine sound classes: default, crossplane_v8, inline_6, flat_4, etc
			throw std::logic_error("properties specify a preset profile instead of a custom one");
		}
	}

	return profile;
}

void EngineSoundSimulator::setProfile(const EngineSoundProfile& profile)
{
	for(unsigned i = 0; i < soundData.size(); i++)
		delete soundData[i];

	this->soundData.clear();
	this->profile = profile;

	for(unsigned i = 0; i < profile.ranges.size(); i++)
		this->soundData.push_back(new Sound(profile.ranges[i].filename));
}

//calculates engine sound pitch for given RPM and max RPM
float calculatePitch(float rpmDiff, float maxRpm)
{
	return exp(rpmDiff/maxRpm);
}

void EngineSoundSimulator::updateSound(float currentRpm)
{
	const unsigned soundCount = soundData.size();
	if(soundCount > 0 and currentRpm > 0) // its no use if there is no engine sound or rpm is too low
	{
		// establish the current range index
		unsigned currentRangeIndex = 0;
		for(unsigned i = 0; i < soundCount; i++)
			if(currentRpm > profile.ranges[i].rpm)
				currentRangeIndex = i;

		// some aliases
		Sound& currentSound = *soundData[currentRangeIndex];
		const float lowerRpmCurrent = profile.ranges[currentRangeIndex].rpm;
		const float upperRpmCurrent = currentRangeIndex+1 < soundCount? profile.ranges[currentRangeIndex+1].rpm : profile.intendedMaxRpm;
		const float rangeSizeCurrent = upperRpmCurrent - lowerRpmCurrent;

		currentSound.setVolume(1.0f);
		if(currentRangeIndex+1 < soundCount or not profile.treatLastRangeAsRedline)
			currentSound.setPlaybackSpeed(calculatePitch(currentRpm - lowerRpmCurrent, profile.intendedMaxRpm));

		if(not currentSound.isPlaying())
			currentSound.loop();

		for(unsigned i = 0; i < soundData.size(); i++)
		{
			// some aliases
			Sound& rangeSound = *soundData[i];
			const short rangeRpm = profile.ranges[i].rpm;

			// current range
			if(i == currentRangeIndex)
				continue;  // already handled

			// preceding range
			else if(i + 1 == currentRangeIndex and currentRpm - lowerRpmCurrent < 0.25*rangeSizeCurrent and currentRangeIndex > 0)
			{
//				snd.setVolume(1.0 - 4*(currentRpm - lowerRpmCurrent)/rangeSizeCurrent); // linear fade out
				rangeSound.setVolume(sqrt(1-16*pow((currentRpm - lowerRpmCurrent)/rangeSizeCurrent, 2))); // quadratic fade out

				rangeSound.setPlaybackSpeed(calculatePitch(currentRpm - rangeRpm, profile.intendedMaxRpm));
				if(not rangeSound.isPlaying())
					rangeSound.loop();
			}

			// succeeding range
			else if(i == currentRangeIndex + 1 and currentRpm - lowerRpmCurrent > 0.75*rangeSizeCurrent and currentRangeIndex < soundData.size()-2)
			{
//				snd.setVolume(-3.0 + 4*(currentRpm - lowerRpmCurrent)/rangeSizeCurrent); // linear fade in
				rangeSound.setVolume(sqrt(1-pow(4*((currentRpm - lowerRpmCurrent)/rangeSizeCurrent)-4, 2)) ); // quadratic fade in

				rangeSound.setPlaybackSpeed(calculatePitch(currentRpm - rangeRpm, profile.intendedMaxRpm));
				if(not rangeSound.isPlaying())
					rangeSound.loop();
			}

			else rangeSound.stop();
		}
	}
}

void EngineSoundSimulator::haltSound()
{
	for(unsigned i = 0; i < soundData.size(); i++)
		soundData[i]->stop();
}
