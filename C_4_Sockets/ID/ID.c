#include "ID.h"

static int IDS[BUFSZ] = { -1 };
static int NOT_USED_IDS[BUFSZ] = { -1 };

static int last_data_fork = 0;
static int last_used_ip = -1;

//returns new IP
int M_CreateID_FromFD (int fd)
{
    pr_info ("Adding ID to fd %d" , fd);

    int outID = 0;
    if (last_used_ip == -1)
    {
        IDS[last_data_fork] = fd;
        last_data_fork++;

        outID = last_data_fork - 1;
    }
    else
    { //last_used_ip >= 0
        int free_space = NOT_USED_IDS[last_used_ip];
        IDS[free_space] = fd;
        last_used_ip--;

        outID = free_space;
    }

    pr_info ("ID set %d" , outID);
    return outID;
}

void M_DeleteID (int ID)
{
    pr_info ("Delete ID %d" , ID);
    close (IDS[ID]);
    IDS[ID] = -1;

    last_used_ip++;
    NOT_USED_IDS[last_used_ip] = ID;
}

//returns -1 if client wasn't found
int M_GetFD_FromID (int ID)
{
    if (ID < 0 || ID >= last_data_fork)
    {
        pr_strerr ("Can't get ID %d, because range is " , ID , last_data_fork);
        return -1;
    }

    pr_info("Got %d fd from %d ID", IDS[ID], ID);
    return IDS[ID];
}

int M_Close_IDS ()
{
    pr_info("Closing IDs");
    for (int cur_id = 0; cur_id < last_data_fork; ++cur_id)
    {
        if (IDS[cur_id] != -1)
        {
            char buf [] = "CLOSE_SERVER";

            M_pack_unnamed* pack = M_CreatePack_Unnamed_Mem (buf , strlen (buf));
            if (pack == NULL)
                return -1;

            if (M_WritePack_Unnamed (IDS[cur_id] , pack) == -1)
                return -1;

            close (IDS[cur_id]);
        }
    }
    
    return 0;
}