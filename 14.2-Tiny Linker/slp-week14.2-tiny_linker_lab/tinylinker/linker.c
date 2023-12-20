
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "linker.h"
FILE *in;   //for read module
FILE *out;  //for output link result

#define MACHINE_SIZE 600

int NM; // number of modules
int ND; // number of definitions;
int NU; // number of uses;

void printSymbolTable();
void printOutput();
void printWarnings();
void printRawData();

struct SymbolTableEntry
{
    char *key;
    int absoluteAddress;
    char message[256];
    int module; // the module in which the symbol is defined
};
struct Definition
{
    char *S;
    int R;
    char *message;
};

struct Use
{
   char *S;
   int R;
   char *message;
   char flag; // if multiple symbols are listed as used in the same instruction ignore all but last usage. flag = 1 proccess usage / flag = 0 do not process
};

struct Word
{
    int input; // 5 digit
    int word; // first 4 digit
    int address; // middle 3 digit
    int opcode; // first digit
    int type; // last digit
    char *message;
    int flag; // must be 1 if type is 4 (external address) and it is on a use list
};
struct Module
{
    struct Definition *DL;
    int ND;
    struct Use *UL;
    int NU;
    struct Word *PT;
    int NT;
    int baseAddress;
};

struct Module *modules;

struct SymbolTableEntry ST[256];
int indexST = 0;


char message1[] = "Error: Immediate address on use list; treated as External.";
char message2[] = "Error: E type address not on use chain; treated as I type.";
char message3[] = "Error: Absolute address exceeds the size of the machine. Largest legal value used.";
char message4[] = "Error: Multiple symbols defined for this instruction. Last given usage used.";

char *warnings[256];
int indexWarning = 0;

/*offset of the current moudle
  assume current module is I+1, the offset of I+1
  will be LENGTH(0) + LENGTH(1) + ...... LENGTH(I)
  where the LENGTH(I) reprent the length of module I
*/
int offset = 0;

/*you can modify this function!*/

/*
  test_num: which file test, it must between 1 and 9
  filename: the file that you should do with
*/

void readData()
{
    int base = 0, instruction, i, j, k;
    char buffer[256];
    int R;
    scanf("%d", &NM);

    modules = (struct Module*)malloc(NM * sizeof(struct Module));

    for(i = 0; i < NM; i++)
    {

        // DEFINITIONS
        modules[i].baseAddress = base; // set base address of module
        scanf("%d", &modules[i].ND); // read number of definitions

        modules[i].DL = (struct Definition*)malloc(modules[i].ND * sizeof(struct Definition));
        for(j = 0; j < modules[i].ND; j++)
        {
            scanf("%s %d", buffer, &R);
            modules[i].DL[j].R = R;
            modules[i].DL[j].S =  (char *)malloc(strlen(buffer) + 1);
            strcpy(modules[i].DL[j].S, buffer);
            modules[i].DL[j].message = NULL;
        }


        // USES
        scanf("%d", &modules[i].NU);
        modules[i].UL = (struct Use*)malloc(modules[i].NU * sizeof(struct Use));
        for(j = 0; j < modules[i].NU; j++)
        {
            scanf("%s %d", buffer, &R);
            modules[i].UL[j].R = R;
            modules[i].UL[j].S =  (char *)malloc(strlen(buffer) + 1);
            strcpy(modules[i].UL[j].S, buffer);
            modules[i].UL[j].message = NULL;
            modules[i].UL[j].flag = 1;
        }



        // PROGRAM TEXT
        scanf("%d", &modules[i].NT);
        base += modules[i].NT;
        modules[i].PT = (struct Word*)malloc(modules[i].NT * sizeof(struct Word));
        for(j = 0; j < modules[i].NT; j++)
        {

            scanf("%d", &instruction);
            modules[i].PT[j].input = instruction;
            modules[i].PT[j].opcode = instruction / 10000;
            modules[i].PT[j].type = instruction % 10;
            modules[i].PT[j].word = instruction / 10;
            modules[i].PT[j].address = instruction / 10 % 1000;
            modules[i].PT[j].message = NULL;
            modules[i].PT[j].flag = 0;

            // check if absoulute address exceeds machine size ???
            //if(modules[i].PT[j].type == 2 && modules[i].PT[j].address >= MACHINE_SIZE)
            //{
            //    modules[i].PT[j].address = MACHINE_SIZE - 1;
            //    modules[i].PT[j].message = (char*)malloc(strlen(message3) + 1);
            //    strcpy(modules[i].PT[j].message, message3);
            //}

        }
        // check uses that must be proccessed;
        // if multiple symbols are listed as used in the same instruction ignore all but last usage given. print error message.
        for(j = modules[i].NU - 1; j > 0; j--)
            for(k = j - 1; k >= 0; k--)
                if(modules[i].UL[j].R == modules[i].UL[k].R)
                {
                    modules[i].UL[k].flag = 0;
                    modules[i].PT[modules[i].UL[k].R].message = (char *)malloc(strlen(message4) + 1);
                    strcpy(modules[i].PT[modules[i].UL[k].R].message, message4);
                }
    }

}


