/**
    @file

    @brief  Digital I/O Interrupts - Sample program that demonstrates how
            use the different interrupt modes available with the aDIO

    @verbatim
             This program allows a user to interactively control the interrupt
             modes of the aDIO.

             The user can issue a series of commands to control the operation
             of the aDIO device.

             Command List:

             MODE <MODE>         Set the interrupt mode of the aDIO.
                                <MODE> can be STROBE, EVENT, MATCH,
                                or DISABLED.

             COMPARE <VAL>      Set the compare register to <VAL>. (hex)

             MASK <VAL>         Set the value of the mask register (hex)

             PORT1 <VAL>       Set the Port 1 output to <VAL>. (hex)

             QUIT               Exit the program.

    @endverbatim

    @note   This file and its contents are copyright (C) RTD Embedded
            Technologies, Inc.  All Rights Reserved.

    @note   This software is licensed as described in the RTD End-User
            Software License Agreement.  For a copy of this agreement, refer
            to the file LICENSE.TXT (which should be included with this
            software) or contact RTD Embedded Technologies, Inc.

    $Id: ints.c 96302 2016-01-25 16:12:47Z rgroner $
 */
#include <errno.h>
#include <error.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>

#include "aDIO_library.h"

#define MAX_STORED_INTS     256
//uncomment the line below to get a print out of all interrupt info at
//the end of the program
//#define DEBUG_INT_QUEUE

#ifdef DEBUG_INT_QUEUE
int_status_t INT_ARRAY[MAX_STORED_INTS];	// save all interrupt info
int REMAIN_INT_ARRAY[MAX_STORED_INTS];	// to print later
unsigned int MISSED_INT_ARRAY[MAX_STORED_INTS];
#endif

#define ONE_MILLISEC	1000

//***********************
//*  Global Variables   *
//***********************

int ISR_ERRORS = 0;

/**
 * Global interrupt counter
 */
uint32_t INTERRUPT_COUNT = 0;

/**
 * Variable to hold the values of the ports
 */
uint8_t PortValue[2];

/**
 * Variable to hold the value of the compare register
 */
uint8_t CompareVal = 0x00;

/**
 * Variable to hold the mask value
 */
uint8_t MaskVal = 0x00;

/**
 * Device Handle
 */
DeviceHandle aDIO_Device;

/**
 * Pointer to program name
 */
static char *program_name_p;

/**
 * Variable used to hold the current interrupt count
 */
int Int_Received = 0;

/**
 * Function to print the program usage
 */
static void usage(void);

/**
 * Function to print the commands used in the program
 */
void PrintHelp();

/**
 * Function to print the values of the registers
 */
void PrintValues();

/**
 * User mode interrupt service routine.  This function is called
 * when an interrupt is recieved.
 */

void ISR(isr_info_t info)
{

	if (info.status) {
		ISR_ERRORS = info.status;
	} else {
		INTERRUPT_COUNT++;
		Int_Received = 1;
		// Save the value of Port 0
		ReadPort_aDIO(aDIO_Device, 0, &PortValue[0]);

		// Save the value of Port 1
		ReadPort_aDIO(aDIO_Device, 1, &PortValue[1]);

#ifdef DEBUG_INT_QUEUE
		if (INTERRUPT_COUNT < MAX_STORED_INTS) {

			INT_ARRAY[INTERRUPT_COUNT].int_count =
			    info.interrupt->int_count;
			INT_ARRAY[INTERRUPT_COUNT].port0 =
			    info.interrupt->port0;
			INT_ARRAY[INTERRUPT_COUNT].port1 =
			    info.interrupt->port1;
			INT_ARRAY[INTERRUPT_COUNT].compare =
			    info.interrupt->compare;
			INT_ARRAY[INTERRUPT_COUNT].control =
			    info.interrupt->control;

			REMAIN_INT_ARRAY[INTERRUPT_COUNT] =
			    info.remaining_interrupts;
			MISSED_INT_ARRAY[INTERRUPT_COUNT] =
			    info.missed_interrupts;

		}
#endif

	}

}

