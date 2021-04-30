#include "crypto.h"

static char RSA (char sym , int type);
static void HashChar (char* sym , int type);

int M_EncryptString (char* str , size_t size)
{
    pr_info ("Before crypt: %c" , str);
    for (size_t i = 0; i < size; ++i)
        HashChar (str + i , ENCRYPT);
    pr_info ("After crypt: %c" , str);
    return 0;
}
int M_DecryptString (char* str , size_t size)
{
    pr_info ("Before crypt: %s" , str);
    for (size_t i = 0; i < size; ++i)
        HashChar (str + i , DECRYPT);
    pr_info ("After crypt: %s" , str);
    return 0;
}
void HashChar (char* sym , int type)
{
    char insert_elem = *sym;
#ifdef CAESAR
    const char shift = 2;
    if (type == ENCRYPT)
        insert_elem += 2;
    else
        insert_elem -= 2;
#else
    insert_elem = RSA (insert_elem , type);
#endif
    * sym = insert_elem;
}


static unsigned GetD (unsigned phi , unsigned e);
char RSA (char sym , int type)
{
    static const unsigned p = 3;
    static const unsigned q = 41;
    static const unsigned n = p * q; //p * q
    static const unsigned phi = (p - 1) * (q - 1); //(p - 1) * (q - 1)
    static const unsigned e = 3; //НОД(e, phi) = 1

    unsigned multiplier = ((unsigned)sym);
    unsigned out = multiplier;
    if (type == ENCRYPT)
    {
        for (unsigned i = 0; i < e - 1; ++i)
            out = (out * multiplier) % n;
    }
    else //DECRYPT
    {
        const unsigned d = GetD (phi , e);

        for (unsigned i = 0; i < d - 1; ++i)
            out = (out * multiplier) % n;
    }

    return ((char)out); //correct and safe
}

unsigned GetD (unsigned phi , unsigned e)
{
    unsigned right_ex = phi + 1;
    for (; right_ex % e != 0; right_ex += phi);
    return (right_ex / e);
}