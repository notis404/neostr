typedef unsigned char unsigned_int_8;
typedef unsigned_int_8 unsigned_byte;
typedef unsigned long unsigned_int_32;
typedef unsigned __int64  unsigned_int_64;
typedef long signed_int_32;
typedef __int64 signed_int_64;

#if defined(_WIN64)
typedef unsigned_int_64 unsigned_pointer_size;
#else
typedef unsigned_int_32 unsigned_pointer_size;
#endif

#if defined(_WIN64)
typedef signed_int_64 signed_pointer_size;
#else
typedef signed_int_32 signed_pointer_size;
#endif

#if !defined NEO_STRING_LENGTH_TYPE
#define NEO_STRING_LENGTH_TYPE unsigned_int_32
#endif

// Statically asserts that NEO_STRING_LENGTH_TYPE must be unsigned
#define STATIC_ASSERT_UNSIGNED(type) \
typedef char static_assertion_##__LINE__[((type)-1) > 0 ? 1 : -1]
STATIC_ASSERT_UNSIGNED(NEO_STRING_LENGTH_TYPE);

typedef NEO_STRING_LENGTH_TYPE neo_string_length_type;
#define NEO_MAX_STRING_LENGTH (((neo_string_length_type)(0) - (unsigned)1) - 1)

typedef struct neo_string
{
	char* data;
	neo_string_length_type length;
}neo_string;

static const neo_string nullString = {"\0", 0};

#if !defined NEO_STRING_USE_LONG_NAMES
typedef neo_string_length_type string_length_type; 
#define CStringLength neo_CStringLength
#define StringCompare neo_StringCompare
#define StringLiteral neo_StringLiteral
#endif

#if defined(_MSC_VER)
#define FORCE_INLINE __forceinline
#endif

// This is a macro so that string literal length can be determined at compile time
// Requires C99 compound literals
#define neo_StringLiteral(stringLiteral) (struct neo_string){stringLiteral, sizeof(stringLiteral) - 1}

FORCE_INLINE neo_string neo_ValidateString(neo_string string)
{
	return (string.data) ? string : nullString;
}

FORCE_INLINE char* neo_ValidateCString(const char* cString)
{
	return cString ? (char*)cString : nullString.data; 
}

neo_string_length_type neo_CStringLength(const char* cString)
{
	cString = neo_ValidateCString(cString);
	neo_string_length_type length;
	for (length = 0; length < NEO_MAX_STRING_LENGTH; ++length)
	{
		if (*cString == '\0')
		{
			break;
		}
		cString++;
	}

	return length;
}

signed_pointer_size neo_StringCompare(neo_string leftString, neo_string rightString)
{
	leftString = neo_ValidateString(leftString);
	rightString = neo_ValidateString(rightString);

	neo_string_length_type compareLength = leftString.length <= rightString.length ? leftString.length : rightString.length;
	neo_string_length_type foundIndex = NEO_MAX_STRING_LENGTH;
	for (neo_string_length_type i = 0; i < compareLength; ++i)
	{
		// Only changes foundIndex from NEO_MAX_STRING_LENGTH at
		// the index of the first difference 
		foundIndex = (foundIndex == NEO_MAX_STRING_LENGTH) && (leftString.data[i] != rightString.data[i]) ? i : foundIndex;
	}
	// If no difference was found in the loop, foundIndex should equal compareLength
	foundIndex = foundIndex == NEO_MAX_STRING_LENGTH ? compareLength : foundIndex; 

	char leftChar = foundIndex < leftString.length  ? leftString.data[foundIndex] : 0;
	char rightChar = foundIndex < rightString.length  ? rightString.data[foundIndex] : 0;
	return leftChar - rightChar;
}

#if defined NEO_STRING_TESTS
#define ASSERT(expression) if(!(expression)) { *(int *)0 = 0;}
void* memcpy(void* dest, const void* src, size_t count);
void* malloc(size_t size);
int printf(const char* format, ...);
void neo_RunStringTests()
{
	// neo_ValidateString
	{
		neo_string invalidString = {0};
		invalidString.length = 10;
		invalidString.data = 0;
		invalidString = neo_ValidateString(invalidString);
		ASSERT(invalidString.data == nullString.data && invalidString.length == nullString.length);

		neo_string validString = {0};
		validString.data = "Hello World";
		validString.length = sizeof("Hello World");
		neo_string result = neo_ValidateString(validString);
		ASSERT(result.data == validString.data && result.length == validString.length);
	}

	//neo_StringLiteral
	{
		neo_string expectedString = {0};
		expectedString.data = "Hello World";
		expectedString.length = (sizeof("Hello World") - 1);
		neo_string stringLiteral = neo_StringLiteral("Hello World");
		ASSERT(neo_StringCompare(expectedString, stringLiteral) == 0 && stringLiteral.length == expectedString.length);
	}

	//neo_ValidateCString
	{
		char* invalidCString = 0;
		invalidCString = neo_ValidateCString(invalidCString);
		ASSERT(invalidCString == nullString.data && invalidCString[0] == '\0');

		char* validCString = "Hello World";
		char* result = neo_ValidateCString(validCString);
		ASSERT(result == validCString);
	}

	//neo_CStringLength
	{
		char* invalidCString = 0;
		neo_string_length_type length = neo_CStringLength(invalidCString);
		ASSERT(length == 0);

		char* validCString = "Hello World";
		length = neo_CStringLength(validCString);
		ASSERT(length == (sizeof("Hello World") - 1));
	}

	//neo_StringCompare
	{
		// left String is shorter and they match till that point
		ASSERT(neo_StringCompare(neo_StringLiteral("abc"), neo_StringLiteral("abcd")) == (0 - 'd'));
		
		// right string is shorter and they match till that point
		ASSERT(neo_StringCompare(neo_StringLiteral("abcd"), neo_StringLiteral("abc")) == 'd');
		
		// leftString is less
		ASSERT(neo_StringCompare(neo_StringLiteral("abc"), neo_StringLiteral("abd")) == 'c'-'d');

		// rightString is less
		ASSERT(neo_StringCompare(neo_StringLiteral("abd"), neo_StringLiteral("abc")) == 1);

		// strings are equal
		ASSERT(neo_StringCompare(neo_StringLiteral("abc"), neo_StringLiteral("abc")) == 0);
		
		// left string data is invalid
		neo_string invalidString = {0};
		ASSERT(neo_StringCompare(invalidString, neo_StringLiteral("abc")) == -'a');

		// right string data is invalid
		ASSERT(neo_StringCompare(neo_StringLiteral("abc"), invalidString) == 'a');

		// both string data is invalid
		ASSERT(neo_StringCompare(invalidString, invalidString) == 0);

		// left string is empty
		ASSERT(neo_StringCompare(neo_StringLiteral(""), neo_StringLiteral("abc")) == -'a');

		// right string is empty
		ASSERT(neo_StringCompare(neo_StringLiteral("abc"), neo_StringLiteral("")) == 'a');

		// both strings are empty
		ASSERT(neo_StringCompare(neo_StringLiteral(""), neo_StringLiteral("")) == 0);
	}
}

#endif