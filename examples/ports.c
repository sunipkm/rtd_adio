/**
    @file

    @brief  Digital I/O Ports - Sample program that demonstrates how
            to write and read values to/from from the digital I/O ports.

    @verbatim
             This program allows a user to read and write
             values to the aDIO ports.

             The user can issue a series of commands to set the direction
             and output of the digital I/O ports.

             Command List:

             PD1 <DIR>                  Sets the direction of Port 1
                                       <DIR> can be INPUT or OUTPUT.

             BD0 <VAL>                  Sets the direction of the
                                        individual bits of Port 0
                                        <VAL> must be in hex.

             PORT0 <VAL>                Writes the specified value
                                        to Port 0.

             PORT1 <VAL>                Writes the specified value
                                        to Port 1.

             HELP                       Prints the command list to screen.

             QUIT                       Exit the program.

    @endverbatim

    @note   This file and its contents are copyright (C) RTD Embedded
            Technologies, Inc.  All Rights Reserved.

    @note   This software is licensed as described in the RTD End-User
            Software License Agreement.  For a copy of this agreement, refer
            to the file LICENSE.TXT (which should be included with this
            software) or contact RTD Embedded Technologies, Inc.

    $Id: ports.c 72139 2013-08-08 19:33:15Z rgroner $
 */
#include <errno.h>
#include <error.h>
#include <stdio.h>
#include <ctype.h>
#include <unistd.h>

#include "aDIO_library.h"

#define ONE_MILLISEC	1000

/**
 * Gloabal device structure
 */
DeviceHandle aDIO_Device;

/**
 * Global array to hold the output values of the ports
 */
uint8_t PortValue[2];

/**
 * Global array to hold the value of the strobe pins
 */
uint8_t Strobe[2];

/**
 * Global variable to hold the direction of port 0
 */
uint8_t P0Direction = 0;

/**
 * Character string used to print the displayd
 */
char *P1Direction = "Output";

/**
 * Character string used to print the displayd
 */
char *Str0 = "High";

/**
 * Character string used to print the displayd
 */
char *Str1 = "High";

/**
 * Pointer to the program name string
 */
static char *program_name;

/**
 * Print usage function
 */
static void usage(void);

/**
 * Function to print the command list
 */
void PrintHelp();

/**
 * Function to print the program output
 */
void PrintValues();

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
	uint32_t minor_number;

	// variable to hold the library function return values
	int aDIO_ReturnVal;

	// buffer to hold the user commands
	char CommandBuffer[20];

	// pointer to the command string
	char *Command;

	// pointer to the argument string
	char *Arg;

	// variable to hold the numeric value of the arugment
	uint8_t Arg_Val;

	// variable to hold the individual bit direction of port 0
	uint8_t P0Bits[8];

	// loop counter used to clear the command buffer
	int clear_count;

	// Quit Flag
	int Quit = 0;

	// loop counter used to break the port 0 direction into individual bits
	int Bit = 0;

	// loop counter used to convert the command string to uppercase
	int u = 0;

	program_name = arguments_p_p[0];

	fprintf(stderr, "\n");
	fprintf(stderr, "    aDIO Ports Example Program\n");
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
	//initialize board
	aDIO_ReturnVal = OpenDIO_aDIO(&aDIO_Device, minor_number);
	if (aDIO_ReturnVal) {
		error(EXIT_FAILURE, errno,
		      "ERROR:  OpenDIO_aDIO(%u) FAILED: MinorNumber(= %u) maybe incorrect",
		      minor_number, minor_number);
	}
	//------------------------------
	// Process the Commands
	//------------------------------

	PrintHelp();
	PrintValues();

	do {
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
		} else if (strcmp(Command, "PORT0") == 0) {
			if (Arg == NULL) {
				fprintf(stderr, "Invalid Argument\n");
				continue;
			}
			// Convert the data and write it to Port 0
			Arg_Val = (uint8_t) strtoul(Arg, NULL, 16);

			aDIO_ReturnVal =
			    WritePort_aDIO(aDIO_Device, 0, Arg_Val);

			usleep(ONE_MILLISEC);

			// Check the return value
			if (aDIO_ReturnVal) {
				error(EXIT_FAILURE, errno,
				      "ERROR:  WritePort_aDIO() FAILED");
			}

		} else if (strcmp(Command, "PORT1") == 0) {
			if (Arg == NULL) {
				fprintf(stderr, "Invalid Argument\n");
				continue;
			}
			// Convert the data and write it to Port 1

			Arg_Val = (uint8_t) strtoul(Arg, NULL, 16);

			aDIO_ReturnVal =
			    WritePort_aDIO(aDIO_Device, 1, Arg_Val);

			usleep(ONE_MILLISEC);

			// Check the return value
			if (aDIO_ReturnVal) {
				error(EXIT_FAILURE, errno,
				      "ERROR:  WritePort_aDIO() FAILED");
			}
		} else if (strcmp(Command, "BD0") == 0) {
			if (Arg == NULL) {
				fprintf(stderr, "Invalid Argument\n");
				continue;
			}

			Arg_Val = (uint8_t) strtoul(Arg, NULL, 16);

			// Break the value into bits
			for (Bit = 0; Bit < 8; Bit++) {
				P0Bits[Bit] = (Arg_Val >> Bit) & 0x01;
			}

			// Set bits of Port 0 to the specified directions
			aDIO_ReturnVal =
			    LoadPort0BitDir_aDIO(aDIO_Device, P0Bits[7],
						 P0Bits[6], P0Bits[5],
						 P0Bits[4], P0Bits[3],
						 P0Bits[2], P0Bits[1],
						 P0Bits[0]);

			usleep(ONE_MILLISEC);

			// Check the return value
			if (aDIO_ReturnVal) {
				error(EXIT_FAILURE, errno,
				      "ERROR:  LoadPort0bitDir_aDIO() FAILED");
			}

			P0Direction = Arg_Val;
		} else if (strcmp(Command, "PD1") == 0) {
			if (Arg == NULL) {
				fprintf(stderr, "Invalid Argument\n");
				continue;
			}
			// Compare the argument, set Port 1 direction and check the return value
			if (strcmp(Arg, "INPUT") == 0) {
				aDIO_ReturnVal =
				    LoadPort1PortDir_aDIO(aDIO_Device, 0);

				usleep(ONE_MILLISEC);

				if (aDIO_ReturnVal) {
					error(EXIT_FAILURE, errno,
					      "ERROR:  LoadPort1PortDir_aDIO() FAILED");
				}
			} else if (strcmp(Arg, "OUTPUT") == 0) {
				aDIO_ReturnVal =
				    LoadPort1PortDir_aDIO(aDIO_Device, 1);

				usleep(ONE_MILLISEC);

				if (aDIO_ReturnVal) {
					error(EXIT_FAILURE, errno,
					      "ERROR:  LoadPort1PortDir_aDIO() FAILED");
				}
			} else {
				fprintf(stderr, "Invalid Argument\n");
				continue;
			}
		} else {
			fprintf(stderr, "Invalid Command\n");
			continue;
		}

		//Print the values to the screen after each command
		PrintValues();

	} while (!Quit);

	//------------------------------
	// All done
	//------------------------------

	//Close the aDIO device and exit the program
	aDIO_ReturnVal = CloseDIO_aDIO(aDIO_Device);
	if (aDIO_ReturnVal) {
		printf("Error while closing ADIO = %d\n", aDIO_ReturnVal);
	}

	return 0;
}

