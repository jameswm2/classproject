/**************************************************************************************************
p3cgf856.c by James Mancillas
Purpose:
    This program reads book information and a command file.   It 
    processes the commands against the book information.
    This is the driver code for the routines written by the students.
Command Parameters:
    p3 -b bookFileName -c commandFileName
Input:
    Book	same as Programming Assignment #2 although there is different data.

    Command	This is different from the previous assignment.  The file 
            contains  text in the form of commands.  
        CUSTOMER BEGIN szBookId szCustomerId szCheckedOutDt dLateFeePerDay dMaxLateFee szTitle  
            specifies the beginning of customer request and includes all the 
            identification information from program 2.
        CUSTOMER ADDRESS szStreetAddress,szCity,szStateCd,szZipCd
            specifies the address for a customer (separated by commas)
        CUSTOMER TRANS cTransType    szBookId   szTransDt
            specifies a single book transaction.  Steps:
            -	Print the Transaction Type, Book Id, and Transaction Date 
            -	Lookup the book ID using a binary search.  If not found, print a 
                warning (but do not terminate your program) and return.
            -	If the transaction date is invalid, show a message stating "invalid date", 
                but do not terminate.  Use the validateDate  function.
            -	If the transaction type is C (meaning checkout):
                o	If the book is already checked out, show a message stating 
                    "already checked out", but do not terminate.
                o	Otherwise, check out the book to this customer, setting the book's 
                    customer ID.  The book's checked out date needs to be set to the 
                    transaction's date.
            -	If the transaction type is R (meaning return):
                o	Use dateDiff to subtract the book's szCheckOutDt from the transaction's szTransDt
                o	If the difference is more than 14:
                    --	Determine the number of days late by subtracting 14 from that difference.  
                    --	Compute the late fee by multiplying the number of days late 
                        by the book's dLateFeePerDay.
                    --	If that late fee is greater than the book's dMaxLateFee, 
                        change it to dMaxLateFee.
                    --  Print the late fee.
                    --	Add the computed late fee to the customer's dFeeBalance
                o	Set the book's customer ID to "NONE".
                o	Set the book's check out date to "0000-00-00".
        CUSTOMER COMPLETE
            specifies the completion   of a customer.  Print the total fees for this customer.
        BOOK CHANGE szBookId dLateFeePerDay dMaxLateFee
            change the Late Fee Per Day and Max Late Fee to the specified values
        BOOK SHOW szBookId    
            requests a display of a particular book.  Show all of its information.
               
Results:
    Prints the Books prior to sorting
    Prints the Books after sorting.
    Processes the commands (see above) and shows any errors.
    Prints the resulting Books
Returns:
    0  normal
    -1 invalid command line syntax
    -2 show usage only
    -3 error during processing, see stderr for more information

Notes:
    p3 -?       will provide the usage information.  In some shells,
                you will have to type p3 -\?

**********************************************************************/

// If compiling using visual studio, tell the compiler not to give its warnings
// about the safety of scanf and printf
#define _CRT_SECURE_NO_WARNINGS 1
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "cs1713p3.h"

/******************** getBooks **************************************
    int getBooks(Book bookM[], char * pszBookFileName)
Purpose:
    Retrieves the books from the file specified..
Parameters:
    O   Book bookM[]          an array which is used for returning the books.
    I   char *pszBookFileName name of the file containing the book data
Returns:
    A count of the number of read.
Notes:
    1. This opens the file based on the specified file name. 
    2. The data in the data file is in this format
    szBookId szCustomerId szCheckedOutDt dLateFeePerDay dMaxLateFee szTitle
    9s        6s             10s           lf            lf          40s (may contain spaces)
**************************************************************************/

