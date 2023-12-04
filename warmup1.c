/*
 * Author:      Steve Regala (sregala@usc.edu)
 *
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>

#include <errno.h>
#include <time.h>
#include <ctype.h>

#include "cs402.h"

#include "my402list.h"


/* ----------------------- Data Object ----------------------- */

// transaction object
typedef struct tagTransactObj{
	char* transactType;
	unsigned int transactTime;
	long cents;
	char description[1024];
	unsigned int lineNumber;
} TransactObj;


/* ----------------------- BEFORE SORTING FUNCTIONS----------------------- */


/*
Function will process the command line arguments and does an error check of each scenario.
It will check for various errors when running the warmup1 program; print errors accordingly.
PRE: argc is the size of the command line (argc>0), argv[] is the array of the input
*/
FILE* ProcessCommandLine(int argc, char* argv[]) {

	if (argc<2) {
		fprintf(stderr, "ERROR: Malformed command.\n");
		return NULL;
	}

	if (strcmp(argv[1],"sort")) {
		fprintf(stderr, "ERROR: Malformed command. \"%s\" is not a valid commandline option.\n", argv[1]);
		return NULL;
	}

	if (argc>3) {
		fprintf(stderr, "ERROR: Malformed command. Too many commandline arguments.\n");
		return NULL;
	}

	if (argc==2) {
		FILE *fp = stdin;
		return fp;
	}

	else if (argc==3) {
		FILE *fp = fopen(argv[2], "r");
		if (fp==NULL) {
			int errorNum=errno;
			fprintf(stderr, "Error opening file \"%s\": %s.\n", argv[2], strerror(errorNum));
			return NULL;
		}
		return fp;
	}

	return NULL;

}


/*
This a helper function for the helper function, ProcessField.
This will copy the startPointer into buffer.
Returns the length of the copied c-string for function ProcessField to error check.
PRE: buffer and startPointer must not be NULL; bufferSize must be sizeof(buffer)
POST: buffer contains a copy of the c-string pointed to by startPointer
*/
int GetField(char* buffer, char* startPointer, int bufferSize) {
	strncpy(buffer, startPointer, bufferSize);
	buffer[bufferSize-1] = '\0';
	return strlen(buffer);
}


/*
This is a helper function for ReadInput, helping process each field of each line.
Returns the field back to ReadInput if it is valid, otherwise, it is an error and exits immediately.
PRE: field and start_Pointer cannot be NULL; size must be sizeof(field)
POST: field contains a copy of the c-string pointed to by start_Pointer
*/
char* ProcessField(char* field, char* start_Pointer, int size, int lineNo) {

	if (GetField(field, start_Pointer, size) > 0) {
		return field;
	} 
	else {
		fprintf(stderr, "ERROR: Invalid transaction input on line %d.\n", lineNo);
		exit(-1);	
	}
	
}


/*
This is a helper function for ReadInput.
This error checks the first field from the read-in line, it prints out errors if applicable.
Returns true if it is either a '+' or '-', return false otherwise.
PRE: theField must not be NUll, lineNo>0
*/
int FirstFieldCheck(char* theField, int lineNo) {
	
	if (strcmp(theField, "-")) {
		
		if (strcmp(theField, "+")) {
			fprintf(stderr, "ERROR: Transaction type is not '-' or '+' on line %d.\n", lineNo);
			return 0;
		}
		else {
			return 1;
		}

		fprintf(stderr, "ERROR: Transaction type is not '-' or '+' on line %d.\n", lineNo);
		return 0;
	}
	
	return 1;
}


/*
This is a helper function for ReadInput.
This error checks the second field from the read-in line; it prints out errors if applicable.
Returns true if the second field passes the errors; returns false otherwise.
PRE: theField must not be NULL, lineNo>0
*/
int SecondFieldCheck(char* theField, int lineNo) { 
	
	if (theField[0]=='0') {
		fprintf(stderr, "ERROR: Transaction time is invalid on line %d.\n", lineNo);
		return 0;
	}

	if (strlen(theField) >= 11) {
		fprintf(stderr, "ERROR: Transaction time is invalid on line %d.\n", lineNo);
		return 0;
	}

	// check if every digit is a between 0 and 9
	for (int i=0; i<strlen(theField); i++) {
		if (!isdigit(theField[i])) {
			fprintf(stderr, "ERROR: Transaction time is invalid on line %d.\n", lineNo);
			return 0;
		}
	}

	int intTime = atoi(theField);
	time_t timeRead = intTime;
	time_t timeNow = time(NULL);

	if (intTime <= 0 || timeRead>timeNow) {
		fprintf(stderr, "ERROR: Transaction time is invalid on line %d.\n", lineNo);
		return 0;
	}

	return 1;

}


