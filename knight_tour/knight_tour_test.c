#include <stdio.h>       /* IO fucntion */ 
#include <stdlib.h>
#include <string.h>
#include "knight_tour.h" /* my header   */

#define CHESS_BOARD_SIZE 64
#define ROWS 8

void Test();

int main()
{
	Test();
	return (0);
}

void Test()
{
	unsigned char array[CHESS_BOARD_SIZE] = {0};
	unsigned char i = 0;
	unsigned char j = 0;
	unsigned char arr[CHESS_BOARD_SIZE] = {0};

	for (i = 0; i < CHESS_BOARD_SIZE; i++)
	{
		array[i] = i;
		printf("\n%d\n", i);
		KnightTour(arr, &array[i]);
		for (j = 0; j < CHESS_BOARD_SIZE; j++)
		{
			if (0 == j % ROWS)
			{
				printf("\n");
			}

			printf("%d, ", arr[j]);
		}
		printf("\n");
		memset(arr, 0, 64 * sizeof(unsigned char));
	}
}