int getBooks(Book bookM[], char * pszBookFileName)
{
    FILE *bfile = fopen("p3book.txt","r");
    char szInputBuffer[100];		// input buffer for reading data
    int i = 0;                      // subscript in bookM
    int iScanfCnt;                  // returned by sscanf
    FILE *pFileBook;              // Stream Input for Books data. 

    /* open the Books stream data file */
    if (pszBookFileName == NULL)
        exitError(ERR_MISSING_SWITCH, "-b");

    pFileBook = fopen(pszBookFileName, "r");
    if (pFileBook == NULL)
        exitError(ERR_BOOK_FILENAME, pszBookFileName);

    /* get book information until EOF
    ** fgets returns null when EOF is reached.
    */
     while (fgets(szInputBuffer, 100, bfile)) 
      { 
	char a;
	a = fgetc(bfile);
	if(a != EOF)
	{
	ungetc(a,bfile);
		
        // check for too many books to fit in the bookM array
        if (i >= MAX_BOOKS)
           exitError(ERR_TOO_MANY_BOOKS, szInputBuffer);
        iScanfCnt = sscanf(szInputBuffer, "%10s %6s %10s %lf %lf %[^\n]\n"
            , bookM[i].szBookId
            , bookM[i].szCustomerId
            , bookM[i].szCheckedOutDt
            , &bookM[i].dLateFeePerDay
            , &bookM[i].dMaxLateFee
            , bookM[i].szTitle);
        // It should have populated all six variables, if not exit with
        // an error.
        if (iScanfCnt < 6)
         exitError(ERR_BOOK_DATA, szInputBuffer);
        i++;
	}
      }

    fclose(pFileBook);
    return i;
}

/******************** sortBooks **************************************
    void sortBooks(Book bookM[], int iBookCnt)
Purpose: Uses a Bubblesort to sort the array of books

Parameters: The array of books to be sorted, size of the array of books

Returns: The same array sroted
    
Notes: Sorts by szBookId
    
**************************************************************************/
void sortBooks(Book bookM[], int iBookCnt)
{
    
    Book temp;
    int i = 0;
    int x = 0;
    for( i =0; i<iBookCnt-1; i++)
    {
      for( x=0; x<iBookCnt-i-1;x++)
       {
        if(strcmp(bookM[x].szBookId,bookM[x+1].szBookId)>0)
         {
	   temp = bookM[x];
	   bookM[x] = bookM[x+1];
	   bookM[x+1] = temp;	
         }   
      }
   }
}

/******************** printBooks **************************************
    void printBooks(char *pszHeading, Book bookM[], int iBookCnt)
Purpose:
    Prints each book in a table format
Parameters:
    I char *pszHeading    Heading to print before printing the table of books
    I Book bookM[]        Array of books
    I int iBookCnt        Number of elements in the bookM array.
Returns:
    n/a
Notes:
    
**************************************************************************/
void printBooks(char *pszHeading, Book bookM[], int iBookCnt)
{
    int i;
    printf("\n%s\n", pszHeading);

    // output column headings
    printf("   %-9s %-40s %-8s %-10s %-8s %-8s \n"
        , "Book Id", "Title", "Customer", "Ck Out Dt", "Late Fee", "Max Late");

    // output each book in a table format
    for (i = 0; i < iBookCnt; i++)
    {
        printf("   %-9s %-40s %-8s %-10s %8.2lf %8.2lf \n"
            , bookM[i].szBookId
            , bookM[i].szTitle
            , bookM[i].szCustomerId
            , bookM[i].szCheckedOutDt
            , bookM[i].dLateFeePerDay
            , bookM[i].dMaxLateFee
            );
    }
}