/*
This is a helper function for another helper function, ThirdFieldCheck.
Returns false if preconditions are not met for the left of the decimal; return true otherwise.
It prints out errors if applicable.
PRE: s must not be NULL, lineNum>0
*/
int ValidateNum(char* s, int lineNum) {
	
	// check if string is empty
	if (strlen(s)==0) {
		fprintf(stderr, "ERROR: Transaction amount is invalid on line %d.\n", lineNum);
		return 0;
	}
	
	//check if every character is a 0
	int count=0;
	for (int i=0; i<strlen(s); i++) {
		if (s[i]=='0') {
			count++;	
		}
	}
	
	if (count==strlen(s)) { // all characters are 0
		return 1;
	} else {
		if (s[0]=='0' || s[0]=='-') { // if 0 is first digit for non-zero number, produce an error
			fprintf(stderr, "ERROR: Transaction amount is invalid on line %d.\n", lineNum);
			return 0;
		}
	}

	// check if every digit is a between 0 and 9
	for (int i=0; i<strlen(s); i++) {
		if (!isdigit(s[i])) {
			fprintf(stderr, "ERROR: Transaction amount is invalid on line %d.\n", lineNum);
			return 0;
		}
	}
	
	if (strlen(s)>7 || atoi(s)>10000000) {
		fprintf(stderr, "ERROR: Transaction amount is invalid on line %d.\n", lineNum);
		return 0;
	}
	
	return 1;
}


/*
This is a helper function for ReadInput.
This error checks the third field from the read-in line.
Returns false if it doesn't pass the error checks; otherwise returns true.
PRE: theField must not be NULL, lineNo>0
POST: theField contains transaction amount(in cents)
*/
int ThirdFieldCheck(char* theField, int lineNo) {
	
	int bufferSize=1024;	
	
	//do checks on the right of the decimal
	char* startPointer = theField;
	char* decPointer = strchr(startPointer, '.');
	// check for # of digits after decimal point
	if (strlen(decPointer)!=3) {
		fprintf(stderr, "ERROR: Transaction amount is invalid on line %d.\n", lineNo);
		return 0;
	}
	*decPointer++ = '\0';

	char rightAmount[bufferSize];
	strcpy(rightAmount, decPointer);
	rightAmount[bufferSize-1]='\0';

	// check if right of decimal is all digits
	for (int i=0; i<strlen(rightAmount); i++) {
		if (!isdigit(rightAmount[i])) {
			fprintf(stderr, "ERROR: Transaction amount is invalid on line %d.\n", lineNo);
			return 0;
		}
	}

	// do checks on left decimal
	char leftAmount[bufferSize];
	strcpy(leftAmount, startPointer);
	leftAmount[bufferSize-1]='\0';
	
	// check if left of decimal is valid	
	if (!ValidateNum(leftAmount, lineNo)) {
		return 0;
	}

	int finalLeft=atoi(leftAmount)*100;
	int finalRight=atoi(rightAmount);
	int finalAmount = finalLeft + finalRight;
	
	// amount must be >0
	if (finalAmount<=0) {
		fprintf(stderr, "ERROR: Transaction amount is invalid on line %d.\n", lineNo);
		return 0;
	}
	
	// change integer to character, store into passed-in theField
	char tempAmount[1024];
	snprintf(tempAmount, sizeof(tempAmount), "%d", finalAmount);
	strcpy(theField, tempAmount);

	return 1;
}


/*
This is a helper function for ReadInput.
This error checks the fourth field from the read-in line; it prints out error messages if applicable.
Returns false if the description field is invalid (according to guidelines); otherwise returns true.
PRE: theField must not be NULL, lineNo>0
POST: theField will point to the first non-leading space character
*/
int FourFieldCheck(char* theField, int lineNo) {
	
	// remove leading white space
	while (isspace((unsigned char)*theField)) {
		theField++;
	}

	if (strlen(theField)<=0) {
		fprintf(stderr, "ERROR: Transaction description is invalid on line %d.\n", lineNo);
		return 0;	
	}
	
	return 1; 
}


/*
This is a helper function for ReadInput. Return number of tabs in c-string.
It will count the number of tabs in a given line of a c-string.
PRE: line must be a c-string and not null
*/
int count_tabs(const char* line) {

	int count=0;
	for (int i=0; i<strlen(line); i++) {
		if (line[i] == '\t') {
			count++;		
		}
	}
	return count;
}


