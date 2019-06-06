/******************************************************************************
cs2123p2Driver.c by Larry Clark
Copyright 2019 Larry Clark, this code may not be copied to any other website  
Purpose:
    This program reads queries, converts them from infix to postfix, and 
    executes the queries. It uses a stack for converting from infix to postfix
    and the execution of the postfix expression.
Command Parameters:
    p2 -c customerFile -q queryFile
Input:
    Customer File:
        The standard input file stream contains two types of records (terminated
        by EOF).  CUSTOMER records are followed by zero to many TRAIT records 
        (terminated by either EOF or another CUSTOMER record).
        CUSTOMER szCustomerId szName
          8s           6s        20s (but the name may have blanks)
        TRAIT    szTraitType  szTraitValue
          8s          10s        12s
     Query File:   
        Input file stream contains queries (one per input text line).
        Tokens in the query will be separated by one space.
        Some sample data:
            SMOKING = N AND GENDER = F
            SMOKING = N AND ( EXERCISE = HIKE OR EXERCISE = BIKE )
            GENDER = F AND EXERCISE NEVER YOGA
            SMOKING = N AND EXERCISE = HIKE OR EXERCISE = BIKE
            ( BOOK = SCIFI )
            ( ( ( BOOK ONLY SCIFI ) ) )
Results:
    Print the customers and their traits.
    For each query: 
        - print the query
        - print postfix representation of the query
        - print a list (IDs and names) of customers 
          satisfying the query
Returns:
    0 - normal
    900 - command line argument syntax error
    999 - algorithm error
    
Notes:
    1. This program only allows for MAX_CUSTOMER customers.
    2. A customer may have multiple occurrences of the same trait type.  For example,
       he/she may have multiple EXERCISE traits because he/she enjoys HIKE, 
       BIKE, and TENNIS.
    3. The number of traits for a customer must not exceed MAX_TRAITS.
    4. This program uses an array to implement the stack.  It has a maximum of
       MAX_STACK_ELEM elements. 
    5. This program uses an Out array for the resulting postfix expression.
       It has a maximum of MAX_OUT_ITEM elements.
    6. On the command line, specifying p2 -? will provide the usage information.  
       In some unix shells, you will have to type p2 -\?
*******************************************************************************/
// If compiling using visual studio, tell the compiler not to give its warnings
// about the safety of scanf and printf
#define _CRT_SECURE_NO_WARNINGS 1

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include "cs2123p2.h"

// the following structure is used by the categorize function to categorize 
// tokens
static struct
{
    char szSymbol[MAX_TOKEN + 1];
    int iCategory;
    int iPrecedence;
} symbolDefM[] =
{
    "(",        CAT_LPAREN,   0
    , ")",      CAT_RPAREN,   0
    , "=",      CAT_OPERATOR, 2
    , "NEVER",  CAT_OPERATOR, 2
    , "ONLY",   CAT_OPERATOR, 2
    , "AND",    CAT_OPERATOR, 1
    , "OR",     CAT_OPERATOR, 1
    , "", 0, 0
};

// Stack implementation using arrays.  You are not required to document these.
void push(Stack stack, Element value)
{
    if (stack->iCount >= MAX_STACK_ELEM)
        errExit("Attempt to PUSH more than %d values on the array stack"
        , MAX_STACK_ELEM);
    else
    {
        stack->stackElementM[stack->iCount] = value;
        stack->iCount++;
    }
}
Element pop(Stack stack)
{
    if (isEmpty(stack))
        errExit("Attempt to POP an empty array stack");
    else
    {
        stack->iCount--;
        return stack->stackElementM[stack->iCount];
    }
}
Element topElement(Stack stack)
{
    if (isEmpty(stack))
        errExit("Attempt to examine topElement of an empty array stack");
    else
        return stack->stackElementM[stack->iCount-1];    // return the top
}
int isEmpty(Stack stack)
{
    return stack->iCount <= 0;
}
Stack newStack()
{
    Stack stack = (Stack) malloc(sizeof(StackImp));
    stack->iCount = 0;
    return stack;
}
void freeStack(Stack stack)
{
    free (stack);
}


