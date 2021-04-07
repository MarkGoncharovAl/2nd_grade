#include "packet.h"

//!Creating
////////////////////////////////////////////////////
////////////////////////////////////////////////////
////////////////////////////////////////////////////
////////////////////////////////////////////////////
////////////////////////////////////////////////////
M_pack_named* M_CreatePack_Named (const char* data , size_t size , int name)
{
#ifdef MAX_INFO
    pr_info ("Creating named pack for\n\t%s\n\tIn %u bytes" , data , size);
#endif
    M_pack_named* out = (M_pack_named*)malloc (sizeof (M_pack_named));
    if (out == NULL)
    {
        pr_strerr ("Can't get dynamic memory for named_pack!");
        return NULL;
    }

    out->name_ = name;
    out->size_ = size;
    out->data_ = (char*)malloc (sizeof (char) * (size));
    if (out->data_ == NULL)
    {
        pr_strerr ("Can't get dynamic memory for named_pack!");
        return NULL;
    }

    if (memcpy (out->data_ , data , size) == NULL)
    {
        pr_strerr ("Copying wasn't success!");
        return NULL;
    }

#ifdef MAX_INFO
    pr_info ("Named_pack successfully created!");
#endif
    return out;
}
M_pack_unnamed* M_CreatePack_Unnamed (const char* data , size_t size)
{
#ifdef MAX_INFO
    pr_info ("Creating un_named pack for\n\t%s\n\tIn %u bytes" , data , size);
#endif

    M_pack_unnamed* out = (M_pack_unnamed*)malloc (sizeof (M_pack_unnamed));
    if (out == NULL)
    {
        pr_strerr ("Can't get dynamic memory for un_named_pack!");
        return NULL;
    }

    out->size_ = size;
    out->data_ = (char*)malloc (sizeof (char) * size);
    if (out->data_ == NULL)
    {
        pr_strerr ("Can't get dynamic memory for un_named_pack!");
        return NULL;
    }

    if (memcpy (out->data_ , data , size) == NULL)
    {
        pr_strerr ("Copying wasn't success!");
        return NULL;
    }

#ifdef MAX_INFO
    pr_info ("Un_named_pack successfully created!");
#endif
    return out;
}
M_pack_named* M_CreatePack_Named_Mem (char* data , size_t size , int name)
{
#ifdef MAX_INFO
    pr_info ("Creating named pack_mem for\n\t%s\n\tIn %u bytes" , data , size);
#endif
    M_pack_named* out = (M_pack_named*)malloc (sizeof (M_pack_named));
    if (out == NULL)
    {
        pr_strerr ("Can't get dynamic memory for named_pack!");
        return NULL;
    }

    out->name_ = name;
    out->data_ = data;
    out->size_ = size;

#ifdef MAX_INFO
    pr_info ("Named_pack_mem successfully created!");
#endif
    return out;
}
M_pack_unnamed* M_CreatePack_Unnamed_Mem (char* data , size_t size)
{
#ifdef MAX_INFO
    pr_info ("Creating un_named pack_mem for\n\t%s\n\tIn %u bytes" , data , size);
#endif

    M_pack_unnamed* out = (M_pack_unnamed*)malloc (sizeof (M_pack_unnamed));
    if (out == NULL)
    {
        pr_strerr ("Can't get dynamic memory for un_named_pack!");
        return NULL;
    }

    out->size_ = size;
    out->data_ = data;

#ifdef MAX_INFO
    pr_info ("Un_named_pack_mem successfully created!");
#endif
    return out;
}

////////////////////////////////////////////////////
////////////////////////////////////////////////////
////////////////////////////////////////////////////
////////////////////////////////////////////////////
////////////////////////////////////////////////////
//!Destroying
////////////////////////////////////////////////////
////////////////////////////////////////////////////
////////////////////////////////////////////////////
////////////////////////////////////////////////////
////////////////////////////////////////////////////

void M_DestroyPack_Named (M_pack_named* packet)
{
#ifdef MAX_INFO
    pr_info ("Destroyng named pack for\n\t%s\n\tIn %u bytes" , packet->data_ , packet->size_);
#endif
    free (packet->data_);
    free (packet);
}
void M_DestroyPack_Unnamed (M_pack_unnamed* packet)
{
#ifdef MAX_INFO
    pr_info ("Destroyng named pack for\n\t%s\n\tIn %u bytes" , packet->data_ , packet->size_);
#endif
    free (packet->data_);
    free (packet);
}

M_pack_unnamed* M_RecoverPack (M_pack_named* pack)
{
#ifdef MAX_INFO
    pr_info ("Recovering pack for\n\t%s\n\tIn %u bytes" , pack->data_ , pack->size_);
#endif
    return ((M_pack_unnamed*)pack);
}
////////////////////////////////////////////////////
////////////////////////////////////////////////////
////////////////////////////////////////////////////
////////////////////////////////////////////////////
////////////////////////////////////////////////////
//!Actions
////////////////////////////////////////////////////
////////////////////////////////////////////////////
////////////////////////////////////////////////////
////////////////////////////////////////////////////
////////////////////////////////////////////////////

