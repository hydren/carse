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
		this->soundData.push_back(new Sound(profile.ranges[i].soundFilename));

	this->simulatedMaximumRpm = maxRpm;
}

void EngineSoundSimulator::setSimulatedMaximumRpm(short rpm)
{
	this->simulatedMaximumRpm = rpm;
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

float EngineSoundSimulator::calculatePitch(float rpmDiff)
{
//	return exp(rpmDiff/simulatedMaximumRpm);
	return profile.pitchVariationFactor*rpmDiff;
}

void EngineSoundSimulator::playIdle()
{
	if(not profile.ranges.empty())
		updateSound(profile.ranges[0].startRpm+1);  //xxx this +1 may be unneccessary
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
		const float lowerRpmCurrent = profile.ranges[currentRangeIndex].startRpm;
		const float upperRpmCurrent = currentRangeIndex + 1 < soundCount? profile.ranges[currentRangeIndex + 1].startRpm : simulatedMaximumRpm;
		const float rangeSizeCurrent = upperRpmCurrent - lowerRpmCurrent;

		currentSound.setVolume(1.0f);
		if(profile.allowRangePitch)
			currentSound.setPlaybackSpeed(calculatePitch(currentRpm - lowerRpmCurrent), true);

		if(not currentSound.isPlaying())
			currentSound.loop();

		for(unsigned i = 0; i < soundData.size(); i++)
		{
			// some aliases
			Sound& rangeSound = *soundData[i];
			const short rangeRpm = profile.ranges[i].startRpm;

			// current range
			if(i == currentRangeIndex)
				continue;  // already handled

			// preceding range
			else if(i + 1 == currentRangeIndex                                    // this range is preceding the current range
					and currentRpm - lowerRpmCurrent < 0.25*rangeSizeCurrent)     // current RPM is within 0-25% of current range
			{
//				snd.setVolume(1.0 - 4*(currentRpm - lowerRpmCurrent)/rangeSizeCurrent); // linear fade out
				rangeSound.setVolume(sqrt(1-16*pow((currentRpm - lowerRpmCurrent)/rangeSizeCurrent, 2))); // quadratic fade out

				rangeSound.setPlaybackSpeed(calculatePitch(currentRpm - rangeRpm), true);
				if(not rangeSound.isPlaying())
					rangeSound.loop();
			}

			// succeeding range
			else if(i == currentRangeIndex + 1                                       // this range is succeeding the current range
					and currentRpm - lowerRpmCurrent > 0.75*rangeSizeCurrent)         // current RPM is within 75-100% of current range
			{
//				snd.setVolume(-3.0 + 4*(currentRpm - lowerRpmCurrent)/rangeSizeCurrent); // linear fade in
				rangeSound.setVolume(sqrt(1-pow(4*((currentRpm - lowerRpmCurrent)/rangeSizeCurrent)-4, 2)) ); // quadratic fade in

				rangeSound.setPlaybackSpeed(calculatePitch(currentRpm - rangeRpm), true);
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
