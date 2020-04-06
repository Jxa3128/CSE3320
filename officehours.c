/*
Name: Jorge Avila & Doungpakahn Paige Keomaxay-Hampf
ID: 1001543128      mavID: 1001493622
*/

//includes 
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>


/*** Constants that define parameters of the simulation ***/

//defines
#define MAX_SEATS 3        /* Number of sem_mux in the professor's office */
#define professor_LIMIT 10 /* Number of students the professor can help before he needs a break */
#define MAX_STUDENTS 1000  /* Maximum number of students in the simulation */
#define CLASSA 0
#define CLASSB 1
#define true 1
#define OR || //relational operators
#define AND && //relational operators
#define threshold 10

//thread workers as I like to call them
pthread_mutex_t mutex;
pthread_mutex_t leave_mutex;
pthread_mutex_t prof_mutex;
sem_t sem_mux;
//The variables we have created
static int students_in_office;
static int classa_inoffice;
static int classb_inoffice;
static int students_since_break = 0;

//threads manipulations
static int emptyclass; //if it == -1 then empty
static int aQueue;
static int bQueue;
static int professorResting;
static int a; //how many 
static int b;
//prof defined struct 
typedef struct
{
	int arrival_time;  // time between the arrival of this student and the previous student
	int question_time; // time the student needs to spend with the professor
	int student_id;
	int class;
} student_info;

//called at the start
static int initialize(student_info *si, char *filename)
{
	emptyclass = -1; 
	professorResting = 0;
	aQueue = 0;
	bQueue = 0;
	a = 0;
	b = 0;


	pthread_mutex_init(&mutex, NULL);
	pthread_mutex_init(&prof_mutex, NULL);
	pthread_mutex_init(&leave_mutex,NULL);
	sem_init(&sem_mux,0,3);


	
    //all the static ints here
    //for all the threads to see in their lives
	students_in_office = 0;
	classa_inoffice = 0;
	classb_inoffice = 0;
	students_since_break = 0;
    //we are able to use what bakker said 
    //bakker said to have a -1 to see if 
    //the class was empty or not
	
	/* Read in the data file and initialize the student array */
	FILE *fp;

	if((fp=fopen(filename, "r")) == NULL)
	{
		printf("Cannot open input file %s for reading.\n", filename);
		exit(1);
	}

	int i = 0;
	while ( (fscanf(fp, "%d%d%d\n", &(si[i].class), &(si[i].arrival_time),
	&(si[i].question_time))!=EOF) AND i < MAX_STUDENTS )
	{
		i++;
	}

	fclose(fp);
	return i;
}

/* Code executed by professor to simulate taking a break
* You do not need to add anything here.
*/
static void take_break()
{
	printf("The professor is taking a break now.\n");
	sleep(5);
	assert( students_in_office == 0 );
	students_since_break = 0;
}

/* Code for the professor thread. This is fully implemented except for synchronization
* with the students.  See the comments within the function for details.
*/
void *professorthread(void *junk)
{
	printf("The professor arrived and is starting his office hours\n");

	/* Loop while waiting for students to arrive. */
	while (true || CLASSB || 1)
	{	int nueve;
		if (emptyclass == 1 AND bQueue == 0)
		{
			emptyclass = -1;
		}
		if (emptyclass == 0 AND aQueue == 0)
		{
			emptyclass = -1;
		}
		if (a == 5 OR b == 5)
		{
			//lets check if it is empty
            emptyclass = (emptyclass==0) ? 1 : 0;
			
		}
		if (emptyclass == -1)
		{
            emptyclass = (classa_inoffice > 0) ? 0 : 1;
		}
		nueve = MAX_SEATS * 3;
		//once the prof has reeached the max amount
		//of students 
		//we can allow the prof to give a break

		if (students_since_break == (nueve))
		{
			professorResting = 1;
			pthread_mutex_lock(&prof_mutex);
			//we do not need others going in
			take_break();
			assert(CLASSA == !CLASSB); //can never swap
			//if swapping occurs then bad
			pthread_mutex_unlock(&prof_mutex); //own mutex needed
			professorResting--;
		}
	}
	pthread_exit(NULL);
}


