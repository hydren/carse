/*
 * race_only_args.hpp
 *
 *  Created on: 22 de jul de 2019
 *      Author: carlos.faruolo
 */

#ifndef RACE_ONLY_ARGS_HPP_
#define RACE_ONLY_ARGS_HPP_

#include <tclap/SwitchArg.h>
#include <tclap/ValueArg.h>

namespace RaceOnlyArgs
{
	extern TCLAP::SwitchArg randomCourse, debugMode;
	extern TCLAP::ValueArg<int> vehicleAlternateSpriteIndex;
	extern TCLAP::ValueArg<unsigned> raceType, lapCount, courseIndex, vehicleIndex, simulationType;
}

#endif /* RACE_ONLY_ARGS_HPP_ */
