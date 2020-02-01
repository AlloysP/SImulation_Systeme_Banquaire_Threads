/**
 * Authors: PETIT Alloys, JOSSE Raphaël
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// Code pris sur le site http://www.kaushikbaruah.com/posts/data-structure-in-c-hashmap/

struct node
{
    long int key;
    long int val;
    struct node *next;
};
struct table
{
    int size;
    struct node **list;
};

struct table *createTable(int size);
void insert(struct table *t, long int key, long int val);
long int lookup(struct table *t, long int key);
