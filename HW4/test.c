/*
Student No.: 111652017
Student Name: Hsiu-I, Liao
Email: hsiuyee.sc11@nycu.edu.tw
SE tag: xnxcxtxuxoxsx
Statement: I am fully aware that this program is not
supposed to be posted to a public server, such as a
public GitHub repository or a public web page.
*/

#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdbool.h>

struct block{
    int free;
    size_t size;
    struct block *prev;
    struct block *next;
};

const size_t Block_Size = 32;
const size_t Max_Size = 20000;

struct block *front = NULL;
void *start = 0;

void find_maximum_chunk_size() {
    struct block *temp = front;
    size_t mx = 0;
    while(temp != NULL){
        if(temp->size > mx && temp->free == 1)
            mx = temp->size;
        temp = temp->next;
    }
    
    char mes[50];
    memset(mes, '\0', sizeof(mes));
    snprintf(mes, 50, "Max Free Chunk Size = %ld\n", mx);
    write(1, mes, 50);

    munmap(start, 20000);
}

void init() {
    start = mmap(NULL, Max_Size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    front = start;
    front->free = 1;
    front->size = (Max_Size - Block_Size);
    front->prev = NULL;
    front->next = NULL;
}


void *malloc(size_t size){
    if(size % 32 != 0){
        size = size / 32 * 32 + 32;
    }
    struct block *temp = NULL;
    if(size == 0){
        find_maximum_chunk_size();
        return NULL;
    }

    if(start == NULL) init();

    struct block *best_fit = NULL;
    temp = front;
    while(temp != NULL){
        if((temp->size >= size) && (temp->free == 1)){
            if(best_fit == NULL || (best_fit->size > temp->size)){
                best_fit = temp;
            }
        }
        temp = temp->next;
    }

    temp = best_fit;
    if(best_fit != NULL){
        if(best_fit->size == size){
            best_fit->free = 0;
        }
        else{
            struct block *pos = best_fit;
            pos = (best_fit + 1 + size/32);
            pos->free = 1;
            pos->size = (best_fit->size - 32 - size);
            pos->prev = best_fit;
            pos->next = best_fit->next;
            
            best_fit->free = 0;
            best_fit->size = size;
            if(best_fit->next != NULL)
                (best_fit->next)->prev = pos;
            best_fit->next = pos;
        }
        return (best_fit + 1);
    }
    else{
        return 0;
    }
}

bool check(struct block *temp){
    return temp != NULL && temp->free == 1;
}

void free(void *ptr){
    struct block *temp = ptr;
    temp -= 1;
    temp->free = 1;
    if(check(temp->prev)){
        (temp->prev)->size += (32 + temp->size);
        (temp->prev)->next =  temp->next;
        if(check(temp->next))
            (temp->next)->prev = temp->prev;
    }
    
    if(check(temp->next)){
        temp->size += (32 + (temp->next)->size);
        if(check(temp->next))
            (temp->next->next)->prev = temp;
        temp->next =  temp->next->next;
    }
}