/********************processCustomerCommand *****************************
    void processCustomerCommand(Book bookM[], int iBookCnt
     , char *pszSubCommand, char *pszRemainingInput
     , Customer *pCustomer)
Purpose:
    Processes the subcommands associated with the CUSTOMER command:
        CUSTOMER BEGIN szBookId szCustomerId szCheckedOutDt dLateFeePerDay dMaxLateFee szTitle  
            specifies the beginning of customer request and includes all the identification information from program 2.
        CUSTOMER ADDRESS szStreetAddress,szCity,szStateCd,szZipCd
            specifies the address for a customer (separated by commas)
        CUSTOMER TRANS cTransType    szBookId   szTransDt
            specifies a single book transaction.  Steps:
            -	Print the Transaction Type, Book Id, and Transaction Date 
            -	Lookup the book ID using a binary search.  If not found, print a warning (but do not terminate your program) and return.
            -	If the transaction date is invalid, show a message stating "invalid date", but do not terminate.  Use the validateDate  function.
            -	If the transaction type is C (meaning checkout):
                o	If the book is already checked out, show a message stating "already checked out", but do not terminate.
                o	Otherwise, check out the book to this customer, setting the book's customer ID.  The book's checked out date needs to be set to the transaction's date.
            -	If the transaction type is R (meaning return):
                o	Use dateDiff to subtract the book's szCheckOutDt from the transaction's szTransDt
                o	If the difference is more than 14:
                    --	Determine the number of days late by subtracting 14 from that difference.  
                    --	Compute the late fee by multiplying the number of days late by the book's dLateFeePerDay.
                    --	If that late fee is greater than the book's dMaxLateFee, change it to dMaxLateFee.
                    --  Print the late fee.
                    --	Add the computed late fee to the customer's dFeeBalance
                o	Set the book's customer ID to "NONE".
                o	Set the book's check out date to "0000-00-00".
        CUSTOMER COMPLETE
            specifies the completion   of a customer.  Print the total fees for this customer.


Parameters:
    I/O Book bookM[]                  Array of books
    I   int   iBookCnt                Number of elements in bookM[]
    I   char  *pszSubCommand          Should be BEGIN, ADDRESS, REQUEST or COMPLETE
    I   char  *pzRemainingInput       Points to the remaining characters in the input
                                      line (i.e., the characters that following the
                                      subcommand).
    I/O Customer *pCustomer           The BEGIN subcommand begins a new customer.  
Notes:

**************************************************************************/
double dSumFeeNG;
char szCurrentCustomer[10];
Customer customerOne;
void processCustomerCommand(Book bookM[], int iBookCnt
    , char *pszSubCommand, char *pszRemainingInput
    , Customer *pCustomer)
    {
    int i;
    int iscanfCnt;
    double dLateFees;
    double dTotalFees;
    char cTransType;
    Transaction transactionX;
    // Determine what to do based on the subCommand
    if (strcmp(pszSubCommand, "BEGIN") == 0)
    {
        // get the Customer Identification Information
        // your code
        sscanf(pszRemainingInput,"%6s" "%lf" "%40s" "%30[^\n]", 
	      customerOne.szCustomerId, &customerOne.dFeeBalance,
              customerOne.szEmailAddr, customerOne.szFullName);
	dSumFeeNG += customerOne.dFeeBalance;
	strcpy(szCurrentCustomer,customerOne.szCustomerId);	
    /* fscanf(xf,
    "%6s"          "%lf"          "%40s"     "%30[^\n]"    "%30[^,]" "%*[,]"
    "%20[^,]"   "%*[,]"             "%2s"       "%*[,]"         "%5s",
    szCustomerId, &dFeeBalance, szEmailAddr, szFullName, szStreetAddress,
    szCity , throw out comma  szStateCd,  throw comma szZipCd);*/

    //	printf("%.2lf",customerOne.dFeeBalance);
        // Check for bad customer identification data
        //if (iscanfCnt < 4)
          //  exitError(ERR_CUSTOMER_ID_DATA, pszRemainingInput);
    }
    else if (strcmp(pszSubCommand, "COMPLETE") == 0)
    {
	printf("\t\t\t\t\t  *** Total Fees are: $%.2lf ***",dSumFeeNG);
        dSumFeeNG = 0;
	// print the customer's total fees
	printf("\n\n");
    }
    else if (strcmp(pszSubCommand, "ADDRESS") == 0)
    {
        // get the postal address and print it
        // your code
        sscanf(pszRemainingInput, "%30[^,]" "%*[,]" 
             "%20[^,]" "%*[,]" "%2s" "%*[,]" "%5s",
	      customerOne.szStreetAddress,   customerOne.szCity,  
              customerOne.szStateCd,     customerOne.szZipCd);
        // print the column heading for the transactions
	printf("*************************LIBRARY RECEIPT***************************\n");
	printf("%s %s %s (previously owed %.2lf)\n", 
	      customerOne.szCustomerId,customerOne.szEmailAddr,
	      customerOne.szFullName, customerOne.dFeeBalance);
	printf("%s\n",customerOne.szStreetAddress);
	printf("%s, %s %s\n", customerOne.szCity,customerOne.szStateCd,
			    customerOne.szZipCd);
        printf("\t\t\t\t\t%-5s  %-9s  %-10s\n", "Trans", "Book", "Date");

    }
    else if (strcmp(pszSubCommand, "TRANS") == 0)
    { 
 	sscanf(pszRemainingInput ,"%s" "%s" "%s" , &cTransType,
              transactionX.szBookId, transactionX.szTransDt);
        printf("\t\t\t\t\t%-5s  %-9s  %-10s\n", &cTransType,
	     transactionX.szBookId, transactionX.szTransDt);
	//printf("%s %s %s", &cTransType, transactionX.szBookId,
	  //    transactionX.szTransDt);
        // your code
 	int i = searchBooks(bookM, iBookCnt, transactionX.szBookId);
  	if(cTransType == 'C' )
         {
	  //printf("%s\n",bookM[i].szBookId);
           if(i==-1)
            printf("ERROR: Book Above Not Found In Data\n");
           else
            {
	     //printf("%s\n",bookM[i].szBookId);
             //printf("detected checkout\n"); /////
              if(strcmp(bookM[i].szCustomerId,"NONE") == 0)
               {
                //printf("Book Not checked out already\n");
                strcpy(bookM[i].szCustomerId, szCurrentCustomer);
                // printf(" customer Id = %s \n",customerOne.szCustomerId);
                strcpy(bookM[i].szCheckedOutDt,transactionX.szTransDt);
                }
             else
              printf("\t\t\t\t\t  Book Above Already Checked Out\n");
            }
         }
     }
   else printf("   *** %s %s\n", ERR_CUSTOMER_SUB_COMMAND, pszSubCommand);
     
   if(cTransType == 'R')
     {
      i  = searchBooks(bookM, iBookCnt, transactionX.szBookId);
      //printf("detected return\n");
      
      if(i==-1)
        printf("ERROR: Book Above Not Found In Data\n");
      else
       {
        //printf("detected return\n");
        int iDays = dateDiff(transactionX.szTransDt,bookM[i].szCheckedOutDt);
        if(iDays-14 <= 0)
         printf("\t\t\t\t\t  (No late Fees: Thank you!)\n");
        else
        {
         dLateFees = (iDays-14)*bookM[i].dLateFeePerDay;
         if(dLateFees > bookM[i].dMaxLateFee)
            dLateFees = bookM[i].dMaxLateFee;
         printf("\t\t\t\t\t (%d Days Late, $%.2lf Due in Fees)\n",iDays-14,dLateFees);
         dSumFeeNG += dLateFees;
        }
      }
	
      if(i!=-1)
       {	
        strcpy(bookM[i].szCustomerId,"NONE");
	//printf("%s\n",bookM[i].szBookId);
	//printf("checking the loop %s\n",bookM[i].szCheckedOutDt);
	
	//char empty[12];
	//strcpy(empty,"0000-00-00");
        strncpy(bookM[i].szCheckedOutDt,"0000-00-00",10);
	//strcpy(bookM[i].szCheckedOutDt, "0000-00-00");
       }
     }
}
/********************processBookCommand *****************************
    void processBookCommand(Book bookM[], int iBookCnt
         , char *pszSubCommand, char *pszRemainingInput)
Purpose:
    Processes the subcommands associated with the BOOK command:
        BOOK CHANGE szBookId dLateFeePerDay dMaxLateFee
            change the Late Fee Per Day and Max Late Fee to the specified values
        BOOK SHOW szBookId    
            requests a display of a particular book.  Show all of its information.
                
Parameters:
    I/O Book bookM[]                  Array of books
    I   int   iBookCnt                Number of elements in bookM[]
    I   char  *pszSubCommand          Should be CHANGE or SHOW
    I   char  *pzRemainingInput       Points to the remaining characters in the input
                                      line (i.e., the characters that following the
                                      subcommand).
Notes:

**************************************************************************/
void processBookCommand(Book bookM[], int iBookCnt
                             , char *pszSubCommand, char *pszRemainingInput)
{
    Book book;
    char szBookCMDType[10];
    int iScanfCnt;
    int i;
    int BOOL;
    // Determine what to do based on the subCommand
    // your code
    sscanf(pszRemainingInput, "%s" "%lf" "%lf",
	  book.szBookId, &book.dLateFeePerDay, &book.dMaxLateFee );
    i = searchBooks(bookM,iBookCnt,book.szBookId);
    
    if(i !=-1)
    { 
     if(strcmp(pszSubCommand,"SHOW") == 0)
       {
	printf("\n");
 	printf("Book ID: %s\t",bookM[i].szBookId);
        printf("Book Title : %s\t",bookM[i].szTitle);
        printf("Current Customer: %s\n",bookM[i].szCustomerId);
        printf("Last Checked out: %s\t",bookM[i].szCheckedOutDt);
        printf("Late Fee Per Day: %.2lf\t",bookM[i].dLateFeePerDay);
        printf("Max Late Fee: %.2lf\n\n",bookM[i].dMaxLateFee);
	//printf("%d\n",i);	
	i=0;	
       }    
    else if(strcmp(pszSubCommand,"CHANGE") == 0)
       {
	printf("\n");
	bookM[i].dLateFeePerDay = book.dLateFeePerDay;
	bookM[i].dMaxLateFee = book.dMaxLateFee;
	i=0;
       }
    }
  if(i!=0)
    printf("Error: Book Not Found\n\n");
}