void createSymbolTable()
{
    int found = 0, k;
    for(int i = 0; i < NM; i++)
    {
        for(int j = 0; j < modules[i].ND; j++)
        {
            found = 0;
            for(k = 0; k < indexST; k++)
                if(strcmp(ST[k].key, modules[i].DL[j].S) == 0) // check if symbol is already defined
                {
                    strcpy(ST[k].message, "Error: This variable is multiply defined; first value used. ");
                    found = 1;
                    break;
                }
            if(!found)
            {
                ST[indexST].key = (char *)malloc(strlen(modules[i].DL[j].S) + 1);
                strcpy(ST[indexST].key, modules[i].DL[j].S);
                ST[indexST].absoluteAddress = modules[i].DL[j].R + modules[i].baseAddress;
                if(modules[i].DL[j].message == NULL)
                    strcpy(ST[indexST].message, "");
                else
                    strcpy(ST[indexST].message, modules[i].DL[j].message);

                ST[indexST].module = i;
                indexST++;
            }

        }
    }
}


// check if address in definitions exceeds the size of module.
// if so, print error message and treat the address as 0 (relative)
void checkDefinitions()
{
    char buffer[256];
    for(int i = 0; i < NM; i++)
    {
        for(int j = 0; j < modules[i].ND; j++)
            if(modules[i].DL[j].R >= modules[i].NT)
            {
                modules[i].DL[j].R = 0;
                sprintf(buffer, "Error: The definition of %s is outside module %d; zero (relative) used. ", modules[i].DL[j].S, i);
                modules[i].DL[j].message = (char *)malloc(sizeof(buffer) + 1);
                strcpy(modules[i].DL[j].message, buffer);
            }

    }
}


//check if there are defined symbols that are not used;
// if so we add a message to the warning list
void checkUnusedSymbols()
{
    char buffer[256];
    int found = 0;
    for(int i = 0; i < indexST; i++)
    {
        found  = 0;
        for(int j = 0; !found && j < NM; j++)
            for(int k = 0; !found && k < modules[j].NU; k++)
                if(strcmp(ST[i].key, modules[j].UL[k].S) == 0)
                    found  = 1;

       if(!found)
       {
           sprintf(buffer, "Warning: %s was defined in module %d but never used.", ST[i].key, ST[i].module);
           warnings[indexWarning] = (char *)malloc(strlen(buffer) + 1);
           strcpy(warnings[indexWarning], buffer);
           indexWarning++;
       }
    }

}

void printSymbolTable()
{
    printf("Symbol Table\n");
    for(int i = 0; i < indexST; i++)
        printf("%s=%d %s\n", ST[i].key, ST[i].absoluteAddress, ST[i].message);
}

void printOutput()
{
    int counter = 0;
    printf("\nMemory Map \n");
    for(int i = 0; i < NM; i++)
        for(int j = 0; j < modules[i].NT; j++)
        {
            printf("%d: %d%03d", counter++, modules[i].PT[j].opcode, modules[i].PT[j].address);
            if( modules[i].PT[j].message != NULL)
                printf(" %s\n", modules[i].PT[j].message);
            else
                printf("\n");

        }


}

