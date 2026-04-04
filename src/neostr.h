typedef unsigned char unsigned_int_8;
typedef unsigned_int_8 unsigned_byte;
typedef unsigned_byte bool; 
typedef unsigned short unsigned_int_16;
typedef unsigned long unsigned_int_32;
typedef unsigned __int64  unsigned_int_64;
typedef long signed_int_32;
typedef __int64 signed_int_64;

#if defined(_WIN64)
typedef unsigned_int_64 register_size;
typedef register_size unsigned_pointer_size;

typedef signed_int_64 signed_register_size;
typedef signed_register_size signed_pointer_size;

#define REGISTER_SIZE_BITS 64
#define REGISTER_SIZE_BYTES 8
#else
typedef unsigned_int_32 register_size;
typedef register_size unsigned_pointer_size;

typedef signed_int_32 signed_register_size;
typedef signed_register_size signed_pointer_size;
#define REGISTER_SIZE_BITS 32
#define REGISTER_SIZE_BYTES 4
#endif

#define NULL 0

#if !defined NEOSTR_LENGTH_TYPE
#define NEOSTR_LENGTH_TYPE register_size
#endif

// Statically asserts that NEO_STRING_LENGTH_TYPE must be unsigned
#define STATIC_ASSERT_UNSIGNED(type) \
typedef char static_assertion_##__LINE__[((type)-1) > 0 ? 1 : -1]
STATIC_ASSERT_UNSIGNED(NEOSTR_LENGTH_TYPE);

typedef NEOSTR_LENGTH_TYPE neostr_length_type;
#define NEOSTR_MAX_LENGTH (((neostr_length_type)(0) - (unsigned)1) - 1)

typedef enum 
{
	NEOSTR_TYPE_RAW = 0, // Unformatted C-string 
											 // 	[string]
	NEOSTR_TYPE_FIXED,   // Inline fixed size string 
											 // 	[length][string]
	NEOSTR_TYPE_DYNAMIC, // Inline buffer of allocated memory with length and capacity 
											 // 	[capacity, length][string......]
	NEOSTR_TYPE_VIRTUAL, // Inline buffer of virtual memory with length, reserved, and committed size 
											 // 	[reserved, committed, length][string..........=========]
	NEOSTR_TYPE_VIEW,    // Pointer to string memory with a size, not inline
	NEOSTR_TYPE_COUNT,
	NEOSTR_TYPE_INVALID = 255
} neostr_type; 

// TODO: This should be converted to a hash or something that is less likely to 
// accidentally collide with invalid data but also needs to be serializable
#define NEOSTR_ID 44

// Raw String
neostr_length_type neostr_CStringLength(const char* cString);

// All neostr types:
typedef struct neostr_metadata // the string meta data that all string types share
{
	neostr_length_type length;
	unsigned_int_8 type;
	unsigned_int_8 id;
}neostr_metadata;

#define NEOSTR_EMPTY_METADATA {0, NEOSTR_TYPE_INVALID, 0}
static const struct 
{
	neostr_metadata meta;
	char data;
} neostr_empty = {NEOSTR_EMPTY_METADATA, '\0'};

bool neostr_IsValid(const char* string);
bool neostr_IsMetadataValid(const neostr_metadata* metadata);
neostr_type neostr_Type(const char* string);
const neostr_metadata* neostr_ConstMetadataAddress(const char* string);
neostr_metadata* neostr_MetadataAddress(char* string);
neostr_metadata neostr_Metadata(const char* string);
neostr_length_type neostr_Length(const char* string);
signed_int_32 neostr_Compare(const char* left, const char* right);

// neostr_view:
// todo: const view type
// bounds checked, only supports neostr types as base string
typedef struct neostr_view
{
  char* baseStringAddress;
  neostr_length_type startOffset;
  neostr_length_type length;
}neostr_view;
static const neostr_view neostr_view_empty = { (char*)&neostr_empty.data, 0, 0 };

bool neostr_view_IsValid(const neostr_view* stringView);
neostr_length_type neostr_view_Length(const neostr_view* stringView);
neostr_view neostr_view_LayoutData(char* baseStringAddress, neostr_length_type startOffset, neostr_length_type length);

// neostr_view_raw:
// NOT-bounds checked
typedef struct neostr_view_raw
{
  char* start;
  neostr_length_type length;
}neostr_view_raw;
static const neostr_view_raw neostr_view_raw_empty = { (char*)&neostr_empty.data, 0 };

// bool neostr_view_raw_IsValid(const neostr_view_raw* rawStringView);

// neostr_fixed
typedef neostr_metadata neostr_fixed_metadata;

