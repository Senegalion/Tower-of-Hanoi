#include "primlib.h"
#include <stdlib.h>
#include <unistd.h>

#define DISKS 70
#define PEGS 8

#define GROUND_THICKNESS 40
#define DISK_THICKNESS PEG_LENGTH / (DISKS + 1)
#define PEG_THICKNESS 20

#define PEG_LENGTH 0.5 * (gfx_screenHeight() - GROUND_THICKNESS)

#define BARRIER_Y_COORDINATE gfx_screenHeight() - GROUND_THICKNESS - PEG_LENGTH - 80

#define DISK_SPEED 1

#define FONT_SIZE 40

int gamestate[PEGS][DISKS];
float disks_coordinates[DISKS][2];
float pegs_coordinates[PEGS];
int disks_widths[DISKS];

int game_over = 0;

int picked_up_disk = 0;

int source_peg;
int destination_peg;

int temporary_disk;
int temporary_value;

int prev_disk_pos;

int destination_disk;

int peg_is_empty;
int valid_move;

void init_game()
{
	for (int peg = 0; peg < PEGS; peg++)
	{
		for (int disk = 0; disk < DISKS; disk++)
		{
			gamestate[peg][disk] = 0;
		}
	}

	float x_coordinate = gfx_screenWidth() / (PEGS + 1);

	for (int peg = 0; peg < PEGS; peg++)
	{
		pegs_coordinates[peg] = x_coordinate * (peg + 1);
	}

	int difference_between_pegs = pegs_coordinates[0];
	float y_coordinate = gfx_screenHeight() - GROUND_THICKNESS + 0.5 * DISK_THICKNESS - DISKS * DISK_THICKNESS;

	for (int disk = 0; disk < DISKS; disk++)
	{
		gamestate[0][disk] = disk + 1;
		disks_coordinates[disk][0] = difference_between_pegs;
		disks_coordinates[disk][1] = y_coordinate + DISK_THICKNESS * disk;

		disks_widths[disk] = PEG_THICKNESS + 2 * (0.5 * difference_between_pegs * (disk + 1) / DISKS);
	}
}

void draw_pegs()
{
	float upper_left_corner_y = gfx_screenHeight() - GROUND_THICKNESS - PEG_LENGTH;
	float lower_right_corner_y = gfx_screenHeight() - GROUND_THICKNESS;

	for (int peg = 0; peg < PEGS; peg++)
	{
		float upper_left_corner_x = pegs_coordinates[peg] - PEG_THICKNESS / 2;
		float lower_right_corner_x = pegs_coordinates[peg] + PEG_THICKNESS / 2;
		gfx_filledRect(upper_left_corner_x, upper_left_corner_y, lower_right_corner_x, lower_right_corner_y, RED);
	}
}

void draw_disks()
{
	for (int disk = 0; disk < DISKS; disk++)
	{
		float upper_left_corner_x = disks_coordinates[disk][0] - 0.5 * disks_widths[disk];
		float upper_left_corner_y = disks_coordinates[disk][1] - 0.5 * DISK_THICKNESS;
		float lower_right_corner_x = disks_coordinates[disk][0] + 0.5 * disks_widths[disk];
		float lower_right_corner_y = disks_coordinates[disk][1] + 0.5 * DISK_THICKNESS;
		gfx_filledRect(upper_left_corner_x, upper_left_corner_y, lower_right_corner_x, lower_right_corner_y, BLUE);
		gfx_rect(upper_left_corner_x, upper_left_corner_y, lower_right_corner_x, lower_right_corner_y, GREEN);
	}
}

void draw_gamestate()
{
	gfx_filledRect(0, 0, gfx_screenWidth(), gfx_screenHeight(), BLACK);
	gfx_filledRect(0, gfx_screenHeight() - GROUND_THICKNESS, gfx_screenWidth(), gfx_screenHeight(), YELLOW);
	draw_pegs();
	draw_disks();
	gfx_updateScreen();
}

void check_peg_capacity(int source_peg)
{
	int disk_counter = 0;
	if (source_peg == -1) // SDLK_0
	{
		source_peg = 9;
	}

	if (source_peg < PEGS)
	{
		for (int disk = 0; disk < DISKS; disk++)
		{
			if (gamestate[source_peg][disk] != 0)
			{
				disk_counter++;
			}
		}
	}

	if (disk_counter == 0)
	{
		peg_is_empty = 1;
	}
	else
	{
		peg_is_empty = 0;
	}
}

void get_key(int *clicked_peg)
{
	int keycode;
	int is_valid_key = 1;
	do
	{
		if (picked_up_disk == 0)
		{
			do
			{
				keycode = gfx_getkey();
				if (keycode >= SDLK_0 && keycode % SDLK_0 <= PEGS)
				{
					check_peg_capacity(keycode % SDLK_0 - 1);
				}
			} while (peg_is_empty == 1);
		}
		else
		{
			do
			{
				keycode = gfx_getkey();
				if (keycode == SDLK_0)
				{
					if (PEGS != 10)
					{
						is_valid_key = 0;
					}
				}
				else
				{
					is_valid_key = 1;
				}
			} while (is_valid_key == 0);
		}	
	} while (keycode != SDLK_ESCAPE && keycode != '\r' && !(keycode >= SDLK_0 && keycode % SDLK_0 <= PEGS && peg_is_empty == 0));
	if (keycode == SDLK_ESCAPE || keycode == '\r')
	{
		game_over = 1;
	}
	else if (keycode > SDLK_0 && keycode % SDLK_0 <= PEGS)
	{
		*clicked_peg = keycode % SDLK_0 - 1;
	}
	else if (keycode == SDLK_0 && PEGS == 10)
	{
		*clicked_peg = 9;
	}
}

