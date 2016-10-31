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
	Race();
	~Race();

	void run();

	private:
	void handleInput();
	void handleRender();
	void handlePhysics();
};



#endif /* RACE_HPP_ */