bool neostr_fixed_IsValid(const char* string);
bool neostr_fixed_IsMetadataValid(const neostr_fixed_metadata* metadata);
neostr_fixed_metadata neostr_fixed_CreateMetadata(neostr_length_type length);
const neostr_fixed_metadata* neostr_fixed_ConstMetadataAddress(const char* string);
neostr_fixed_metadata* neostr_fixed_MetadataAddress(char* string);
neostr_fixed_metadata neostr_fixed_Metadata(const char* string);
register_size neostr_fixed_RequiredMemory(const neostr_fixed_metadata* fixedMetadata);
char* neostr_fixed_LayoutData(const neostr_fixed_metadata* fixedMetadata, 
                              const char* stringData,
                              unsigned_byte* stringMemory);
char* neostr_fixed_LayoutViewData(const neostr_fixed_metadata* fixedMetadata,
                                  const neostr_view* stringView,
                                  unsigned_byte* stringMemory);
// creates a literal neostr_fixed (in .rodata just like string literals) 
// Must be a declaration statement, cannot be used as an expression
#define neostr_fixed_Literal(outFixedLiteral, cStringLiteral)         \
do                                                                    \
{                                                                     \
  static const struct                                                 \
  {                                                                   \
    neostr_metadata meta;                                             \
    const char data[sizeof(cStringLiteral)];                          \
  } outFixedLiteral##_obj_##__LINE__ =                                \
      {                                                               \
        { sizeof(cStringLiteral) - 1, NEOSTR_TYPE_FIXED, NEOSTR_ID }, \
        cStringLiteral                                                \
      };                                                              \
  outFixedLiteral = outFixedLiteral##_obj_##__LINE__.data;            \
} while (0)

// There is a way to create a literal neostr_fixed that works as an expression
// with an anonymous compound literal but it is not supported with MSVC C99
// This literal would have statement lifetime so it could not be used
// anywhere that needs the data to persist over multiple lines but
// it could be used as a parameter to a function.
// In GCC/Clang C99 it should be possible to do something like this:

// #define neostr_fixed_LiteralExpr(cStringLiteral)               \
// (struct                                                        \
//  {                                                             \
//    neostr_metadata meta;                                       \
//    const char data[sizeof(cStringLiteral)];                    \
//  }                                                             \
// ){                                                             \
//    {sizeof(cStringLiteral) - 1, NEOSTR_TYPE_FIXED, NEOSTR_ID}, \
//    cStringLiteral                                              \
//  }   

// neostr_dynamic:
typedef struct neostr_dynamic_metadata
{
  neostr_length_type capacity;
  neostr_metadata meta;
}neostr_dynamic_metadata;

#define NEOSTR_DYNAMIC_EMPTY_METADATA {0, NEOSTR_EMPTY_METADATA}
static const struct 
{
  neostr_dynamic_metadata meta;
  char data;
} neostr_dynamic_empty = {NEOSTR_DYNAMIC_EMPTY_METADATA, '\0'};

bool neostr_dynamic_IsValid(const char* string);
bool neostr_dynamic_IsMetadataValid(const neostr_dynamic_metadata* metadata);
neostr_dynamic_metadata neostr_dynamic_CreateMetadata(neostr_length_type capacity);
const neostr_dynamic_metadata* neostr_dynamic_ConstMetadataAddress(const char* string);
neostr_dynamic_metadata* neostr_dynamic_MetadataAddress(char* string);
neostr_dynamic_metadata neostr_dynamic_Metadata(const char* string);
neostr_length_type neostr_dynamic_Capacity(const char* string);
register_size neostr_dynamic_RequiredMemory(const neostr_dynamic_metadata* dynamicMetadata);
char* neostr_dynamic_LayoutData(const neostr_dynamic_metadata* dynamicMetadata,
                                unsigned_byte* stringMemory);
char* neostr_dynamic_LayoutAndPushData(neostr_dynamic_metadata* dynamicMetadata,
                                        const char* sourceString,
                                        unsigned_byte* stringMemory);
char* neostr_dynamic_LayoutAndPushViewData(const neostr_dynamic_metadata* dynamicMetadata,
                                            const neostr_view* sourceView,
                                            unsigned_byte* stringMemory);
void neostr_dynamic_PushData(char* destDynamic, const char* sourceString);
void neostr_dynamic_PushViewData(char* destDynamic, const neostr_view* sourceView);
void neostr_dynamic_PopData(char* destDynamic, neostr_length_type size);

// neostr_virtual:
typedef struct neostr_virtual_metadata
{
  neostr_length_type reserved;
  neostr_length_type committed;
  neostr_metadata meta;
}neostr_virtual_metadata;

