/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
#define _CRT_SECURE_NO_WARNINGS 1

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include "cs2123p2.h"

/************************checkLeft*****************************************
 int checkLeft(Stack stack, Out out)
 Purpose: 
    This function loops through each line containing parentheses to find the 
 matching left parentheses.
 Parameters:
    I Stack stack             Array of elements to be potentially modified.
    I Out out                 Array of postfix expression.

 Returns:
    TRUE (1)                    If corresponding left paren is found.
    FALSE (0)                   If left paren isn't found.
 
 Notes:
 
 **************************************************************************/

int checkLeft(Stack stack, Out out)
{
    Element popped;
    while(isEmpty(stack)== FALSE) //check to see if stack is empty
    {
        popped = pop(stack); 
        //if popped element is a '(' then return True
        if(strcmp(popped.szToken, "(") == 0)
        {
            return TRUE;
        }
        //if not add it to the out array
        addOut(out, popped);
    }
    return FALSE;
   
    
}

/****************covertToPostfix***************************************
 int convertToPostfix(char *pszInfix, Out, out)
 Purpose:
    Convert infix expressions from a file to postfix expressions.
 Parameters:
        I char *pszInfix          Pointer to each line in the input 
        O Out out                 Stores the postfix expression
 Return Value:
        0                           The function converted successfully
        "CAT_RPAREN" 801            Missing right parentheses
        "CAT_LPAREN" 802            Missing left Parentheses
 Notes:
    This code should free the stack after modifying it
 
 **********************************************************************/

int convertToPostfix(char *pszInfix, Out out)
{
    char szToken[MAX_TOKEN];  //
    // creating a new stack using the new stack function
    Stack stack = newStack();
    Element element;   // variable used to keep track of which token is current
    Element popped;   // temporary variable used to compare 
   
    int bLeft;  //True or false variable to check for right parentheses
    //Function to get first token
    pszInfix = getToken(pszInfix, szToken, MAX_TOKEN + 1); 
    while(pszInfix!= NULL)
    {
        strcpy(element.szToken,szToken); 
        categorize(&element); // whether operator, operand, parentheses
        switch(element.iCategory)
        {
            case CAT_OPERAND:
            {
                addOut(out, element); //add to the Out array if operand
                break;
            }
            case CAT_OPERATOR:
            {
                //Compare top of stack's element and current element 
                while(isEmpty(stack)==FALSE && (element.iPrecedence <= topElement(stack).iPrecedence))
                {
                    addOut(out, pop(stack));
                    
                }
                push(stack, element);
                break;
            }
            case CAT_LPAREN:
            {
                push(stack,element); // push all Left parentheses to stack
                break;
            }
            case CAT_RPAREN:
            {
                bLeft = FALSE; //set initial condition to false
                bLeft = checkLeft(stack, out);
                //After the below function if bLeft is still false then 
                // a left parentheses was missing
                if(bLeft == FALSE)
                {
                    return WARN_MISSING_LPAREN;
                }
            }
        }
        //function to loop through each token
        pszInfix = getToken(pszInfix, szToken, MAX_TOKEN+1); 
    }
    //while loop to check for missing Right parentheses
    while (isEmpty(stack)== FALSE)
    {
        if(topElement(stack).iCategory == CAT_LPAREN)
        {
            free(stack);
            return WARN_MISSING_RPAREN;
        }
        addOut(out, pop(stack));
    }
    freeStack(stack);
    return 0;
}


/***********************evaluatePostfix*****************************************
 void evaluatePostfix(Out out, Customer customerM[], int iNumCustomer, QueryResult resultM[])
 Purpose:
 This function takes the postfix expression from the convert function and evaluates
 the query for each customer and stores the result.
 Parameters:
    I Out out                          Postfix expression 
    I Customer customerM[]             Array of valid customers
    I iNumCustomer                     Number of valid customers
    O QueryResult resultM[]            Result for each customer after new stack evaluation 
 
 Returns:
 QueryResult resultM[]              Array results for the customers
 
 Notes:
 * This function calls other helper functions that return boolean values.
 
 
 
 ******************************************************************************/


