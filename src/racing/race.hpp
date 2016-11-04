/*
 * race.hpp
 *
 *  Created on: 25/08/2014
 *      Author: carlosfaruolo
 */

#ifndef RACE_HPP_
#define RACE_HPP_

struct Race
{
	struct Implementation;
	Implementation* self;

	~Race();

	void load();
	void start();
};

#endif /* RACE_HPP_ */
