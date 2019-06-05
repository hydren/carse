/*
 * util.h
 *
 *  Created on: 6 de dez de 2017
 *      Author: carlosfaruolo
 */

#ifndef UTIL_HPP_
#define UTIL_HPP_

#include "fgeal/filesystem.hpp"

#include <string>

#define scaledToSize(imgPtr, size) (size).getWidth()/(float)((imgPtr)->getWidth()), (size).getHeight()/(float)((imgPtr)->getHeight())
#define scaledToRect(imgPtr, rect) (rect).w/(float)((imgPtr)->getWidth()), (rect).h/(float)((imgPtr)->getHeight())

// functor to produce a resolution-relative size (size based on a 480px tall display)
struct FontSizer
{
	const unsigned referenceSize;
	FontSizer(unsigned rs) : referenceSize(rs) {}
	inline unsigned operator()(unsigned size) const { return size * referenceSize / 480.f; }
};

template <typename T>
inline int sgn(T val)
{
    return (T(0) < val) - (val < T(0));
}

template <typename T>
static inline T pow2(T val)
{
	return val*val;
}

inline float fractional_part(float value)
{
	return value - (int) value;
}

#if __cplusplus < 201103L
	inline double trunc(double d){ return (d>0) ? floor(d) : ceil(d) ; }
#endif

// returns a spaced outline, like a margin or something
inline fgeal::Rectangle getSpacedOutline(const fgeal::Rectangle& bounds, float spacing)
{
	const fgeal::Rectangle outline = {
		bounds.x - spacing,
		bounds.y - spacing,
		bounds.w + 2*spacing,
		bounds.h + 2*spacing
	};

	return outline;
}

/** Attempt to get a contextualized filename.
 *  First it attempts to check if "baseDir1 + specifiedFilename" is a valid file and returns it if true.
 *  If not, then it tries the same with "baseDir2 + specifiedFilename".
 *  If not, then it tries the same with "baseDir3 + specifiedFilename".
 *  If not, then it tries the same with "current working dir. + specifiedFilename".
 *  If not, then it tries the same with "specifiedFilename" alone by itself.
 *  If not, then we could not come up with a valid filename and an empty string is returned. */
inline std::string getContextualizedFilename(const std::string& specifiedFilename, const std::string& baseDir1, const std::string& baseDir2, const std::string& baseDir3)
{
	if(fgeal::filesystem::isFilenameArchive(baseDir1 + specifiedFilename))
		return baseDir1 + specifiedFilename;

	if(fgeal::filesystem::isFilenameArchive(baseDir2 + specifiedFilename))
		return baseDir2 + specifiedFilename;

	if(fgeal::filesystem::isFilenameArchive(baseDir3 + specifiedFilename))
		return baseDir3 + specifiedFilename;

	if(fgeal::filesystem::isFilenameArchive(fgeal::filesystem::getCurrentWorkingDirectory() + specifiedFilename))
		return fgeal::filesystem::getCurrentWorkingDirectory() + specifiedFilename;

	if(fgeal::filesystem::isFilenameArchive(specifiedFilename))
		return specifiedFilename;

	return std::string();
}

/// Same as the 4-argument version, but with only two "baseDir" option.
inline std::string getContextualizedFilename(const std::string& specifiedFilename, const std::string& baseDir1, const std::string& baseDir2)
{
	return getContextualizedFilename(specifiedFilename, baseDir1, baseDir2, baseDir2);
}

/// Same as the 3-argument version, but with only one "baseDir" option.
inline std::string getContextualizedFilename(const std::string& specifiedFilename, const std::string& baseDir)
{
	return getContextualizedFilename(specifiedFilename, baseDir, baseDir, baseDir);
}

#endif /* UTIL_HPP_ */
