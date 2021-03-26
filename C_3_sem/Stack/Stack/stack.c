#include "stack.h"

///
// например есть память
// 56347548574435784598343
// IPC_EXCLUSIVE
// (size_off = 65) 5 6 3 5
//
///

WAITING wait_param = WAIT_INF;
struct timespec waiting_time;

mystack_t *attach_stack(key_t key, int size)
{
    mystack_t *new_stack = (mystack_t *)calloc(1, sizeof(mystack_t));
    if (new_stack == NULL)
    {
        WARNING("Got nullptr after calloc!");
        return new_stack;
    }

    int shared_id = shmget(key, size * sizeof(void *) + 3 * sizeof(int), IPC_CREAT | IPC_EXCL | 0666);
    //! Типо создаём новую память. IPC_CREAT|IPC_EXCL возращают ошибку, если память уже создана
    //! Ниже проверяем на ошибку

    if (shared_id != MY_IPC_ERROR)
    {
        //! Это старая функция первой инициализации
        sem_t sem = semget(key, 1, IPC_CREAT | IPC_EXCL | 0777);
        if (sem == MY_IPC_ERROR)
            ERROR("semget wasn't done properly!");

        initialize_semaphors_for_library(0);

        new_stack->owner_ = (int *)shmat(shared_id, NULL, 0);
        if (new_stack->owner_ == (int *)MY_IPC_ERROR)
            ERROR("Shmat wasn't done properly!");

        new_stack->owner_[0] = 0;
        new_stack->owner_[1] = size;
        new_stack->owner_[2] = sem;

        sem_increase(sem);
    }
    else
    {
        //!EEXIST ошибка при создании в уже существующий
        if (errno != EEXIST)
            ERROR("Shmget wasn't done properly with earlier created memory!");

        //printf("TRANSLATING!\n");
        shared_id = shmget(key, size * sizeof(void *) + 3 * sizeof(int), 0);
        if (shared_id == MY_IPC_ERROR)
            ERROR("Shmget didn't do with alloc memory!");

        new_stack->owner_ = (int *)shmat(shared_id, NULL, 0);
        if (new_stack->owner_ == (void *)MY_IPC_ERROR)
            ERROR("Shmat wasn't done properly!");
    }

    new_stack->key_ = key;
    new_stack->cap_ = new_stack->owner_[1];
    new_stack->mem_ = (void **)(new_stack->owner_ + 3);
    new_stack->sem_ = new_stack->owner_[2];

    return new_stack;
}

int detach_stack(mystack_t *stack)
{
    int out = shmdt((void *)stack->owner_);
    if (out == MY_IPC_ERROR)
        ERROR("Shmdt wasn't done properly!");

    return out;
}

int mark_destruct(mystack_t *stack)
{
    int shared_block_id = shmget(stack->key_, stack->cap_ * sizeof(void *) + 3 * sizeof(int), 0);
    if (shared_block_id == MY_IPC_ERROR)
        ERROR("Shmget wasn't done properly!");

    int check_destroy = semctl(stack->sem_, 1, IPC_RMID);
    if (check_destroy == MY_IPC_ERROR)
        ERROR("Semaphor wasn't destroyed properly!");

    int out = shmctl(shared_block_id, IPC_RMID, NULL);
    if (out == MY_IPC_ERROR)
        ERROR("shctl wasn't done properly!");

    return out;
}
int get_size(mystack_t *stack)
{
    return stack->owner_[0];
}
int get_count(mystack_t *stack)
{
    return stack->cap_;
}

