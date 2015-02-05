#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <string.h>

clock_t startClock;

#define INVALID_DATA_INDEX  (-1)
typedef int dataindex_t;

#define MAX_DATA            (512*1024)

int dataCount;
char *dataKey[MAX_DATA];
char *dataValue[MAX_DATA];

#define DATA_HASH_SIZE      (2*MAX_DATA)

dataindex_t dataHash[DATA_HASH_SIZE];

#define BUILD_COUNT         10
#define SEARCH_COUNT        100000

#define FNV_32_PRIME ((uint32_t)0x01000193)

uint32_t fnv_32_str(const char *str, uint32_t hval)
{
    unsigned char *s = (unsigned char *)str;
    while (*s) 
    {
        hval *= FNV_32_PRIME;
        hval ^= (uint32_t)*s++;
    }
    return hval;
}

bool InsertDataByName(const char* key, dataindex_t dataIndex) 
{
    uint32_t startSlot = fnv_32_str(key, 0) % DATA_HASH_SIZE;
    uint32_t slot = startSlot;
    for (;;)
    {
        if (dataHash[slot] == INVALID_DATA_INDEX)
        {
            dataHash[slot] = dataIndex;
            return true;
        }

        slot = (slot + 1) % DATA_HASH_SIZE;
    }
}

dataindex_t LookupDataByName(const char* key) 
{
    uint32_t startSlot = fnv_32_str(key, 0) % DATA_HASH_SIZE;
    uint32_t slot = startSlot;
    for (;;)
    {
        dataindex_t dataIndex = dataHash[slot];
        if (dataIndex == INVALID_DATA_INDEX)
            return dataIndex; // not found
        if (strcmp(dataKey[dataIndex], key) == 0)
            return dataIndex;

        slot = (slot + 1) % DATA_HASH_SIZE;
    }
}

void ParseData(char *csvText)
{
    char    *textPos;
    char    *comma;
    char    *newline;

    startClock = clock();

    dataCount = 0;

    textPos = csvText;
    while (*textPos)
    {
        while (isspace(*textPos))
            textPos++;

        comma = strchr(textPos, ',');
        if (!comma)
            break;

        dataKey[dataCount] = textPos;
        *comma = 0;
        textPos = comma + 1;

        newline = strpbrk(textPos, "\r\n");
        if (!newline)
            break;

        dataValue[dataCount] = textPos;
        *newline = 0;
        textPos = newline + 1;

        dataCount++;
        if (dataCount >= MAX_DATA)
        {
            fprintf(stderr, "ERROR: Exceeded maximum of %d data records.\n", MAX_DATA);
            exit(2);
        }
    }

    printf("[%zi] Parsed %zi records.\n", clock() - startClock, dataCount);
}

void BuildHashTable()
{
    int buildIter;
    int hashIndex;
    int dataIndex;

    startClock = clock();

    for (buildIter = 0; buildIter < BUILD_COUNT; buildIter++)
    {
        for (hashIndex = 0; hashIndex < DATA_HASH_SIZE; hashIndex++)
            dataHash[hashIndex] = INVALID_DATA_INDEX;

        for (dataIndex = 0; dataIndex < dataCount; dataIndex++)
            InsertDataByName(dataKey[dataIndex], dataIndex);
    }

    printf("[%zi] Built table %zi times.\n", clock() - startClock, BUILD_COUNT);
}

void SearchHashTable()
{
    int searchIter;
    int searchIndex;
    int dataIndex;

    startClock = clock();

    srandom(123456);

    for (searchIter = 0; searchIter < SEARCH_COUNT; searchIter++)
    {
        searchIndex = random() % dataCount;

        dataIndex = LookupDataByName(dataKey[searchIndex]);
        if (dataIndex != searchIndex)
        {
            fprintf(stderr, "ERROR: Hash table lookup error: %s -> %s != %s at index %d.\n", 
                dataKey[searchIndex], dataValue[searchIndex], dataValue[dataIndex], dataIndex);
            exit(2);
        }
    }

    printf("[%zi] Looked up %zi records.\n", clock() - startClock, SEARCH_COUNT);
}

int main(int argc, const char *argv[])
{
    FILE    *input;
    size_t  inputSize;
    char    *csvText;

    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s csvfile\n", argv[0] );
        exit(1);
    }

    input = fopen(argv[1], "r");
    if (!input)
    {
        perror("ERROR: Unable to open input file");
        exit(2);
    }

    fseek(input, 0, SEEK_END);
    inputSize = ftell(input);
    fseek(input, 0, SEEK_SET);

    csvText = (char *)malloc(inputSize + 1);
    fread(csvText, 1, inputSize, input);
    csvText[inputSize] = 0;

    fclose(input);

    ParseData(csvText);

    BuildHashTable();
    SearchHashTable();

    free(csvText);
}