// File pointers for the customer file and the query file
FILE *pFileCustomer;    // Used with the -c Customer File
FILE *pFileQuery;       // Used with the -q Query File

// Main program for the driver

int main(int argc, char *argv[])
{
    Customer customerM[MAX_CUSTOMERS];  // customer array
    int iNumberOfCustomers = 0;         // number of customers in customerM
    char *pszCustomerFileNm = NULL;     // Pointer to an argv[] for customer file name
    char *pszQueryFileNm = NULL;        // Pointer to an argv[] for query file name

    // get the file names from the command argument switches
    processCommandSwitches(argc, argv, &pszCustomerFileNm, &pszQueryFileNm);

    // Open the Customer File if a file name was provided
    
    if (pszCustomerFileNm == NULL)
        exitUsage(USAGE_ERR, "Missing switch ", "-c");

    pFileCustomer = fopen(pszCustomerFileNm, "r");
    if (pFileCustomer == NULL)
        exitUsage(USAGE_ERR, "Invalid customer file name, found "
            , pszCustomerFileNm);

    // Open the Query file if a file name was provided

    if (pszQueryFileNm == NULL)
        exitUsage(USAGE_ERR, "Missing switch ", "-q");

    pFileQuery = fopen(pszQueryFileNm, "r");
    if (pFileQuery == NULL)
        exitUsage(USAGE_ERR, "Invalid query file name, found "
            , pszQueryFileNm);

    // get and print the customer data including traits
    getCustomerData(customerM, &iNumberOfCustomers);

    printCustomerData(customerM, iNumberOfCustomers);

    // Read and process the queries
    readAndProcessQueries(customerM, iNumberOfCustomers);

}
/******************** readAndProcessQueries **************************************
   void readAndProcessQueries(Customer customerM[], int iNumberOfCustomers)
Purpose:
    Reads queries from the Query File, converts them to postfix (via 
    convertToPostfix), evaluates the postfix (via evaluatePostfix), and
    shows the customers that satisfied the queries (via printQueryResult).   
Parameters:
    i Customer customerM[]    array of customers and traits   
    i int iNumCustomer        number of customers in customerM 
Notes:
    - References the global:  pFileQuery    
**************************************************************************/
void readAndProcessQueries(Customer customerM[], int iNumberOfCustomers)
{
    Out out = malloc(sizeof(OutImp));     // postfix form of a query

    // array (which corresponds to customerM via subscript) of booleans 
    // showing which customers satisfied a query
    QueryResult queryResultM[MAX_CUSTOMERS];

    char szInputBuffer[MAX_LINE_SIZE];    // entire input line
    int rc;                               // return code from convertToPostfix
    int iQueryCnt = 1;                    // 
    
    // read text lines containing queries until EOF
    while (fgets(szInputBuffer, MAX_LINE_SIZE, pFileQuery) != NULL)
    {
        printf("Query # %d: %s", iQueryCnt, szInputBuffer);
        out->iOutCount = 0;                             // reset out to empty
        memset(queryResultM, 0, sizeof(queryResultM));  // reset query result

        // Convert query from infix to postfix and check the rc for success
        rc = convertToPostfix(szInputBuffer, out);
        switch (rc)
        {
        case 0:   // Conversion was successful
            printOut(out);
            evaluatePostfix(out, customerM, iNumberOfCustomers, queryResultM);
            printQueryResult(customerM, iNumberOfCustomers, queryResultM);
            break;
        case WARN_MISSING_LPAREN:
            printf("\tWarning: missing left parenthesis\n");
            break;
        case WARN_MISSING_RPAREN:
            printf("\tWarning: missing right parenthesis\n");
            break;
        case WARN_MISSING_OPERATOR:
            printf("\tWarning: missing operator\n");
            break;
        case WARN_MISSING_OPERAND:
            printf("\tWarning: missing operand\n");
            break;    
        default:
            printf("\t warning = %d\n", rc);
        }
        iQueryCnt++;
    }
    free(out);
    printf("\n");
}

