#include "crypto.h"

int M_EncryptString (char* str , size_t size)
{
    pr_info("Before crypt: %c", str[0]);
    //for (size_t i = 0; i < size; ++i)
    //str[i] ^= 0xff;
    pr_info("After crypt: %c", str[0]);
}
int M_DecryptString (char* str , size_t size)
{
    pr_info("Before crypt: %c", str[0]);
    //for (size_t i = 0; i < size; ++i)
    //str[i] ^= 0xff;
    pr_info("After crypt: %c", str[0]);
}