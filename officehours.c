//Jorge Avila CSE3320
//mavID: 1001543128

#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>

/*** Constants that define parameters of the simulation ***/

#define MAX_SEATS 3        /* Number of seats in the professor's office */
#define professor_LIMIT 10 /* Number of students the professor can help before he needs a break */
#define MAX_STUDENTS 1000  /* Maximum number of students in the simulation */

#define CLASSA 0
#define CLASSB 1

/* Add your synchronization variables here */
sem_t sem_mutex;
pthread_cond_t students_left;
pthread_mutex_t mutex;
pthread_cond_t students_joined;

static int students_in_office; /* Total numbers of students currently in the office */
static int classa_inoffice;    /* Total numbers of students from class A currently in the office */
static int classb_inoffice;    /* Total numbers of students from class B in the office */
static int students_since_break = 0;

static int profResting;
static int nextClassType;
static int classEmpty;


typedef struct
{
    int arrival_time;  // time between the arrival of this student and the previous student
    int question_time; // time the student needs to spend with the professor
    int student_id;
    int class;
} student_info;

//This is called at the start of the simulation
static int initialize(student_info *si, char *filename)
{
    students_in_office = 0;
    classa_inoffice = 0;
    classb_inoffice = 0;
    students_since_break = 0;
    classEmpty = -1;

    //Our variable initialization

    nextClassType = -1;
    trocarEstudantes = 0;
    sem_init(&sem_mutex, 0, 3);
    pthread_cond_init(&students_joined, NULL);
    pthread_cond_init(&students_left, NULL);

    /* Read in the data file and initialize the student array */
    FILE *fp;

    if ((fp = fopen(filename, "r")) == NULL)
    {
        printf("Cannot open input file %s for reading.\n", filename);
        exit(1);
    }

    int i = 0;
    while ((fscanf(fp, "%d%d%d\n", &(si[i].class), &(si[i].arrival_time), &(si[i].question_time)) != EOF) &&
           i < MAX_STUDENTS)
    {
        i++;
    }

    fclose(fp);
    return i;
}

//stays the same - no change is needed
static void take_break()
{
    printf("The professor is taking a break now.\n");
    sleep(5);
    assert(students_in_office == 0);
    students_since_break = 0;
}

/* Code for the professor thread. This is fully implemented except for synchronization
 * with the students.  See the comments within the function for details.
 */
void *professorthread(void *junk)
{
    printf("The professor arrived and is starting his office hours\n");

    /* Loop while waiting for students to arrive. */
    while (1)
    {

        pthread_mutex_lock(&mutex);
        //sem_wait(&sem_mutex);
        if(nextClassType ==  CLASSB && b_is_waiting ==0) ||
        (next_type == CLASSA  && a_is_waiting == 0)
        {
            nextClassType = -1;
        }
        //after every 5th student (from same class) we need to let
        //new student in from other class and start over - like so
        trocarEstudantes++;
        if (trocarEstudantes == 5)
        {
            if (nextClassType == CLASSA)
            {
                nextClassType = CLASSB;
            }
            else if (nextClassType == CLASSB)
            {
                nextClassType = CLASSA;
            }
        }
        pthread_cond_wait(&students_joined, &mutex);
        // if we get 10 people then professor takes a break.
        students_since_break++;
        if (students_since_break == professor_LIMIT)
        {
            //only goes in here after 10 people have
            //came inside
            take_break();
        }

        //sem_post(&sem_mutex);
        pthread_mutex_unlock(&mutex);
    }
    pthread_exit(NULL);
}

/* Code executed by a class A student to enter the office.
 * You have to implement this.  Do not delete the assert() statements,
 * but feel free to add your own.
 */
void classa_enter()
{

    //printf("A THREAD ENTERED\n");
    pthread_mutex_lock(&mutex);

    while (!(nextClassType == CLASSA) && (students_in_office <= MAX_SEATS) && (classa_inoffice >= 5))
    {
        //printf("WE ARE IN THE A THREAD WHILE - LOOP\n");
        pthread_cond_wait(&students_left, &mutex);
    }

    students_in_office += 1;
    students_since_break += 1;
    classa_inoffice += 1;

    pthread_mutex_unlock(&mutex);
}

/* Code executed by a class B student to enter the office.
 * You have to implement this.  Do not delete the assert() statements,
 * but feel free to add your own.
 */