/******************** getCustomerData **************************************
void getCustomerData(Customer customerM[], int *piNumCustomer)
Purpose:
    Gets customer data and their corresponding traits (type and values).  
Parameters:
    O Customer customerM[]      an array of customers to be returned  
    O int *piNumCustomer        the number of customers is returned via
                                this pointer 
Notes:
    - This function attempts to give warnings for minor errors found in 
      reading customer data (e.g., bad command, bad format of data); 
      however, some problems cause termination (e.g., too many 
      traits for a customer).
    - It reads a customer file using the global pFileCustomer
        Contains two types of records (terminated
        by EOF).  CUSTOMER records are followed by zero to many TRAIT records 
        (terminated by either EOF or another CUSTOMER record).
        CUSTOMER szCustomerId szName
          8s           6s        20s (but the name may have blanks)
        TRAIT    szTraitType  szTraitValue
          8s          10s        12s
**************************************************************************/
void getCustomerData(Customer customerM[], int *piNumCustomer)
{
    char szInputBuffer[MAX_LINE_SIZE + 1];  // input buffer for fgets

    int iNumTrait = 0;                      // Number of traits for the current customer
    char szRecordType[11];                  // record type of either CUSTOMER or TRAIT
    int i = -1;                             // current customer subscript. -1 indicates 
                                            // not on a customer yet
    int iScanfCnt;                          // scanf returns the number of successful inputs
    char *pszRemainingTxt;                  // After grabbing a token, this is the next
                                            // position.  This will be after the delimiter
                                            // unless the string was terminated by a zero
                                            // byte, then it will be on the zero byte.

    // print a simple header to show that we are printing the input
    printf("Input data:\n");

    // read data input lines of text until EOF.  fgets returns NULL at EOF
    while (fgets(szInputBuffer, MAX_LINE_SIZE, pFileCustomer) != NULL)
    {
        // if the line is just a line feed, skip it.
        if (szInputBuffer[0] == '\n')
            continue;
    
        // print the input buffer as is (it also has a linefeed)
        printf("%s", szInputBuffer);

        // get the CUSTOMER or TRAIT command
        pszRemainingTxt = getToken(szInputBuffer, szRecordType, sizeof(szRecordType)-1);

        // see if getting a customer or a trait
        if (strcmp(szRecordType, "CUSTOMER") == 0)
        {
            i++;
            // see if we have too many customers to fit in the array
            if (i >= MAX_CUSTOMERS)
                errExit("Bad Input - Invalid input file, max customers is %d"
                    , MAX_CUSTOMERS);

            iNumTrait = 0;   // since we have a new customer, reset his/her number of traits
            customerM[i].iNumberOfTraits = iNumTrait;
            iScanfCnt = sscanf(pszRemainingTxt, "%6s %20[^\n]\n"
                , customerM[i].szCustomerId
                , customerM[i].szCustomerName);
            
            // Check for bad input.  scanf returns the number of valid conversions
            if (iScanfCnt < 2)
            {
                printf("\tWarning: Expected ID and name, received %d successful values\n"
                    , iScanfCnt);
                continue;
            }
        }
        else if (strcmp(szRecordType, "TRAIT") == 0)
        {
            // what if we haven't received a CUSTOMER record yet
            if (i < 0)
                errExit("Bad Input - TRAIT record without CUSTOMER");

            // what if we have too many traits 
            if (iNumTrait >= MAX_TRAITS)
                errExit("Bad Input - Too many traits for Customer ID %s, only %d traits allowed"
                    , customerM[i].szCustomerId
                    , MAX_TRAITS);

            iScanfCnt = sscanf(pszRemainingTxt, "%10s %12s"
                , customerM[i].traitM[iNumTrait].szTraitType
                , customerM[i].traitM[iNumTrait].szTraitValue);

            // Check for bad input.  scanf returns the number of valid conversions
            if (iScanfCnt < 2)
            {
                printf("\tWarning: Expected trait type and value, received %d successful values\n"
                    , iScanfCnt);
                continue;
            }
            iNumTrait++;
            customerM[i].iNumberOfTraits = iNumTrait;
        }
        else
        {
            printf("\tWarning: Bad Command in input, found '%s'\n"
                , szRecordType);
            continue;
        }
    }
    *piNumCustomer = i + 1;
}

