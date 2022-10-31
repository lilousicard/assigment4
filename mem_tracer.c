#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>


/**
 * TRACE_NODE_STRUCT is a linked list of
 * pointers to function identifiers
 * TRACE_TOP is the head of the list is the top of the stack
 * Please note that an output to the file memtrace.out is written any time
 * a TRACE_NODE_STRUCT is allocated, reallocated, or free
**/
struct TRACE_NODE_STRUCT {
    char *functionid;                // ptr to function identifier (a function name)
    struct TRACE_NODE_STRUCT *next;  // ptr to next frama
};
typedef struct TRACE_NODE_STRUCT TRACE_NODE;
static TRACE_NODE *TRACE_TOP = NULL;       // ptr to the top of the stack



/* ---------------------------------------------- */
/* function PRINT_TRACE prints out the sequence of function calls that are on the stack at this instance */
/* For example, it returns a string that looks like: global:funcA:funcB:funcC. */
/* Printing the function call sequence the other way around is also ok: funcC:funcB:funcA:global */
char *PRINT_TRACE() {
    int depth = 50; //A max of 50 levels in the stack will be combined in a string for printing out.
    int i, length, j;
    TRACE_NODE *tnode;
    static char buf[100];

    if (TRACE_TOP == NULL) {     // stack not initialized yet, so we are
        strcpy(buf, "global");   // still in the 'global' area
        return buf;
    }

    /* peek at the depth(50) top entries on the stack, but do not
       go over 100 chars and do not go over the bottom of the
       stack */

    sprintf(buf, "%s", TRACE_TOP->functionid);
    length = strlen(buf);                  // length of the string so far
    for (i = 1, tnode = TRACE_TOP->next;
         tnode != NULL && i < depth;
         i++, tnode = tnode->next) {
        j = strlen(tnode->functionid);             // length of what we want to add
        if (length + j + 1 < 100) {              // total length is ok
            sprintf(buf + length, ":%s", tnode->functionid);
            length += j + 1;
        } else                                // it would be too long
            break;
    }
    return buf;
} /*end PRINT_TRACE*/

// -----------------------------------------
// function REALLOC calls realloc and print the following line
// "File tracemem.c, line X, function F reallocated the memory segment at address A to a new size S"
// Information about the function F should be printed by printing the stack (use PRINT_TRACE)
void *REALLOC(void *p, int t, char *file, int line) {
    printf("File %s, line %d, function %s reallocated the memory segament at address %p to a new size %d\n", file, line,
           PRINT_TRACE(), p, t);
    p = realloc(p, t);
    return p;
}

// -------------------------------------------
// function MALLOC calls malloc and print the following line
// "File tracemem.c, line X, function F allocated new memory segment at address A to size S"
// Information about the function F should be printed by printing the stack (use PRINT_TRACE)
void *MALLOC(int t, char *file, int line) {
    void *p;
    p = malloc(t);
    printf("File %s, line %d, function %s allocated new memory segment at address %p to of size %d\n", file, line,
           PRINT_TRACE(), p, t);
    return p;
}

// ----------------------------------------------
// function FREE calls free and print the following line
// "File tracemem.c, line X, function F deallocated the memory segment at address A"
// Information about the function F should be printed by printing the stack (use PRINT_TRACE)
void FREE(void *p, char *file, int line) {
    printf("File %s, line %d, function %s deallocated the memory segment at address %p \n", file, line, PRINT_TRACE(),
           p);
    free(p);
}

#define realloc(a, b) REALLOC(a,b,__FILE__,__LINE__)
#define malloc(a) MALLOC(a,__FILE__,__LINE__)
#define free(a) FREE(a,__FILE__,__LINE__)




/* --------------------------------*/
/* function PUSH_TRACE */
/* 
 * The purpose of this stack is to trace the sequence of function calls,
 * just like the stack in your computer would do. 
 * The "global" string denotes the start of the function call trace.
 * The char *p parameter is the name of the new function that is added to the call trace.
 * See the examples of calling PUSH_TRACE and POP_TRACE below
 * in the main, make_extend_array, add_column functions.
**/
void PUSH_TRACE(char *p)          // push p on the stack
{
    TRACE_NODE *tnode;
    static char glob[] = "global";

    if (TRACE_TOP == NULL) {

        // initialize the stack with "global" identifier
        TRACE_TOP = (TRACE_NODE *) malloc(sizeof(TRACE_NODE));

        // no recovery needed if allocation failed, this is only
        // used in debugging, not in production
        if (TRACE_TOP == NULL) {
            printf("PUSH_TRACE: memory allocation error\n");
            exit(1);
        }

        TRACE_TOP->functionid = glob;
        TRACE_TOP->next = NULL;
    }//if

    // create the node for p
    tnode = (TRACE_NODE *) malloc(sizeof(TRACE_NODE));

    // no recovery needed if allocation failed, this is only
    // used in debugging, not in production
    if (tnode == NULL) {
        printf("PUSH_TRACE: memory allocation error\n");
        exit(1);
    }//if

    tnode->functionid = p;
    tnode->next = TRACE_TOP;  // insert fnode as the first in the list
    TRACE_TOP = tnode;          // point TRACE_TOP to the first node

}/*end PUSH_TRACE*/

/* --------------------------------*/
/* function POP_TRACE */
/* Pop a function call from the stack */
void POP_TRACE()    // remove the op of the stack
{
    TRACE_NODE *tnode;
    tnode = TRACE_TOP;
    TRACE_TOP = tnode->next;
    free(tnode);

}/*end POP_TRACE*/