/*
Returns true if file passes all errors, and false otherwise.
This function will read in each line and do various error checks on them.
File contents will be added to a transaction object (transactObj), and object will be added to My402List list.
PRE: fp must not be null, and list must be a valid list
POST: list will have My402ListElem elements with transactObj after successfully passing the errors
*/
int ReadInput(FILE *fp, My402List *list) {

	char buf[2000];
	int lineNum=0;

	while (fgets(buf,sizeof(buf), fp)) {
		
		lineNum++;
		// check if line exceeds maximum number of characters
		if (strlen(buf) > 1024) {
			fprintf(stderr, "ERROR: Too many characters on line %d.\n", lineNum);
			return 0;	
		}

		// check the number of tabs in a given line
		if (count_tabs(buf) != 3) {
			fprintf(stderr, "ERROR: Number of tabs is not 3 on line %d.\n", lineNum);
			return 0;
		}

		char firField[1024];
		char secField[1024];
		char thirdField[1024];
		char fourField[1024];

		// allocate memory for transaction object
		TransactObj* myObj = (TransactObj*)malloc(sizeof(TransactObj));
		

		// --------- process FIRST field: transaction type ---------
		char* start_ptr = buf;
		char* tab_ptr = strchr(start_ptr, '\t');
		if (tab_ptr != NULL) {
			*tab_ptr++ = '\0';	
		}

		ProcessField(firField, start_ptr, sizeof(firField), lineNum);
		if (!FirstFieldCheck(firField, lineNum)) {
			return 0;
		}
		myObj->transactType=strdup(firField);	


		// --------- process SECOND field: transaction time ---------
		start_ptr = tab_ptr;
		tab_ptr = strchr(start_ptr, '\t');
		if (tab_ptr != NULL) {
			*tab_ptr++ = '\0';		
		}

		ProcessField(secField, start_ptr, sizeof(secField), lineNum);
		if (!SecondFieldCheck(secField, lineNum)) {
			return 0;
		}
		myObj->transactTime = atoi(secField);

		
		// --------- process THIRD field: transaction amount ---------
		start_ptr = tab_ptr;
		tab_ptr = strchr(start_ptr, '\t');
		if (tab_ptr != NULL) {
			*tab_ptr++ = '\0';
		}
		
		ProcessField(thirdField, start_ptr, sizeof(thirdField), lineNum);
		if (!ThirdFieldCheck(thirdField, lineNum)) {
			return 0;
		}
		int centsAmount = atoi(thirdField);
		myObj->cents = centsAmount;


		// --------- process FOURTH field: description ---------
		start_ptr = tab_ptr;
		tab_ptr = strchr(start_ptr, '\t');
		if (tab_ptr != NULL) {
			*tab_ptr++ = '\0';
		}

		ProcessField(fourField, start_ptr, sizeof(fourField), lineNum);
		if (!FourFieldCheck(fourField, lineNum)) {
			return 0;
		}
		strncpy(myObj->description, fourField, strlen(fourField)-1);

		myObj->lineNumber = lineNum;
		
		// add to my402List
		My402ListAppend(list, (void*)myObj);
		
	}

	if (lineNum == 0) {
		return 0;
	}
	
	return 1;

}


/*
MAIN CODE WAS PROVIDED BY THE PROFESSOR FROM listtest.c
This is a helper function for the BubbleSortForwardList function.
This function swaps the corresponding elements in the element list.
PRE: pList must be a valid list, pp_elem1 and pp_elem2 cannot be NULL
POST: corresponding elements in list will be swapped
*/
static
void BubbleForward(My402List *pList, My402ListElem **pp_elem1, My402ListElem **pp_elem2)
    /* (*pp_elem1) must be closer to First() than (*pp_elem2) */
{
    My402ListElem *elem1=(*pp_elem1), *elem2=(*pp_elem2);
    void *obj1=elem1->obj, *obj2=elem2->obj;
    My402ListElem *elem1prev=My402ListPrev(pList, elem1);
/*  My402ListElem *elem1next=My402ListNext(pList, elem1); */
/*  My402ListElem *elem2prev=My402ListPrev(pList, elem2); */
    My402ListElem *elem2next=My402ListNext(pList, elem2);

    My402ListUnlink(pList, elem1);
    My402ListUnlink(pList, elem2);
    if (elem1prev == NULL) {
        (void)My402ListPrepend(pList, obj2);
        *pp_elem1 = My402ListFirst(pList);
    } else {
        (void)My402ListInsertAfter(pList, obj2, elem1prev);
        *pp_elem1 = My402ListNext(pList, elem1prev);
    }
    if (elem2next == NULL) {
        (void)My402ListAppend(pList, obj1);
        *pp_elem2 = My402ListLast(pList);
    } else {
        (void)My402ListInsertBefore(pList, obj1, elem2next);
        *pp_elem2 = My402ListPrev(pList, elem2next);
    }
}