/******************** printQueryResult **************************************
void printQueryResult(Customer customerM[], int iNumCustomer, QueryResult resultM[])
Purpose:
    Prints the customers which have a corresponding element in the resultM array
    turned ON.   
Parameters:
    i Customer customerM[]    array of customers and traits   
    i int iNumCustomer        number of customers in customerM 
    i QueryResult resultM[]   array (which corresponds to customerM bia subscript)
                              of booleans showing which customers satisfied a query
Notes:
    
**************************************************************************/
void printQueryResult(Customer customerM[], int iNumCustomer, QueryResult resultM[])
{
    int i;
    printf("\tQuery Result:\n");
    printf("\t%-6s  %-20s\n", "ID", "Customer Name");
    // Loop through each customer
    for (i = 0; i < iNumCustomer; i++)
    {
        // Print customers having a corresponding result boolean which is TRUE
        if (resultM[i])
            printf("\t%-6s  %-20s\n", customerM[i].szCustomerId
                , customerM[i].szCustomerName);
    }
}
/******************** never **************************************
int never(Customer *pCustomer, Trait *pTrait)
Purpose:
    Determines whether a customer has a particular trait (type and
    value).  If he doesn't, never returns TRUE; otherwise, it 
    returns FALSE.
Parameters:
    I Customer pCustomer     One customer structure which also
                             contains traits of that customer.
    I Trait    pTrait        The trait that we don't want this 
                             customer to have.
Notes:
    This function could be used by the function atLeast().
Return value:
    TRUE - customer didn't have the specified trait
    FALSE - customer did have it
**************************************************************************/
int never(Customer *pCustomer, Trait *pTrait)
{
    int i;
    if (pCustomer == NULL)
        errExit("received a NULL pointer");
    // Look at each of the traits for a customer
    for (i = 0; i < (pCustomer->iNumberOfTraits); i++)
    {
        // If the specified trait and trait value exist, never would be FALSE
        if (strcmp(pCustomer->traitM[i].szTraitType, pTrait->szTraitType) == 0
            && strcmp(pCustomer->traitM[i].szTraitValue, pTrait->szTraitValue) == 0)
            return FALSE;
    }
    return TRUE;
}

/******************** addOut **************************************
void addOut(Out out, Element element)
Purpose:
    Adds an element to out.  
Parameters:
    I/O Out out                 Stores the postfix expression 
    I Element element           Element structure to be added to out. 
Notes:
    - Since out uses an array, addOut does a boundary check for overflow. 
**************************************************************************/
void addOut(Out out, Element element)
{
    if (out->iOutCount >= MAX_OUT_ITEM)
        errExit("Overflow in the out array");
    out->outM[out->iOutCount++] = element;
}

/******************** printOut **************************************
void printOut(Out out)
Purpose:
    prints the contents of the out array to stdout 
Parameters:
    I Out out                 The postfx expression  
Notes:
    - Prints 12 tokens from out per line
**************************************************************************/
void printOut(Out out)
{
    int i;
    printf("\t");
    // loop through each element in the out array
    for (i = 0; i < out->iOutCount; i++)
    {
        printf("%s ", out->outM[i].szToken);
        if ((i + 1) % 12 == 0)
            printf("\n\t");
    }
    printf("\n");
}

