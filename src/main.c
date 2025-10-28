#define NEO_STRING_TESTS
#include "neo_string.h"
#define UNREFERENCED(P) (P)

void* memcpy(void* dest, const void* src, size_t count);
void* malloc(size_t size);
int printf(const char* format, ...);

int main()
{
	neo_string string = neo_StringLiteral("Hello World");

	// "weak" string copy
	neo_string weakCopyString = {0};
	weakCopyString.length = string.length;
	weakCopyString.data = string.data;

	// String data copy
	neo_string copyString = {0};
	copyString.length = string.length;
	copyString.data = malloc(string.length);
	memcpy(copyString.data, string.data, string.length);

	// String comparison
	if (neo_StringCompare(weakCopyString, copyString) == 0)
	{
		printf("Weak copy equals deep copy\n");
	}

	if (neo_StringCompare(copyString, neo_StringLiteral("Hello World")))
	{
		printf("String literal equals deep copy\n");
	}

	// String concatenation
	neo_string concatenatedString = {0};
	concatenatedString.length = weakCopyString.length + copyString.length;
	concatenatedString.data = malloc(weakCopyString.length + copyString.length);
	memcpy(concatenatedString.data, weakCopyString.data, weakCopyString.length);
	memcpy(concatenatedString.data + weakCopyString.length, copyString.data, copyString.length);


	if (neo_StringCompare(copyString, concatenatedString) == 0)
	{
		printf("String comparison failed\n");
	}

	neo_RunStringTests();
}