int push(mystack_t *stack, void *val)
{
    if (stack->owner_[0] >= stack->cap_)
    {
        return -1;
    }

    if (sem_decrease(stack->sem_) == MY_IPC_ERROR)
    {
        if (errno == EAGAIN)
            return -1;
        ERROR("Decreasing wasn't done properly!");
    }

    //Check process was killed there
    stack->mem_[stack->owner_[0]] = val;
    stack->owner_[0]++;

    if (sem_increase(stack->sem_) == MY_IPC_ERROR)
    {
        if (errno == EAGAIN)
            return -1;
        ERROR("Increasing wasn't done properly!");
    }

    return 0;
}
int pop(mystack_t *stack, void **val)
{
    if (stack->owner_[0] <= 0)
    {
        *val = NULL;
        return -1;
    }

    // !Critical section
    if (sem_decrease(stack->sem_) == MY_IPC_ERROR)
    {
        if (errno == EAGAIN)
            return -1;
        ERROR("Decreasing wasn't done properly!");
    }

    stack->owner_[0]--;
    *val = stack->mem_[stack->owner_[0]];

    if (sem_increase(stack->sem_) == MY_IPC_ERROR)
    {
        if (errno == EAGAIN)
            return -1;
        ERROR("Increasing wasn't done properly!");
    }

    return 0;
}

// !ADDED
int set_wait(int val, struct timespec *timeout)
{
    switch (val)
    {
    case 0:
        wait_param = WAIT_INF;
        break;
    case -1:
        wait_param = NO_WAIT;
        break;
    case 1:
        wait_param = WAIT_TIME;
        if (timeout != NULL)
        {
            waiting_time.tv_sec = timeout->tv_sec;
            waiting_time.tv_nsec = timeout->tv_nsec;
        }
        else
            ERROR("Time wasn't identified!");
        break;
    }

    // !Initializing sem_increase and sem_decreas
    // !according new wait_param and waiting_time
    // !-1 and 1 - operations
    // !0 - set new values
    initialize_semaphors_for_library(0);
    return 1;
}
// !TILL THIS

void dump(const mystack_t *stack, const char extra_info[])
{
    if (sem_decrease(stack->sem_) == MY_IPC_ERROR && errno != EAGAIN)
        ERROR("Decreasing wasn't done properly!");

    printf("\nDump has started! %s\n", extra_info);
    for (int i = 0; i < stack->owner_[0]; ++i)
    {
        printf("%d\t", *((int *)(stack->mem_[i])));
    }
    printf("\nDump has finished! %s\n", extra_info);

    if (sem_increase(stack->sem_) == MY_IPC_ERROR && errno != EAGAIN)
        ERROR("Increasing wasn't done properly!");
}

pid_t fork_s()
{
    pid_t out = fork();
    if (out == -1)
        ERROR("FORK!");

    return out;
}

// -1 and +1 operation, 0 - reset
struct sembuf *initialize_semaphors_for_library(int flag)
{
    static struct sembuf sem;

    if (flag == 0)
    {
        sem.sem_num = 0;

        switch (wait_param)
        {
        case NO_WAIT:
            sem.sem_flg = IPC_NOWAIT | SEM_UNDO;
            break;
        case WAIT_INF:
        case WAIT_TIME:
            sem.sem_flg = SEM_UNDO;
            break;
        }
    }
    else
    {
        sem.sem_op = flag;
    }

    return &sem;
}

int sem_increase(sem_t sem)
{
    if (wait_param == WAIT_TIME)
        return semtimedop(sem, initialize_semaphors_for_library(1), 1, &waiting_time);
    else
        return semop(sem, initialize_semaphors_for_library(1), 1);
}
int sem_decrease(sem_t sem)
{
    if (wait_param == WAIT_TIME)
        return semtimedop(sem, initialize_semaphors_for_library(-1), 1, &waiting_time);
    else
        return semop(sem, initialize_semaphors_for_library(-1), 1);
}

void errror(const char info[], size_t LINE, char *FILE)
{
    printf("Mistake was found in %Ilu in file: %s", LINE, FILE);
    printf("\n\nProblem: %s\n", info);
    perror("Problem in perror");
    printf("\n");
    abort();
}

void warrning(const char info[], size_t LINE, char *FILE)
{
    printf("Warning was found in %Ilu in file: %s", LINE, FILE);
    printf("\n\nProblem: %s\n", info);
    perror("Problem in perror");
    printf("\n");
}