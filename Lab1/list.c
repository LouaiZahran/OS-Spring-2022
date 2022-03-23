#include <stddef.h>
#include <string.h>
#include <malloc.h>
#include <stdbool.h>

struct listNode{
    char* name;
    char* value;
    struct listNode* next;
};

struct listNode* createListNode(char* name, char* value){
    struct listNode *node = malloc(sizeof(struct listNode));
    strcpy(node->name, name);
    strcpy(node->value, value);
    node->next = NULL;
    return node;
}

struct list{
    struct listNode* head;
    int size;
};

struct list* createList(){
    struct list* list = malloc(sizeof(struct list));
    list->head = NULL;
    list->size = 0;
    return list;
}

void addVariable(struct list* list, char* name, char* value){
    if(list->head == NULL){
        list->head = createListNode(name, value);
        return;
    }
    struct listNode* pCrawl = list->head;
    while(true){
        if(strcmp(pCrawl->name, name) == 0){
            strcpy(pCrawl->value, value);
            return;
        }
        if(pCrawl->next == NULL)
            break;
        pCrawl = pCrawl->next;
    }
    pCrawl->next = createListNode(name, value);
}

char* getFromList(struct list* list, char* name){
    struct listNode* pCrawl = list->head;
    while(pCrawl != NULL){
        if(strcmp(pCrawl->name, name) == 0)
            return pCrawl->value;
        pCrawl = pCrawl->next;
    }
    return NULL;
}