#define NEOSTR_VIRTUAL_EMPTY_METADATA {0, 0, NEOSTR_EMPTY_METADATA}
static const struct 
{
  neostr_virtual_metadata meta;
  char data;
} neostr_virtual_empty = {NEOSTR_VIRTUAL_EMPTY_METADATA, '\0'};

bool neostr_virtual_IsValid(const char* virtualString);
bool neostr_virtual_IsMetadataValid(const neostr_virtual_metadata* metadata);
const neostr_virtual_metadata* neostr_virtual_ConstMetadataAddress(const char* virtualString);
neostr_virtual_metadata* neostr_virtual_MetadataAddress(char* largeString);
neostr_virtual_metadata neostr_virtual_Metadata(const char* largeString);
neostr_virtual_metadata neostr_virtual_CreateMetadata(neostr_length_type reservedSize, 
                                                      neostr_length_type committedSize);
register_size neostr_virtual_RequiredReserveSize(const neostr_virtual_metadata* virtualMetadata);
register_size neostr_virtual_InitialCommitSize(const neostr_virtual_metadata* virtualMetadata);
char* neostr_virtual_LayoutData(const neostr_virtual_metadata* virtualMetadata,
                                unsigned_byte* stringMemory);
char* neostr_virtual_LayoutAndPushData(const neostr_virtual_metadata* virtualMetadata,
                                          const char* sourceString,
                                          unsigned_byte* stringMemory);
char* neostr_virtual_LayoutAndPushViewData(const neostr_virtual_metadata* virtualMetadata,
                                              const neostr_view* stringView,
                                              unsigned_byte* stringMemory);
bool neostr_virtual_CanPushData(char* destVirtualString, neostr_length_type stringLength);
bool neostr_virtual_CanPushViewData(char* destVirtualString, const neostr_view* sourceView);
void neostr_virtual_PushData(char* destVirtualString, const char* sourceString);
void neostr_virtual_IncreaseCommittedMemory(char* destVirtualString, neostr_length_type increaseSize);

#if defined NEOSTR_IMPLEMENTATION
void* memcpy(void* dest, const void* src, size_t count);
void* memset(void* dest, unsigned_byte value, size_t n);
int memcmp(const void* left, const void* right, unsigned_pointer_size count);

// Raw (no metadata) Strings:
neostr_length_type neostr_CStringLength(const char* cString)
{
  cString = cString ? cString : "\0";
  neostr_length_type length;
  for (length = 0; length < NEOSTR_MAX_LENGTH; ++length)
  {
    if (*cString == '\0')
    {
      break;
    }
    cString++;
  }

  return length;
}

// All neostr types:
bool neostr_IsValid(const char* string)
{
	return string && ((const neostr_metadata*)string - 1)->id == NEOSTR_ID;
}

bool neostr_IsMetadataValid(const neostr_metadata* metadata)
{
	return metadata && metadata->id == NEOSTR_ID && metadata->type < NEOSTR_TYPE_COUNT;
}

neostr_type neostr_Type_Unchecked(const char* string)
{
  return ((const neostr_metadata*)string - 1)->type;
}

neostr_type neostr_Type(const char* string)
{
	return neostr_IsValid(string) ? 
				  neostr_Type_Unchecked(string) :
			 		string ?
						NEOSTR_TYPE_RAW : // if the string is not a valid neostr but is a valid pointer, it's a raw c string
            NEOSTR_TYPE_INVALID; 
}

const neostr_metadata* neostr_ConstMetadataAddress_Unchecked(const char* string)
{
  return ((const neostr_metadata*)string) - 1;
}

const neostr_metadata* neostr_ConstMetadataAddress(const char* string)
{
	return neostr_IsValid(string) ? neostr_ConstMetadataAddress_Unchecked(string) : &neostr_empty.meta; 
}

neostr_metadata* neostr_MetadataAddress_Unchecked(char* string)
{
  return (neostr_metadata*)neostr_ConstMetadataAddress_Unchecked(string); 
}

neostr_metadata* neostr_MetadataAddress(char* string)
{
	return (neostr_metadata*)neostr_ConstMetadataAddress(string); 
}

neostr_metadata neostr_Metadata_Unchecked(const char* string)
{
  return *neostr_ConstMetadataAddress_Unchecked(string); 
}

neostr_metadata neostr_Metadata(const char* string)
{
	return *neostr_ConstMetadataAddress(string); 
}

neostr_length_type neostr_Length_Unchecked(const char* string)
{   
  return neostr_ConstMetadataAddress_Unchecked(string)->length;
}

