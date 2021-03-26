#include "Stack/stack.h"
#include <math.h>

void DO_TESTS();
void test1();
void test2();
void test3();
void test4();

//const int MY_IPC_ERROR = -1;

int main()
{
    DO_TESTS();
}

#define GET_KEY(var)                 \
    key_t var = ftok("Hah", rand()); \
    if (var == MY_IPC_ERROR)         \
    ERROR("ftok wasn't done properly!")

void DO_TESTS()
{
    srand(time(0));

    test1();
    printf("\n\nTESTED -- 1!!!\n\n");
    test2();
    printf("\n\nTESTED -- 2!!!\n\n");
    test3();
    printf("\n\nTESTED -- 3!!!\n\n");
    test4();
    printf("\n\nTESTED -- 4!!!\n\n");
}

void test1()
{
    GET_KEY(key);
    GET_KEY(extra_key1);
    GET_KEY(extra_key);

    sem_t main_sem = semget(extra_key1, 1, IPC_CREAT | IPC_EXCL | 0777);
    sem_t extra_sem = semget(extra_key, 1, IPC_CREAT | IPC_EXCL | 0777);

    if (main_sem == -1 || extra_sem == -1)
        ERROR("Semaphors wasn't properly created!");

    initialize_semaphors_for_library(0);

    pid_t pd = fork();

    if (pd == 0)
    {
        mystack_t *my_st = attach_stack(key, 21);

        int data[] = {6, 7, 8, 9, 10};
        for (int i = 0; i < 5; ++i)
        {
            push(my_st, (void *)(data + i));
            dump(my_st, "Child");
        }

        sem_increase(main_sem);
        sem_decrease(extra_sem);
        dump(my_st, "Child");
        detach_stack(my_st);

        free(my_st);

        raise(SIGKILL);
    }

    sem_decrease(main_sem);

    mystack_t *my_st = attach_stack(key, 21);

    int data[] = {1, 2, 3, 4, 5};
    for (int i = 0; i < 5; ++i)
    {
        push(my_st, (void *)(data + i));
        dump(my_st, "Parent");
    }

    detach_stack(my_st);

    sem_increase(extra_sem);
    struct timespec time = {0, 10000000};
    nanosleep(&time, NULL);
    mark_destruct(my_st);
    free(my_st);
}

void test2()
{
    //одновременно push с разных мест
    GET_KEY(key_sem);
    GET_KEY(key_stack);

    sem_t sem = semget(key_sem, 1, IPC_CREAT | IPC_EXCL | 0777);
    if (sem == MY_IPC_ERROR)
        ERROR("sem wasn't created properly!");
    initialize_semaphors_for_library(0);

    //! CHECK SET_WAIT
    //set_wait(-1, NULL);

    pid_t pd = fork_s();
    if (pd == 0)
    {
        int data[] = {1, 2, 3, 4, 5};
        sem_increase(sem);
        mystack_t *my_st = attach_stack(key_stack, 10);

        for (int i = 0; i < 5; ++i)
        {
            push(my_st, data + i);
            dump(my_st, "Child");
        }

        for (int i = 0; i < 5; ++i)
        {
            void *val = NULL;
            pop(my_st, &val);
            printf("Popped kids value : %d\n", *(int *)val);
            dump(my_st, "Child");
        }

        detach_stack(my_st);
        sem_increase(sem);

        free(my_st);
        raise(SIGKILL);
    }

    int data[] = {6, 7, 8, 9, 10};
    sem_decrease(sem);
    mystack_t *my_st = attach_stack(key_stack, 10);

    for (int i = 0; i < 5; ++i)
    {
        push(my_st, data + i);
        dump(my_st, "Parent");
    }

    for (int i = 0; i < 5; ++i)
    {
        void *val = NULL;
        pop(my_st, &val);
        printf("Popped main value : %d\n", *(int *)val);
        dump(my_st, "Parent");
    }

    detach_stack(my_st);
    sem_decrease(sem);

    mark_destruct(my_st);
    free(my_st);
}

void test3()
{
    GET_KEY(key);
    mystack_t *my_st = attach_stack(key, 2);
    set_wait(0, NULL);

    int data[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    for (int i = 0; i < 10; ++i)
    {
        int out = push(my_st, data + i);
        dump(my_st, "");
        if ((i < 2 && out == -1) ||
            (i >= 2 && out != -1))
        {
            ERROR("Push out of range is dead!");
        }
    }

    for (int i = 0; i < 10; ++i)
    {
        void *val = NULL;
        int out = pop(my_st, &val);
        dump(my_st, "");
        if ((i < 2 && out == -1) ||
            (i >= 2 && out != -1))
        {
            ERROR("Pop out of range is dead!");
        }
    }

    detach_stack(my_st);
    mark_destruct(my_st);

    free(my_st);
}

void test4()
{
    GET_KEY(key); //! на дефайнах создаёт ключ ftok
                  //! и сразу проверяет на ошибки

    const int capacity = (1 << 16) * 10;
    int data[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    pid_t main_pd = fork_s();
    // !главный поток, анализирующий работу стэка

    if (main_pd == 0)
    {
        for (int i = 0; i < 15; ++i)
            fork();

        mystack_t *my_st = attach_stack(key, capacity);
        if (my_st == NULL)
            ERROR("MISTAKE IN CREATING!");

        for (int i = 0; i < 10; ++i)
            push(my_st, data + i);

        void *val = NULL;
        for (int i = 0; i < 10; ++i)
            pop(my_st, &val);

        free(my_st);
        raise(SIGKILL);
    }

    mystack_t *my_st = attach_stack(key, capacity);
    for (int i = 0; i < 7; ++i)
    {
        //!каждую секунду проверяем размер занятый данными
        printf("%d: %d\n", i, get_size(my_st));
        sleep(1);
    }
}