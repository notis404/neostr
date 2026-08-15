// neostr consists of 3 parts, sequential in memory
// [payload][metadata][string data]
// payload: user-supplied, can be a struct of any size
// metadata: contains information about the payload, such as size and the most recent checksum of the payload
// string data: the memory allocated for the string data

// Users hold onto a pointer to the string data so the address appears like a normal string
// Base memory address can be retrieved by retrieving the address to the payload 
// Macros are used to access the members of different payload types by name
// Certain macro functions expect a certain kind of member variable to exist to function correctly (length, capacity, etc)
// Comments above these macro functions will specify which member names are required. 
// The actual text of the member name can be configured with the member name macros (NEOSTR_MEMBERNAME_LENGTH, etc)  

#define NEOSTR_MEMBERNAME_LENGTH length
#define NEOSTR_MEMBERNAME_CAPACITY capacity

// Basic types
typedef unsigned char neostr_uInt8;
typedef neostr_uInt8 neostr_uByte;
typedef neostr_uByte neostr_uBool8; 
typedef unsigned short neostr_uInt16;
typedef neostr_uInt16 neostr_uChar16;
typedef unsigned long neostr_uInt32;
typedef unsigned __int64  neostr_uInt64;
typedef int neostr_sInt32;
typedef __int64 neostr_sInt64;
typedef float neostr_float32;
typedef double neostr_float64;

// Some simple payload structs supplied for ease of use
typedef struct neostr_fixed_payload64
{
  neostr_uInt64 length;
}neostr_fixed_payload64;

typedef struct neostr_dynamic_payload64
{
  neostr_uInt64 capacity;
  neostr_uInt64 length;
}neostr_dynamic_payload64;

typedef struct neostr_metadata
{
  neostr_uInt64 payloadSize;
  neostr_uInt64 payloadHash;
}neostr_metadata;

#define neostr_Metadata(string) ((neostr_metadata*)(string - sizeof(neostr_metadata)))
#define neostr_Payload(string, payloadType) ((payloadType*)((neostr_uByte*)neostr_Metadata(string) - neostr_Metadata(string)->payloadSize))
#define neostr_IsValid(string) (neostr_Hash(neostr_Payload(string, void), neostr_Metadata(string)->payloadSize) == neostr_Metadata(string)->payloadHash)
#define neostr_ResetHash(string) neostr_Metadata(string)->payloadHash = neostr_Hash(neostr_Payload(string, void), neostr_Metadata(string)->payloadSize)
#define neostr_HeaderSize(payloadType) (sizeof(payloadType) + sizeof(neostr_metadata))
#define neostr_LayoutHeader(stringMemory, payloadType)                                                 \
  (memset(stringMemory, 0, sizeof(payloadType))),                                                      \
  (memset(stringMemory + sizeof(payloadType), 0, sizeof(neostr_metadata))),                            \
  (stringMemory = (void*)((neostr_uByte*)stringMemory + sizeof(payloadType) + sizeof(neostr_metadata))),  \
  (neostr_Metadata(stringMemory)->payloadSize = sizeof(payloadType)),                                  \
  (neostr_Metadata(stringMemory)->payloadHash = neostr_Hash(neostr_Payload(stringMemory, payloadType), sizeof(payloadType)))
#define neostr_SetPayloadMember(string, payloadType, memberName, newMemberValue)  \
  ((neostr_Payload(string, payloadType)->memberName = newMemberValue), (neostr_ResetHash(string)))
#define neostr_SetPayloadMember_Checked(string, payloadType, memberName, newMemberValue)  \
  (neostr_IsValid(string) ? neostr_SetPayloadMember(string, payloadType, memberName, newMemberValue) : 0)
#define neostr_GetPayloadMember(string, payloadType, memberName) \
  (neostr_Payload(string, payloadType)->memberName)
#define neostr_GetPayloadMember_Checked(string, payloadType, memberName) \
  (neostr_IsValid(string) ? neostr_GetPayloadMember(string, payloadType, memberName) : 0)
#define neostr_GetLength(string, payloadType) \
  neostr_GetPayloadMember(string, payloadType, NEOSTR_MEMBERNAME_LENGTH)
#define neostr_GetLength_Checked(string, payloadType) \
  neostr_GetPayloadMember_Checked(string, payloadType, NEOSTR_MEMBERNAME_LENGTH)
#define neostr_SetLength(string, payloadType, newLength) \
  neostr_SetPayloadMember(string, payloadType, NEOSTR_MEMBERNAME_LENGTH, newLength)
#define neostr_SetLength_Checked(string, payloadType, newLength) \
  neostr_SetPayloadMember_Checked(string, payloadType, NEOSTR_MEMBERNAME_LENGTH, newLength)
#define neostr_GetCapacity(string, payloadType) \
  neostr_GetPayloadMember(string, payloadType, NEOSTR_MEMBERNAME_CAPACITY)
#define neostr_GetCapacity_Checked(string, payloadType) \
  neostr_GetPayloadMember_Checked(string, payloadType, NEOSTR_MEMBERNAME_CAPACITY)
