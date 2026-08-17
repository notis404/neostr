// neostr consists of 3 parts, sequential in memory
// [payload][metadata][string data]
// payload: user-supplied, can be a struct of any size
// metadata: contains information about the payload, such as size and the most recent checksums of the payload struct and the metadata struct
// string data: the memory allocated for the string data

// Users hold onto a pointer to the string data so the address appears like a normal string
// Base memory address can be retrieved by retrieving the address to the payload 
// Macros are used to access the members of different payload types by name
// Certain macro functions expect a certain kind of member variable to exist to function correctly (length, capacity, etc)
// Comments above these macro functions will specify which member names are required. 
// The actual text of the member name can be configured with the member name macros (NEOSTR_MEMBERNAME_LENGTH, etc)  

#if !defined NEOSTR_MEMBERNAME_LENGTH
#define NEOSTR_MEMBERNAME_LENGTH length
#endif

#if !defined NEOSTR_MEMBERNAME_CAPACITY
#define NEOSTR_MEMBERNAME_CAPACITY capacity
#endif

// Basic types
typedef unsigned char neostr_uInt8;
typedef neostr_uInt8 neostr_uByte;
typedef neostr_uByte neostr_uBool8; 
typedef unsigned short neostr_uInt16;
typedef neostr_uInt16 neostr_uChar16;
typedef unsigned int neostr_uInt32;
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
  neostr_uInt32 payloadCRC;
  neostr_uInt32 metadataCRC;
}neostr_metadata;
#define NEOSTR_METADATA_WITHOUT_CRC_SIZE sizeof(neostr_metadata) - sizeof(neostr_uInt32)

#pragma section(".rdata$neostr_lit_a", read)
#pragma section(".rdata$neostr_lit_data", read)
#pragma section(".rdata$neostr_lit_z", read)

__declspec(allocate(".rdata$neostr_lit_a")) const char neostr_literal_data_start = 0;
__declspec(allocate(".rdata$neostr_lit_z")) const char neostr_literal_data_end = 0;

extern const char neostr_literal_data_start;
extern const char neostr_literal_data_end;

// Creates a literal neostr in .rdata. CRC is not used to verify validity of literals. Instead, literals are validated by confirming they are 
// in the correct data section
#define neostr_Literal(literalVariableName, cStringLiteral)                                   \
__declspec(allocate(".rdata$neostr_lit_data")) static const struct                                  \
{                                                                                             \
  neostr_fixed_payload64 payload;                                                             \
  neostr_metadata metadata;                                                                   \
  char data[sizeof(cStringLiteral)];                                                          \
} literalVariableName##_memory_##__LINE__ =                                                   \
  {                                                                                           \
    {sizeof(cStringLiteral)},                                                                 \
    {sizeof(neostr_fixed_payload64), 0, 0},                                                   \
    cStringLiteral                                                                            \
  };                                                                                          \
static const char* const literalVariableName = literalVariableName##_memory_##__LINE__.data;

#define neostr_Metadata(string) ((neostr_metadata*)(string - sizeof(neostr_metadata)))
#define neostr_Payload(string, payloadType) ((payloadType*)((neostr_uByte*)neostr_Metadata(string) - neostr_Metadata(string)->payloadSize))
#define neostr_IsValid(string) \
  (string && (neostr_IsMetadataValid(string) && neostr_IsPayloadValid(string)) || neostr_IsLiteral(string))
  
#define neostr_IsPayloadValid(string) \
  (neostr_Metadata(string)->payloadCRC == neostr_CRC32((neostr_uByte*)neostr_Payload(string, void), neostr_Metadata(string)->payloadSize))

#define neostr_IsMetadataValid(string) \
  (neostr_Metadata(string)->metadataCRC == neostr_CRC32((neostr_uByte*)neostr_Metadata(string), NEOSTR_METADATA_WITHOUT_CRC_SIZE))

#define neostr_IsLiteral(string) \
  (string < &neostr_literal_data_end && string > &neostr_literal_data_start)

// NOTE: must reset CRC for payload first because the payload CRC is part of the metadata CRC data stream
#define neostr_ResetCRC(string) \
  (neostr_ResetPayloadCRC(string), neostr_ResetMetadataCRC(string))

#define neostr_ResetMetadataCRC(string) \
  (neostr_Metadata(string)->metadataCRC = neostr_CRC32((neostr_uByte*)neostr_Metadata(string), NEOSTR_METADATA_WITHOUT_CRC_SIZE))