void pick_up_disk()
{
	if (game_over == 0)
	{
		for (int disk = 0; disk < DISKS; disk++)
		{
			if (gamestate[source_peg][disk] > 0)
			{
				while (disks_coordinates[gamestate[source_peg][disk] - 1][1] > BARRIER_Y_COORDINATE)
				{
					disks_coordinates[gamestate[source_peg][disk] - 1][1] -= DISK_SPEED;
					draw_gamestate();
				}

				if (disks_coordinates[gamestate[source_peg][disk] - 1][1] <= BARRIER_Y_COORDINATE)
				{
					picked_up_disk = 1;
					temporary_disk = gamestate[source_peg][disk] - 1;
					prev_disk_pos = disk;
					temporary_value = gamestate[source_peg][disk];
				}
				break;
			}
		}
	}
}

void move_up()
{
	get_key(&source_peg);
	pick_up_disk();
}

void check_size()
{
	int index;
	if (destination_disk < DISKS - 1)
	{
		index = destination_disk + 1;
	}
	else
	{
		index = destination_disk;
	}

	if (temporary_value <= gamestate[destination_peg][index] || gamestate[destination_peg][index] == 0)
	{
		valid_move = 1;
	}
	else
	{
		valid_move = 0;
	}
}

void check_move_down()
{
	if (game_over == 0)
	{
		for (int disk = DISKS - 1; disk >= 0; disk--)
		{
			if (destination_peg == source_peg)
			{
				if (gamestate[destination_peg][disk] == 0)
				{
					destination_disk = disk + 1;
					break;
				}
			}
			else if (gamestate[destination_peg][disk] == 0)
			{
				destination_disk = disk;
				break;
			}
		}

		check_size();
		if (!valid_move)
		{
			get_key(&destination_peg);
			check_move_down();
		}
	}
}

void move_left_or_right()
{
	if (game_over == 0)
	{
		if (source_peg != destination_peg)
		{
			if (source_peg < destination_peg)
			{
				while (disks_coordinates[temporary_disk][0] < pegs_coordinates[destination_peg])
				{
					disks_coordinates[temporary_disk][0] += DISK_SPEED;
					draw_gamestate();
				}
			}
			else if (destination_peg < source_peg)
			{
				while (pegs_coordinates[destination_peg] < disks_coordinates[temporary_disk][0])
				{
					disks_coordinates[temporary_disk][0] -= DISK_SPEED;
					draw_gamestate();
				}
			}
		}
	}
}

void move_down()
{
	if (game_over == 0)
	{
		while (disks_coordinates[temporary_disk][1] < gfx_screenHeight() - GROUND_THICKNESS + 0.5 * DISK_THICKNESS - (DISKS - destination_disk) * DISK_THICKNESS)
		{
			disks_coordinates[temporary_disk][1] += DISK_SPEED;
			draw_gamestate();
		}

		if (disks_coordinates[temporary_disk][1] >= gfx_screenHeight() - GROUND_THICKNESS + 0.5 * DISK_THICKNESS - (DISKS - destination_disk) * DISK_THICKNESS)
		{
			picked_up_disk = 0;
			gamestate[source_peg][prev_disk_pos] = 0;
			gamestate[destination_peg][destination_disk] = temporary_value;
		}
	}
}

void move()
{
	get_key(&destination_peg);
	check_move_down();
	move_left_or_right();
	move_down();
}

void check_win()
{
	int victory = 1;

	for (int peg = PEGS - 1, disk = 0; disk < DISKS; disk++)
	{
		if (gamestate[peg][disk] != disk + 1)
		{
			victory = 0;
		}
	}

	if (victory)
	{
		gfx_filledRect(0, 0, gfx_screenWidth(), gfx_screenHeight(), BLACK);
		gfx_textout(gfx_screenWidth() / 2 - 2 * FONT_SIZE, gfx_screenHeight() / 2 - FONT_SIZE / 4, "CONGRATULATIONS! YOU WIN!", CYAN);
		gfx_updateScreen();
		game_over = 1;
		int keycode;
		do
		{
			keycode = gfx_getkey();
		} while (keycode != SDLK_ESCAPE && keycode != '\r');
	}
}

int main(int argc, char *argv[])
{
	if (gfx_init())
	{
		exit(3);
	}

	init_game();

	while (!game_over)
	{
		draw_gamestate();

		if (picked_up_disk == 0)
		{
			move_up();
		}
		else if (picked_up_disk == 1)
		{
			move();
		}
		check_win();
	}

	return 0;
}