/*
MAIN CODE WAS PROVIDED BY THE PROFESSOR FROM listtest.c
This is a helper function to sort the list called in the main sorting function.
This function implements the bubble sort algorithm.
PRE: list must be a valid list, num_items is length of list
POST: list will be in sorted ascending order according to timestamps
*/
static
void BubbleSortForwardList(My402List *pList, int num_items)
{
	My402ListElem *elem=NULL;
	int i=0;

	if (My402ListLength(pList) != num_items) {
		fprintf(stderr, "List length is not %1d in BubbleSortForwardList().\n", num_items);
		exit(1);
	}
	for (i=0; i < num_items; i++) {
		int j=0, something_swapped=FALSE;
		My402ListElem *nextElem=NULL;

		for (elem=My402ListFirst(pList), j=0; j < num_items-i-1; elem=nextElem, j++) {
			
			TransactObj* currVal = (TransactObj*)(elem->obj);
			int intCurrVal = currVal->transactTime, intNextVal=0;
			
			nextElem = My402ListNext(pList, elem);
			TransactObj* nextVal = (TransactObj*)(nextElem->obj);
			intNextVal = nextVal->transactTime;
			
			if (intCurrVal > intNextVal) {
			BubbleForward(pList, &elem, &nextElem);
			something_swapped = TRUE;
			}
		}
	if (!something_swapped) break;
	}
}


/*
This is a helper function called in the main sorting function.
Returns true if there are no duplicates that exist in the list; false otherwise with an error message.
PRE: myList must be a valid list
*/
int FindDuplicates(My402List *myList) {
	
	int length = My402ListLength(myList);

	My402ListElem *elem=NULL;
	
	My402ListElem* curr = My402ListFirst(myList);	
	
	for (int i=0; i<length; i++) {
		int j=0;
		
		TransactObj* currVal = (TransactObj*)(curr->obj);
		int intCurrVal = currVal->transactTime;
		int prevTimeStamp = currVal->lineNumber;
		
		My402ListElem* nextElem=NULL;

		for (elem=My402ListNext(myList, curr), j=0; j<length-i-1; elem=nextElem, j++) {
			
			TransactObj* elemVal = (TransactObj*)(elem->obj);
			int intElemVal = elemVal->transactTime;
			if (intCurrVal == intElemVal) {
				int timeStamp = elemVal->lineNumber;
				fprintf(stderr, "ERROR: There is a duplicate timestamp on line number %d and %d.\n", prevTimeStamp, timeStamp);
				return 0;
			}
			
			nextElem=My402ListNext(myList, elem);

		}
		curr = My402ListNext(myList, curr);
	}
	
	return 1;
}


/*
This is the main sorting function called in the main function.
It will first find any possible duplicates in timestamps and then sort according to timestamps.
PRE: list is a valid My402List list
POST: list will be in sorted ascending order of timestamps
*/
static void SortInput(My402List *list) {

	if (!FindDuplicates(list)) {
		exit(-1);
	}

	int listSize = My402ListLength(list);
	BubbleSortForwardList(list, listSize);
}


/*
This function is a helper function called in the PrintStatement function.
This primarily takes care of formatting the resulting AMOUNT and BALANCE on the bank statement sheet.
PRE: type must not be null, finalAmount>0
*/
static void FormatOutput(int finalAmount, char* type) {		
			
	char finalProduct[20]; // with parenthesis and spaces
		
		// only entered when BALANCE exceeds this amount
		if (finalAmount >= 100000000) {
			if (!strcmp(type, "-")) {
				printf(" %s |", "(?,???,???.\?\?)");
			}
			else {
				printf(" %s |", " ?,???,???.\?\? ");
			}
		}

		else {
			int hundredths = finalAmount%10;
			finalAmount = finalAmount/10;
			int tenths = finalAmount%10;
			finalAmount = finalAmount/10;
			char cents[4];
			sprintf(cents, ".%d%d", tenths, hundredths); // formulate cents
		
			char dollars[8];
			if (finalAmount>999) {
				int ones = finalAmount%10;
				finalAmount = finalAmount/10;
				int tens = finalAmount%10;
				finalAmount = finalAmount/10;
				int hundreds = finalAmount%10;
				finalAmount = finalAmount/10;
			
				sprintf(dollars, "%d,%d%d%d", finalAmount, hundreds, tens, ones);

			} else {
				sprintf(dollars, "%d", finalAmount);
			}
			

			char totalAmount[11]; // before parenthesis and spaces
			strcpy(totalAmount, dollars);
			strcat(totalAmount, cents);

			char leftSpace[13];
			strcpy(leftSpace, " ");
			int num_spaces = 12 - strlen(totalAmount);
			for (int i=0; i<num_spaces-1; i++) {
				strcat(leftSpace, " ");
			}

			// Add spaces to the left
			strcat(leftSpace, totalAmount);

			// Deal with parenthesis --> finalProduct is empty at this point
			if (!strcmp(type, "+")) {
				strcpy(finalProduct, " "); // add beginning space			
				strcat(finalProduct, leftSpace);
				strcat(finalProduct, " "); // add the ending space
			} else {
				strcpy(finalProduct, "("); // add beginning space			
				strcat(finalProduct, leftSpace);
				strcat(finalProduct, ")"); // add the ending space
			}

			printf(" %s |", finalProduct);

		}

}


