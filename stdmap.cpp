#include <stdlib.h>
#include <string>
#include <vector>

clock_t startClock;

#define USE_UNORDERED_MAP

#ifdef USE_UNORDERED_MAP
#include <unordered_map>
std::unordered_map<std::string, std::string> dataMap;
#else
#include <map>
std::map<std::string, std::string> dataMap;
#endif

std::vector<std::pair<std::string, std::string> > data;

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

void ParseData(char *csvText)
{
    char    *textPos;
    char    *comma;
    char    *newline;

    startClock = clock();

    textPos = csvText;
    while (*textPos)
    {
        while (isspace(*textPos))
            textPos++;

        comma = strchr(textPos, ',');
        if (!comma)
            break;

        *comma = 0;
        std::string key(textPos);
        textPos = comma + 1;

        newline = strpbrk(textPos, "\r\n");
        if (!newline)
            break;

        *newline = 0;
        std::string value(textPos);
        textPos = newline + 1;

        data.push_back(std::pair<std::string, std::string>(key, value));
    }

    printf("[%zi] Parsed %zi records.\n", clock() - startClock, data.size());
}

void BuildHashTable()
{
    int buildIter;

    startClock = clock();

    for (buildIter = 0; buildIter < BUILD_COUNT; buildIter++)
    {
        dataMap.clear();
        for (auto i : data)
            dataMap.insert(std::pair<std::string, std::string>(i.first, i.second));
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
        searchIndex = random() % data.size();

        std::string result = dataMap[data[searchIndex].first];
        if (result != data[searchIndex].second)
        {
            fprintf(stderr, "ERROR: Hash table lookup error: %s -> %s != %s at index %d.\n", 
                data[searchIndex].second.c_str(), 
                data[searchIndex].second.c_str(), 
                result.c_str(), searchIndex);
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