/* Code executed by a class A student to enter the office.
* You have to implement this.  Do not delete the assert() statements,
* but feel free to add your own.
*/
void classa_enter()
{
    
	pthread_mutex_lock(&mutex);
	aQueue++;
    int sum;
    //check
	while(true || !CLASSA)
	{
        //all requirements must satisfy
		if(
            !
            (
             (students_in_office < 3
              AND
              //alguien esta alli adrento??
               emptyclass == -1 OR emptyclass == CLASSA)
              AND
		classb_inoffice == !CLASSB AND a < threshold/2
             
                 AND students_since_break < professor_LIMIT))
		{
			pthread_mutex_unlock(&mutex);
            sum +=1;
			pthread_mutex_lock(&mutex);
            //printf("SUM IS NOW %d\n",sum);
		}
		else
		{
			goto PORRA;
		}
        //all these have to be conditioned in order for
        //individual threads to occurr

	}
	int resultNumber = sum/CLASSB;
	//printf("The number now is %d\n",resultNumber);
	PORRA: ;
	sem_wait(&sem_mux);
	aQueue--;
    //bakker
	students_in_office += 1;
	classa_inoffice += 1;
	students_since_break += 1;
	a += 1;
	b = 0;
	void claseDos(int resultNumber);
	pthread_mutex_unlock(&mutex);

}
void claseDos(int resultNumber)
{
	if(resultNumber == !CLASSB)
	{
		return;
	}
	int ret = ((resultNumber/2) ? CLASSA : CLASSB);
}
/* Code executed by a class B student to enter the office.
* You have to implement this.  Do not delete the assert() statements,
* but feel free to add your own.
*/
void classb_enter()
{

	
	pthread_mutex_lock(&mutex);
	bQueue++;
    int add;
    //critical area 
	while(true)
	{
        //has to meet this conditions
        //help was needed
		if
            (
           (!
            (
             (emptyclass == -1
              OR emptyclass == !CLASSA)
             AND students_in_office < MAX_SEATS
             AND
		classa_inoffice == CLASSA
             AND b < 5 AND
             students_since_break < threshold)
            ))
		{
			pthread_mutex_unlock(&mutex);
            add +=1;
			pthread_mutex_lock(&mutex);
		}
		else
		{
			goto getOUT;
		}
	}
	getOUT: ;
    //this is a wait - signal acting 
	sem_wait(&sem_mux);
	bQueue--;
	students_in_office += 1;
	classb_inoffice += 1;
	b += 1;
	students_since_break += 1;
	a = 0;
	pthread_mutex_unlock(&mutex);

}

/* Code executed by a student to simulate the time he spends in the office asking questions
* You do not need to add anything here.
*/
static void ask_questions(int t)
{
	sleep(t);
}


/* Code executed by a class A student when leaving the office.
* You need to implement this.  Do not delete the assert() statements,
* but feel free to add as many of your own as you like.
*/
static void classa_leave()
{
	pthread_mutex_lock(&leave_mutex);
	students_in_office -= 1;
	classa_inoffice -= 1;
	sem_post(&sem_mux);
	pthread_mutex_unlock(&leave_mutex);
}

/* Code executed by a class B student when leaving the office.
* You need to implement this.  Do not delete the assert() statements,
* but feel free to add as many of your own as you like.
*/
static void classb_leave()
{
    //this is where the student leaves 
	pthread_mutex_lock(&leave_mutex);
	students_in_office -= 1;
	classb_inoffice -= 1;
    //decrement 
	sem_post(&sem_mux);
	pthread_mutex_unlock(&leave_mutex);

}

