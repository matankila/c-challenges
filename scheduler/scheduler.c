#include <stdlib.h>         /* allocation   */
#include <assert.h>         /* assert       */
#include <unistd.h>         /* sleep        */
#include "priority_queue.h" /* my header    */
#include "task.h"           /* my header    */
#include "scheduler.h"      /* my header    */

#define EMPTY_SCH 0
#define STOP      1
#define EQ_ERR    2

struct sch_t
{
	p_queue_t *pqueue;
	task_t *curr_task;
	int is_running;
};

static int SchIsBefore(const void *data1, const void *data2, void *param);
static int SchIsSame(const void *data1, const void *data2);
static int TimeHandler(sch_t *sch,
                       struct timeval start_time,
                       task_t **t, 
                       int *exit_status);
static int RunHandler(sch_t *sch, task_t **t, int *exit_status);

/******************************  local fucntions ****************************/
static int SchIsBefore(const void *data1, const void *data2, void *param)
{
	(void)(param);

	return (TaskGetNextRunTime((task_t*)data1) < 
		    TaskGetNextRunTime((task_t*)data2));
}

static int SchIsSame(const void *data1, const void *data2)
{
	return (UidIsSame(TaskGetUID((task_t*)data1), 
		                        *((nuid_t*)data2)));
}

static int TimeHandler(sch_t *sch,
                       struct timeval start_time,
                       task_t **t, 
                       int *exit_status)
{
	struct timeval curr = {0};
	int res             = 0;

	if(PQIsEmpty(sch->pqueue))
	{
		*exit_status = EMPTY_SCH;
	}

	sch->curr_task = PQDequeue(sch->pqueue);
	*t             = sch->curr_task;

	res = gettimeofday(&curr, NULL);
	assert(0 == res);

	sleep((TaskGetNextRunTime(sch->curr_task)) -
			 (curr.tv_sec - start_time.tv_sec));

	return *exit_status;
}

static int RunHandler(sch_t *sch, task_t **t, int *exit_status)
{
	int x = 0;
	x = TaskRun(sch->curr_task);
	if (x && sch->curr_task)
	{
		TaskUpdateNextRunTime(sch->curr_task);		
		if (PQEnqueue(sch->pqueue, sch->curr_task))
		{
			*exit_status = EQ_ERR; /* enqueue error */
		}
	}
	else                           
	{		
		TaskDestroy(*t);
	}

	return *exit_status;
}

/********************************** fucntions ********************************/
/*
*	return value - pointer to the scheduler management struct
*	arguments - none
*	this function creates a scheduler management struct
*/
sch_t *SchCreate(void)
{
	sch_t *sch_ptr = NULL;

	sch_ptr = malloc(sizeof(sch_t));
	if (sch_ptr)
	{
		sch_ptr->is_running = 0;
		sch_ptr->curr_task = NULL;

		sch_ptr->pqueue = PQCreate(SchIsBefore, NULL);
		if (!sch_ptr->pqueue)
		{
			free(sch_ptr);
			sch_ptr = NULL;
		}
	}

	return sch_ptr;
}

/*
*	return value - none
*	arguments - scheduler management struct
*	this function receives a schedular management struct
*	and frees its allocated memory
*/
void SchDestroy(sch_t *sch)
{
	assert(sch);

	SchClear(sch);
	PQDestroy(sch->pqueue);

	free(sch);
	sch = NULL;
}

/*
*	return value - unique id struct
*	arguments - scheduler management struct, task function pointer, 
*	pointer to task function arguments, timing value
*	this function adds a new task into the schedule, with the given
*	arguments, where timing determines the place in the schedule
*/
nuid_t SchAdd(sch_t *sch, task_func task, void *args, struct timeval timing)
{
	task_t *new_task = NULL;

	assert(sch);
	assert(task);

	new_task = TaskCreate(task, args, timing);
	if (!new_task)
	{
		return g_bad_uid;
	}

	if (PQEnqueue(sch->pqueue, new_task))
	{
		TaskDestroy(new_task);
		return g_bad_uid;
	}

	return (TaskGetUID(new_task));
}

/*
*	return value - unique id struct
*	arguments - scheduler management struct, unique id struct
*	this function finds and removes a task with the received unique id 
*	from the schedule, returning the id of the removed task
*/
nuid_t SchRemove(sch_t *sch, nuid_t id)
{
	task_t *tmp = NULL;

	assert(sch);

	if (sch->curr_task && UidIsSame(id, TaskGetUID(sch->curr_task)))
	{
		sch->curr_task = NULL;
		return id;
	}

	tmp = (task_t*)PQErase(sch->pqueue, SchIsSame, &id);
	if (!tmp)
	{
		return g_bad_uid;
	}

	return (TaskDestroy(tmp));
}

/*
*	return value - return status
*	arguments - scheduler management struct
*	this function starts the execution phase of the schedule, allowing
*	it to start executing scheduled tasks
*/
int SchRun(sch_t *sch)
{
	struct timeval start = {0};
	int res              = 0;
	task_t *task_cpy     = NULL;
	int exit_status      = STOP;

	assert(sch);

	sch->is_running = 1;
	res = gettimeofday(&start, NULL);
	assert(0 == res);

	/* 
	* time handler - handle sleep and dequeue 
	* run handler - handle run and remove of tasks
	* if one of them changes the default exit status the loop will end!
	*/
	while (sch->is_running && 
		  !PQIsEmpty(sch->pqueue) && 
		  (EMPTY_SCH != TimeHandler(sch, start, &task_cpy, &exit_status)) && 
		  (EQ_ERR    != RunHandler(sch, &task_cpy, &exit_status)))
	{
	}

	sch->is_running = 0;
	sch->curr_task  = NULL;

	return exit_status;
}

/*
*	return value - return status
*	arguments - scheduler management struct
*	this function stops the execution phase of the schedule
*/
int SchStop(sch_t *sch)
{
	assert(sch);

	sch->is_running = 0;

	return 0;
}

/*
*	return value - number of tasks in the schedule
*	arguments - scheduler management struct
*	this function returns the number of tasks in the schedule
*/
size_t SchSize(const sch_t *sch)
{
	assert(sch);

	return (PQSize(sch->pqueue) + sch->is_running);
}

/*
*	return value - boolean value
*	arguments - scheduler management struct
*	this function returns 1 if the provided schedule is empty
*	and 0 otherwise
*/
int SchIsEmpty(const sch_t *sch)
{
	assert(sch);

	return (PQIsEmpty(sch->pqueue) && !sch->is_running);
}

/*
*	return value - void
*	arguments - scheduler management struct
*	this function clears all remaining tasks from the schedule
*/
void SchClear(sch_t *sch)
{	
	assert(sch);

	while (!PQIsEmpty(sch->pqueue))
	{
		TaskDestroy((task_t *)PQDequeue(sch->pqueue));
	}
}