void classb_enter()
{
    //printf("B CLASS ENTERED\n");
    pthread_mutex_lock(&mutex);

    while (!(nextClassType == CLASSB) && (students_in_office <= MAX_SEATS))
    {
        //printf("WE ARE IN THE B WHILE LOOP\n");
        pthread_cond_wait(&students_left, &mutex);
    }
    students_in_office += 1;
    students_since_break += 1;
    classb_inoffice += 1;

    pthread_cond_wait(&students_left, &mutex);
    pthread_mutex_unlock(&mutex);
}

static void ask_questions(int t)
{
    sleep(t);
}

static void classa_leave()
{
    pthread_mutex_lock(&mutex);
    students_in_office -= 1;
    classa_inoffice -= 1;
    //unblocks the thread & signals student left
    pthread_cond_signal(&students_left);
    pthread_mutex_unlock(&mutex);
}

static void classb_leave()
{
    //always need a mutex to
    //prevent condition race
    pthread_mutex_lock(&mutex);

    //decrements once one of the thread leaves
    students_in_office -= 1;
    classb_inoffice -= 1;

    //same here with the signal student left
    pthread_cond_signal(&students_left);
    pthread_mutex_unlock(&mutex);
}

void *classa_student(void *si)
{
    student_info *s_info = (student_info *)si;

    /* enter office */
    classa_enter();

    printf("Student %d from class A enters the office\n", s_info->student_id);

    //this asset function checks if these conditions are met
    assert(students_in_office <= MAX_SEATS && students_in_office >= 0);
    assert(classa_inoffice >= 0 && classa_inoffice <= MAX_SEATS);
    assert(classb_inoffice >= 0 && classb_inoffice <= MAX_SEATS);
    assert(classb_inoffice == 0);

    /* ask questions  --- do not make changes to the 3 lines below*/
    printf("Student %d from class A starts asking questions for %d minutes\n", s_info->student_id, s_info->question_time);
    ask_questions(s_info->question_time);
    printf("Student %d from class A finishes asking questions and prepares to leave\n", s_info->student_id);

    /* leave office */
    classa_leave();

    printf("Student %d from class A leaves the office\n", s_info->student_id);

    assert(students_in_office <= MAX_SEATS && students_in_office >= 0);
    assert(classb_inoffice >= 0 && classb_inoffice <= MAX_SEATS);
    assert(classa_inoffice >= 0 && classa_inoffice <= MAX_SEATS);

    pthread_exit(NULL);
}

void *classb_student(void *si)
{
    student_info *s_info = (student_info *)si;

    /* enter office */
    classb_enter();

    printf("Student %d from class B enters the office\n", s_info->student_id);

    assert(students_in_office <= MAX_SEATS && students_in_office >= 0);
    assert(classb_inoffice >= 0 && classb_inoffice <= MAX_SEATS);
    assert(classa_inoffice >= 0 && classa_inoffice <= MAX_SEATS);
    assert(classa_inoffice == 0);

    printf("Student %d from class B starts asking questions for %d minutes\n", s_info->student_id, s_info->question_time);
    ask_questions(s_info->question_time);
    printf("Student %d from class B finishes asking questions and prepares to leave\n", s_info->student_id);

    /* leave office */
    classb_leave();

    printf("Student %d from class B leaves the office\n", s_info->student_id);

    assert(students_in_office <= MAX_SEATS && students_in_office >= 0);
    assert(classb_inoffice >= 0 && classb_inoffice <= MAX_SEATS);
    assert(classa_inoffice >= 0 && classa_inoffice <= MAX_SEATS);

    pthread_exit(NULL);
}

/* Main function sets up simulation and prints report
 * at the end.
 * GUID: 355F4066-DA3E-4F74-9656-EF8097FBC985
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
    if (num_students > MAX_STUDENTS || num_students <= 0)
    {
        printf("Error:  Bad number of student threads. "
               "Maybe there was a problem with your input file?\n");
        return 1;
    }

    printf("Starting officehour simulation with %d students ...\n",
           num_students);

    result = pthread_create(&professor_tid, NULL, professorthread, NULL);

    if (result)
    {
        printf("officehour:  pthread_create failed for professor: %s\n", strerror(result));
        exit(1);
    }

    for (i = 0; i < num_students; i++)
    {

        s_info[i].student_id = i;
        sleep(s_info[i].arrival_time);

        student_type = random() % 2;

        if (s_info[i].class == CLASSA)
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
