/*
 * rect.hpp
 *
 *  Created on: 21/08/2014
 *      Author: felipe
 */

#ifndef RECT_HPP_
#define RECT_HPP_

/** Class that contains position and size of a box.
 * It has a method to check for intersection between other Rect's.
 * It would be named Rectangle but there is a internal name conflict in windows' and allegro's headers that prevents from using the name. For more info see: https://www.allegro.cc/forums/thread/612387
 */
struct Rect
{
    float x, y, w, h;

    Rect() :
    x(0),y(0),w(0),h(0) {}

    //Creates a Rect with the given parameters
    Rect(float x1, float y1, float w1, float h1) :
    x(x1), y(y1), w(w1), h(h1) {}

    //Returns true if this bounding box intersects with the given Rect.
    bool intersects(const Rect& box );

    /* Returns a Rect that represents the intersection of this Rect with the given Rect.
     * If there is no intersection, throws an Exception. TODO exchange this behavior with returning null
     * In other words, you should check if the Rects intersects before trying to call this method, with the intersects() method.
     */
    Rect intersection(const Rect& box);

    /* Returns the area of this Rect.
     * The calculation is pretty silly: w*h (width x height)
     */
    int area();
};

#endif /* RECT_HPP_ */
