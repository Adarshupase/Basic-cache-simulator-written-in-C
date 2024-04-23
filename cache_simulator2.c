#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define CACHE_SIZE 2048 // 2^11 5kB
#define MEM_SIZE 65536   // 2^15 KB
#define BLOCK_SIZE 16    // 16 bytes
#define WORD_SIZE 4
#define NUM_OF_BLOCKS MEM_SIZE / BLOCK_SIZE
#define NUM_OF_LINES CACHE_SIZE / BLOCK_SIZE
#define BITS_BLOCK (int)log2(NUM_OF_BLOCKS)
#define BITS_LINE (int)log2(NUM_OF_LINES)
int cache_hit;
int cache_miss;
int word_count = 0;
int BLOCK_ADDRESSES[NUM_OF_BLOCKS];
int MEMORY_BLOCK[NUM_OF_BLOCKS];
int cache[CACHE_SIZE];
int MEMORY[MEM_SIZE];
int lines = CACHE_SIZE / BLOCK_SIZE;
int tag = BITS_BLOCK + BITS_LINE;
int offset = (int)log2(BLOCK_SIZE / WORD_SIZE);
int MemCurrentAddressPtr = 0;
int CurrentBlockNo = 0;
int evictions = 0;

struct CacheAddressing
{
    int tag;
    int lineNo;
    int offset;
    int word;
    int dirty;
    int loaded ;// Flag to indicate whether the cache block has been modified
};

int CACHE_PRESENT[CACHE_SIZE] = {0}; // Array to track presence of cache blocks
struct CacheAddressing CACHE[CACHE_SIZE];

int decimalToBinary(int n)
{
    int binaryNumber = 0, remainder, i = 1;

    while (n != 0)
    {
        remainder = n % 2;
        n /= 2;
        binaryNumber += remainder * i;
        i *= 10;
    }

    return binaryNumber;
}

void initializeMemory()
{
    for (int i = 0; i < MEM_SIZE; i++)
    {
        MEMORY[i] = 0;
    }
}

void generateBlockAddress()
{
    for (int i = 0; i < NUM_OF_BLOCKS; i++)
    {
        BLOCK_ADDRESSES[i] = i * BLOCK_SIZE;
    }
}

int assignMemory(int *data, int dataSize, int address)
{
    int dataPointer = 0;
    int PrevAddressPtr = address;

    while (dataSize--)
    {
        MEMORY[address++] = data[dataPointer++];
        word_count++;
    }

    CurrentBlockNo = address / BLOCK_SIZE;
    return PrevAddressPtr;
}

void checkMemory(int address)
{
    if (MEMORY[address] != 0)
    {
        printf("Content at %d is %d\n ", address, MEMORY[address]);
    }
    else
    {
        printf("NO MEMORY ASSIGNED");
    }
}

int checkCache(int address)
{
    int LineNo = (address >> offset) & ((1 << BITS_LINE) - 1);
    int TagNo = (address >> (offset + BITS_LINE)) & ((1 << BITS_BLOCK) - 1);
    printf("%d\n",LineNo);
    int CACHE_INDEX = LineNo * BLOCK_SIZE;
    int offset_bits = (int)log2(BLOCK_SIZE); // Calculate the number of offset bits
    int offset_mask = (1 << offset_bits) - 1; // Create a mask to extract the offset bits

    // Calculate the offset using the address
    int offsetCache = address & offset_mask;
    int cache_address_value = (LineNo << offset_bits) | offsetCache;

    // Calculate block number and starting address
    int BLOCK_NO = address / BLOCK_SIZE;
    int BLOCK_STARTING_ADDR = BLOCK_NO * BLOCK_SIZE;

    // Calculate the starting and ending index of the cache lines to check
    int start_index = CACHE_INDEX;
    int end_index = CACHE_INDEX + BLOCK_SIZE;

    for (int i = start_index; i < end_index; i++)
    {
        if (CACHE[i].lineNo == LineNo && CACHE[i].tag == TagNo)
        {
            cache_hit++;
            printf("CACHE HIT\n");
            printf("LineNo %d | Tag %d  | offset %d | dirty \n",CACHE[i].lineNo,CACHE[i].tag,CACHE[i].offset,CACHE[i].dirty);
            printf("Content at %d is %d present at address %d in cache\n", address, CACHE[i].word, i);
            return 1;
        }
    }

    printf("CACHE MISS\n");
   
    cache_miss++;
    
    

    // Assign memory block to cache

    for (int i = CACHE_INDEX; i < end_index; i++)
    {
        printf("LineNo %d | Tag %d  | dirty %d\n ",CACHE[i].lineNo,CACHE[i].tag,CACHE[i].dirty);
        CACHE[i].word = MEMORY[BLOCK_STARTING_ADDR++];
        CACHE[i].tag = TagNo;
        CACHE[i].offset = i - CACHE_INDEX;
        CACHE[i].lineNo = LineNo;
       
        
        if (CACHE[i].loaded >=1)
        {
            // If the cache block has been loaded before, set the dirty bit
            CACHE[i].dirty = 1;
        }
         CACHE[i].loaded = 1;
       
        
        

        if (CACHE[i].dirty)
        {
            // Write back to memory before replacing the cache block
            printf("Writeback");
            printf("LineNo %d | Tag %d  |  dirty %d (eviction)\n ",CACHE[i].lineNo,CACHE[i].tag,CACHE[i].dirty);
            evictions++;
            int memory_address = (CACHE[i].tag << (offset + BITS_LINE)) | (CACHE[i].lineNo << offset);
            MEMORY[memory_address] = CACHE[i].word;
             // Mark the cache block as not present
        }
    }

    // Mark the newly loaded cache block as present
   

    return 0;
}

int main()
{
    for(int i = 0;i<CACHE_SIZE;i++){
        CACHE[i].loaded = 0;
        CACHE[i].dirty = 0;
    }

    initializeMemory();
    generateBlockAddress();

    int memory_addresses[] = {100, 200, 300};
    int data[] = {42, 55, 67};
    int num_data = sizeof(data) / sizeof(data[0]);

    assignMemory(data, num_data, 2000);

    checkCache(26706); //110100001010010
    checkCache(26706);//
    checkCache(22612);//101100001010100
    checkCache(16464);//100000001010000
    checkCache(16470);//100000001010110
 
    
//     100000001010110
// 100000001010111
// 100000001011110
// 100000001011111
// 100000001100110
// 100000001100111
// 100000001101110
// 100000001101111
// 100000001110110
// 100000001110111
// 100000001111110

// 100000001111111

    int totalEvictions = evictions/BLOCK_SIZE;

    printf(" Cache hits %d\n",cache_hit);
    printf(" Cache miss %d\n",cache_miss);
    printf("Evictions: %d\n", totalEvictions);


    return 0;
}