/**
@brief
    Main entry point for the application

@param
    argument_count

    Number of arguments

@param
    arguments_p_p

    Array of pointers to each argument

*/

int main(int argument_count, char **arguments_p_p)
{
	//------------------------------
	// Initialization Code
	//------------------------------

	// Variable to hold the minor number from the command line
	uint32_t minor_number;

	// Variable used to check the return value of the library functions
	int aDIO_ReturnVal;

	// Array used to hold the user command
	char CommandBuffer[20];

	// Pointer to the command string
	char *Command;

	// Pointer to the argument string
	char *Arg;

	// Variable to hold the numeric value of the argument
	uint8_t Arg_Val;

	// Array used to hold the individual bits of the mask value
	uint8_t Mask[8];

	// Variable to hold the current interrupt mode
	uint8_t IntMode;

	// Loop counter used to clear the command buffer
	int clear_count;

	// Quit Flag
	int Quit = 0;

	// Loop counter used to break the mask value into individual bits
	int Bit = 0;

	// Loop counter used to covert the command string to upper case
	int u = 0;

#ifdef DEBUG_INT_QUEUE
	int x = 0;
#endif

	program_name_p = arguments_p_p[0];

	fprintf(stderr, "\n");
	fprintf(stderr, "\taDIO ints example program\n");
	fprintf(stderr,
		"Copyright (c), RTD Embedded Techologies, Inc. All Rights Reserved\n");
	fprintf(stderr, "\n");

	//process command line options
	if (argument_count != 2) {
		fprintf(stderr, "ERROR:  Invalid number of options given!\n");
		usage();
	}

	if (sscanf(arguments_p_p[1], "%u", &minor_number) == 0) {
		fprintf(stderr, "ERROR:  Minor number must be an interger!\n");
		usage();
	}
	// Open the adio device and check the function return value
	aDIO_ReturnVal = OpenDIO_aDIO(&aDIO_Device, minor_number);
	if (aDIO_ReturnVal) {
		error(EXIT_FAILURE, errno, "ERROR:  OpenDIO_aDIO(%u) FAILED:"
		      " MinorNumber(= %u) maybe incorrect",
		      minor_number, minor_number);
	}
	//set all bits of Port 0 to input
	aDIO_ReturnVal =
	    LoadPort0BitDir_aDIO(aDIO_Device, 0, 0, 0, 0, 0, 0, 0, 0);

	usleep(ONE_MILLISEC);

	if (aDIO_ReturnVal) {
		error(EXIT_FAILURE, errno,
		      "ERROR:  LoadPort0bitDir_aDIO() FAILED");
	}
	//set Port 1 to output
	aDIO_ReturnVal = LoadPort1PortDir_aDIO(aDIO_Device, 1);

	usleep(ONE_MILLISEC);

	if (aDIO_ReturnVal) {
		error(EXIT_FAILURE, errno,
		      "ERROR:  LoadPort1PortDir_aDIO() FAILED");
	}

	/* Install the ISR, and check to make sure it
	   really worked */
	aDIO_ReturnVal = InstallISR_aDIO(aDIO_Device, ISR, SCHED_FIFO, 99);
	if (aDIO_ReturnVal) {
		error(EXIT_FAILURE, errno, "ERROR:  InstallISR_aDIO() FAILED");
	}

	PrintHelp();
	PrintValues();

	//Verify that aDIO interrupts are enabled in the BIOS
	if (ISR_ERRORS != 0) {
		printf("\nERROR: ISR recieved an error code of %d from the ISR "
		       "call back function\n", ISR_ERRORS);
		if (ISR_ERRORS == -EIO) {
			printf("%d indicates that interrupts are disabled.\n",
			       ISR_ERRORS);
			printf
			    ("Be sure the aDIO interrupt is enabled in the BIOS.\n");
			Quit = 1;
		}
	}
	//------------------------------
	// Process the Commands
	//------------------------------

	while (!Quit) {
		//Clear the command buffers
		for (clear_count = 0; clear_count < 20; clear_count++) {
			CommandBuffer[clear_count] = 0x00;
		}

		Command = NULL;
		Arg = NULL;

		printf("\n>");

		if (fgets(CommandBuffer, 19, stdin) == NULL) {
			fprintf(stderr, "Invalid Command\n");
			continue;
		}
		// Convert strings to uppercase to make the code case in-sensative
		for (u = 0; u < 20; u++) {
			CommandBuffer[u] = toupper(CommandBuffer[u]);
		}

		// Break up the command string into the command and argument
		Command = strtok(CommandBuffer, " \n");

		if (Command == NULL) {
			PrintValues();
			continue;
		}

		Arg = strtok(NULL, " \n");

		if (strcmp(Command, "QUIT") == 0) {
			// Set the quit flag
			Quit = 1;
			continue;
		} else if (strcmp(Command, "HELP") == 0) {
			PrintHelp();
		} else if (strcmp(Command, "PORT1") == 0) {
			// Convert the data and write it to Port 1
			if (Arg != NULL) {
				Arg_Val = (uint8_t) strtoul(Arg, NULL, 16);
			} else {
				fprintf(stderr, "Invalid Argument\n");
				continue;
			}

			aDIO_ReturnVal =
			    WritePort_aDIO(aDIO_Device, 1, Arg_Val);

			usleep(ONE_MILLISEC);

			// Check the return value
			if (aDIO_ReturnVal) {
				error(EXIT_FAILURE, errno,
				      "ERROR:  WritePort_aDIO() FAILED");
			}
		} else if (strcmp(Command, "MODE") == 0) {
			if (Arg == NULL) {
				fprintf(stderr, "Invalid Argument\n");
				continue;
			}

			if (strcmp(Arg, "DISABLED") == 0) {
				//Disable interrupts

				aDIO_ReturnVal =
				    EnableInterrupts_aDIO(aDIO_Device,
							  DISABLE_INT_MODE);

				usleep(ONE_MILLISEC);

				if (aDIO_ReturnVal) {
					RemoveISR_aDIO(aDIO_Device);
					error(EXIT_FAILURE, errno,
					      "ERROR:  EnableInterrupts_aDIO() FAILED");
				}

				GetInterruptMode_aDIO(aDIO_Device, &IntMode);
				if (IntMode != DISABLE_INT_MODE) {
					RemoveISR_aDIO(aDIO_Device);
					error(EXIT_FAILURE, errno,
					      "ERROR:  GetInterruptMode_aDIO() FAILED");
				}
			} else if (strcmp(Arg, "MATCH") == 0) {
				//enable match mode
				aDIO_ReturnVal =
				    EnableInterrupts_aDIO(aDIO_Device,
							  MATCH_INT_MODE);

				usleep(ONE_MILLISEC);

				if (aDIO_ReturnVal) {
					RemoveISR_aDIO(aDIO_Device);
					error(EXIT_FAILURE, errno,
					      "ERROR:  EnableInterrupts_aDIO() FAILED");
				}

				GetInterruptMode_aDIO(aDIO_Device, &IntMode);
				if (IntMode != MATCH_INT_MODE) {
					RemoveISR_aDIO(aDIO_Device);
					error(EXIT_FAILURE, errno,
					      "ERROR:  GetInterruptMode_aDIO() FAILED");
				}

			} else if (strcmp(Arg, "EVENT") == 0) {
				//enable event mode
				aDIO_ReturnVal =
				    EnableInterrupts_aDIO(aDIO_Device,
							  EVENT_INT_MODE);

				usleep(ONE_MILLISEC);

				if (aDIO_ReturnVal) {
					RemoveISR_aDIO(aDIO_Device);
					error(EXIT_FAILURE, errno,
					      "ERROR:  EnableInterrupts_aDIO() FAILED");
				}

				GetInterruptMode_aDIO(aDIO_Device, &IntMode);
				if (IntMode != EVENT_INT_MODE) {
					RemoveISR_aDIO(aDIO_Device);
					error(EXIT_FAILURE, errno,
					      "ERROR:  GetInterruptMode_aDIO() FAILED");
				}

			} else if (strcmp(Arg, "STROBE") == 0) {
				//enable strobe mode
				aDIO_ReturnVal =
				    EnableInterrupts_aDIO(aDIO_Device,
							  STROBE_INT_MODE);

				usleep(ONE_MILLISEC);

				if (aDIO_ReturnVal) {
					RemoveISR_aDIO(aDIO_Device);
					error(EXIT_FAILURE, errno,
					      "ERROR:  EnableInterrupts_aDIO() FAILED");
				}

				GetInterruptMode_aDIO(aDIO_Device, &IntMode);
				if (IntMode != STROBE_INT_MODE) {
					RemoveISR_aDIO(aDIO_Device);
					error(EXIT_FAILURE, errno,
					      "ERROR:  GetInterruptMode_aDIO() FAILED");
				}
			} else {
				fprintf(stderr, "Invalid Argument\n");
				continue;
			}
		} else if (strcmp(Command, "MASK") == 0) {

			// Convert the argument into numeric value
			if (Arg != NULL) {
				MaskVal = (uint8_t) strtoul(Arg, NULL, 16);
			} else {
				fprintf(stderr, "Invalid Argument\n");
				continue;
			}

			// Break the value into bits
			for (Bit = 0; Bit < 8; Bit++) {
				Mask[Bit] = (MaskVal >> Bit) & 0x01;
			}

			aDIO_ReturnVal = LoadMask_aDIO(aDIO_Device,
						       Mask[7], Mask[6],
						       Mask[5], Mask[4],
						       Mask[3], Mask[2],
						       Mask[1], Mask[0]);
			usleep(ONE_MILLISEC);

			if (aDIO_ReturnVal) {
				RemoveISR_aDIO(aDIO_Device);
				error(EXIT_FAILURE, errno,
				      "ERROR:  LoadMask_aDIO() FAILED");
			}

		} else if (strcmp(Command, "COMPARE") == 0) {
			if (Arg == NULL) {
				fprintf(stderr, "Invalid Argument\n");
				continue;
			}
			//Convert the argument into numeric value
			CompareVal = (uint8_t) strtoul(Arg, NULL, 16);

			//load the compare register with the value
			//and check the function return value
			aDIO_ReturnVal = LoadComp_aDIO(aDIO_Device, CompareVal);

			usleep(ONE_MILLISEC);

			if (aDIO_ReturnVal) {
				RemoveISR_aDIO(aDIO_Device);
				error(EXIT_FAILURE, errno,
				      "ERROR:  LoadComp_aDIO() FAILED");
			}
		} else {
			fprintf(stderr, "Invalid Command\n");
			continue;
		}

		//wait for an interrupt
		usleep(500);

		//Print the values to the screen after each command
		PrintValues();

	};
	//------------------------------
	// All done
	//------------------------------

	EnableInterrupts_aDIO(aDIO_Device, DISABLE_INT_MODE);

	usleep(ONE_MILLISEC);

	RemoveISR_aDIO(aDIO_Device);

	//Close the aDIO device and exit the program
	aDIO_ReturnVal = CloseDIO_aDIO(aDIO_Device);
	if (aDIO_ReturnVal) {
		printf("Error while closing ADIO = %d\n", aDIO_ReturnVal);
	}
	//------------------------------
	// Print all interrupt info
	//------------------------------

#ifdef DEBUG_INT_QUEUE
	printf
	    ("Below is a list of the status of each register during each interrupt\n");
	for (x = 0; (x < MAX_STORED_INTS) && (x <= INTERRUPT_COUNT); x++) {
		printf
		    ("count:%d port0:%2x port1:%2x compare:%2x control:%2x queued_ints:%d missed_ints:%d\n",
		     INT_ARRAY[x].int_count, INT_ARRAY[x].port0,
		     INT_ARRAY[x].port1, INT_ARRAY[x].compare,
		     INT_ARRAY[x].control, REMAIN_INT_ARRAY[x],
		     MISSED_INT_ARRAY[x]

		    );
	}
#endif

	return 0;
}