neostr_length_type neostr_Length(const char* string)
{  
	// TODO: Make a macro to make handling raw C strings optional:
	// If the source string pointer is valid but is not a neostr, it's a raw c string  
	return neostr_IsValid(string) ?
					neostr_Length_Unchecked(string) :
					string ?
			 			neostr_CStringLength(string) :
			 			0;
}

signed_int_32 neostr_Compare_Unchecked(const char* left, neostr_length_type leftLength, const char* right, neostr_length_type rightLength)
{
  neostr_length_type smallerLength = leftLength < rightLength ? leftLength : rightLength;

  signed_int_32 textCompare = memcmp(left, right, smallerLength);
  if (textCompare == 0 && leftLength != rightLength)
  {
    // one of the strings is longer
    textCompare = leftLength != smallerLength ? left[smallerLength] : -right[smallerLength];    
  }
  return textCompare;
}


signed_int_32 neostr_Compare(const char* left, const char* right)
{ 
  return neostr_Compare_Unchecked(left ? left : &neostr_empty.data, neostr_Length(left), 
                                  right ? right : &neostr_empty.data, neostr_Length(right));
}

// neostr_view:
// =====================

bool neostr_view_IsValid(const neostr_view* stringView)
{
  return stringView && neostr_IsValid(stringView->baseStringAddress) && stringView->length <= neostr_Length_Unchecked(stringView->baseStringAddress); 
}

neostr_length_type neostr_view_Length(const neostr_view* stringView)
{
  return neostr_view_IsValid(stringView) ? stringView->length : 0;
}

const char* neostr_view_ConstData_Unchecked(const neostr_view* stringView)
{
  return stringView->baseStringAddress + stringView->startOffset;
}

const char* neostr_view_ConstData(const neostr_view* stringView)
{
  return neostr_view_IsValid(stringView) ? neostr_view_ConstData_Unchecked(stringView) : &neostr_empty.data;
}

char* neostr_view_Data_Unchecked(neostr_view* stringView)
{
  return (char*)neostr_view_ConstData_Unchecked(stringView);
}

char* neostr_view_Data(neostr_view* stringView)
{
  return (char*)neostr_view_ConstData(stringView);
}

neostr_view neostr_view_LayoutData(char* baseStringAddress, neostr_length_type startOffset, neostr_length_type length)
{
  neostr_view out = neostr_view_empty;
  if (neostr_IsValid(baseStringAddress))
  { 
    out.baseStringAddress = baseStringAddress;
    out.startOffset = startOffset;
    out.length = length;
  }
	return out;
}

signed_int_32 neostr_view_Compare(const neostr_view* leftView, const neostr_view* rightView)
{
  leftView = neostr_view_IsValid(leftView) ? leftView : &neostr_view_empty;
  rightView = neostr_view_IsValid(rightView) ? rightView : &neostr_view_empty;
  return neostr_Compare_Unchecked(neostr_view_ConstData_Unchecked(leftView), leftView->length,
                                  neostr_view_ConstData_Unchecked(rightView), rightView->length);
}

signed_int_32 neostr_view_CompareToString(const neostr_view* leftView, const char* rightString)
{  
  leftView = neostr_view_IsValid(leftView) ? leftView : &neostr_view_empty;
  rightString = rightString ? rightString : &neostr_empty.data;

  // Todo make handling c string optional
  neostr_length_type rightLength = neostr_IsValid(rightString) || rightString == &neostr_empty.data ?
                                    neostr_Length_Unchecked(rightString) :
                                    neostr_CStringLength(rightString);
  return neostr_Compare_Unchecked(neostr_view_ConstData_Unchecked(leftView),
                                  leftView->length,
                                  rightString,
                                  rightLength);
}

// neostr_fixed:
// =====================
bool neostr_fixed_IsValid(const char* string)
{
	return neostr_IsValid(string) && neostr_Type_Unchecked(string) == NEOSTR_TYPE_FIXED;
}

bool neostr_fixed_IsMetadataValid(const neostr_fixed_metadata* metadata)
{
	return metadata && metadata->id == NEOSTR_ID && metadata->type == NEOSTR_TYPE_FIXED;
}

neostr_fixed_metadata neostr_fixed_CreateMetadata(neostr_length_type length)
{
	neostr_fixed_metadata metadata;
	metadata.length = length;
	metadata.type = NEOSTR_TYPE_FIXED;
	metadata.id = NEOSTR_ID;
	return metadata;
}

const neostr_fixed_metadata* neostr_fixed_ConstMetadataAddress_Unchecked(const char* string)
{
  return ((const neostr_fixed_metadata*)string) - 1;
}

const neostr_fixed_metadata* neostr_fixed_ConstMetadataAddress(const char* string)
{
	return neostr_fixed_IsValid(string) ?
			 		neostr_fixed_ConstMetadataAddress_Unchecked(string) :
			 		&neostr_empty.meta;
}