/******************** searchBooks *****************************
    int searchBooks(Book bookM[], int iBookCnt, char *pszMatchBookId)
Purpose:
    Using a binary search, it finds the specified book in the booKM array.
Parameters:
    I   Book bookM[]                 Array of books
    I   int   iBookCnt               Number of elements in bookM[]
    I   char *pszMatchBookId         Book Id to find in the array
Returns:
    >= 0   subscript of where the match value was found
    -1     not found
Notes:

**************************************************************************/
int searchBooks(Book bookM[], int iBookCnt, char *pszMatchBookId)
{
    // your code
    int iupper, ilower, imiddle;
    //*pszmatchbookId = match
    iupper = iBookCnt-1;
    ilower = 0;
    while(ilower<=iupper)
    {
     imiddle = (ilower+iupper)/2;
     if(strcmp(bookM[imiddle].szBookId,pszMatchBookId) ==0)
	return imiddle;
     else if (strcmp(bookM[imiddle].szBookId,pszMatchBookId) > 0)
        iupper = imiddle-1;
     else
	ilower=imiddle+1;
    }
    
    return -1;
}

//older code
/*** include your processTransaction code from program #2 ***
void mProcessCustomers(FILE *xf,Book bookM[], int iBookCount)
{
 Customer customerOne = customerInformation(xf);
 Transaction transactionOne = customerTransaction(xf,bookM,iBookCount,customerOne);
}
////////////////////////
Customer customerInformation(FILE *xf)
{
 char        szCustomerId[7];            // 6 character customer ID
 char        szEmailAddr[41];            // customer's email address is used
                                         // to uniquely define a customer
 char        szFullName[31];             // customer full name
 char        szStreetAddress[31];        // street number, street name, and 
                                         // (optional) apartment number
 char        szCity[21];                 // address city
 char        szStateCd[3];               // address state code
 char        szZipCd[6];                 // address zip code
 double      dFeeBalance;                // amount this customer owes in fe
 Customer customerOne;
 //scain for data objects
  fscanf(xf,
 "%6s"          "%lf"          "%40s"     "%30[^\n]"    "%30[^,]" "%*[,]"
 "%20[^,]"   "%*[,]"             "%2s"       "%*[,]"         "%5s",
 szCustomerId, &dFeeBalance, szEmailAddr, szFullName, szStreetAddress,
 szCity , throw out comma  szStateCd,  throw comma   szZipCd);

 //copy data from scan into structure
 strcpy(customerOne.szCustomerId,    szCustomerId);
 customerOne.dFeeBalance =           dFeeBalance;
 strcpy(customerOne.szEmailAddr,     szEmailAddr);
 strcpy(customerOne.szFullName,      szFullName);
 strcpy(customerOne.szCity,          szCity);
 strcpy(customerOne.szStreetAddress, szStreetAddress);
 strcpy(customerOne.szStateCd,       szStateCd);
 strcpy(customerOne.szZipCd,         szZipCd);
 printf("\n");
 printf("**************** Library Receipt ****************\n");
 printf(" %6s %s %s (previously owed %.2lf)",
        szCustomerId, szEmailAddr, szFullName,dFeeBalance);
 printf("%2s \n ", szStreetAddress);
 printf("%s %s %s\n", szCity, szStateCd,szZipCd);
 printf(" Trans    Book         Date\n");

 return customerOne;
 }
//////////////////////////////////
Transaction customerTransaction (FILE *xf, Book bookM[], int iBookCount, Customer customerOne )
{
 char cTransType[10];               // C - check out, R - return
 char szBookId[10];                 // Individual Book Id (not ISBN since
                                    // multiple copies have the same value)
 char szTransDt[11];                // Date for the transaction
 int i;
 double dTotalFees;
 // findBook(Book bookM[], int iBookCount, char szBookId
 double dLateFees;
 int iDays;
 Transaction transactionX;

           //   &bookM[i].dLateFeePerDay
           //   &bookM[i].dMaxLateFee

 while(strcmp(cTransType, "X") !=0)
  {
   fscanf(xf,"%s" "%s" "%s" , cTransType, szBookId, szTransDt);
   printf("%3s    %s   %s\n", cTransType, szBookId, szTransDt);
   i = findBook(bookM,iBookCount,szBookId);

   if(strcmp(cTransType, "C") ==0)
     {
      if(i==-1)
        printf("ERROR: Book Above Not Found In Data\n");
      else
       {
        //printf("detected checkout\n");
        if(strcmp(bookM[i].szCustomerId,"NONE") == 0)
         {
         // printf("Book Not checked out already\n");
          strcpy(bookM[i].szCustomerId, customerOne.szCustomerId);
         // printf(" customer Id = %s \n",customerOne.szCustomerId);
          strcpy(bookM[i].szCheckedOutDt,szTransDt);

         }
        else
          printf(" Book Above Already Checked Out\n");
       }
     }
   if(strcmp(cTransType, "R") ==0)
     {
      if(i==-1)
        printf("ERROR: Book Above Not Found In Data\n");
      else
       {
        //printf("detected return\n");
        iDays = dateDiff(szTransDt,bookM[i].szCheckedOutDt);
        if(iDays-14 <= 0)
          printf("     (No late Fees)\n");
        else
        {
         dLateFees = (iDays-14)*bookM[i].dLateFeePerDay;
         if(dLateFees > bookM[i].dMaxLateFee)
         dLateFees = bookM[i].dMaxLateFee;
         printf(" (%d Days Late, $%.2lf Due in Fees)\n",iDays-14,dLateFees);
         dTotalFees += dLateFees;
        }
      }
      if(i!=-1)
       {
        strcpy(bookM[i].szCustomerId,"NONE");
        strcpy(bookM[i].szCheckedOutDt,"0000-00-00");
       }
     }
  }
 dTotalFees = dTotalFees + customerOne.dFeeBalance;
 printf("Total Late Fees Due = %.2lf\n",dTotalFees);
 dTotalFees = 0;
 return transactionX;
}*/