/**
 *  Reads the port values and prints them on the screen.
 */
void PrintValues()
{
	char *ModeStr = "        ";
	unsigned char Mode;
	int aDIO_ReturnVal;

	if (Int_Received) {	//if interrupt occurred

		printf("\n");
		printf("                    ****************************\n");
		printf("                    *    INTERRUPT RECEIVED    *\n");
		printf("                    *                          *\n");
		printf("                    *      Port 0 = 0x%02x       *\n",
		       PortValue[0]);
		printf("                    *      Port 1 = 0x%02x       *\n",
		       PortValue[1]);
		printf("                    *                          *\n");
		printf("                    ****************************\n");
		printf("\n");
		//Set the local variable to the global variable to be able to see
		//if a new interrupt occurred.

		Int_Received = 0;
	}
	//Read the value from Port 0
	aDIO_ReturnVal = ReadPort_aDIO(aDIO_Device, 0, &PortValue[0]);
	if (aDIO_ReturnVal) {
		error(EXIT_FAILURE, errno, "ERROR:  ReadPort_aDIO() FAILED");
	}
	//Read the value from Port 1
	aDIO_ReturnVal = ReadPort_aDIO(aDIO_Device, 1, &PortValue[1]);
	if (aDIO_ReturnVal) {
		error(EXIT_FAILURE, errno, "ERROR:  ReadPort_aDIO() FAILED");
	}
	//Read the value of the compare register
	aDIO_ReturnVal = ReadComp_aDIO(aDIO_Device, &CompareVal);
	if (aDIO_ReturnVal) {
		error(EXIT_FAILURE, errno, "ERROR:  ReadComp_aDIO() FAILED");
	}
	//Get the interrupt mode
	aDIO_ReturnVal = GetInterruptMode_aDIO(aDIO_Device, &Mode);
	if (aDIO_ReturnVal) {
		error(EXIT_FAILURE, errno,
		      "ERROR:  GetInterruptMode_aDIO() FAILED");
	}

	if (Mode == DISABLE_INT_MODE) {
		ModeStr = "Disabled";
	} else if (Mode == STROBE_INT_MODE) {
		ModeStr = "Strobe  ";
	} else if (Mode == EVENT_INT_MODE) {
		ModeStr = "Event   ";
	} else if (Mode == MATCH_INT_MODE) {
		ModeStr = "Match   ";
	}

	printf("\n\n");
	printf
	    ("                                             CURRENT        TOTAL\n");
	printf
	    ("PORT 0     PORT 1     COMPARE     MASK      INTERRUPT     INTERRUPT\n");
	printf
	    ("INPUT      OUTPUT      VALUE    REGISTER       MODE         COUNT\n");
	printf
	    ("----------------------------------------------------------------------\n");
	printf
	    ("0x%02x        0x%02x       0x%02x       0x%02x        %s        %d\n",
	     PortValue[0], PortValue[1], CompareVal, MaskVal, ModeStr,
	     INTERRUPT_COUNT);
	printf("\nPress ENTER to refresh the screen.\n");

}

/**
 *  Displays command list on the screen.
 */
void PrintHelp()
{
	printf("\n");
	printf("Command List: \n");
	printf("    MODE <INTMODE>      Set the interrupt mode of the aDIO\n");
	printf
	    ("                        Valid modes are STROBE, EVENT, MATCH,\n");
	printf("                        DISABLED.\n");
	printf("    PORT1 <VAL>         Set the output value of Port 1.\n");
	printf
	    ("    COMPARE <VAL>       Set the value of the Compare Register.\n");
	printf("    MASK <VAL>          Set the value of the Mask Register.\n");
	printf("    HELP                Print the command list.\n");
	printf("    QUIT                Exit the program.\n");

}

/**
 *  Display program usage instructions
 */
static void usage(void)
{
	fprintf(stderr, "\n");
	fprintf(stderr, "%s\n", program_name_p);
	fprintf(stderr, "\n");
	fprintf(stderr, "Usage: %s MinorNumber\n", program_name_p);
	fprintf(stderr, "\n");
	fprintf(stderr, "    MinorNumber:    aDIO device file minor number.\n");
	fprintf(stderr, "\n");
	exit(EXIT_FAILURE);
}
