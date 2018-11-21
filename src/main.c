
/*
TODO

Genesis Bugs:
- man sprite hidden by pad sprite, read up on sorting, and transparency
- game often loads up with jacked up tiles from the beginning needing to be reset
- or the game puts VDP into bad sync mode
- Got a stack dump, probably invalid address or memory issue, corruption

Thruster noise
page edge borders
Up Please, then high score screen, then restart back to main menu
3 cabby lives, then main menu

sampled "hey taxi" sound

*/

#include "genesis.h"
#include "sprite.h"
#include "main.h"

#define TEXT_WIDTH 40
#define START_Y_POSITION 70
#define START_X_POSITION 50
#define REFRESH_RATE 3	// control the speed of the game, smaller is faster

u16 game_state=0;    // gets set to 1 when "start" button is pressed at main menu
u16 game_over=0;

struct taxi_state game_taxi;
Sprite *taxi;
struct pad pads[3];
struct man theman;

void drawTextCenter(char *msg, char y)
{
  VDP_drawText(msg, (TEXT_WIDTH/2)-(strlen(msg)/2), y);
}

static void joyEvent(u16 joy, u16 changed, u16 state)
{
	/*//debug code to see event parameters
	char buf[250];
	sprintf(buf, "  C%d S%d  ", changed, state);
	VDP_drawText(buf, 30, 27); */

	// check if START button is pressed
	if (game_state == 0 && changed == 128 && state == 128)
	{
		game_state=1;
	}

	if (game_state == 1) // check if game is started
	{
		// set the varios thruster positions in taxi based on joystick input
		game_taxi.top = (BUTTON_UP & state) ? 1 : 0;
		game_taxi.bottom = (BUTTON_DOWN & state) ? 1 : 0;
		game_taxi.left = (BUTTON_LEFT & state) ? 1 : 0;
		game_taxi.right = (BUTTON_RIGHT & state) ? 1 : 0;
		game_taxi.lander = (BUTTON_A & state) ? 1 : 0;
	}
}


void displayStartMenu()
{
    drawTextCenter("Rocket Taxi", 6);
    drawTextCenter("Press START to play", 15);
    drawTextCenter("by Jean-Christophe Smith", 18);
    drawTextCenter("js@intrusoft.com", 19);
    drawTextCenter("Inspired by Commodore 64 \"Space Taxi\"", 25);
    drawTextCenter("by John F. Kutcher (1984)", 26);

    SPR_init(0, 0, 0); // TODO, make sure I'm calling this correctly

    // setup initial TAXI sprite that will actually carry over into game mode
    taxi = SPR_addSprite(&taxi_sprite, 100, 75, TILE_ATTR_FULL(PAL2, TRUE, FALSE, FALSE, 0));
    SPR_update(taxi, 80);

    // setup the pallets for the taxi/man(shared) and pad
    VDP_setPalette(PAL2, taxi_sprite.palette->data);
    VDP_setPalette(PAL3, pad_sprite.palette->data);
}

/*
 * Clear text written in start menu
 */
void clearMenu()
{
	for(int i=0;i<27;i++)
	VDP_clearText(0, i, 40);
}

/*
 * Setup the pads in a specific configuration
 */
void setupPads()
{
	for(int i=0;i<3;i++)
	{
		pads[i].x = 90 * i+40;
		pads[i].y = 60 * i+40;
		pads[i].spr = SPR_addSprite(&pad_sprite, pads[i].x, pads[i].y, TILE_ATTR_FULL(PAL3, FALSE, FALSE, FALSE, 0));
		SPR_setFrame(pads[i].spr, i);
		pads[i].num = i+1;
	}
}

/*
 * Setup the sprite for the man riding in the taxi
 */
void setupMan()
{
	int chosen_pad = random() % 3;	// choose random pads
	theman.x = pads[chosen_pad].x+10;
	theman.y = pads[chosen_pad].y-8;
	theman.fare = 1000;	// start fare at $10.00

	theman.current_pad = chosen_pad; //1
	theman.destination_pad = random() % 3; //2
	// if I happen to pick the same destination as current, then change destination
	if (theman.current_pad == theman.destination_pad)
	{
		theman.destination_pad++;
		if (theman.destination_pad==3) theman.destination_pad =0;
	}
	theman.enroute = 0;	//flag to know if man is riding in cab yet

	// if this is the first time called we create sprite, otherwise just reposition the existing one
	if (!theman.spr)
		{
			theman.spr = SPR_addSprite(&man_sprite, theman.x, theman.y, TILE_ATTR_FULL(PAL2, TRUE, FALSE, FALSE, 0));
		}
	else
	{
		SPR_setPosition(theman.spr, theman.x, theman.y);
	}
	drawTextCenter("        HEY, TAXI!         ", 27);
}

/*
 * Main loop for game play
 */
