#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/mman.h>

int initial = 1;
char s[32] = "111\n";

struct block
{
    size_t size;
    int free;
    struct block *prev;
    struct block *next;
};

void *rawMemory;
struct block *memory;

void endOfTest()
{
    size_t maxFreeChunkSize = 0;
    struct block *Block = (void *)memory;
    while (Block != NULL)
    {
        if (Block->free == 1 && Block->size > maxFreeChunkSize)
            maxFreeChunkSize = Block->size;
        Block = Block->next;
    }
    char str[32] = "Max Free Chunk Size = ";
    write(1, str, strlen(str));
    char ctn[32];
    sprintf(ctn, "%zu\n", maxFreeChunkSize);
    write(1, ctn, strlen(ctn));
    munmap((void *)memory, 20000);
}

void *malloc(size_t size)
{
    if (initial == 1)
    {
        memory = (struct block *)mmap(NULL, 20000, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
        memory->size = 20000 - 32;
        memory->free = 1;
        memory->prev = NULL;
        memory->next = NULL;
        initial = 0;
    }
    if (size == 0)
    {
        endOfTest();
        return NULL;
    }
    if (size % 32 != 0)
        size += (32 - (size % 32));
    struct block *Block = (void *)memory;
    while (Block != NULL)
    {
        if (Block->size >= size && Block->free == 1)
        {
            if (Block->size > size)
            {
                struct block *remainBlock;
                remainBlock = (void *)Block + 32 + size;
                remainBlock->size = (Block->size) - size - 32;
                Block->size = size;
                remainBlock->free = 1;
                Block->free = 0;
                remainBlock->prev = Block;
                remainBlock->next = Block->next;
                Block->next = remainBlock;
            }
            else
                Block->free = 0;
            break;
        }
        Block = Block->next;
    }
    return (void *)Block + 32;
}

void free(void *ptr)
{
    struct block *freeBlock = ptr - 32;
    struct block *prev = freeBlock->prev;
    struct block *next = freeBlock->next;
    freeBlock->free = 1;
    if (next != NULL && next->free == 1)
    {
        freeBlock->size = freeBlock->size + 32 + next->size;
        freeBlock->next = next->next;
    }
    if (prev != NULL && prev->free == 1)
    {
        prev->size = prev->size + 32 + freeBlock->size;
        prev->next = freeBlock->next;
    }
}