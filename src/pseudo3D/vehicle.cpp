/*
 * vehicle.cpp
 *
 *  Created on: 6 de abr de 2017
 *      Author: carlosfaruolo
 */

#include "vehicle.hpp"

#include "futil/string/more_operators.hpp"

#include <cstdlib>

using util::Properties;
using std::map;
using std::string;

Vehicle::Vehicle()
: isLastSoundRedline(false),
  mass(1250)
{}

Vehicle::Vehicle(const Properties& prop)
{
	string key;

	key = "vehicle_name";
	name = prop.containsKey(key)? prop.get(key) : "unnamed";

	key = "sprite_sheet_file";
	sheetFilename = prop.containsKey(key)? prop.get(key) : "assets/car.png";

	key = "vehicle_mass";
	if(prop.containsKey(key))
		mass = atof(prop.get(key).c_str());

	mass = prop.containsKey(key) and prop.get(key) != "default"? atof(prop.get(key).c_str()) : 1250;

	key = "engine_maximum_rpm";
	engine.maxRpm = prop.containsKey(key) and prop.get(key) != "default"? atoi(prop.get(key).c_str()) : 7000;

	key = "engine_torque";
	engine.torque = prop.containsKey(key) and prop.get(key) != "default"? atof(prop.get(key).c_str()) : 750;

	key = "wheel_radius";
	engine.wheelRadius = prop.containsKey(key) and prop.get(key) != "default"? atof(prop.get(key).c_str()) : 0.34;

	// todo read more data from properties

	key = "gear_count";
	engine.gearCount = prop.containsKey(key) and prop.get(key) != "default"? atoi(prop.get(key).c_str()) : 6;

	engine.gearRatio = new float[engine.gearCount+1];

	// first, set default ratios, then override
	engine.reverseGearRatio = 3.25;
	engine.gearRatio[0] = 3.0;
	for(int g = 1; g <= engine.gearCount; g++)
		engine.gearRatio[g] = 3.0 + 2.0*((g - 1.0)/(1.0 - engine.gearCount)); // generic gear ratio

	key = "gear_ratios";
	if(prop.containsKey(key))
	{
		string ratiosTxt = prop.get(key);
		if(ratiosTxt == "custom")
		{
			key = "gear_differential_ratio";
			if(prop.containsKey(key)) engine.gearRatio[0] = atof(prop.get(key).c_str());

			key = "gear_reverse_ratio";
			if(prop.containsKey(key)) engine.reverseGearRatio = atof(prop.get(key).c_str());

			for(int g = 1; g <= engine.gearCount; g++)
			{
				key = string("gear_") + g + "_ratio";
				if(prop.containsKey(key))
					engine.gearRatio[g] = atof(prop.get(key).c_str());
			}
		}
	}

	// first set default sounds
	soundsFilenames[0] = "assets/engine_idle.ogg";
	soundsFilenames[1250] = "assets/engine_high.ogg";
	isLastSoundRedline = false;

	key = "sound";
	if(prop.containsKey(key) and prop.get(key) != "default")
	{
		string soundOption = prop.get(key);
		soundsFilenames.clear();
		if(soundOption == "no")
			soundsFilenames.clear();

		// todo create engine sound classes: crossplane_v8, inline_6, flat_4, etc

		else if(soundOption == "custom")
		{
			key = "sound_redline_last";
			if(prop.containsKey(key) and prop.get(key) == "true")
				isLastSoundRedline = true;

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
					else       rpm = (engine.maxRpm - soundsFilenames.rbegin()->first)/2;
				}

				// save filename for given rpm
				soundsFilenames[rpm] = filename;
				i += 1;
				key = string("sound") + i;
			}
		}
	}
}