neostr_fixed_metadata* neostr_fixed_MetadataAddress_Unchecked(char* string)
{
  return (neostr_fixed_metadata*)neostr_fixed_ConstMetadataAddress_Unchecked(string);
}

neostr_fixed_metadata* neostr_fixed_MetadataAddress(char* string)
{
	return (neostr_fixed_metadata*)neostr_fixed_ConstMetadataAddress(string);
}

neostr_fixed_metadata neostr_fixed_Metadata_Unchecked(const char* string)
{
  return *neostr_fixed_ConstMetadataAddress_Unchecked(string);
}

neostr_fixed_metadata neostr_fixed_Metadata(const char* string)
{
	return *neostr_fixed_ConstMetadataAddress(string);
}

register_size neostr_fixed_RequiredMemory(const neostr_fixed_metadata* fixedMetadata)
{
	// TODO: make null-terminator toggleable with a macro switch
	// will always ask for room for a null terminator to ensure compatibilty with other C libs
	return neostr_fixed_IsMetadataValid(fixedMetadata) ? 
					sizeof(neostr_fixed_metadata) + fixedMetadata->length + 1 :
					0;
}

char* neostr_fixed_LayoutData_Unchecked(const neostr_fixed_metadata* fixedMetadata,
                                        const char* stringData,
                                        neostr_length_type stringLength,
                                        unsigned_byte* stringMemory)
{
  memcpy(stringMemory, fixedMetadata, sizeof(neostr_fixed_metadata));
  memcpy(stringMemory + sizeof(neostr_fixed_metadata), stringData, stringLength);
  
  // TODO: Null terminator toggle
  // If valid memory was copied, write a null terminator to the end of memory
  memset(stringMemory + sizeof(neostr_fixed_metadata) + stringLength, 0, 1);

  return (char*)(stringMemory + sizeof(neostr_fixed_metadata));
}

char* neostr_fixed_LayoutData(const neostr_fixed_metadata* fixedMetadata,
                              const char* stringData,
                              unsigned_byte* stringMemory)
{
  char* out = (char*)&neostr_empty.data;
  if (neostr_fixed_IsMetadataValid(fixedMetadata) && stringData && stringMemory)
  {
    neostr_length_type stringLength = neostr_IsValid(stringData) ? neostr_Length_Unchecked(stringData) : neostr_CStringLength(stringData);
    if (stringLength <= fixedMetadata->length)
    {
      out = neostr_fixed_LayoutData_Unchecked(fixedMetadata, stringData, stringLength, stringMemory);
    }
  }
	return out;
}

char* neostr_fixed_LayoutViewData(const neostr_fixed_metadata* fixedMetadata, const neostr_view* stringView, unsigned_byte* stringMemory)
{
  char* out = (char*)&neostr_empty.data;
  if (neostr_fixed_IsMetadataValid(fixedMetadata) && neostr_view_IsValid(stringView) && stringMemory && stringView->length <= fixedMetadata->length)
  {
    out = neostr_fixed_LayoutData_Unchecked(fixedMetadata, neostr_view_ConstData_Unchecked(stringView), stringView->length, stringMemory);
  }
  return out; 
}

// neostr_dynamic:
// =====================
bool neostr_dynamic_IsValid(const char* string)
{
	return neostr_IsValid(string) && neostr_Type(string) == NEOSTR_TYPE_DYNAMIC;
}

bool neostr_dynamic_IsMetadataValid(const neostr_dynamic_metadata* metadata)
{
	return metadata && metadata->meta.id == NEOSTR_ID && metadata->meta.type == NEOSTR_TYPE_DYNAMIC;
}

neostr_length_type neostr_dynamic_Capacity_Unchecked(const char* string)
{  
  return neostr_dynamic_ConstMetadataAddress(string)->capacity;
}

neostr_length_type neostr_dynamic_Capacity(const char* string)
{  
  return neostr_dynamic_IsValid(string) ? neostr_dynamic_Capacity_Unchecked(string) : 0;
}

neostr_dynamic_metadata neostr_dynamic_CreateMetadata(neostr_length_type capacity)
{
	neostr_dynamic_metadata dynamicMetadata;
	dynamicMetadata.capacity = capacity;
	dynamicMetadata.meta.length = 0;
	dynamicMetadata.meta.type = NEOSTR_TYPE_DYNAMIC;
	dynamicMetadata.meta.id = NEOSTR_ID;
	return dynamicMetadata;
}

const neostr_dynamic_metadata* neostr_dynamic_ConstMetadataAddress_Unchecked(const char* string)
{
  return ((const neostr_dynamic_metadata*)string) - 1;
}