/******************** categorize **************************************
void categorize(Element *pElement)
Purpose:
    Categorizes a token providing its precedence (0 is low, higher 
    integers are a higher precedence) and category (operator, operand,
    left paren, right paren).  Since the category is an integer, it can
    be used in a switch statement.
Parameters:
    I/O Element *pElement       pointer to an element structure which
                                will be modified by this function
Returns:
    n/a 
Notes:
    - The input parameter, pElement must point to an existing Element
      structure.  Its attribute, szToken, must already be populated.
    - This function will populate the iCategory and
      iPrecedence values in the Element structure referenced by pElement.
    - Uses the symbolDefM array to help categorize tokens 
**************************************************************************/
void categorize(Element *pElement)
{
    int i;
    // loop through the symbolDefM array until an empty symbol value is found
    // marking the end of the symbolDefM array
    for (i = 0; symbolDefM[i].szSymbol[0] != '\0'; i++) 
    {
        // does the element's token match the symbol in the symbolDefM array?
        if (strcmp(pElement->szToken, symbolDefM[i].szSymbol) == 0)
        {   // matched, so use its precedence and category
            pElement->iPrecedence = symbolDefM[i].iPrecedence;
            pElement->iCategory = symbolDefM[i].iCategory;
            return;
        }
    }
    // must be an operand
    pElement->iPrecedence = 0;
    pElement->iCategory = CAT_OPERAND;
}

/******************** processCommandSwitches *****************************
    void processCommandSwitches(int argc, char *argv[], char **ppszCustomerFileName
        , char **ppszQueryFileName)
Purpose:
    Checks the syntax of command line arguments and returns the filenames.  
    If any switches are unknown, it exits with an error.
Parameters:
    I   int argc                        count of command line arguments
    I   char *argv[]                    array of command line arguments
    O   char **ppszCustomerFileName     Customer File Name to return
    O   char **ppszQueryFileName        Query File Name to return 
Notes:
    If a -? switch is passed, the usage is printed and the program exits
    with USAGE_ONLY.
    If a syntax error is encountered (e.g., unknown switch), the program
    prints a message to stderr and exits with ERR_COMMAND_LINE_SYNTAX.
**************************************************************************/
void processCommandSwitches(int argc, char *argv[], char **ppszCustomerFileName
    , char **ppszQueryFileName)
{
    int i;
    // Examine each of the command arguments other than the name of the program.
    for (i = 1; i < argc; i++)
    {
        // check for a switch
        if (argv[i][0] != '-')
            exitUsage(i, "expected switch in command line, found ", argv[i]);
        // determine which switch it is
        switch (argv[i][1])
        {
        case 'c':                   // Customer File Name
            if (++i >= argc)
                exitUsage(i, "-c switch needs a file name", argv[i - 1]);
            // check for too long of a file name
            else
                *ppszCustomerFileName = argv[i];
            break;
        case 'q':                   // Query File Name
            if (++i >= argc)
                exitUsage(i, "-q switch needs a file name", argv[i - 1]);
            else
                *ppszQueryFileName = argv[i];
            break;
        case '?':
            exitUsage(USAGE_ONLY, "", "");
            break;
        default:
            exitUsage(i, "expected switch in command line, found ", argv[i]);
        }
    }
}


/******************** exitUsage *****************************
    void exitUsage(int iArg, char *pszMessage, char *pszDiagnosticInfo)
Purpose:
    In general, this routine optionally prints error messages and diagnostics.
    It also prints usage information.

    If this is an argument error (iArg >= 0), it prints a formatted message 
    showing which argument was in error, the specified message, and
    supplemental diagnostic information.  It also shows the usage. It exits 
    with ERR_COMMAND_LINE.

    If this is a usage error (but not specific to the argument), it prints 
    the specific message and its supplemental diagnostic information.  It 
    also shows the usage and exist with ERR_COMMAND_LINE. 

    If this is just asking for usage (iArg will be -1), the usage is shown.
    It exits with USAGE_ONLY.
Parameters:
    I int iArg                      command argument subscript or control:
                                    > 0 - command argument subscript
                                    0 - USAGE_ONLY - show usage only
                                    -1 - USAGE_ERR - show message and usage
    I char *pszMessage              error message to print
    I char *pszDiagnosticInfo       supplemental diagnostic information
Notes:
    This routine causes the program to exit.
**************************************************************************/
void exitUsage(int iArg, char *pszMessage, char *pszDiagnosticInfo)
{
    switch (iArg)
    {
        case USAGE_ERR:
            fprintf(stderr, "Error: %s %s\n"
                , pszMessage
                , pszDiagnosticInfo);
            break;
        case USAGE_ONLY:
            break;
        default:
            fprintf(stderr, "Error: bad argument #%d.  %s %s\n"
                , iArg
                , pszMessage
                , pszDiagnosticInfo);
    }
    // print the usage information for any type of command line error
    fprintf(stderr, "p2 -c customerFileName -q queryFileName\n");
    if (iArg == USAGE_ONLY)
        exit(USAGE_ONLY); 
    else 
        exit(ERR_COMMAND_LINE);
}