void printWarnings()
{
    printf("\n");
    for(int i = 0; i < indexWarning; i++)
        printf("%s\n", warnings[i]);

}


void printRawData()
{
    for(int i = 0; i < NM; i++)
    {
        printf("Module %d\n", i + 1);
        printf("Definition List:\n");
        for(int j = 0; j < modules[i].ND; j++)
            printf("\t%s %d\n", modules[i].DL[j].S, modules[i].DL[j].R);
        printf("Use List:\n");
        for(int j = 0; j < modules[i].NU; j++)
            printf("\t%s %d\n", modules[i].UL[j].S, modules[i].UL[j].R);
        printf("Program Text:\n");
        for(int j = 0; j < modules[i].NT; j++)
            printf("\t%d %03d %d\n", modules[i].PT[j].opcode, modules[i].PT[j].address, modules[i].PT[j].type);
    }
}

void link(int test_num, const char *filename)
{
	char outputfile[10];
	memset(outputfile, 0, 10);
	sprintf(outputfile, "output-%d.txt", test_num);
	in = fopen(filename, "r");    //open file for read,the file contains module that you should do with
	out = fopen(outputfile, "w"); //open the "output" file to output link result
	if(in==NULL||out==NULL)
	{
		fprintf(stderr, "can not open file for read or write\n");
		exit(-1);
	}

    readData();
    checkDefinitions();
    createSymbolTable();
    printSymbolTable();
    checkUnusedSymbols();

	process_one(); //resolve
	process_two(); //relocate

	printOutput();
    printWarnings();

	fclose(in);
	fclose(out);
}


/*you must implement this function*/
void process_one()
{
	char buffer[256];
    int usedAddress = 0, nextUse, found, temp;

    buffer[0] = '\0';
    for(int i = 0; i < NM; i++)
    {

        for(int j = 0; j < modules[i].NU; j++)
        {
            if(modules[i].UL[j].flag == 1)
            {
                // check if symbol is defined
                found = 0;
                buffer[0] = '\0';
                for(int k = 0; !found && k < indexST; k++)
                    if(strcmp(modules[i].UL[j].S, ST[k].key) == 0)
                    {
                        found = 1;
                        usedAddress = ST[k].absoluteAddress;
                    }

                if(!found)
                {
                    sprintf(buffer, " Error: %s is not defined; zero used.", modules[i].UL[j].S);
                    usedAddress = 0;

                }

                nextUse = modules[i].UL[j].R;

                int count = 0;
                while(count < modules[i].NT && nextUse != 777)
                {
                    count++;
                    if(modules[i].PT[nextUse].type == 4 || modules[i].PT[nextUse].type == 3 || modules[i].PT[nextUse].type == 1)
                    {
                        temp = modules[i].PT[nextUse].address;

                        if(!found)
                        {
                            modules[i].PT[nextUse].message = (char *)malloc(strlen(buffer) +  1);
                            strcpy(modules[i].PT[nextUse].message, buffer);
                        }

                        if(modules[i].PT[nextUse].type == 1)
                        {
                            modules[i].PT[nextUse].message = (char *)malloc(strlen(message1) +  1);
                            strcpy(modules[i].PT[nextUse].message, message1);
                        }
                        modules[i].PT[nextUse].address = usedAddress;
                        modules[i].PT[nextUse].flag = 1; // external address is on use list
                        nextUse = temp;
                    }

                }

            }


        }

        // check if all external addresses in current module are on use list. if not print error message an treat de address as immediate
        for(int j = 0; j < modules[i].NT; j++)
        {
            if(modules[i].PT[j].type == 4 && modules[i].PT[j].flag == 0)
            {
                modules[i].PT[j].message = (char *)malloc(strlen(message2) +  1);
                strcpy(modules[i].PT[j].message, message2);
            }
        }
    }
}

/*you must implement this function*/
void process_two()
{
    for(int i = 0; i < NM; i++)
    {
        for(int j = 0; j < modules[i].NT; j++)
            if(modules[i].PT[j].type == 3)
                modules[i].PT[j].address += modules[i].baseAddress;
    }
}
	

