STB-style header-only string library for C/C++

Allocates string data inline with a user-supplied struct type which can contain information about the string, such as it's length or capacity. This user-supplied struct type is referred to as the string "payload" within the library. When string data is initialized, it's memory is laid out in the following format:
  \[payload\]\[metadata\]\[string data .......\]

- The payload is the user-supplied struct data of any size.
- The metadata struct is a library-internal struct which stores both the size of the user-supplied struct and a checksum of the user-supplied struct which is used to verify that payload memory is valid before performing any string operations 
- String data is a block of string memory like any other in C. The string pointer returned to the user is a pointer to this block of memory which means that a neostr with any type of payload can be used anywhere a normal C string can be used

To support user-supplied payload types generically, the library is implemented primarily in macros which rely on the assumption that the user-supplied payload struct implements a struct member with a specific name. These names are controlled by macros which can be set by the user. For example, by default functions like neostr_GetLength or neostr_SetLength assume that there is a payload struct member called "length". However, if a different member name is preferred it can be set by defining NEOSTR_MEMBERNAME_LENGTH before including neostr.h  

This is a stb-style header only library so NEOSTR_IMPLEMENTATION should be defined prior to including neostr.h in the single compilation unit where the library implementations should reside. All other compilation units should include the header file without defining the macro. An simple example of a compilation unit defining the macro can be found in examples/helloworld/src/  