#define NEOSTR_IMPLEMENTATION
#include "neostr\neostr.h"
#undef NEOSTR_IMPLEMENTATION

#define NEOCORE_IMPLEMENTATION
#include "neobuild\neocore.h"
#undef NEOCORE_IMPLEMENTATION

#include <stdio.h>

#define UNREFERENCED(...) (__VA_ARGS__)
int main(int argc, char* argv[])
{
  UNREFERENCED(argc, argv);
  neo_uInt64 length = sizeof("Hello World");
  neo_uInt64 capacity = length;
  
  char* helloWorldDynamic = malloc(neostr_HeaderSize(neostr_dynamic_payload64) + capacity);
  neostr_LayoutHeader(helloWorldDynamic, neostr_dynamic_payload64);
  neostr_SetCapacity(helloWorldDynamic, neostr_dynamic_payload64, capacity);
  neostr_Push(helloWorldDynamic, neostr_dynamic_payload64, "Hello World", length);

  char* helloWorldFixed = malloc(neostr_HeaderSize(neostr_fixed_payload64) + length);
  neostr_LayoutHeader(helloWorldFixed, neostr_fixed_payload64);
  neostr_SetString(helloWorldFixed, neostr_fixed_payload64, "Hello World", length);


  if (neostr_IsValid(helloWorldDynamic))
  {
    if (neostr_IsValid(helloWorldFixed))
    {
      if (neostr_Compare(helloWorldDynamic, neostr_dynamic_payload64, helloWorldFixed, neostr_fixed_payload64) == 0)
      {
        printf("\nStrings are equal!\n");
      }
    }
  }

  neostr_Literal(testLiteral, "Hello World");


  if (neostr_IsLiteral(testLiteral))
  {
    printf("Literal check correct!\n");
  }
  else
  {
    printf("Literal check incorrect!\n");
  }

  if (neostr_IsLiteral(helloWorldFixed))
  {
    printf("Fixed string literal check passed incorrectly!");
  }
  else
  {
    printf("Fixed string literal check failed correctly!");
  }

  if (neostr_IsValid(testLiteral))
  {
    if (neostr_Compare(testLiteral, neostr_fixed_payload64, helloWorldFixed, neostr_fixed_payload64) == 0)
    {
      printf("Literal is equal!\n");
    }   
  }

  printf(helloWorldFixed);
  printf(helloWorldDynamic);
  return 0;
}