#define neostr_ResetPayloadCRC(string) \
  (neostr_Metadata(string)->payloadCRC = neostr_CRC32((neostr_uByte*)neostr_Payload(string, void), neostr_Metadata(string)->payloadSize))

#define neostr_HeaderSize(payloadType) (sizeof(payloadType) + sizeof(neostr_metadata))
#define neostr_LayoutHeader(stringMemory, payloadType)                                                    \
  (memset(stringMemory, 0, sizeof(payloadType))),                                                         \
  (memset(stringMemory + sizeof(payloadType), 0, sizeof(neostr_metadata))),                               \
  (stringMemory = (void*)((neostr_uByte*)stringMemory + sizeof(payloadType) + sizeof(neostr_metadata))),  \
  (neostr_Metadata(stringMemory)->payloadSize = sizeof(payloadType)),                                     \
  (neostr_ResetCRC(stringMemory))
#define neostr_SetPayloadMember(string, payloadType, memberName, newMemberValue)  \
  ((neostr_Payload(string, payloadType)->memberName = newMemberValue), (neostr_ResetCRC(string)))
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
#define neostr_IncrementCapacity(string, payloadType, incrementSize) \
  neostr_SetPayloadMember(string, payloadType, NEOSTR_MEMBERNAME_CAPACITY, neostr_GetPayloadMember(string, payloadType, NEOSTR_MEMBERNAME_CAPACITY) + incrementSize)
#define neostr_IncrementCapacity_Checked(string, payloadType, incrementSize) \
  neostr_IsValid(string) ? neostr_IncrementCapacity(string, payloadType, incrementSize) : 0

// Requires payload members NEOSTR_MEMBERNAME_LENGTH, NEOSTR_MEMBERNAME_CAPACITY 
#define neostr_Push(string, payloadType, data, dataSize) \
  (neostr_GetLength(string, payloadType) + dataSize <= neostr_GetCapacity(string, payloadType)) ?  \
    (memcpy(string + neostr_GetLength(string, payloadType), data, dataSize)),                     \
    (neostr_Payload(string, payloadType)->NEOSTR_MEMBERNAME_LENGTH += dataSize),                  \
    (neostr_ResetCRC(string)) : 0
#define neostr_Push_Checked(string, payloadType, data, dataSize) \
  neostr_IsValid(string) ? neostr_Push(string, payloadType, data, dataSize) : 0


// Requires payload members NEOSTR_MEMBERNAME_LENGTH
#define neostr_SetString(stringAddress, payloadType, stringData, stringSize)  \
    (memcpy(stringAddress, stringData, stringSize)),                          \
    (neostr_SetLength(stringAddress, neostr_fixed_payload64, stringSize)),    \
    (neostr_ResetCRC(stringAddress))
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
neostr_uInt64 neostr_CStringLength(const char* cString);
neostr_sInt32 neostr_Compare_Sized(const char* left, neostr_uInt64 leftLength, const char* right, neostr_uInt64 rightLength);


#if defined NEOSTR_IMPLEMENTATION
void __cpuid(neostr_sInt32 cpuInfo[4], neostr_sInt32 function_id);
neostr_uInt64 _mm_crc32_u64(neostr_uInt64 crc, neostr_uInt64 data);
neostr_uInt32 _mm_crc32_u8(neostr_uInt32 crc, neostr_uInt8 data);
neostr_uBool8 neostr_SupportsHardwareCRC()
{
  neostr_sInt32 info[4];
  __cpuid(info, 1);
  return (info[2] & (1 << 20)) != 0;
}

neostr_uInt32 neostr_CRC32(neostr_uByte* memory, neostr_uInt64 memorySize)
{
  neostr_uInt32 crc = 0;
  if (neostr_SupportsHardwareCRC())
  {
    for (neostr_uInt64 i = 0; i < memorySize;)
    {
      if (memorySize - i > 7)
      {
        crc = (neostr_uInt32)_mm_crc32_u64(crc, (neostr_uInt64)memory[i]); // NOTE: downcasting to uInt32 is safe because function only returns lower bits
        i += 8;
      }
      else
      {
        crc = _mm_crc32_u8(crc, (neostr_uInt8)memory[i]);
        ++i;
      }
    }
  } 
  else
  {
    // TODO: write a software CRC function
  }

  return crc;
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
#endif // defined NEOSTR_IMPLEMENTATION