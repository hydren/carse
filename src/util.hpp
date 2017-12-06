/*
 * util.h
 *
 *  Created on: 6 de dez de 2017
 *      Author: carlosfaruolo
 */

#ifndef UTIL_HPP_
#define UTIL_HPP_

#define scaledToSize(imgPtr, size) (size).getWidth()/(float)((imgPtr)->getWidth()), (size).getHeight()/(float)((imgPtr)->getHeight())
#define scaledToRect(imgPtr, rect) (rect).w/(float)((imgPtr)->getWidth()), (rect).h/(float)((imgPtr)->getHeight())

// device-independent pixel; can only be used if there is a 'display' instance in the scope
#define dip(px) (px*(display.getHeight()/480.0))

#endif /* UTIL_HPP_ */
