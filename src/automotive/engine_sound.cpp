/*
 * engine_sound.cpp
 *
 *  Created on: 12 de abr de 2017
 *      Author: carlosfaruolo
 */

#include "engine_sound.hpp"

#include "futil/string/more_operators.hpp"

#include <cmath>
#include <cstdlib>

#include <stdexcept>
#include <algorithm>

using futil::Properties;
using std::string;
using std::vector;
using fgeal::Sound;

const string
	KEY_SOUND = "sound",
	KEY_SOUND_RELINE_LAST = "sound_redline_last",
	KEY_ENGINE_MAX_RPM = "engine_maximum_rpm",
	KEY_SOUND_PREFIX = "sound";

//static
bool EngineSoundProfile::requestsPresetProfile(const Properties& prop)
{
	return prop.containsKey(KEY_SOUND) and prop.get(KEY_SOUND) != "custom" and prop.get(KEY_SOUND) != "no";
}

//static
string EngineSoundProfile::getSoundDefinitionFromProperties(const Properties& prop)
{
	return prop.get(KEY_SOUND);
}

static bool rangeProfileCompareFunction(const EngineSoundProfile::RangeProfile& p1, const EngineSoundProfile::RangeProfile& p2)
{
	return p1.isRedline? false : p2.isRedline? true : p1.rpm < p2.rpm;
}

//static
EngineSoundProfile EngineSoundProfile::loadFromProperties(const Properties& prop)
{
	EngineSoundProfile profile;
	short maxRpm = 0;

	if(prop.containsKey(KEY_ENGINE_MAX_RPM))
		maxRpm = atoi(prop.get(KEY_ENGINE_MAX_RPM).c_str());

	if(maxRpm <= 0)
		maxRpm = 7000;

	if(prop.containsKey(KEY_SOUND))
	{
		if(prop.get(KEY_SOUND) == "no")
		{
			profile.ranges.clear();
		}
		else if(prop.get(KEY_SOUND) == "custom")
		{
			int i = 0;
			string key = KEY_SOUND_PREFIX + i;
			while(prop.containsKey(key))
			{
				string filename = prop.get(key);

				// now try to read _rpm property
				key += "_rpm";
				short rpm = -1;
				bool isRedline = false;
				if(prop.containsKey(key))
				{
					if(prop.get(key) == "redline")
						isRedline = true;
					else
						rpm = atoi(prop.get(key).c_str());
				}

				// if rpm < 0, either rpm wasn't specified, or was intentionally left -1 (or other negative number)
				if(rpm < 0 and not isRedline)
				{
					if(i == 0) rpm = 0;
					else       rpm = (maxRpm - profile.ranges.rbegin()->rpm)/2;
				}

				// save filename and settings for given rpm
				RangeProfile range = {rpm, filename, isRedline};
				profile.ranges.push_back(range);
				i += 1;
				key = KEY_SOUND_PREFIX + i;
			}

			std::stable_sort(profile.ranges.begin(), profile.ranges.end(), rangeProfileCompareFunction);
		}
		else throw std::logic_error("properties specify a preset profile instead of a custom one");
	}

	return profile;
}

EngineSoundSimulator::~EngineSoundSimulator()
{
	for(unsigned i = 0; i < soundData.size(); i++)
		delete soundData[i];
}

void EngineSoundSimulator::setProfile(const EngineSoundProfile& profile, short maxRpm)
{
	// cleanup
	for(unsigned i = 0; i < soundData.size(); i++)
		delete soundData[i];

	this->soundData.clear();

	this->profile = profile;  // copies the profile (preserves the original intact)

	// loads sound data
	for(unsigned i = 0; i < profile.ranges.size(); i++)
		this->soundData.push_back(new Sound(profile.ranges[i].filename));

	this->simulatedMaximumRpm = maxRpm;

	// assign max rpm if contains a 'redline'-type range
	for(unsigned i = 0; i < this->profile.ranges.size(); i++)
		if(this->profile.ranges[i].isRedline)
		{
			this->profile.ranges[i].rpm = maxRpm-50;
			break;
		}
}

void EngineSoundSimulator::setSimulatedMaximumRpm(short rpm)
{
	this->simulatedMaximumRpm = rpm;
}

unsigned EngineSoundSimulator::getRangeIndex(float rpm)
{
	unsigned rangeIndex = 0;
	for(unsigned i = 0; i < soundData.size(); i++)
		if(rpm > profile.ranges[i].rpm)
			rangeIndex = i;

	return rangeIndex;
}

vector<Sound*>& EngineSoundSimulator::getSoundData()
{
	return soundData;
}

float EngineSoundSimulator::calculatePitch(float rpmDiff)
{
	return exp(rpmDiff/simulatedMaximumRpm);
}

void EngineSoundSimulator::playIdle()
{
	if(not profile.ranges.empty())
		updateSound(profile.ranges[0].rpm+1); // this +1 may be unneccessary
}

void EngineSoundSimulator::updateSound(float currentRpm)
{
	const unsigned soundCount = soundData.size();
	if(soundCount > 0 and currentRpm > 0) // its no use if there is no engine sound or rpm is too low
	{
		// establish the current range index
		const unsigned currentRangeIndex = this->getRangeIndex(currentRpm);

		// some aliases
		Sound& currentSound = *soundData[currentRangeIndex];
		const float lowerRpmCurrent = profile.ranges[currentRangeIndex].rpm;
		const float upperRpmCurrent = currentRangeIndex + 1 < soundCount? profile.ranges[currentRangeIndex + 1].rpm : simulatedMaximumRpm;
		const float rangeSizeCurrent = upperRpmCurrent - lowerRpmCurrent;

		currentSound.setVolume(1.0f);
		if(not profile.ranges[currentRangeIndex].isRedline)
			currentSound.setPlaybackSpeed(calculatePitch(currentRpm - lowerRpmCurrent));

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
			else if(i + 1 == currentRangeIndex                                    // this range is preceding the current range
					and currentRpm - lowerRpmCurrent < 0.25*rangeSizeCurrent)     // current RPM is within 0-25% of current range
			{
//				snd.setVolume(1.0 - 4*(currentRpm - lowerRpmCurrent)/rangeSizeCurrent); // linear fade out
				rangeSound.setVolume(sqrt(1-16*pow((currentRpm - lowerRpmCurrent)/rangeSizeCurrent, 2))); // quadratic fade out

				rangeSound.setPlaybackSpeed(calculatePitch(currentRpm - rangeRpm));
				if(not rangeSound.isPlaying())
					rangeSound.loop();
			}

			// succeeding range
			else if(i == currentRangeIndex + 1                                       // this range is succeeding the current range
					and currentRpm - lowerRpmCurrent > 0.75*rangeSizeCurrent         // current RPM is within 75-100% of current range
					and not profile.ranges[i].isRedline)                             // this range is NOT a redline range
			{
//				snd.setVolume(-3.0 + 4*(currentRpm - lowerRpmCurrent)/rangeSizeCurrent); // linear fade in
				rangeSound.setVolume(sqrt(1-pow(4*((currentRpm - lowerRpmCurrent)/rangeSizeCurrent)-4, 2)) ); // quadratic fade in

				rangeSound.setPlaybackSpeed(calculatePitch(currentRpm - rangeRpm));
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