const neostr_dynamic_metadata* neostr_dynamic_ConstMetadataAddress(const char* string)
{
	return neostr_dynamic_IsValid(string) ?
			 		neostr_dynamic_ConstMetadataAddress_Unchecked(string) :
			 		&neostr_dynamic_empty.meta;
}

neostr_dynamic_metadata* neostr_dynamic_MetadataAddress_Unchecked(char* string)
{
  return (neostr_dynamic_metadata*)neostr_dynamic_ConstMetadataAddress_Unchecked(string);
}

neostr_dynamic_metadata* neostr_dynamic_MetadataAddress(char* string)
{
	return (neostr_dynamic_metadata*)neostr_dynamic_ConstMetadataAddress(string);
}

neostr_dynamic_metadata neostr_dynamic_Metadata_Unchecked(const char* string)
{
  return *neostr_dynamic_ConstMetadataAddress_Unchecked(string);
}

neostr_dynamic_metadata neostr_dynamic_Metadata(const char* string)
{
	return *neostr_dynamic_ConstMetadataAddress(string);
}

register_size neostr_dynamic_RequiredMemory(const neostr_dynamic_metadata* dynamicMetadata)
{
	// TODO: make null-terminator toggleable with a macro switch
	// will always ask for room for a null terminator to ensure compatibilty with other C libs
	return neostr_dynamic_IsMetadataValid(dynamicMetadata) ? 
					sizeof(neostr_dynamic_metadata) + dynamicMetadata->capacity + 1 :
					0;
}

char* neostr_dynamic_LayoutData_Unchecked(const neostr_dynamic_metadata* dynamicMetadata, unsigned_byte* stringMemory)
{
  memcpy(stringMemory, dynamicMetadata, sizeof(neostr_dynamic_metadata));
  memset(stringMemory + sizeof(neostr_dynamic_metadata), 0, dynamicMetadata->capacity);

  // TODO: Toggleable null-terminator
  // Using memset so that memory is not written to if the layout was not valid
  memset(stringMemory + sizeof(neostr_dynamic_metadata) + dynamicMetadata->capacity, 0, 1);
  return (char*)(stringMemory + sizeof(neostr_dynamic_metadata));
}

char* neostr_dynamic_LayoutData(const neostr_dynamic_metadata* dynamicMetadata, unsigned_byte* stringMemory)
{
  char* out = (char*)&neostr_dynamic_empty.data;
  if (neostr_dynamic_IsMetadataValid(dynamicMetadata) && stringMemory)
  {
    out = neostr_dynamic_LayoutData_Unchecked(dynamicMetadata, stringMemory);
  }
	return out;
}

void neostr_dynamic_PushData_Unchecked(char* destDynamic, const char* sourceString, neostr_length_type sourceLength)
{
  neostr_dynamic_metadata* destMetadata = neostr_dynamic_MetadataAddress_Unchecked(destDynamic);
	memcpy(destDynamic + destMetadata->meta.length, sourceString, sourceLength);
	destMetadata->meta.length += sourceLength;
}

void neostr_dynamic_PushData(char* destDynamic, const char* sourceString)
{
  if (neostr_dynamic_IsValid(destDynamic) && sourceString)
  {
    neostr_length_type sourceLength = neostr_IsValid(sourceString) ? neostr_Length_Unchecked(sourceString) : neostr_CStringLength(sourceString);
    if (sourceLength <= neostr_dynamic_Capacity_Unchecked(destDynamic) - neostr_Length_Unchecked(destDynamic))
    {
      neostr_dynamic_PushData_Unchecked(destDynamic, sourceString, sourceLength);
    }
  }
}

char* neostr_dynamic_LayoutAndPushData_Unchecked(const neostr_dynamic_metadata* dynamicMetadata,
                                        const char* sourceString,
                                        neostr_length_type sourceLength,
                                        unsigned_byte* stringMemory)
{
  char* dynamicString = neostr_dynamic_LayoutData_Unchecked(dynamicMetadata, stringMemory);
  neostr_dynamic_PushData_Unchecked(dynamicString, sourceString, sourceLength);
  return dynamicString;
}

char* neostr_dynamic_LayoutAndPushData(neostr_dynamic_metadata* dynamicMetadata,
                                        const char* sourceString,
                                        unsigned_byte* stringMemory)
{
  char* dynamicString = (char*)&neostr_empty.data;
  if (neostr_dynamic_IsMetadataValid(dynamicMetadata) && sourceString && stringMemory)
  {
    neostr_length_type sourceLength = neostr_IsValid(sourceString) ? neostr_Length_Unchecked(sourceString) : neostr_CStringLength(sourceString);
    if (sourceLength <= dynamicMetadata->capacity - dynamicMetadata->meta.length)
    {
      dynamicString = neostr_dynamic_LayoutAndPushData_Unchecked(dynamicMetadata, sourceString, sourceLength, stringMemory); 
    }
  }
  
  return dynamicString;
}

