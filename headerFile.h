/**********************************************************************
cs2123p2.h
Copyright 2019 Larry Clark, this code may not be copied to any other website  
Purpose:
   Defines constants: 
       max constants 
       error constants
       warning constants
       categories of tokens (operator, operand, etc.)
       boolean constants
   Defines typedef for 
       Token
       Element  (values placed in stack or out)
       StackImp (array stack implementation)
       Stack    (pointer to a StackImp)
       OutImp   (out implementation)
       Out      (pointer to an OutImp)
       Trait    (customer's trait type and trait value)
       Customer (customer id, name, and array of Trait entries)
       QueryResult this is simply an int.  When true, the customer is included
                 in the result.
   Prototypes
       Functions provided by student
       Stack functions provided by Larry
       Other functions provided by Larry
       Utility functions providd by Larry
Notes:
   
**********************************************************************/
/*** constants ***/
// Maximum constants
#define MAX_STACK_ELEM 20       // Maximum number of elements in the stack array
#define MAX_TOKEN 50            // Maximum number of actual characters for a token
#define MAX_OUT_ITEM 50         // Maximum number of Out items
#define MAX_CUSTOMERS 30        // Maximum number of customers
#define MAX_TRAITS 12           // Maximum number of traits per customer       
#define MAX_LINE_SIZE 100       // Maximum number of character per input line

// Error constants (program exit values)
#define ERR_EXIT            999  // Unexpected error
#define ERR_COMMAND_LINE    900  // invalid command line argument

// exitUsage control 
#define USAGE_ONLY          0      // user only requested usage information
#define USAGE_ERR           -1     // usage error, show message and usage information

// Warning constants.  Warnings do not cause the program to exit.
#define WARN_MISSING_RPAREN   801
#define WARN_MISSING_LPAREN   802
#define WARN_MISSING_OPERATOR 803
#define WARN_MISSING_OPERAND  804

// categories of tokens
#define CAT_LPAREN 1
#define CAT_RPAREN 2
#define CAT_OPERATOR 3      // Operators are =, NOTANY, ONLY, AND, OR
#define CAT_OPERAND 4       // These are trait types and trait values

// boolean constants
#define FALSE 0
#define TRUE 1

/*** typedef ***/

// Token typedef used for operators, operands, and parentheses
typedef char Token[MAX_TOKEN + 1];

// Element typedef used for Element values placed in the stack or out
// The evaluation stack can have two types of data.
typedef union 
{
    struct 
    {
        Token szToken;      // token from the query
        int iCategory;      // category CAT_LPAREN, CAT_RPAREN, CAT_OPERATOR, CAT_OPERAND
        int iPrecedence;    // operator precedence with 0 being lowest
    };
    struct 
    {
        char szBoolean[MAX_TOKEN]; // This is to aid in debugging
        int bInclude;       // TRUE - intermediate include result, FALSE - intermediate don't include
    };
} Element;

// StackImp typedef defines how we implement a stack using an array
typedef struct
{
    int iCount;  // number of elements in stack.  0 is empty 
    Element stackElementM[MAX_STACK_ELEM];
} StackImp;

// Stack typedef defines a pointer to a stack
typedef StackImp *Stack;

// OutImp typedef defines how we implement out
typedef struct
{
    int iOutCount;
    Element outM[MAX_OUT_ITEM];
} OutImp;

// Out typedef defines a pointer to out
typedef OutImp *Out;

/* Trait typedef defines a trait type (e.g., GENDER) and
** corresponding trait value (e.g., M, F)
*/
typedef struct
{
    char szTraitType[11];        // Trait Type can be one of:
    //    GENDER
    //    SMOKING
    //    MOVIE
    //    BOOK
    //    EXERCISE 
    char szTraitValue[13];       // This corresponds to the Trait Type:
    //    GENDER:   M, F
    //    SMOKING:  Y, N
    //    MOVIE:    ROMANCE, COMEDY, ACTION, FAMILY, HORROR
    //    BOOK:     SCIFI, DRAMA, MYSTERY, HORROR, TRAVEL, ROMANCE, 
    //              COOKING
    //    EXERCISE: TENNIS, GOLF, JOG, RUN, YOGA, DANCE, HIKE, BIKE 
} Trait;

/* Customer typedef contains customer Id, customer name, and an array of traits */
typedef struct
{
    char szCustomerId[7];               // Customer Identifier
    char szCustomerName[21];            // Customer Full Name
    int  iNumberOfTraits;               // The number of traits for each customer
    Trait traitM[MAX_TRAITS];
} Customer;

typedef int QueryResult;                // This is simply TRUE or FALSE
                                        // We use this to create an array that
                                        // corresponds to the customerM array.

/**********   prototypes ***********/

// functions that each student must implement
// (Note that the student might implement several others for modularity.)

// your code from program #0
void printCustomerData(Customer customerM[], int iNumCustomer);

// your code from program #1 (and other functions for modularity)
int convertToPostFix(char *pszInfix, Out out);

// your code for program #2
void evaluatePostfix(Out out, Customer customerM[], int iNumCustomer, QueryResult resultM[]);
int atLeastOne(Customer *pCustomer, Trait *pTrait);
int only(Customer *pCustomer, Trait *pTrait);

// Stack functions that Larry provided

void push(Stack stack, Element value);
Element pop(Stack stack);
int isEmpty(Stack stack);
Stack newStack();
void freeStack(Stack stack);
Element topElement(Stack stack);

// Other functions that Larry provided
void categorize(Element *pElement);
void addOut(Out out, Element element);
void printOut(Out out);
void printQueryResult(Customer customerM[], int iNumCustomer, QueryResult resultM[]);
int never(Customer *pCustomer, Trait *pTrait);
void getCustomerData(Customer customerM[], int *piNumCustomer);
void readAndProcessQueries(Customer customerM[], int iNumberOfCustomers);

// functions in most programs, but require modifications
void processCommandSwitches(int argc, char *argv[], char **ppszCustomerFileName
    , char **ppszQueryFileName);
void exitUsage(int iArg, char *pszMessage, char *pszDiagnosticInfo);

// Utility routines provided by Larry
void errExit(char szFmt[], ...);
char * getToken(char *pszInputTxt, char szToken[], int iTokenSize);