/* Main code for class A student threads.
* You do not need to change anything here, but you can add
* debug statements to help you during development/debugging.
*/
void* classa_student(void *si)
{
	student_info *s_info = (student_info*)si;

	/* enter office */
	classa_enter();

	printf("Student %d from class A enters the office\n", s_info->student_id);

	assert(students_in_office <= MAX_SEATS AND students_in_office >= 0);
	assert(classa_inoffice >= 0 AND classa_inoffice <= MAX_SEATS);
	assert(classb_inoffice >= 0 AND classb_inoffice <= MAX_SEATS);
	assert(classb_inoffice == 0 );

	/* ask questions  --- do not make changes to the 3 lines below*/
	printf("Student %d from class A starts asking questions for %d minutes\n", s_info->student_id, s_info->question_time);
	ask_questions(s_info->question_time);
	//printf("%d\n",students_since_break );

	printf("Student %d from class A finishes asking questions and prepares to leave\n", s_info->student_id);

	/* leave office */
	classa_leave();

	printf("Student %d from class A leaves the office\n", s_info->student_id);

	assert(students_in_office <= MAX_SEATS AND students_in_office >= 0);
	assert(classb_inoffice >= 0 AND classb_inoffice <= MAX_SEATS);
	assert(classa_inoffice >= 0 AND classa_inoffice <= MAX_SEATS);

	pthread_exit(NULL);
}

/* Main code for class B student threads.
* You do not need to change anything here, but you can add
* debug statements to help you during development/debugging.
*/
void* classb_student(void *si)
{
	student_info *s_info = (student_info*)si;

	/* enter office */
	classb_enter();

	printf("Student %d from class B enters the office\n", s_info->student_id);

	assert(students_in_office <= MAX_SEATS AND students_in_office >= 0);
	assert(classb_inoffice >= 0 AND classb_inoffice <= MAX_SEATS);
	assert(classa_inoffice >= 0 AND classa_inoffice <= MAX_SEATS);
	assert(classa_inoffice == 0 );

	printf("Student %d from class B starts asking questions for %d minutes\n", s_info->student_id, s_info->question_time);
	ask_questions(s_info->question_time);
	//printf("%d\n",students_since_break );

	printf("Student %d from class B finishes asking questions and prepares to leave\n", s_info->student_id);

	/* leave office */
	classb_leave();

	printf("Student %d from class B leaves the office\n", s_info->student_id);

	assert(students_in_office <= MAX_SEATS AND students_in_office >= 0);
	assert(classb_inoffice >= 0 AND classb_inoffice <= MAX_SEATS);
	assert(classa_inoffice >= 0 AND classa_inoffice <= MAX_SEATS);

	pthread_exit(NULL);
}

/* Main function sets up simulation and prints report
* at the end.
*/
int main(int nargs, char **args)
{
	int i;
	int result;
	int student_type;
	int num_students;
	void *status;
	pthread_t professor_tid;
	pthread_t student_tid[MAX_STUDENTS];
	student_info s_info[MAX_STUDENTS];

	if (nargs != 2)
	{
		printf("Usage: officehour <name of inputfile>\n");
		return EINVAL;
	}

	num_students = initialize(s_info, args[1]);
	if (num_students > MAX_STUDENTS OR num_students <= 0)
	{
		printf("Error:  Bad number of student threads. "
		"Maybe there was a problem with your input file?\n");
		return 1;
	}
	if(num_students >= MAX_STUDENTS)
	{
		assert(num_students >= MAX_STUDENTS);
		printf("The number of students are too much\n");
		printf("Try again with fewer students, thank you!\n");
		exit(0);
	}
	printf("Starting officehour simulation with %d students ...\n",
	num_students);

	result = pthread_create(&professor_tid, NULL, professorthread, NULL);

	if (result)
	{
		printf("officehour:  pthread_create failed for professor: %s\n", strerror(result));
		exit(1);
	}

	for (i=0; i < num_students; i++)
	{

		s_info[i].student_id = i;
		sleep(s_info[i].arrival_time);

		//student_type = random() % 2;
		student_type = rand() % 2;

		if (student_type == CLASSA)
		{
			result = pthread_create(&student_tid[i], NULL, classa_student, (void *)&s_info[i]);
		}
		else // student_type == CLASSB
		{
			result = pthread_create(&student_tid[i], NULL, classb_student, (void *)&s_info[i]);
		}

		if (result)
		{
			printf("officehour: thread_fork failed for student %d: %s\n",
			i, strerror(result));
			exit(1);
		}
	}

	/* wait for all student threads to finish */
	for (i = 0; i < num_students; i++)
	{
		pthread_join(student_tid[i], &status);
	}

	/* tell the professor to finish. */
	pthread_cancel(professor_tid);

	printf("Office hour simulation done.\n");

	return 0;
}