M_pack_unnamed* M_ReadPack_Unnamed (int fd)
{
#ifdef MAX_INFO
    pr_info ("Reading packet");
#endif
    M_pack_unnamed* out = (M_pack_unnamed*)malloc (sizeof (M_pack_unnamed));
    if (out == NULL)
    {
        pr_strerr ("Can't get dynamic memory for un_named_pack!");
        return NULL;
    }

    if (read (fd , &(out->size_) , sizeof (size_t)) == READ_ERR)
    {
        pr_strerr ("Can't properly read from pipe %d" , fd);
        return NULL;
    }

    out->data_ = (char*)malloc (out->size_ * sizeof (char));
    if (out->data_ == NULL)
    {
        pr_strerr ("Can't get dynamic memory for un_named_pack!");
        return NULL;
    }

    if (read (fd , out->data_ , out->size_) == READ_ERR)
    {
        pr_strerr ("Can't properly read from pipe %d" , fd);
        return NULL;
    }

#ifdef MAX_INFO
    pr_info ("Successfully readed: %s in %d bytes" , out->data_ , out->size_);
#endif
    return out;
}

int M_WritePack_Unnamed (int fd , M_pack_unnamed* pack)
{
#ifdef MAX_INFO
    pr_info ("Writting to buffer %d info %s" , fd , pack->data_);
#endif
    if (pack == NULL)
    {
        pr_strerr ("Pack is empty!");
        return -1;
    }

    if (write (fd , &(pack->size_) , sizeof (size_t)) == WRITE_ERR)
    {
        pr_strerr ("Can't properly write size %d" , pack->size_);
        return -1;
    }

    if (write (fd , pack->data_ , pack->size_) == WRITE_ERR)
    {
        pr_strerr ("Can't properly write buffer %s in %d bytes" , pack->data_ , pack->size_);
        return -1;
    }

#ifdef MAX_INFO
    pr_info ("Successfully writted");
#endif
    return 0;
}

M_pack_named* M_ReadPack_Named (int fd , struct sockaddr* addr)
{
#ifdef MAX_INFO
    pr_info ("Getting info from socket %d" , fd);
#endif
    M_pack_named* out = (M_pack_named*)malloc (sizeof (M_pack_named));
    if (out == NULL)
    {
        pr_strerr ("Can't get dynamic memory for un_named_pack!");
        return NULL;
    }

    socklen_t sock_len = sizeof (struct sockaddr_in);
    if (recvfrom (fd , &(out->name_) , sizeof (out->name_) , 0 , addr , &sock_len) == SOCK_ERR)
    {
        pr_strerr ("Can't properly read size from socket %d" , fd);
        free (out);
        return NULL;
    }
    if (recvfrom (fd , &(out->size_) , sizeof (out->size_) , 0 , addr , &sock_len) == SOCK_ERR)
    {
        pr_strerr ("Can't properly read size from socket %d" , fd);
        free (out);
        return NULL;
    }

    out->data_ = (char*)malloc (out->size_ * sizeof (char));
    if (out->data_ == NULL)
    {
        pr_strerr ("Can't get dynamic memory for info in pack: size: %u from socket %d!", out->size_, out->name_);
        free (out);
        return NULL;
    }
    if (recvfrom (fd , out->data_ , out->size_ * sizeof (char) , 0 , addr , &sock_len) == SOCK_ERR)
    {
        pr_strerr ("Can't read data form socket %d" , fd);
        M_DestroyPack_Named (out);
        return NULL;
    }

#ifdef MAX_INFO
    pr_info ("Received:\n\t%s\n\tIn %u bytes from %d" , out->data_ , out->size_, out->name_);
#endif
    return out;
}

int M_WritePack_Named (int fd , struct sockaddr* addr , M_pack_named* pack)
{
    if (pack == NULL)
    {
        pr_err ("Pack is empty!");
        return -1;
    }

#ifdef MAX_INFO
    pr_info ("Writting:\n\t%s\n\t%u bytes to %d socket" , pack->data_ , pack->size_ , fd);
#endif

    socklen_t sock_len = sizeof (struct sockaddr_in);
    if (sendto (fd , &(pack->name_) , sizeof (pack->name_) , 0 , addr , sock_len) == SOCK_ERR)
    {
        pr_strerr ("Can't properly write name %d" , pack->name_);
        return -1;
    }

    if (sendto (fd , &(pack->size_) , sizeof (pack->size_) , 0 , addr , sock_len) == SOCK_ERR)
    {
        pr_strerr ("Can't properly write size %u" , pack->size_);
        return -1;
    }

    if (sendto (fd , pack->data_ , pack->size_ * sizeof (char) , 0 , addr , sock_len) == SOCK_ERR)
    {
        pr_strerr ("Can't properly write buffer %s" , pack->data_);
        return -1;
    }

#ifdef MAX_INFO
    pr_info ("Pack\n\t%s\n\tWas successfully sent" , pack->data_);
#endif
    return 0;
}

M_pack_named* M_CreatePack_STATIC (char* data , size_t size , int name)
{
#ifdef MAX_INFO
    pr_info ("Creating pack %s without new dynamic memory" , data);
#endif

    M_pack_named* out = (M_pack_named*)malloc (sizeof (M_pack_named));
    if (out == NULL)
    {
        pr_strerr ("Can't get dynamic memory for un_named_pack!");
        return NULL;
    }

    out->size_ = size;
    out->name_ = name;
    out->data_ = data;
    out->data_[size - 1] = '\0';

#ifdef MAX_INFO
    pr_info ("Created pack %s" , data);
#endif
    return out;
}
////////////////////////////////////////////////////