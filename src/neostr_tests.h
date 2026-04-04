#define ASSERT(expression) if(!(expression)) { *(int *)0 = 0;}
void* malloc(size_t size);
void free(void* ptr);
int printf(const char* format, ...);
void neostr_RunTests()
{
  // All of the string types manually formatted:
  #define TEST_STRING "Hello World"
  #define TEST_STRING_LENGTH (sizeof(TEST_STRING) - 1)
  #define TEST_STRING_END_INDEX (TEST_STRING_LENGTH - 1)

  // nullptr
  const char* nulldata = NULL;

  // c string
  const char* emptyCString = "";


  const char* cString = TEST_STRING;

  // fixed
  neostr_fixed_metadata fixedMetadata = {TEST_STRING_LENGTH, NEOSTR_TYPE_FIXED, NEOSTR_ID};
  char* fixedMemory = malloc(sizeof(neostr_fixed_metadata) + TEST_STRING_LENGTH + 1);
  memcpy(fixedMemory, &fixedMetadata, sizeof(neostr_fixed_metadata));
  memcpy(fixedMemory + sizeof(neostr_fixed_metadata), cString, TEST_STRING_LENGTH);
  memset(fixedMemory + sizeof(neostr_fixed_metadata) + TEST_STRING_LENGTH, 0, 1);
  const char* fixedString = (char*)(fixedMemory + sizeof(neostr_fixed_metadata));

  // string view
  neostr_view stringView = {(char*)fixedString, 0, TEST_STRING_LENGTH};

  // fixed literal
  static const struct                                                 
  {                                                                   
    neostr_metadata meta;                                             
    const char data[sizeof(TEST_STRING)];                          
  } fixedLiteral_obj = { { sizeof(TEST_STRING) - 1, NEOSTR_TYPE_FIXED, NEOSTR_ID }, TEST_STRING };
  const char* fixedLiteral = fixedLiteral_obj.data;  

  // dynamic
  neostr_dynamic_metadata dynamicMetadata = {TEST_STRING_LENGTH,{TEST_STRING_LENGTH, NEOSTR_TYPE_DYNAMIC, NEOSTR_ID}};
  char* dynamicMemory = malloc(sizeof(neostr_dynamic_metadata) + TEST_STRING_LENGTH + 1);
  memcpy(dynamicMemory, &dynamicMetadata, sizeof(neostr_dynamic_metadata));
  memcpy(dynamicMemory + sizeof(neostr_dynamic_metadata), cString, TEST_STRING_LENGTH);
  memset(dynamicMemory + sizeof(neostr_dynamic_metadata) + dynamicMetadata.capacity, 0, 1);
  char* dynamicString = (char*)(dynamicMemory + sizeof(neostr_dynamic_metadata));

  // virtual
  neostr_virtual_metadata virtualMetadata = {TEST_STRING_LENGTH, TEST_STRING_LENGTH, {TEST_STRING_LENGTH, NEOSTR_TYPE_VIRTUAL, NEOSTR_ID}};
  char* virtualMemory = malloc(sizeof(neostr_virtual_metadata) + TEST_STRING_LENGTH + 1);
  memcpy(virtualMemory, &virtualMetadata, sizeof(neostr_virtual_metadata));
  memcpy(virtualMemory + sizeof(neostr_virtual_metadata), cString, TEST_STRING_LENGTH);
  memset(virtualMemory + sizeof(neostr_virtual_metadata) + virtualMetadata.reserved, 0, 1);
  char* virtualString = (char*)(virtualMemory + sizeof(neostr_virtual_metadata));

  // neostr_CStringLength
  {
    ASSERT(neostr_CStringLength(nulldata) == 0);
    ASSERT(neostr_CStringLength(cString) == TEST_STRING_LENGTH);
    ASSERT(neostr_CStringLength(neostr_view_Data_Unchecked(&stringView)) == TEST_STRING_LENGTH);
    ASSERT(neostr_CStringLength(fixedString) == TEST_STRING_LENGTH);
    ASSERT(neostr_CStringLength(fixedLiteral) == TEST_STRING_LENGTH);
    ASSERT(neostr_CStringLength(dynamicString) == TEST_STRING_LENGTH);
    ASSERT(neostr_CStringLength(virtualString) == TEST_STRING_LENGTH);
  }

  // neostr_IsValid
  {
    ASSERT(!neostr_IsValid(nulldata));
    ASSERT(!neostr_IsValid(cString));
    ASSERT(neostr_IsValid(neostr_view_Data_Unchecked(&stringView)));
    ASSERT(neostr_IsValid(fixedString));
    ASSERT(neostr_IsValid(fixedLiteral));
    ASSERT(neostr_IsValid(dynamicString));
    ASSERT(neostr_IsValid(virtualString));
  }

  // neostr_Length
  {
    ASSERT(neostr_Length(nulldata) == 0);
    ASSERT(neostr_Length(cString) == TEST_STRING_LENGTH);
    ASSERT(neostr_Length(neostr_view_Data_Unchecked(&stringView)) == TEST_STRING_LENGTH);
    ASSERT(neostr_Length(fixedString) == TEST_STRING_LENGTH);
    ASSERT(neostr_Length(fixedLiteral) == TEST_STRING_LENGTH);
    ASSERT(neostr_Length(dynamicString) == TEST_STRING_LENGTH);
    ASSERT(neostr_Length(virtualString) == TEST_STRING_LENGTH);
  }

  // neostr_IsMetadataValid
  {
    ASSERT(!neostr_IsMetadataValid(neostr_ConstMetadataAddress(nulldata)));
    ASSERT(!neostr_IsMetadataValid(neostr_ConstMetadataAddress(cString)));
    ASSERT(neostr_IsMetadataValid(neostr_ConstMetadataAddress(neostr_view_Data_Unchecked(&stringView))));
    ASSERT(neostr_IsMetadataValid(neostr_ConstMetadataAddress(fixedString)));
    ASSERT(neostr_IsMetadataValid(neostr_ConstMetadataAddress(fixedLiteral)));
    ASSERT(neostr_IsMetadataValid(neostr_ConstMetadataAddress(dynamicString)));
    ASSERT(neostr_IsMetadataValid(neostr_ConstMetadataAddress(virtualString)));
  }

  // neostr_Type
  {
    ASSERT(neostr_Type(nulldata) == NEOSTR_TYPE_INVALID);
    ASSERT(neostr_Type(cString) == NEOSTR_TYPE_RAW);
    ASSERT(neostr_Type(neostr_view_Data_Unchecked(&stringView)) > NEOSTR_TYPE_RAW);
    ASSERT(neostr_Type(fixedString) == NEOSTR_TYPE_FIXED);
    ASSERT(neostr_Type(fixedLiteral) == NEOSTR_TYPE_FIXED);
    ASSERT(neostr_Type(dynamicString) == NEOSTR_TYPE_DYNAMIC);
    ASSERT(neostr_Type(virtualString) == NEOSTR_TYPE_VIRTUAL);
  }

  // neostr_ConstMetadataAddress
  {
    ASSERT(neostr_ConstMetadataAddress(nulldata) == &neostr_empty.meta);
    ASSERT(neostr_ConstMetadataAddress(cString) == &neostr_empty.meta);
    ASSERT(neostr_ConstMetadataAddress(neostr_view_Data_Unchecked(&stringView)) != &neostr_empty.meta);
    ASSERT(neostr_ConstMetadataAddress(fixedString) == (const neostr_metadata*)fixedString - 1);
    ASSERT(neostr_ConstMetadataAddress(fixedLiteral) == (const neostr_metadata*)fixedLiteral - 1);
    ASSERT(neostr_ConstMetadataAddress(dynamicString) == (const neostr_metadata*)dynamicString - 1);
    ASSERT(neostr_ConstMetadataAddress(virtualString) == (const neostr_metadata*)virtualString - 1);
  }

  // neostr_MetadataAddress
  {
    ASSERT(neostr_MetadataAddress((char*)nulldata) == &neostr_empty.meta);
    ASSERT(neostr_MetadataAddress((char*)cString) == &neostr_empty.meta);
    ASSERT(neostr_MetadataAddress(neostr_view_Data_Unchecked(&stringView)) != &neostr_empty.meta);
    ASSERT(neostr_MetadataAddress((char*)fixedString) == (neostr_metadata*)fixedString - 1);
    ASSERT(neostr_MetadataAddress((char*)fixedLiteral) == (neostr_metadata*)fixedLiteral - 1);
    ASSERT(neostr_MetadataAddress(dynamicString) == (neostr_metadata*)dynamicString - 1);
    ASSERT(neostr_MetadataAddress(virtualString) == (neostr_metadata*)virtualString - 1);
  }

  // neostr_Metadata
  {
    #define NEOSTR_COMPARE_METADATA(left, right) (left.length == right.length && left.type == right.type && left.id == right.id)  

    ASSERT(NEOSTR_COMPARE_METADATA(neostr_Metadata(nulldata), neostr_empty.meta));
    ASSERT(NEOSTR_COMPARE_METADATA(neostr_Metadata(cString), neostr_empty.meta));
    ASSERT(!NEOSTR_COMPARE_METADATA(neostr_Metadata(neostr_view_Data_Unchecked(&stringView)), neostr_empty.meta));
    
    neostr_metadata tempFixedMetadata = {TEST_STRING_LENGTH, NEOSTR_TYPE_FIXED, NEOSTR_ID};
    ASSERT(NEOSTR_COMPARE_METADATA(neostr_Metadata(fixedString), tempFixedMetadata));
    ASSERT(NEOSTR_COMPARE_METADATA(neostr_Metadata(fixedLiteral), tempFixedMetadata));

    neostr_metadata tempDynamicMetadata = {TEST_STRING_LENGTH, NEOSTR_TYPE_DYNAMIC, NEOSTR_ID};
    ASSERT(NEOSTR_COMPARE_METADATA(neostr_Metadata(dynamicString), tempDynamicMetadata));

    neostr_metadata tempVirtualMetadata = {TEST_STRING_LENGTH, NEOSTR_TYPE_VIRTUAL, NEOSTR_ID};
    ASSERT(NEOSTR_COMPARE_METADATA(neostr_Metadata(virtualString), tempVirtualMetadata));
  }

  // neostr_Compare
  {
    ASSERT(neostr_Compare(nulldata, TEST_STRING) != 0);
    ASSERT(neostr_Compare(cString, TEST_STRING) == 0);
    ASSERT(neostr_Compare(neostr_view_Data_Unchecked(&stringView), TEST_STRING) == 0);
    ASSERT(neostr_Compare(fixedString, TEST_STRING) == 0);
    ASSERT(neostr_Compare(fixedLiteral, TEST_STRING) == 0);
    ASSERT(neostr_Compare(dynamicString, TEST_STRING) == 0);
    ASSERT(neostr_Compare(virtualString, TEST_STRING) == 0);

    ASSERT(neostr_Compare("a", "b") == -1);
    ASSERT(neostr_Compare("b", "a") == 1);
    ASSERT(neostr_Compare("a", "a") == 0);
    ASSERT(neostr_Compare(emptyCString, nulldata) == 0);
    ASSERT(neostr_Compare("abc", "ab") == 'c');
    ASSERT(neostr_Compare("ab", "abc") == -'c');
  }

  // neostr_view_IsValid
  {
    ASSERT(!neostr_view_IsValid(NULL));
    ASSERT(neostr_view_IsValid(&stringView));
  }

  // neostr_view_Length
  {
    ASSERT(neostr_view_Length(NULL) == 0);
    ASSERT(neostr_view_Length(&stringView) == TEST_STRING_LENGTH);
  }

  // neostr_view_LayoutData
  {
#define NEOSTR_VIEW_COMPARE(left, right) (left.baseStringAddress == right.baseStringAddress &&  \
                                          left.startOffset == right.startOffset &&              \
                                          left.length == right.length)                    
    ASSERT(NEOSTR_VIEW_COMPARE(neostr_view_LayoutData((char*)nulldata, 0, TEST_STRING_LENGTH), neostr_view_empty));
    ASSERT(NEOSTR_VIEW_COMPARE(neostr_view_LayoutData((char*)cString, 0, TEST_STRING_LENGTH), neostr_view_empty));
    ASSERT(NEOSTR_VIEW_COMPARE(neostr_view_LayoutData(neostr_view_Data_Unchecked(&stringView), 0, TEST_STRING_LENGTH), stringView));
    
    const char* oldAddr = stringView.baseStringAddress;
    stringView.baseStringAddress = (char*)fixedString;
    ASSERT(NEOSTR_VIEW_COMPARE(neostr_view_LayoutData((char*)fixedString, 0, TEST_STRING_LENGTH), stringView));
    
    stringView.baseStringAddress = (char*)fixedLiteral;
    ASSERT(NEOSTR_VIEW_COMPARE(neostr_view_LayoutData((char*)fixedLiteral, 0, TEST_STRING_LENGTH), stringView));
    
    stringView.baseStringAddress = dynamicString;
    ASSERT(NEOSTR_VIEW_COMPARE(neostr_view_LayoutData(dynamicString, 0, TEST_STRING_LENGTH), stringView));
    
    stringView.baseStringAddress = virtualString; 
    ASSERT(NEOSTR_VIEW_COMPARE(neostr_view_LayoutData(virtualString, 0, TEST_STRING_LENGTH), stringView));

    stringView.baseStringAddress = (char*)oldAddr;
  }

  // neostr_fixed_IsValid
  {
    ASSERT(!neostr_fixed_IsValid(nulldata));
    ASSERT(!neostr_fixed_IsValid(cString));
    ASSERT(neostr_fixed_IsValid(neostr_view_Data_Unchecked(&stringView)));
    ASSERT(neostr_fixed_IsValid(fixedString));
    ASSERT(neostr_fixed_IsValid(fixedLiteral));
    ASSERT(!neostr_fixed_IsValid(dynamicString));
    ASSERT(!neostr_fixed_IsValid(virtualString));
  }

  // neostr_fixed_IsMetadataValid
  {
    ASSERT(!neostr_fixed_IsMetadataValid(neostr_fixed_ConstMetadataAddress(nulldata)));
    ASSERT(!neostr_fixed_IsMetadataValid(neostr_fixed_ConstMetadataAddress(cString)));
    ASSERT(neostr_fixed_IsMetadataValid(neostr_fixed_ConstMetadataAddress(neostr_view_Data_Unchecked(&stringView))));
    ASSERT(neostr_fixed_IsMetadataValid(neostr_fixed_ConstMetadataAddress(fixedString)));
    ASSERT(neostr_fixed_IsMetadataValid(neostr_fixed_ConstMetadataAddress(fixedLiteral)));
    ASSERT(!neostr_fixed_IsMetadataValid(neostr_fixed_ConstMetadataAddress(dynamicString)));
    ASSERT(!neostr_fixed_IsMetadataValid(neostr_fixed_ConstMetadataAddress(virtualString)));
  }

  // neostr_fixed_CreateMetadata
  {
    ASSERT(NEOSTR_COMPARE_METADATA(neostr_fixed_CreateMetadata(TEST_STRING_LENGTH), fixedMetadata));
  }

  // neostr_fixed_ConstMetadataAddress
  {
    ASSERT(neostr_fixed_ConstMetadataAddress(nulldata) == &neostr_empty.meta);
    ASSERT(neostr_fixed_ConstMetadataAddress(cString) == &neostr_empty.meta);
    ASSERT(neostr_fixed_ConstMetadataAddress(neostr_view_Data_Unchecked(&stringView)) == 
                                              ((const neostr_fixed_metadata*)neostr_view_Data_Unchecked(&stringView)) - 1);
    ASSERT(neostr_fixed_ConstMetadataAddress(fixedString) == (const neostr_fixed_metadata*)fixedString - 1);
    ASSERT(neostr_fixed_ConstMetadataAddress(fixedLiteral) == (const neostr_fixed_metadata*)fixedLiteral - 1);
    ASSERT(neostr_fixed_ConstMetadataAddress(dynamicString) == &neostr_empty.meta);
    ASSERT(neostr_fixed_ConstMetadataAddress(virtualString) == &neostr_empty.meta);
  }

  // neostr_fixed_MetadataAddress
  {
    ASSERT(neostr_fixed_MetadataAddress((char*)nulldata) == &neostr_empty.meta);
    ASSERT(neostr_fixed_MetadataAddress((char*)cString) == &neostr_empty.meta);
    ASSERT(neostr_fixed_MetadataAddress(neostr_view_Data_Unchecked(&stringView)) == 
                                          ((neostr_fixed_metadata*)neostr_view_Data_Unchecked(&stringView)) - 1);
    ASSERT(neostr_fixed_MetadataAddress((char*)fixedString) == (neostr_fixed_metadata*)fixedString - 1);
    ASSERT(neostr_fixed_MetadataAddress((char*)fixedLiteral) == (neostr_fixed_metadata*)fixedLiteral - 1);
    ASSERT(neostr_fixed_MetadataAddress(dynamicString) == &neostr_empty.meta);
    ASSERT(neostr_fixed_MetadataAddress(virtualString) == &neostr_empty.meta);
  }

  // neostr_fixed_Metadata
  {  
    ASSERT(NEOSTR_COMPARE_METADATA(neostr_fixed_Metadata(nulldata), neostr_empty.meta));
    ASSERT(NEOSTR_COMPARE_METADATA(neostr_fixed_Metadata(cString), neostr_empty.meta));
    
    ASSERT(NEOSTR_COMPARE_METADATA(neostr_fixed_Metadata(neostr_view_Data_Unchecked(&stringView)), fixedMetadata));
    ASSERT(NEOSTR_COMPARE_METADATA(neostr_fixed_Metadata(fixedString), fixedMetadata));
    ASSERT(NEOSTR_COMPARE_METADATA(neostr_fixed_Metadata(fixedLiteral), fixedLiteral_obj.meta));
    ASSERT(NEOSTR_COMPARE_METADATA(neostr_fixed_Metadata(dynamicString), neostr_empty.meta));
    ASSERT(NEOSTR_COMPARE_METADATA(neostr_fixed_Metadata(virtualString), neostr_empty.meta));
  }

  // neostr_fixed_RequiredMemory
  {
    ASSERT(neostr_fixed_RequiredMemory(NULL) == 0);
    ASSERT(neostr_fixed_RequiredMemory(neostr_fixed_ConstMetadataAddress(cString)) == 0);
    ASSERT(neostr_fixed_RequiredMemory(&fixedMetadata) == sizeof(neostr_fixed_metadata) + TEST_STRING_LENGTH + 1);
  }

  // neostr_fixed_LayoutData
  {
    void* memoryBlock = malloc(neostr_fixed_RequiredMemory(&fixedMetadata));
    ASSERT(neostr_fixed_LayoutData(NULL, fixedString, memoryBlock) == &neostr_empty.data);
    ASSERT(neostr_fixed_LayoutData(neostr_fixed_ConstMetadataAddress(cString), fixedString, memoryBlock) == &neostr_empty.data);
    ASSERT(neostr_fixed_LayoutData(&fixedMetadata, NULL, memoryBlock) == &neostr_empty.data);
    ASSERT(neostr_fixed_LayoutData(&fixedMetadata, fixedString, NULL) == &neostr_empty.data);

    const char* woahThatsTooLong = "Hello World and Goodnight Moon";
    ASSERT(neostr_fixed_LayoutData(&fixedMetadata, woahThatsTooLong, memoryBlock) == &neostr_empty.data);
    
    const char* tempFixedString = neostr_fixed_LayoutData(&fixedMetadata, cString, memoryBlock);
    ASSERT(neostr_fixed_IsValid(tempFixedString));
    ASSERT(neostr_Length(tempFixedString) == TEST_STRING_LENGTH);
    ASSERT(neostr_Compare(tempFixedString, "Hello World") == 0);

    memset(memoryBlock, 0, neostr_fixed_RequiredMemory(neostr_fixed_ConstMetadataAddress_Unchecked(tempFixedString)));
    tempFixedString = neostr_fixed_LayoutData(&fixedMetadata, fixedString, memoryBlock);
    ASSERT(neostr_fixed_IsValid(tempFixedString));
    ASSERT(neostr_Length(tempFixedString) == TEST_STRING_LENGTH);
    ASSERT(neostr_Compare(tempFixedString, "Hello World") == 0);
    
    memset(memoryBlock, 0, neostr_fixed_RequiredMemory(neostr_fixed_ConstMetadataAddress_Unchecked(tempFixedString)));
    tempFixedString = neostr_fixed_LayoutData(&fixedMetadata, fixedLiteral, memoryBlock);
    ASSERT(neostr_fixed_IsValid(tempFixedString));
    ASSERT(neostr_Length(tempFixedString) == TEST_STRING_LENGTH);
    ASSERT(neostr_Compare(tempFixedString, "Hello World") == 0);

    memset(memoryBlock, 0, neostr_fixed_RequiredMemory(neostr_fixed_ConstMetadataAddress_Unchecked(tempFixedString)));
    tempFixedString = neostr_fixed_LayoutData(&fixedMetadata, dynamicString, memoryBlock);
    ASSERT(neostr_fixed_IsValid(tempFixedString));
    ASSERT(neostr_Length(tempFixedString) == TEST_STRING_LENGTH);
    ASSERT(neostr_Compare(tempFixedString, "Hello World") == 0);
    
    tempFixedString = NULL;
    tempFixedString = neostr_fixed_LayoutData(&fixedMetadata, virtualString, memoryBlock);
    ASSERT(neostr_fixed_IsValid(tempFixedString));
    ASSERT(neostr_Length(tempFixedString) == TEST_STRING_LENGTH);
    ASSERT(neostr_Compare(tempFixedString, "Hello World") == 0);
    free(memoryBlock);
  }

  // neostr_fixed_LayoutViewData
  {
    void* memoryBlock = malloc(neostr_fixed_RequiredMemory(&fixedMetadata));
    ASSERT(neostr_fixed_LayoutViewData(NULL, &stringView, memoryBlock) == &neostr_empty.data);
    ASSERT(neostr_fixed_LayoutViewData(neostr_fixed_ConstMetadataAddress(cString), &stringView, memoryBlock) == &neostr_empty.data);
    ASSERT(neostr_fixed_LayoutViewData(&fixedMetadata, NULL, memoryBlock) == &neostr_empty.data);
    ASSERT(neostr_fixed_LayoutViewData(&fixedMetadata, &stringView, NULL) == &neostr_empty.data);

    char* woahThatsTooLong = "Hello World and Goodnight Moon";
    const neostr_view woahTooLongView = neostr_view_LayoutData(woahThatsTooLong, 0, sizeof("Hello World and Goodnight Moon") - 1);
    ASSERT(neostr_fixed_LayoutViewData(&fixedMetadata, &woahTooLongView, memoryBlock) == &neostr_empty.data);
    
    const char* tempFixedString = neostr_fixed_LayoutViewData(&fixedMetadata, &stringView, memoryBlock);
    ASSERT(neostr_fixed_IsValid(tempFixedString));
    ASSERT(neostr_Length(tempFixedString) == TEST_STRING_LENGTH);
    ASSERT(neostr_Compare(tempFixedString, "Hello World") == 0);
    free(memoryBlock);
  }

  // neostr_fixed_Literal
  {
    const char* tempLiteral;
    neostr_fixed_Literal(tempLiteral, "Hello World");
    ASSERT(neostr_fixed_IsValid(tempLiteral));
    ASSERT(neostr_Length(tempLiteral) == TEST_STRING_LENGTH);
    ASSERT(neostr_Compare(tempLiteral, "Hello World") == 0);
  }

  // neostr_dynamic_IsValid
  {
    ASSERT(!neostr_dynamic_IsValid(nulldata));
    ASSERT(!neostr_dynamic_IsValid(cString));
    ASSERT(!neostr_dynamic_IsValid(neostr_view_Data_Unchecked(&stringView)));
    ASSERT(!neostr_dynamic_IsValid(fixedString));
    ASSERT(!neostr_dynamic_IsValid(fixedLiteral));
    ASSERT(neostr_dynamic_IsValid(dynamicString));
    ASSERT(!neostr_dynamic_IsValid(virtualString));
  }

  // neostr_dynamic_IsMetadataValid
  {
    ASSERT(!neostr_dynamic_IsMetadataValid(neostr_dynamic_ConstMetadataAddress(nulldata)));
    ASSERT(!neostr_dynamic_IsMetadataValid(neostr_dynamic_ConstMetadataAddress(cString)));
    ASSERT(!neostr_dynamic_IsMetadataValid(neostr_dynamic_ConstMetadataAddress(neostr_view_Data_Unchecked(&stringView))));
    ASSERT(!neostr_dynamic_IsMetadataValid(neostr_dynamic_ConstMetadataAddress(fixedString)));
    ASSERT(!neostr_dynamic_IsMetadataValid(neostr_dynamic_ConstMetadataAddress(fixedLiteral)));
    ASSERT(neostr_dynamic_IsMetadataValid(neostr_dynamic_ConstMetadataAddress(dynamicString)));
    ASSERT(!neostr_dynamic_IsMetadataValid(neostr_dynamic_ConstMetadataAddress(virtualString)));
  }

  // neostr_dynamic_CreateMetadata
  {
    #define NEOSTR_COMPARE_DYNAMIC_METADATA(left, right) (NEOSTR_COMPARE_METADATA(left.meta, right.meta) && (left.capacity == right.capacity))
    neostr_dynamic_metadata emptyDynamicMeta = {TEST_STRING_LENGTH, {0, NEOSTR_TYPE_DYNAMIC, NEOSTR_ID}}; 
    ASSERT(NEOSTR_COMPARE_DYNAMIC_METADATA(neostr_dynamic_CreateMetadata(TEST_STRING_LENGTH), emptyDynamicMeta));
  }

  // neostr_dynamic_ConstMetadataAddress
  {
    ASSERT(neostr_dynamic_ConstMetadataAddress(nulldata) == &neostr_dynamic_empty.meta);
    ASSERT(neostr_dynamic_ConstMetadataAddress(cString) == &neostr_dynamic_empty.meta);
    ASSERT(neostr_dynamic_ConstMetadataAddress(neostr_view_Data_Unchecked(&stringView)) == &neostr_dynamic_empty.meta);
    ASSERT(neostr_dynamic_ConstMetadataAddress(fixedString) == &neostr_dynamic_empty.meta);
    ASSERT(neostr_dynamic_ConstMetadataAddress(fixedLiteral) == &neostr_dynamic_empty.meta);
    ASSERT(neostr_dynamic_ConstMetadataAddress(dynamicString) == (const neostr_dynamic_metadata*)dynamicString - 1);
    ASSERT(neostr_dynamic_ConstMetadataAddress(virtualString) == &neostr_dynamic_empty.meta);
  }

  // neostr_dynamic_MetadataAddress
  {
    ASSERT(neostr_dynamic_MetadataAddress((char*)nulldata) == &neostr_dynamic_empty.meta);
    ASSERT(neostr_dynamic_MetadataAddress((char*)cString) == &neostr_dynamic_empty.meta);
    ASSERT(neostr_dynamic_MetadataAddress(neostr_view_Data_Unchecked(&stringView)) == &neostr_dynamic_empty.meta);
    ASSERT(neostr_dynamic_MetadataAddress((char*)fixedString) == &neostr_dynamic_empty.meta);
    ASSERT(neostr_dynamic_MetadataAddress((char*)fixedLiteral) == &neostr_dynamic_empty.meta);
    ASSERT(neostr_dynamic_MetadataAddress(dynamicString) == (neostr_dynamic_metadata*)dynamicString - 1);
    ASSERT(neostr_dynamic_MetadataAddress(virtualString) == &neostr_dynamic_empty.meta);
  }

  // neostr_dynamic_Metadata
  {  
    ASSERT(NEOSTR_COMPARE_DYNAMIC_METADATA(neostr_dynamic_Metadata(nulldata), neostr_dynamic_empty.meta));
    ASSERT(NEOSTR_COMPARE_DYNAMIC_METADATA(neostr_dynamic_Metadata(cString), neostr_dynamic_empty.meta));
    ASSERT(NEOSTR_COMPARE_DYNAMIC_METADATA(neostr_dynamic_Metadata(neostr_view_Data_Unchecked(&stringView)), neostr_dynamic_empty.meta));
    ASSERT(NEOSTR_COMPARE_DYNAMIC_METADATA(neostr_dynamic_Metadata(fixedString), neostr_dynamic_empty.meta));
    ASSERT(NEOSTR_COMPARE_DYNAMIC_METADATA(neostr_dynamic_Metadata(fixedLiteral), neostr_dynamic_empty.meta));
    ASSERT(NEOSTR_COMPARE_DYNAMIC_METADATA(neostr_dynamic_Metadata(dynamicString), dynamicMetadata));
    ASSERT(NEOSTR_COMPARE_DYNAMIC_METADATA(neostr_dynamic_Metadata(virtualString), neostr_dynamic_empty.meta));
  }

  // neostr_dynamic_Capacity
  {
    ASSERT(neostr_dynamic_Capacity(nulldata) == 0);
    ASSERT(neostr_dynamic_Capacity(cString) == 0);
    ASSERT(neostr_dynamic_Capacity(neostr_view_Data_Unchecked(&stringView)) == 0);
    ASSERT(neostr_dynamic_Capacity(fixedString) == 0);
    ASSERT(neostr_dynamic_Capacity(fixedLiteral) == 0);
    ASSERT(neostr_dynamic_Capacity(dynamicString) == TEST_STRING_LENGTH);
    ASSERT(neostr_dynamic_Capacity(virtualString) == 0);
  }

  // neostr_dynamic_RequiredMemory
  {
    ASSERT(neostr_dynamic_RequiredMemory(NULL) == 0);
    ASSERT(neostr_dynamic_RequiredMemory(neostr_dynamic_ConstMetadataAddress(cString)) == 0);
    ASSERT(neostr_dynamic_RequiredMemory(&dynamicMetadata) == sizeof(neostr_dynamic_metadata) + TEST_STRING_LENGTH + 1);
  }

  // neostr_dynamic_LayoutData
  {
    neostr_dynamic_metadata tempDynamicMetadata = neostr_dynamic_CreateMetadata(TEST_STRING_LENGTH);
    void* memoryBlock = malloc(neostr_dynamic_RequiredMemory(&tempDynamicMetadata));
    ASSERT(neostr_dynamic_LayoutData(NULL, memoryBlock) == &neostr_dynamic_empty.data);
    ASSERT(neostr_dynamic_LayoutData(neostr_dynamic_ConstMetadataAddress(cString), memoryBlock) == &neostr_dynamic_empty.data);
    ASSERT(neostr_dynamic_LayoutData(&tempDynamicMetadata, NULL) == &neostr_dynamic_empty.data);

    const char* tempDynamicString = neostr_dynamic_LayoutData(&tempDynamicMetadata, memoryBlock);
    ASSERT(neostr_dynamic_IsValid(tempDynamicString));
    ASSERT(neostr_Length(tempDynamicString) == 0);
    ASSERT(neostr_dynamic_Capacity(tempDynamicString) == TEST_STRING_LENGTH);    
    free(memoryBlock);
  }

  // neostr_dynamic_PushData
  {
    neostr_dynamic_metadata prevEmptyMeta = neostr_dynamic_empty.meta;
    char prevEmptyData = neostr_dynamic_empty.data;
    neostr_dynamic_PushData((char*)nulldata, cString);

    // Assert that no data was written to the empty string
    ASSERT(NEOSTR_COMPARE_METADATA(neostr_dynamic_empty.meta.meta, prevEmptyMeta.meta));
    ASSERT(neostr_dynamic_empty.data == prevEmptyData);

    neostr_dynamic_metadata tempDynamicMetadata = neostr_dynamic_CreateMetadata(TEST_STRING_LENGTH);
    void* memoryBlock = malloc(neostr_dynamic_RequiredMemory(&tempDynamicMetadata));
    char* tempDynamicString = neostr_dynamic_LayoutData(&tempDynamicMetadata, memoryBlock);

    neostr_dynamic_PushData(tempDynamicString, nulldata);
    ASSERT(neostr_Length(tempDynamicString) == 0);
    ASSERT(neostr_Compare(tempDynamicString, emptyCString) == 0);

    const char* woahThatsTooLong = "Hello World and Goodnight Moon";
    neostr_dynamic_PushData(tempDynamicString, woahThatsTooLong);
    ASSERT(neostr_Length(tempDynamicString) == 0);
    ASSERT(neostr_Compare(tempDynamicString, emptyCString) == 0);

    neostr_dynamic_PushData(tempDynamicString, cString);
    ASSERT(neostr_Length(tempDynamicString) == TEST_STRING_LENGTH);
    ASSERT(neostr_Compare(tempDynamicString, "Hello World") == 0);

    neostr_dynamic_PopData_Unchecked(neostr_dynamic_MetadataAddress_Unchecked(tempDynamicString), TEST_STRING_LENGTH);
    neostr_dynamic_PushData(tempDynamicString, fixedString);
    ASSERT(neostr_Length(tempDynamicString) == TEST_STRING_LENGTH);
    ASSERT(neostr_Compare(tempDynamicString, "Hello World") == 0);
    
    neostr_dynamic_PopData_Unchecked(neostr_dynamic_MetadataAddress_Unchecked(tempDynamicString), TEST_STRING_LENGTH);
    neostr_dynamic_PushData(tempDynamicString, fixedLiteral);
    ASSERT(neostr_Length(tempDynamicString) == TEST_STRING_LENGTH);
    ASSERT(neostr_Compare(tempDynamicString, "Hello World") == 0);

    neostr_dynamic_PopData_Unchecked(neostr_dynamic_MetadataAddress_Unchecked(tempDynamicString), TEST_STRING_LENGTH);
    neostr_dynamic_PushData(tempDynamicString, dynamicString);
    ASSERT(neostr_Length(tempDynamicString) == TEST_STRING_LENGTH);
    ASSERT(neostr_Compare(tempDynamicString, "Hello World") == 0);

    neostr_dynamic_PopData_Unchecked(neostr_dynamic_MetadataAddress_Unchecked(tempDynamicString), TEST_STRING_LENGTH);
    neostr_dynamic_PushData(tempDynamicString, virtualString);
    ASSERT(neostr_Length(tempDynamicString) == TEST_STRING_LENGTH);
    ASSERT(neostr_Compare(tempDynamicString, "Hello World") == 0);
    free(memoryBlock);
  }

  // TODO: remainder of dynamic functions and virtual functions

  free(fixedMemory);
  free(dynamicMemory);
  free(virtualMemory);
}