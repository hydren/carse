/*
 * engine_sound.cpp
 *
 *  Created on: 12 de abr de 2017
 *      Author: carlosfaruolo
 */

#include "engine_sound.hpp"

using util::Properties;
using std::string;

EngineSoundProfile::EngineSoundProfile()
: ranges(), treatLastRangeAsRedline(false)
{}

EngineSoundProfile EngineSoundProfile::loadFromProperties(const Properties& prop, short maxRpm, bool forceCustomOnly)
{
	EngineSoundProfile profile;

	string key = "sound";
	if(prop.containsKey(key))
	{
		if(prop.get(key) != "custom" and not forceCustomOnly)
		{
			string soundOption = prop.get(key);
			profile.ranges.clear();
			if(soundOption == "no")
				profile.ranges.clear();

			// todo create engine sound classes: default, crossplane_v8, inline_6, flat_4, etc
		}
		else if(prop.get(key) == "custom")
		{
			key = "sound_redline_last";
			if(prop.containsKey(key) and prop.get(key) == "true")
				profile.treatLastRangeAsRedline = true;

			int i = 0;
			key = "sound0";
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
					else       rpm = (maxRpm - profile.ranges.rbegin()->rpm)/2;
				}

				// save filename for given rpm
				profile.ranges[rpm] = filename;
				i += 1;
				key = string("sound") + i;
			}
		}
	}

	return profile;
}