char* neostr_dynamic_LayoutAndPushViewData(const neostr_dynamic_metadata* dynamicMetadata,
                                            const neostr_view* sourceView,
                                            unsigned_byte* stringMemory)
{
  char* dynamicString = (char*)&neostr_empty.data;
  if (neostr_dynamic_IsMetadataValid(dynamicMetadata) && neostr_view_IsValid(sourceView) && stringMemory &&
      sourceView->length <= dynamicMetadata->capacity - dynamicMetadata->meta.length)
  {
    dynamicString = neostr_dynamic_LayoutAndPushData_Unchecked(dynamicMetadata, neostr_view_ConstData(sourceView), sourceView->length, stringMemory); 
  }
  return dynamicString;
}

void neostr_dynamic_PushViewData(char* destDynamic, const neostr_view* sourceView)
{
  if (neostr_dynamic_IsValid(destDynamic) && neostr_view_IsValid(sourceView))
  {
    neostr_dynamic_PushData_Unchecked(destDynamic, neostr_view_ConstData_Unchecked(sourceView), sourceView->length);
  }
}

void neostr_dynamic_PopData_Unchecked(neostr_dynamic_metadata* destMetadata, neostr_length_type size)
{
  destMetadata->meta.length -= size;
}

void neostr_dynamic_PopData(char* destDynamic, neostr_length_type size)
{
  neostr_dynamic_metadata* destMetadata = neostr_dynamic_MetadataAddress(destDynamic);
  size = destMetadata != &neostr_dynamic_empty.meta && neostr_Length_Unchecked(destDynamic) >= size ? size : 0;
  neostr_dynamic_PopData_Unchecked(destMetadata, size);
}

// neostr_virtual
// =====================
bool neostr_virtual_IsValid(const char* virtualString)
{
	return neostr_IsValid(virtualString) && neostr_Type(virtualString) == NEOSTR_TYPE_VIRTUAL;
}

bool neostr_virtual_IsMetadataValid(const neostr_virtual_metadata* metadata)
{
	return metadata && metadata->meta.id == NEOSTR_ID && metadata->meta.type == NEOSTR_TYPE_VIRTUAL;
}

const neostr_virtual_metadata* neostr_virtual_ConstMetadataAddress_Unchecked(const char* virtualString)
{
  return ((const neostr_virtual_metadata*)virtualString) - 1;
}

const neostr_virtual_metadata* neostr_virtual_ConstMetadataAddress(const char* virtualString)
{
	return neostr_virtual_IsValid(virtualString) ?
				 	neostr_virtual_ConstMetadataAddress_Unchecked(virtualString) :
				 	&neostr_virtual_empty.meta;
}

neostr_virtual_metadata* neostr_virtual_MetadataAddress_Unchecked(char* virtualString)
{
  return (neostr_virtual_metadata*)neostr_virtual_ConstMetadataAddress_Unchecked(virtualString);
}

neostr_virtual_metadata* neostr_virtual_MetadataAddress(char* largeString)
{
	return (neostr_virtual_metadata*)neostr_virtual_ConstMetadataAddress(largeString);
}

neostr_virtual_metadata neostr_virtual_Metadata_Unchecked(const char* largeString)
{
  return *neostr_virtual_ConstMetadataAddress_Unchecked(largeString);
}

neostr_virtual_metadata neostr_virtual_Metadata(const char* largeString)
{
	return *neostr_virtual_ConstMetadataAddress(largeString);
}

neostr_virtual_metadata neostr_virtual_CreateMetadata(neostr_length_type reservedSize, neostr_length_type committedSize)
{
	neostr_virtual_metadata metadata;
	metadata.reserved = reservedSize;
	metadata.committed = committedSize;
	metadata.meta.length = 0;
	metadata.meta.type = NEOSTR_TYPE_VIRTUAL;
	metadata.meta.id = NEOSTR_ID;
	return metadata;
}

register_size neostr_virtual_RequiredReserveSize(const neostr_virtual_metadata* virtualMetadata)
{
	// TODO: make this toggleable with a macro switch
	// will always ask for room for a null terminator to ensure compatibilty with other C libs
	return neostr_virtual_IsMetadataValid(virtualMetadata) ? 
					sizeof(neostr_virtual_metadata) + virtualMetadata->reserved + 1 :
					0; 
}