/**
 *  Display program usage instructions
 */

static void usage(void)
{
	fprintf(stderr, "\n");
	fprintf(stderr, "%s\n", program_name);
	fprintf(stderr, "\n");
	fprintf(stderr, "Usage: %s MinorNumber\n", program_name);
	fprintf(stderr, "\n");
	fprintf(stderr, "    MinorNumber:    aDIO device file minor number.\n");
	fprintf(stderr, "\n");

	exit(EXIT_FAILURE);
}

/**
 *  Displays command list on the screen.
 */

void PrintHelp()
{
	printf("\n");
	printf("Command list:\n");
	printf("    PORT0 <VAL>     Set the output value of Port 0.\n");
	printf("    PORT1 <VAL>     Set the output value of Port 1.\n");
	printf("    BD0 <VAL>       Set the bit-direction of Port 0.\n");
	printf("    PD1 <VAL>       Set the port direction of Port 1.\n");
	printf("    HELP            Prints this command list.\n");
	printf("    QUIT            Exit the program.\n");
}

/**
 *  Reads the port values and prints them on the screen.
 */

void PrintValues()
{
	int aDIO_ReturnVal;
	uint8_t ControlReg;

	int i = 0;

	//Read the values of Port 0 and Port 1
	for (i = 0; i < 2; i++) {
		aDIO_ReturnVal = ReadPort_aDIO(aDIO_Device, i, &PortValue[i]);
		if (aDIO_ReturnVal) {
			error(EXIT_FAILURE, errno,
			      "ERROR:  ReadPort_aDIO() FAILED");
		}
	}

	//Get the direction of Port 1
	//aDIO_ReturnVal = InPort(aDIO_Device->hDevice, rCONTROL, &ControlReg);
	aDIO_ReturnVal = ReadControlRegister_aDIO(aDIO_Device, &ControlReg);
	if (aDIO_ReturnVal) {
		error(EXIT_FAILURE, errno, "ERROR:  InPort() FAILED");
	}

	if ((ControlReg & 0x04) == 0) {
		P1Direction = "Input ";
	} else {
		P1Direction = "Output";
	}

	//Read Strobe 0
	aDIO_ReturnVal = ReadStrobe0_aDIO(aDIO_Device, &Strobe[0]);
	if (aDIO_ReturnVal) {
		error(EXIT_FAILURE, errno, "ERROR:  ReadStrobe0_aDIO() FAILED");
	}

	if (Strobe[0] == 0) {
		Str0 = "Low ";
	} else {
		Str0 = "High";
	}

	//Read Strobe 1
	aDIO_ReturnVal = ReadStrobe1_aDIO(aDIO_Device, &Strobe[1]);
	if (aDIO_ReturnVal)
		error(EXIT_FAILURE, errno, "ERROR:  ReadStrobe1_aDIO() FAILED");

	if (Strobe[1] == 0) {
		Str1 = "Low ";
	} else {
		Str1 = "High";
	}

	// actually print the values to the screen

	printf("\n\n");
	printf
	    ("               PORT 0        PORT 1        STROBE 0        STROBE 1\n");
	printf
	    ("             --------------------------------------------------------\n");
	printf
	    ("Direction      0x%02x          %s                                   \n",
	     P0Direction, P1Direction);
	printf
	    ("Value Read     0x%02x          0x%02x            %s            %s    \n",
	     PortValue[0], PortValue[1], Str0, Str1);
	printf("\nPress ENTER to refresh screen.\n");

}
