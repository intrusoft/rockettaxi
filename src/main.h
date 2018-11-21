/*
 * main.h
 *
 *  Created on: Aug 11, 2018
 *      Author: jc
 */

#ifndef SRC_MAIN_H_
#define SRC_MAIN_H_

struct taxi_state
{
	u16 top;
	u16 bottom;
	u16 right;
	u16 left;
	u16 lander;
	u16 crashed;
	u16 pad;
	u16 pay;
};

struct pad
{
	u16 x;
	u16 y;
	Sprite *spr;
	u16 num;
};

struct man
{
	u16 x;
	u16 y;
	u16 current_pad;
	u16 destination_pad;
	u16 enroute;
	u16 fare;
	Sprite *spr;

};



#endif /* SRC_MAIN_H_ */
