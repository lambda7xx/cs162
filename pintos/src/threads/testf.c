#include <stdio.h>

#include "fixed-point.h"

int main()
{
   fixed_point_t p2 = fix_inv(fix_int(60));//1/60
   printf("%d\n",p2.f);
}