register_size neostr_virtual_InitialCommitSize(const neostr_virtual_metadata* virtualMetadata)
{
	// TODO: make this toggleable with a macro switch
	// will always ask for room for a null terminator to ensure compatibilty with other C libs
	return neostr_virtual_IsMetadataValid(virtualMetadata) ?
					sizeof(neostr_virtual_metadata) + virtualMetadata->committed + 1 :
					0;
}

char* neostr_virtual_LayoutData_Unchecked(const neostr_virtual_metadata* virtualMetadata, unsigned_byte* stringMemory)
{
	memcpy(stringMemory, virtualMetadata, sizeof(neostr_virtual_metadata));
	memset(stringMemory + sizeof(neostr_virtual_metadata), 0, virtualMetadata->committed);

  // TODO: Toggleable null terminator
  memset(stringMemory + sizeof(neostr_virtual_metadata) + virtualMetadata->committed, 0, 1);
  return (char*)(stringMemory + sizeof(neostr_virtual_metadata)); 
}

char* neostr_virtual_LayoutData(const neostr_virtual_metadata* virtualMetadata, unsigned_byte* stringMemory)
{
	return neostr_virtual_IsMetadataValid(virtualMetadata) && stringMemory ?
          neostr_virtual_LayoutData_Unchecked(virtualMetadata, stringMemory) :
          (char*)&neostr_virtual_empty.data; 
}

bool neostr_virtual_CanPushData(char* destVirtualString, neostr_length_type stringLength)
{ 
  const neostr_virtual_metadata* virtualMetadata = neostr_virtual_ConstMetadataAddress(destVirtualString);
	return virtualMetadata != &neostr_virtual_empty.meta &&
          virtualMetadata->meta.length + stringLength <= virtualMetadata->committed;
}

void neostr_virtual_PushData_Unchecked(char* destVirtualString, const char* sourceString, neostr_length_type sourceLength)
{
	memcpy(destVirtualString + neostr_Length_Unchecked(destVirtualString), sourceString, sourceLength);
	neostr_virtual_MetadataAddress_Unchecked(destVirtualString)->meta.length += sourceLength;
}

void neostr_virtual_PushData(char* destVirtualString, const char* sourceString)
{
  sourceString = sourceString ? sourceString : &neostr_empty.data;
  neostr_length_type sourceLength = neostr_IsValid(sourceString) || sourceString != &neostr_empty.data ?
                                      neostr_Length_Unchecked(sourceString) :
                                      neostr_CStringLength(sourceString);
	if (neostr_virtual_CanPushData(destVirtualString, sourceLength))
	{
    neostr_virtual_PushData_Unchecked(destVirtualString, sourceString, sourceLength);
	}
}

char* neostr_virtual_LayoutAndPushData(const neostr_virtual_metadata* virtualMetadata,
                                        const char* sourceString,
                                        unsigned_byte* stringMemory)
{
  char* virtualString = neostr_virtual_LayoutData(virtualMetadata, stringMemory);
  sourceString = sourceString ? sourceString : &neostr_empty.data;
  neostr_length_type sourceLength = neostr_IsValid(sourceString) || sourceString != &neostr_empty.data ?
                                      neostr_Length_Unchecked(sourceString) :
                                      neostr_CStringLength(sourceString);
  if (neostr_virtual_CanPushData(virtualString, sourceLength))
  {
    neostr_virtual_PushData_Unchecked(virtualString, sourceString, sourceLength);
  }
  return virtualString;
}

char* neostr_virtual_LayoutAndPushViewData(const neostr_virtual_metadata* virtualMetadata,
                                            const neostr_view* stringView,
                                            unsigned_byte* stringMemory)
{
  char* virtualString = neostr_virtual_LayoutData(virtualMetadata, stringMemory);
  stringView = virtualString != &neostr_virtual_empty.data && stringView ? stringView : &neostr_view_empty;
  neostr_virtual_PushData_Unchecked(virtualString, neostr_view_ConstData_Unchecked(stringView), stringView->length);
  return virtualString;
}

void neostr_virtual_IncreaseCommittedSize_Unchecked(neostr_virtual_metadata* destVirtualMetadata, neostr_length_type increaseSize)
{
  destVirtualMetadata->committed += increaseSize;
}

void neostr_virtual_IncreaseCommittedSize(char* destVirtualString, neostr_length_type increaseSize)
{
	if (neostr_virtual_IsValid(destVirtualString))
  {
    neostr_virtual_IncreaseCommittedSize_Unchecked(neostr_virtual_MetadataAddress_Unchecked(destVirtualString), increaseSize);
  }
}

#endif // NEOSTR_IMPLEMENTATION

#if defined NEOSTR_TESTS
#include "neostr_tests.h"
#endif