void startGame()
{
	clearMenu();
	setupPads();
	setupMan();

	game_taxi.top = 0;
	game_taxi.bottom = 0;
	game_taxi.left = 0;
	game_taxi.right = 0;
	game_taxi.lander = 0;
	game_taxi.crashed =0;
	game_over = 0;

	u16 y=START_Y_POSITION;
	u16 x=START_X_POSITION;
	u16 landed=0;

	SPR_setPosition(taxi, y, x);
	game_taxi.pad = -1;
	game_taxi.pay = 0;

	char flapper=1;
	char buff[100];

	u16 i=0;
	u16 c=0;

    while(1)
    {
    	VDP_waitVSync();	// import sega stuff


    	if (i++ % REFRESH_RATE == 0)  // don't run all this code on every vsync, skip some
    	{
    		if (i % 5 == 0)	// animate the man, either as waving or as walking toward taxi
    		{
    			// shift between frames, ugly hack for now, fix with sprite animation later
    			if (theman.current_pad == game_taxi.pad)
    			{
    				SPR_setFrame(theman.spr, flapper%2+2);
    			}
    			else
    			{
    				SPR_setFrame(theman.spr, flapper%2);
    			}
    		flapper++;
    		}


			if (!landed)
			{
					y++;	// let gravity pull the taxi downward
					VDP_drawText("X", 1, 27);
			}

			if (game_taxi.crashed)	// the taxi is in the process of crashing down
			{
				y+=10;
				game_taxi.lander=1;
				game_taxi.bottom=0;
				game_taxi.top=0;
				game_taxi.left=0;
				game_taxi.right=0;
			}

			if (!game_taxi.lander)  // toggle the landing gear sprite frame or not
				SPR_setFrame(taxi, 4);
			else
				SPR_setFrame(taxi, 0);

			if (game_taxi.top == 1)	// taxi is thrusting up
			{
				y-= 5;
				VDP_drawText("^", 1, 27);
				landed=0;	// if you were landed, this would reset that
				game_taxi.pad = -1;
				SPR_setFrame(taxi, 1);
			}

			if (game_taxi.bottom == 1)
			{
				y+=5;
				VDP_drawText(".", 1, 27);
				SPR_setFrame(taxi, 3);
			}

			if (game_taxi.right == 1 && landed==0)
			{
				x+=5;
				VDP_drawText(">", 1, 27);
				SPR_setFrame(taxi, 2);
				SPR_setHFlip(taxi, 0);	// turn taxi in direction you are going
			}
			if (game_taxi.left == 1 && landed==0)
			{
				x-=5;
				VDP_drawText("<", 1, 27);
				SPR_setFrame(taxi, 2);
				SPR_setHFlip(taxi, 1); // turn taxi in direction you are going
			}

			// landing bounding, detect if taxi landed on a pad
			for (c=0;c<3;c++)
			{
				if ((x >= pads[c].x-5 && x <= pads[c].x+5) && pads[c].y-15 == y)
				{
					if (game_taxi.lander)
						{
							landed = 1;
							game_taxi.pad = c;
						}

				}
			}

			if (!game_taxi.lander) landed=0;	// if landing gear is pulled up, you are no longer considered landed

			// handle collision with pad
			// sizeof(pads) caused a nasty bug, = 30 not 3
			for (int c=0;c<3;c++)
			{
				if ((x >= pads[c].x-29 && x <= pads[c].x+29) && (y >= pads[c].y-5 && y <= pads[c].y+15))
				{
					// crash!
					SPR_setFrame(taxi, 0);
					SPR_setVFlip(taxi, 1);
					game_taxi.crashed = 1;

					drawTextCenter("   TAXI CRASHED!   ", 23);
					game_over=1;

				}
			}

			// detect if TAXI landed on pad with man on it
			if(landed == 1 && game_taxi.pad == theman.current_pad)
			{
				SPR_setPosition(theman.spr, theman.x--, theman.y); // man, walk towards TAXI
				if (x == theman.x)	// if man and taxi have same x-axis, then man can go in the TAXI
				{
					sprintf(buff, "          PAD %d, PLEASE!         ", pads[theman.destination_pad].num);
					drawTextCenter(buff, 27);
					theman.enroute = 1;
					SPR_setNeverVisible(theman.spr, TRUE);

				}
			}

			// if the man was dropped off, and we hopefully let several cycles go by so there is a delay between spawns
			if (theman.enroute == 2 && i % 100 == 0)
			{
				setupMan();
				theman.enroute = 0;
			}

			// detect if the TAXI dropped the man off on the correct pad
			if(landed == 1 && game_taxi.pad == theman.destination_pad && theman.enroute ==1)
			{
				theman.x = x;
				theman.y = y+8;
				SPR_setPosition(theman.spr, theman.x, theman.y);
				drawTextCenter("            THANKS!           ", 27);
				theman.enroute = 2;
				SPR_setNeverVisible(theman.spr, FALSE);	// hide temporarily
				SPR_setAlwaysVisible(theman.spr, TRUE);
				game_taxi.pay += theman.fare;

			}


			// occasionally update the amount of money taxi has been paid, and the current fare
			if (i%10==0)
			{
				sprintf(buff, "FARE:$%d.%2d ", theman.fare/100, theman.fare%100);
				VDP_drawText(buff, 29, 27);
				sprintf(buff, "PAY:$%d.%2d ", game_taxi.pay/100, game_taxi.pay%100);
				VDP_drawText(buff, 29, 26);
				theman.fare-=5;		// the longer you take, the less money you get
			}

			// If you fly too low, we just trigger that you landed on the floor
			if (y >= 200)
			{
				y=199;
				landed = 1;
			}

			SPR_setPosition(taxi, x, y);	// update the sprite position based on above logic
			SPR_update();
    	}
    	// if game over was detected we break out of the main game loop
		if (game_over)
			{
				game_over++;
				if (game_over > 400 )
					{
						SPR_setFrame(taxi, 0);
						SPR_setVFlip(taxi, 0);
						break;
					}
			}
    }

}

int main(void)
{
        SYS_disableInts();
        VDP_clearPlan(PLAN_A, 0);
        VDP_clearPlan(PLAN_B, 0);
        SPR_init(0, 0, 0);

        SYS_enableInts();
        JOY_setEventHandler(&joyEvent);

        displayStartMenu();

        VDP_waitVSync();

	while(1)
	{
		    VDP_waitVSync();
		    if (game_state == 1)
		    	startGame();


	}
}
