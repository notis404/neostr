#if !defined NEO_STRING_USE_LONG_NAMES
#define GetStringLength neo_GetStringLength
#define GetRawStringLength neo_GetRawStringLength
#define GetStringMemorySize neo_GetStringMemorySize
#define CreateString neo_CreateString
#define StringCompare neo_StringCompare
#endif

typedef unsigned __int64  uint_64;
typedef unsigned long uint_32;
typedef unsigned char byte;
typedef signed char int8;

#if !defined NEO_STRING_SIZE_TYPE
#define NEO_STRING_SIZE_TYPE uint_32
#endif

#if !defined NEO_MAX_STRING_SEARCH_LEN
// Max value of string size type 
#define NEO_MAX_STRING_SEARCH_LEN ((NEO_STRING_SIZE_TYPE)(0) - 1)
#endif

#if defined _WIN32
#if !defined NEO_SWAP_ENDIANNESS_32BIT
#define NEO_SWAP_ENDIANNESS_32BIT(x) ( \
    (((x) & 0x000000FFUL) << 24) | \
    (((x) & 0x0000FF00UL) <<  8) | \
    (((x) & 0x00FF0000UL) >>  8) | \
    (((x) & 0xFF000000UL) >> 24) )
#endif 
#endif

#define NEO_STRING_SIGNATURE NEO_SWAP_ENDIANNESS_32BIT(0xDEADBEEF)
typedef struct neo_stringHeader
{
#if defined ALLOW_C_STRINGS
	uint_32 signature;
#endif
	NEO_STRING_SIZE_TYPE size;
}neo_stringHeader;

#if defined ALLOW_C_STRINGS
#define neo_CheckStringSignature(string) ((string) ? neo_GetStringHeader(string)->signature == NEO_STRING_SIGNATURE : 0)
#endif

#define neo_GetStringMemorySize(string) (sizeof(neo_stringHeader) + GetRawStringLength(string) + 1)
#define neo_GetStringHeader(string) ((neo_stringHeader*)string - 1)
#define neo_GetStringLength(string) ( (string) ? neo_GetStringHeader(string)->size : 0)

#define neo_MacroContcatImplementation(a, b) a##b
#define neo_MacroConcat(a, b) neo_MacroContcatImplementation(a,b)

// Intercepts string literals before static allocation and allocates them in a
// custom read-only data section (.lits) using the neo string structure, making
// them compatible with the library functions
#if defined _WIN32
#pragma section(".lits", read) 
#if defined ALLOW_C_STRINGS
#define neo_StringLiteral(variableName, literal) 																		\
__declspec(allocate(".lits")) 																											\
static struct 																																			\
{ 																																									\
	uint_32 signature; 																																\
	NEO_STRING_SIZE_TYPE size; 																																		\
	char literalData[sizeof(literal)]; 																								\
} neo_MacroConcat(literalStruct, __LINE__) = {NEO_STRING_SIGNATURE, sizeof(literal), literal};\
const char* variableName = neo_MacroConcat(literalStruct, __LINE__).literalData;		
#else // ALLOW_C_STRINGS
#define neo_StringLiteral(variableName, literal) 																\
__declspec(allocate(".lits")) 																									\
static struct 																																	\
{ 																																							\
	NEO_STRING_SIZE_TYPE size; 																																\
	char literalData[sizeof(literal)]; 																						\
} neo_MacroConcat(literalStruct, __LINE__) = {sizeof(literal), literal};				\
const char* variableName = neo_MacroConcat(literalStruct, __LINE__).literalData;
#endif // !ALLOW_C_STRINGS
#endif // _WIN32

NEO_STRING_SIZE_TYPE neo_GetRawStringLength(const char* rawString)
{
	NEO_STRING_SIZE_TYPE size = 0;
	const char* c = rawString;
	while (*c != '\0' && size <= NEO_MAX_STRING_SEARCH_LEN)
	{
		c++;
		size++;
	} 
	return size;
}

// Buffer must be at least the size returned by neo_GetStringMemorySize
void neo_CreateString(char** buffer, const char* rawString)
{
	neo_stringHeader* stringHeader = ((neo_stringHeader*)*buffer);
	stringHeader->size = GetRawStringLength(rawString);
	char* string = (char*)(stringHeader + 1);
	
	for (NEO_STRING_SIZE_TYPE i = 0; i <= stringHeader->size; ++i)
	{
		string[i] = rawString[i];
	}

#if defined ALLOW_C_STRINGS
	stringHeader->signature = NEO_STRING_SIGNATURE;
#endif
	// Point the string to the start of the characters, after the header
	*buffer = string; 
}

uint_32 neo_StringCompare(const char* leftString, const char* rightString)
{
	NEO_STRING_SIZE_TYPE leftSize = neo_GetStringLength(leftString);
	NEO_STRING_SIZE_TYPE rightSize = neo_GetStringLength(rightString); 
#if defined ALLOW_C_STRINGS
	if (!neo_CheckStringSignature(leftString))
		 leftSize = neo_GetRawStringLength(leftString);

	if (!neo_CheckStringSignature(rightString))
		 rightSize = neo_GetRawStringLength(rightString);
#endif
	NEO_STRING_SIZE_TYPE shortestLength = leftSize <= rightSize ? leftSize : rightSize;

	NEO_STRING_SIZE_TYPE i;
	for (i = 0; i < shortestLength; ++i)
	{
		if (leftString[i] != rightString[i])
			break;
	}
	return leftString[i] - rightString[i];
}