// -----------------------------------------
// function add_column will add an extra column to a 2d array of ints.
// This function is intended to demonstrate how memory usage tracing of realloc is done
// Returns the number of new columns (updated)
int add_column(int **array, int rows, int columns) {
    PUSH_TRACE("add_column");
    int i;

    for (i = 0; i < rows; i++) {
        array[i] = (int *) realloc(array[i], sizeof(int) * (columns + 1));
        array[i][columns] = 10 * i + columns;
    }//for
    POP_TRACE();
    return (columns + 1);
}// end add_column

/**
 * STRING_NODE_STRUCT is a linked list of
 * pointers to an input string
 * and the index of that input
 * LL_TOP is the head of the list and the top of the stack
**/

struct STRING_NODE {
    char *string;
    int index;
    struct STRING_NODE *next;
};
static struct STRING_NODE *LL_TOP = NULL;       // ptr to the top of the stack

/* --------------------------------*/
/* function PUSH_STRING */
/**
 * The function PUSH_STRING is use to create a new STRING_NODE
 * That will become the top of the stack. It malloc the size
 * of a node in a memory.
 * @param string The user input to be store in the Node
 * @param index The index of the input
 */
void PUSH_STRING(char *string, int index) {
    PUSH_TRACE("PUSH_STRING");
    struct STRING_NODE *tnode;
    tnode = (struct STRING_NODE *) malloc(sizeof(struct STRING_NODE));
    tnode->string = string;
    tnode->index = index;
    if (LL_TOP == NULL) {
        LL_TOP = tnode;
        LL_TOP->next = NULL;
    } else {
        tnode->next = LL_TOP;
        LL_TOP = tnode;
    }
    POP_TRACE();
}
/* ------------------------------*/
/* function POP_STRING */
/**
 * The function POP_STRING is used to remove the
 * STRING_NODE at the top of the string.
 * The removed node is freed and the node
 * that followed it becomes the top of the stack
 */
void POP_STRING() {
    PUSH_TRACE("POP_STRING");
    struct STRING_NODE *tnode;
    tnode = LL_TOP;
    LL_TOP = tnode->next;
    free(tnode);
    POP_TRACE();
}

/* --------------------------------*/
/* function FREE_LL_STRING */
/**
 * FREE_LL_STRING is a function that will free the entire
 * Linked List of STRING_NODE by calling POP_STRING in a
 * loop while the pointer to the top of the Linked list
 * is not null.
 */

void FREE_LL_STRING() {
    PUSH_TRACE("FREE_LL_STRING");
    while (LL_TOP != NULL) {
        POP_STRING();
    }
    POP_TRACE();
}

/* --------------------------------*/
/* function Print_STRING */
/**
 * The function print_String is use to recursively print the Linked List
 * of STRING_NODE.
 * @param node The STRING_NODE parameter is used to traverse the Linked List
 */
void print_String(struct STRING_NODE *node) {
    PUSH_TRACE("print_String");
    if (node->next != NULL) {
        print_String(node->next);
    }
    printf("array[%d] = %s\n", node->index, node->string);
    POP_TRACE();
}

/**
 * The FREE_STRING_IN_ARRAY function free all the char* inside the
 * main char** one by one.
 * @param array the char** containing the char* that needs to be freed
 * @param index the number of char* inside the char**
 */
void FREE_STRING_IN_ARRAY(char **array, int index) {
    PUSH_TRACE("FREE_STRING_IN_ARRAY");
    for (int i = 0; i < index; i++) {
        free(array[i]);
    }
    POP_TRACE();
}

// ------------------------------------------
// function make_extend_array
//Please read: On Discord, Channel a4,on 10/14/2022 at 13:33, ProfB Mentioned 
//"Else, you can assume commands will have a 100 character limit (they wont be tested on more than 100 chars). 
// The bash shell cmd line also has a limit to the length of the commands it can read"
//This is why size_T buff is set to 100
/**
 * The make_extend_array is a function that takes the user input
 * and store them in a dynamically allocated array
 * The array start with a size of 10 char* and the
 * size gets increase by 10 everytime that we reach
 * the limit previously set (see the ROW variable)
 */
void make_extend_array() {
    PUSH_TRACE("make_extend_array");
    size_t buff = 100;//See the comment above
    char **array = NULL; //The array
    int ROW = 10; // The number of row in the array
    int index = 0;// The number of input in the array
    char *input = (char *) malloc(sizeof(char) * buff); // The char* use to read the user input
    array = (char **) malloc(sizeof(char *) * ROW); // Allocate the space for the array


    //Read all the input available to store them
    while (getline(&input, &buff, stdin) != -1) {
        input[strlen(input) - 1] = 0;//Set \n to 0
        array[index] = strdup(input);//copy the string
        printf("File mem_tracer.c, line %d, function %s allocated new memory segment at address %p to of size %lu\n",
               __LINE__ - 1, PRINT_TRACE(), array[index], sizeof(char) * buff); //print to the file

        PUSH_STRING(array[index], index);//Push the string in the linked list
        index++;//increase the number of string stored by 1

        //if the space allocated to store all the string is insufficient
        //we need to reallocate the memory
        if (index >= ROW) {
            ROW += 10;
            array = realloc(array, sizeof(char *) * ROW);
        }
    }//End of While loop

    print_String(LL_TOP);//Print the linked list recursively
    FREE_LL_STRING();//Free the linked list
    FREE_STRING_IN_ARRAY(array, index);//Free all Char* in array

    free(array);//Free the char**
    free(input);//Free the char* used for user input
    POP_TRACE();
    return;
}//end make_extend_array


// ----------------------------------------------
// function main
int main() {
    int fd;
    fd = open("memtrace.out", O_RDWR | O_CREAT | O_TRUNC | O_APPEND, 0777);
    dup2(fd, 1);
    PUSH_TRACE("main");

    make_extend_array();

    POP_TRACE();
    return (0);
}// end main





