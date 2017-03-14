/*
 * track.hpp
 *
 *  Created on: 13/09/2014
 *      Author: felipe
 */

#ifndef TRACK_HPP_
#define TRACK_HPP_

#include <vector>
#include <string>

/** The track is specified by stretches, and in between, it is interpolated.
 * Implement track drawing by drawing rectangles (primitives) with offsets.
 * examples: Top Gear series, Lotus Challenge series, Rad Racer series, Chase HQ series, F-Zero,
 * 8-bit Rally: https://itunes.apple.com/app/8-bit-rally/id441201921?mt=8
 */
struct Track
{
	struct Stretch
	{
		float stretchLength;
		float angleDelta;
		float heightDelta;
		int roadTypeID;
		int grassTypeID;
	};

	//stretches to interpolate
	std::vector<Stretch> stretches;

	//general info
	std::string name;

	float getLength() { return length; }

	protected:
	float length; //needs to be updated when stretches changes
};

#endif /* TRACK_HPP_ */