#define neostr_SetCapacity(string, payloadType, newCapacity) \
  neostr_SetPayloadMember(string, payloadType, NEOSTR_MEMBERNAME_CAPACITY, newCapacity)
#define neostr_SetCapacity_Checked(string, payloadType, newCapacity) \
  neostr_SetPayloadMember_Checked(string, payloadType, NEOSTR_MEMBERNAME_CAPACITY, newCapacity)

// Requires payload members NEOSTR_MEMBERNAME_LENGTH, NEOSTR_MEMBERNAME_CAPACITY 
#define neostr_Push(string, payloadType, data, dataSize) \
  (neostr_GetLength(string, payloadType) + dataSize <= neostr_GetCapacity(string, payloadType)) ?  \
    (memcpy(string + neostr_GetLength(string, payloadType), data, dataSize)),                     \
    (neostr_Payload(string, payloadType)->NEOSTR_MEMBERNAME_LENGTH += dataSize),                  \
    (neostr_ResetHash(string)) : 0
#define neostr_Push_Checked(string, payloadType, data, dataSize) \
  neostr_IsValid(string) ? neostr_Push(string, payloadType, data, dataSize) : 0


// Requires payload members NEOSTR_MEMBERNAME_LENGTH
#define neostr_SetString(stringAddress, payloadType, stringData, stringSize)  \
    (memcpy(stringAddress, stringData, stringSize)),                          \
    (neostr_SetLength(stringAddress, neostr_fixed_payload64, stringSize)),    \
    (neostr_ResetHash(stringAddress))
#define neostr_SetString_Checked(stringAddress, payloadType, stringData, stringSize)  \
  neostr_IsValid(stringAddress) ?                                                     \
    neostr_SetString(stringAddress, payloadType, stringData, stringSize) : 0

#define neostr_Compare_Sized_Checked(leftString, leftLength, rightString, rightLength)  \
  neostr_Compare_Sized(                                                                 \
    leftString ? leftString : "\0",                                                     \
    leftString ? leftLength : 0,                                                        \
    rightString ? rightString : "\0",                                                   \
    rightString ? rightLength : 0)                             
#define neostr_Compare_Checked(leftString, leftPayloadType, rightString, rightPayloadType)                            \
  neostr_Compare_Sized_Checked(                                                                                       \
    leftString,                                                                                                       \
    neostr_IsValid(leftString) ? neostr_GetLength(leftString, leftPayloadType) : neostr_CStringLength(leftString),    \
    rightString,                                                                                                      \
    neostr_IsValid(rightString) ? neostr_GetLength(rightString, rightPayloadType) : neostr_CStringLength(rightString))
#define neostr_Compare(leftString, leftPayloadType, rightString, rightPayloadType)  \
  neostr_Compare_Sized(                                                             \
    leftString,                                                                     \
    neostr_GetLength(leftString, leftPayloadType),                                  \
    rightString,                                                                    \
    neostr_GetLength(rightString, rightPayloadType))

void* malloc(neostr_uInt64 size);
void* memcpy(void* destination, const void* source, neostr_uInt64 size);
neostr_sInt32 memcmp(const void* left, const void* right, neostr_uInt64 count);
void* memset(void* dest, neostr_uByte value, neostr_uInt64 n);

neostr_uInt64 neostr_Hash(void* payloadMemory, neostr_uInt64 sizeOfPayload);

#if defined NEOSTR_V2_IMPLEMENTATION
static const neostr_uInt64 neostr_FNV_Prime = 0x00000100000001B3ULL;
static const neostr_uInt64 neostr_FNV_OffsetBasis = 0xCBF29CE484222325ULL;
neostr_uInt64 neostr_Hash(void* payloadMemory, neostr_uInt64 sizeOfPayload)
{
  neostr_uInt64 hash = neostr_FNV_OffsetBasis;
  neostr_uByte* memory = payloadMemory;
  for (neostr_uInt64 byteIndex = 0; byteIndex < sizeOfPayload; ++byteIndex)
  {
    hash ^= memory[byteIndex];
    hash *= neostr_FNV_Prime;
  }

  return hash;
}

// Max uInt64 value
#define NEOSTR_CSTRING_MAX_LENGTH ~((neostr_uInt64)0)
neostr_uInt64 neostr_CStringLength(const char* cString)
{
  cString = cString ? cString : "\0";
  neostr_uInt64 length;
  for (length = 0; length < NEOSTR_CSTRING_MAX_LENGTH; ++length)
  {
    if (*cString == '\0')
    {
      break;
    }
    cString++;
  }

  return length;
}

neostr_sInt32 neostr_Compare_Sized(const char* left, neostr_uInt64 leftLength, const char* right, neostr_uInt64 rightLength)
{
  neostr_uInt64 smallerLength = leftLength < rightLength ? leftLength : rightLength;

  neostr_sInt32 textCompare = memcmp(left, right, smallerLength);
  if (textCompare == 0 && leftLength != rightLength)
  {
    // one of the strings is longer
    textCompare = leftLength != smallerLength ? left[smallerLength] : -right[smallerLength];    
  }
  return textCompare;
}
#endif // defined NEOSTR_V2_IMPLEMENTATION