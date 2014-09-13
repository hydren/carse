/*
 * track.hpp
 *
 *  Created on: 13/09/2014
 *      Author: felipe
 */

#ifndef TRACK_HPP_
#define TRACK_HPP_

#include "../util.hpp"

/** The track is specified by stretches, and in between, it is interpolated.
 * Implement track drawing by drawing rectangles (primitives) with offsets.
 * examples: Top Gear series, Lotus Challenge series, Rad Racer series, Chase HQ series, F-Zero,
 * 8-bit Rally: https://itunes.apple.com/app/8-bit-rally/id441201921?mt=8
 */
struct Track
{
	struct Stretch
	{
		float trackAngleDelta;
		float trackHeightDelta;
		int trackType;
		int grassType;
	};

	vector<Stretch> trackStretches; //stretches to interpolate
	string name;

	float length() { return estimatedTrackLength; }

	protected:
	float estimatedTrackLength; //read-only
};



#endif /* TRACK_HPP_ */
