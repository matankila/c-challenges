#include <assert.h>      /* assert    */
#include "knight_tour.h" /* my header */

#define X_MAX_LEN 8
#define Y_MAX_LEN 8
#define POSSIBLE_MOVES 8
#define MATRIX_SIZE 64
#define GET_X(point)((point) / X_MAX_LEN)
#define GET_Y(point)((point) % Y_MAX_LEN)
#define TRUE 1
#define FALSE 0
#define FAIL -1
#define SUCCESS 0
#define CALC_OFFSET(x, y) (X_MAX_LEN * x + y)

static const int x_moves[8] = {2, 1, -1, -2, -2, -1,  1,  2}; 
static const int y_moves[8] = {1, 2,  2,  1, -1, -2, -2, -1}; 

/***************************** Non API fucntions **************************/

/********************************/
static int NextX(int x, int move)
{
    x += x_moves[move];

    return x;
}

/********************************/
static int NextY(int y, int move)
{
    y += y_moves[move];

    return y;
}

/********************************/
static int KnightLimits(int x, int y) 
{ 
    return ((x >= 0 && y >= 0) && (x < X_MAX_LEN && y < Y_MAX_LEN)); 
} 

/********************************/ 
static int IsPointEmpty(unsigned char *path, int x, int y) 
{ 
    return (KnightLimits(x, y)) && (path[CALC_OFFSET(x, y)] == 0); 
} 

/********************************/
static int CountOfPossbilities(unsigned char *path, int x, int y) 
{ 
    int count = 0; 
    int i = 0;

    for (i = 0; i < X_MAX_LEN; ++i)
    { 
        if (IsPointEmpty(path, NextX(x, i), NextY(y, i))) 
        {
            ++count; 
        }
    }

    return count; 
}

/*********************************/
static void UpdateBoard(unsigned char *path, int new_x, int new_y, 
                        int *x, int *y, int best_direction_index)
{
    new_x = NextX(*x, best_direction_index);  
    new_y = NextY(*y, best_direction_index); 
    path[CALC_OFFSET(new_x, new_y)] = path[CALC_OFFSET(*x, *y)] + 1; 
    *x = new_x; 
    *y = new_y; 
}

/*********************************/
static int NextMove(unsigned char *path, int *x, int *y) 
{ 
    int best_direction_index = -1;
    int counter = 0;
    int best_direction = (X_MAX_LEN + 1);
    int new_x = 0;
    int new_y = 0; 
    int count = 0;

    for (count = 0; count < X_MAX_LEN; ++count) 
    { 
        new_x = NextX(*x, count);  
        new_y = NextY(*y, count); 
        if ((IsPointEmpty(path, new_x, new_y)) && 
            (counter = CountOfPossbilities(path, new_x, new_y)) < 
                                           best_direction) 
        { 
            best_direction_index = count; 
            best_direction = counter; 
        } 
    } 
  
    if (best_direction_index == -1) 
    {
        return FALSE; 
    }

    UpdateBoard(path, new_x, new_y, x, y, best_direction_index);
  
    return TRUE; 
}

/********************************/
#define EMPTY 0
static int KnightTourRec(unsigned char *path, int x, int y, int counter)
{
    if (counter == MATRIX_SIZE)
    {
        return FAIL;
    }
    else if (NextMove(path, &x, &y) == FALSE)
    {
        return FAIL;
    }

    if (KnightTourRec(path, x, y, counter + 1))
    {
        return FAIL;
    }

    return SUCCESS;
}
#undef EMPTY

/******************************** API Function ******************************/
/*
* gets a knight random point on a chess board(0 - 63) and fill the array
  with path that the knight go without be on the same block wtice or more.
* Arguments: path: empty array (size 64) will contain in the future the knights
                   path.
             begin_point: random point on the board which the knight will
                          start.
* Return: none, (will fill our path array).
*/
void KnightTour(unsigned char *path, unsigned char *begin_point)
{
    int counter = 0;
    int x = 0;
    int y = 0;

    assert(path);
    assert(begin_point);

    x = GET_X(*begin_point);
    y = GET_Y(*begin_point);
    path[CALC_OFFSET(x, y)] = 1;
    KnightTourRec(path, x, y, counter);
}