/******************** getToken **************************************
char * getToken (char *pszInputTxt, char szToken[], int iTokenSize)
Purpose:
    Examines the input text to return the next token.  It also 
    returns the position in the text after that token.
Parameters:
    I   char *pszInputTxt       input buffer to be parsed
    O   char szToken[]          Returned token.
    I   int iTokenSize          The size of the token variable.  This is used
                                to prevent overwriting memory.  The size 
                                should be the memory size minus 1 (for
                                the zero byte).
Returns:
    Functionally: 
         Pointer to the next character following the delimiter after the token.  
         NULL - no token found.
    szToken parm - the returned token.  If not found, it will be an 
         empty string.
Notes:
    - If the token is larger than the szToken parm, we return a truncated value.
    - If a token isn't found, szToken is set to an empty string
    - This function does not skip over white space occurring prior to the token.
    - If the input buffer pointer is NULL, the function terminates with
      an algorithm error.
**************************************************************************/
char * getToken (char *pszInputTxt, char szToken[], int iTokenSize)
{
    int iDelimPos;                      // found position of delim
    int iCopy;                          // number of characters to copy
    char szDelims[20] = " \n\r";        // delimiters
    szToken[0] = '\0';

    // check for NULL pointer 
    if (pszInputTxt == NULL)
        errExit("getToken passed a NULL pointer"); 

    // Check for no token if at zero byte
    if (*pszInputTxt == '\0')
        return NULL;

    // get the position of the first delim
    iDelimPos = strcspn(pszInputTxt, szDelims);
    
    // See if the token has no characters before delim
    if (iDelimPos == 0)
        return NULL;

    // see if we have more characters than target token, if so, trunc
    if (iDelimPos > iTokenSize)
        iCopy = iTokenSize;             // truncated size
    else
        iCopy = iDelimPos;

    // copy the token into the target token variable
    memcpy(szToken, pszInputTxt, iCopy);
    szToken[iCopy] = '\0';              // null terminate

    // advance the position
    pszInputTxt += iDelimPos;
    if (*pszInputTxt == '\0')
        return pszInputTxt;
    else
        return pszInputTxt + 1;
}
/******************** errExit **************************************
    void errExit(char szFmt[], ... )
Purpose:
    Prints an error message defined by the printf-like szFmt and the
    corresponding arguments to that function.  The number of 
    arguments after szFmt varies dependent on the format codes in
    szFmt.  
    It also exits the program, returning ERR_EXIT.
Parameters:
    I   char szFmt[]            This contains the message to be printed
                                and format codes (just like printf) for 
                                values that we want to print.
    I   ...                     A variable-number of additional arguments
                                which correspond to what is needed
                                by the format codes in szFmt. 
Returns:
    Returns a program exit return code:  the value of ERR_EXIT.
Notes:
    - Prints "ERROR: " followed by the formatted error message specified 
      in szFmt. 
    - Prints the file path and file name of the program having the error.
      This is the file that contains this routine.
    - Requires including <stdarg.h>
**************************************************************************/
void errExit(char szFmt[], ... )
{
    va_list args;               // This is the standard C variable argument list type
    va_start(args, szFmt);      // This tells the compiler where the variable arguments
                                // begins.  They begin after szFmt.
    printf("ERROR: ");
    vprintf(szFmt, args);       // vprintf receives a printf format string and  a
                                // va_list argument
    va_end(args);               // let the C environment know we are finished with the
                                // va_list argument
    printf("\n\tEncountered in file %s\n", __FILE__);  // this 2nd arg is filled in by
                                // the pre-compiler
    exit(ERR_EXIT);
}