/*
This function prints the final outcome of the desired results. It is called in the main function.
PRE: list must be a valid list
*/
static void PrintStatement(My402List* list) {

	printf("+-----------------+--------------------------+----------------+----------------+\n");
	printf("|       Date      | Description              |         Amount |        Balance |\n");
	printf("+-----------------+--------------------------+----------------+----------------+\n");

	My402ListElem *elem=NULL;
	int count = 0;	// keep track of each element read in
	int previous_amount = 0; // keep track of previous amount of BALANCE
	int current_amount=0; // keep track of CALCULATED BALANCE

	for (elem=My402ListFirst(list); elem != NULL; elem=My402ListNext(list, elem)) {
		count++;
 		TransactObj* elemVal = (TransactObj*)(elem->obj);
		
		printf("| ");

		// ------ convert the TIME from "Thu Aug 21 15:00:33 2008" to "Thu Aug 21 2008" ------
		int currTime = elemVal->transactTime; // TIME IN TRANS OBJECT
		time_t time = currTime;
		char* finalTime = ctime(&time);

		printf("%.*s", 11, finalTime);	// this will print the first 11 characters of the c-string
		printf("%.*s", 4, finalTime+20); // this will move to the 20th space of the character and print everything afterward
		printf("%s", " | ");
		

		// ------ convert the DESCRIPTION ------
		char* currDesc = elemVal->description; // DESCRIPTION IN TRANS OBJECT
		char finalDesc[25];
		strncpy(finalDesc, currDesc, sizeof(finalDesc));
		finalDesc[sizeof(finalDesc)-1] = '\0';

		int remain = 24 - strlen(finalDesc);
		for (int i=0; i<remain; i++) {
			strcat(finalDesc, " ");
		}
		printf("%s |", finalDesc);

	
		// ------ convert the AMOUNT ------
		char* thisType = elemVal->transactType; // TYPE IN TRANS OBJECT
		int amountCents = elemVal->cents; // AMOUNT IN CENTS
		FormatOutput(amountCents, thisType);


		// ------ convert the BALANCE ------
		if (count==1) {
			previous_amount = 0;
			current_amount = amountCents;

			// accounts for possibility of negative amount in the beginning			
			if (!strcmp(thisType, "-")) {
				FormatOutput(current_amount, "-");
				current_amount = amountCents*(-1);
			} else {
				FormatOutput(current_amount, "+");
			}
		} else {
			previous_amount = current_amount;
			current_amount = amountCents;

			if (!strcmp(thisType, "+")) {
				current_amount = previous_amount + amountCents;
			} else {
				current_amount = previous_amount - amountCents;
			}
			if (current_amount < 0) {
				FormatOutput(abs(current_amount), "-");
			} else {
				FormatOutput(abs(current_amount), "+");
			}
			
		}

		printf("\n");

	}

	printf("+-----------------+--------------------------+----------------+----------------+\n");

}


/* ----------------------- main() ----------------------- */


int main(int argc, char *argv[])
{
	// process command line arguments
	FILE* fp = ProcessCommandLine(argc, argv);
	if (fp==NULL) {
		return 0;	
	}

	My402List list;	

	// initialize My402List
	if (!My402ListInit(&list)) {
		fprintf(stderr, "ERROR: There was an error in initializing the list.\n");
		return 0;
	}

	// read in each line of file
	if (!ReadInput(fp, &list)) {
		fprintf(stderr, "ERROR: Input file is not in the right format.\n");
		return 0;
	}

	if (fp!=stdin) {
		fclose(fp);
	}
	
	SortInput(&list);
	PrintStatement(&list);

	return(0);
}