void evaluatePostfix(Out out, Customer customerM[], int iNumCustomer, QueryResult resultM[])
{
    
    int i;
    int j;
    Element postElem;   //token from out array used in switch statement 
    Element evalElem;
    Element elemRight;
    Element elemLeft;
    Element elemResult; //evaluate element variable used for each customer
    Element popped;
    Stack stack = newStack(); //create evaluation stack
    for(i = 0; i < iNumCustomer; i++)
    {
        for(j = 0; j < out->iOutCount; j++)
        {
            postElem = out->outM[j];
            switch( postElem.iCategory)
            {
                case CAT_OPERAND: //push every operand
                    push(stack, postElem);
                    break;
                case CAT_OPERATOR: 
                    strcpy(evalElem.szBoolean,"Boolean result of ");
                    strcat(evalElem.szBoolean, postElem.szToken);
                    //for each operator compare values in the stack and store result
                    elemRight = pop(stack);
                    elemLeft = pop(stack);
                    if(strcmp(postElem.szToken, "AND")== 0)
                        elemResult.bInclude = elemLeft.bInclude && elemRight.bInclude;
                    else if (strcmp(postElem.szToken, "OR")== 0)
                        elemResult.bInclude = elemLeft.bInclude || elemRight.bInclude;
                    else
                    {
                        Trait trait; //create trait variable for customer traits
                        strcpy(trait.szTraitType, elemLeft.szToken);
                        strcpy(trait.szTraitValue, elemRight.szToken);
                        if (strcmp(postElem.szToken, "NEVER")== 0)
                            elemResult.bInclude = never(&customerM[i], &trait);
                        else if(strcmp(postElem.szToken, "ONLY")==0)
                            elemResult.bInclude = only(&customerM[i], &trait);
                        else if(strcmp(postElem.szToken, "=")== 0)
                            elemResult.bInclude = atLeastOne(&customerM[i], &trait);
                    }
                    //after above code execution, push result for each customer into stack
                    push(stack, elemResult);
                    break;
                    
            }
        }
        //at the end of the out array for each customer, pop the stack and store the result
        popped = pop(stack);
        resultM[i] = popped.bInclude;
    }
    freeStack(stack);
}

/******************** atLeastOne **************************************
int atLeastOne(Customer *pCustomer, Trait *pTrait)
Purpose:
    Determines whether a customer has at least one particular trait (type and
    value).  If he does, atLeastOne returns TRUE; otherwise, it 
    returns FALSE.
Parameters:
    I Customer pCustomer     One customer structure which also
                             contains traits of that customer.
    I Trait    pTrait        The trait that we want this 
                             customer to have at least one of.
 * 
Return value:
    TRUE - customer did have at least one of the specified trait
    FALSE - customer didn't have it
**************************************************************************/



int atLeastOne(Customer *pCustomer, Trait *pTrait)
{
    int i;
    if (pCustomer == NULL)
        errExit("received a NULL pointer");
    // Look at each of the traits for a customer
    for (i = 0; i < (pCustomer->iNumberOfTraits); i++)
    {
        //compare each trait
        if (strcmp(pCustomer->traitM[i].szTraitType, pTrait->szTraitType) == 0
            && strcmp(pCustomer->traitM[i].szTraitValue, pTrait->szTraitValue) == 0)
            return TRUE;
    }
    return FALSE;
    
}

/******************** only*******************************************
int only(Customer *pCustomer, Trait *pTrait)
Purpose:
    Determines whether a customer has only a particular trait (type and
    value).  If he does, only returns TRUE; otherwise, it 
    returns FALSE.
Parameters:
    I Customer pCustomer     One customer structure which also
                             contains traits of that customer.
    I Trait    pTrait        The trait that we only want this 
                             customer to have.
 * 
Return value:
    TRUE - customer did have only one of the specified trait
    FALSE - customer didn't have it
**************************************************************************/
int only(Customer *pCustomer, Trait *pTrait)
{
    int i;
    int j = 0; //incrementing variable to check after for loop
    int bValue; //boolean variable to check for Trait value 
    if (pCustomer == NULL)
        errExit("received a NULL pointer");
     for (i = 0; i < (pCustomer->iNumberOfTraits); i++)
    {
        if (strcmp(pCustomer->traitM[i].szTraitType, pTrait->szTraitType) == 0)
            j = j+1;
        if (strcmp(pCustomer->traitM[i].szTraitValue, pTrait->szTraitValue) == 0)
            bValue = TRUE;
            
    }
    if(j == 1 && bValue == TRUE) //if there is only one trait type and trait value
        return TRUE;
    else 
        return FALSE;
    
    
}



/******************printCustomerData***********************************
void printCustomerData(Customer customerM[], int iNumCustomer)
Purpose:
    Print out the customer information in a formatted style using 
    for loops and printf statements.
Parameters:
    I Customer customerM[]              Array of customers
    I int iNumCustomer                  number of valid customers
Notes:

Return:
    None

********************************************************************/
void printCustomerData(Customer customerM[], int iNumCustomer)
{
    int i;
    int j;
    // Print a heading for the list of customers and traits
    printf("%-6s %-20s\n", "ID", "Customer Name");
    printf("%10s %-10s %-12s\n", " ", "Trait", "Value");
    for (i = 0; i < iNumCustomer; i++)
    {
        // Print the customer information
        /* TODO: your printf statement */
        printf("%-6s %-20s\n", customerM[i].szCustomerId, customerM[i].szCustomerName);
        
        // Print each of the traits
        for (j = 0; j < customerM[i].iNumberOfTraits; j++)
        {
            // Print a trait
            /* TODO: your printf statement */
            printf("%10s %-10s %-12s\n"," ", customerM[i].traitM[j].szTraitType, customerM[i].traitM[j].szTraitValue);
        
        }
    }
}

