/*
 * engine_sound.cpp
 *
 *  Created on: 12 de abr de 2017
 *      Author: carlosfaruolo
 */

#include "engine_sound.hpp"

#include <cmath>

using std::string;
using std::vector;
using fgeal::Sound;

EngineSoundSimulator::~EngineSoundSimulator()
{
	freeAssetsData();
}

void EngineSoundSimulator::setProfile(const EngineSoundProfile& profile, short maxRpm)
{
	this->profile = profile;  // copies the profile (preserves the original intact)
	this->simulatedMaximumRpm = maxRpm;
}

void EngineSoundSimulator::loadAssetsData()
{
	if(not soundData.empty())
		this->freeAssetsData();

	// loads sound data
	for(unsigned i = 0; i < profile.ranges.size(); i++)
		this->soundData.push_back(new Sound(profile.ranges[i].soundFilename));
}

void EngineSoundSimulator::freeAssetsData()
{
	// cleanup
	for(unsigned i = 0; i < soundData.size(); i++)
		delete soundData[i];

	this->soundData.clear();
}

unsigned EngineSoundSimulator::getRangeIndex(float rpm)
{
	unsigned rangeIndex = 0;
	for(unsigned i = 0; i < soundData.size(); i++)
		if(rpm > profile.ranges[i].startRpm)
			rangeIndex = i;

	return rangeIndex;
}

vector<Sound*>& EngineSoundSimulator::getSoundData()
{
	return soundData;
}

void EngineSoundSimulator::play()
{
	if(not profile.ranges.empty())
		update(profile.ranges[0].startRpm+1);  //XXX this +1 may be unneccessary
}

void EngineSoundSimulator::update(float currentRpm)
{
	const unsigned soundCount = soundData.size();
	if(soundCount > 0 and currentRpm > 0) // its no use if there is no engine sound or rpm is too low
	{
		// establish the current range index
		const unsigned currentRangeIndex = this->getRangeIndex(currentRpm);

		// some aliases
		Sound& currentSound = *soundData[currentRangeIndex];
		const float currentRangeLowerRpm = profile.ranges[currentRangeIndex].startRpm,
					currentRangeUpperRpm = currentRangeIndex + 1 < soundCount? profile.ranges[currentRangeIndex + 1].startRpm : simulatedMaximumRpm,
					currentRangeSize = currentRangeUpperRpm - currentRangeLowerRpm,
					currentRangeSoundDepictedRpm = profile.ranges[currentRangeIndex].depictedRpm;

		currentSound.setVolume(volume);

		if(profile.allowRpmPitching)
			currentSound.setPlaybackSpeed(currentRpm/currentRangeSoundDepictedRpm, true);

		if(not currentSound.isPlaying())
			currentSound.loop();

		for(unsigned i = 0; i < soundData.size(); i++)
		{
			// some aliases
			Sound& rangeSound = *soundData[i];
			const short rangeSoundDepictedRpm = profile.ranges[i].depictedRpm;

			// current range
			if(i == currentRangeIndex)
				continue;  // already handled

			// preceding range
			else if(i + 1 == currentRangeIndex                                      // this range is preceding the current range
					and currentRpm - currentRangeLowerRpm < 0.25*currentRangeSize)  // current RPM is within 0-25% of current range
			{
//				snd.setVolume(1.0 - 4*(currentRpm - lowerRpmCurrent)/rangeSizeCurrent);  // linear fade out
				rangeSound.setVolume(volume * sqrt(1-16*pow((currentRpm - currentRangeLowerRpm)/currentRangeSize, 2)));  // quadratic fade out

				if(profile.allowRpmPitching)
					rangeSound.setPlaybackSpeed(currentRpm/rangeSoundDepictedRpm, true);

				if(not rangeSound.isPlaying())
					rangeSound.loop();
			}

			// succeeding range
			else if(i == currentRangeIndex + 1                                      // this range is succeeding the current range
					and currentRpm - currentRangeLowerRpm > 0.75*currentRangeSize)  // current RPM is within 75-100% of current range
			{
//				snd.setVolume(-3.0 + 4*(currentRpm - lowerRpmCurrent)/rangeSizeCurrent);  // linear fade in
				rangeSound.setVolume(volume * sqrt(1-pow(4*((currentRpm - currentRangeLowerRpm)/currentRangeSize)-4, 2)));  // quadratic fade in

				if(profile.allowRpmPitching)
					rangeSound.setPlaybackSpeed(currentRpm/rangeSoundDepictedRpm, true);

				if(not rangeSound.isPlaying())
					rangeSound.loop();
			}

			else rangeSound.stop();
		}
	}
}

void EngineSoundSimulator::halt()
{
	for(unsigned i = 0; i < soundData.size(); i++)
		soundData[i]->stop();
}

void EngineSoundSimulator::setVolume(float vol)
{
	this->volume = (vol < 0.f? 0.f : vol > 1.f? 1.f : vol);
}

float EngineSoundSimulator::getVolume() const
{
	return this->volume;
}
