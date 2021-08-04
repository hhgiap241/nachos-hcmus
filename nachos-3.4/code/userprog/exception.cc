// exception.cc 
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.  
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "syscall.h"

//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2. 
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//	"which" is the kind of exception.  The list of possible exceptions 
//	are in machine.h.
//----------------------------------------------------------------------
void IncreasePC()
{
	int counter = machine->ReadRegister(PCReg);
   	machine->WriteRegister(PrevPCReg, counter);
    	counter = machine->ReadRegister(NextPCReg);
    	machine->WriteRegister(PCReg, counter);
   	machine->WriteRegister(NextPCReg, counter + 4);
}
char* User2System(int virtAddr, int limit)
{
	int i; //chi so index
	int oneChar;
	char* kernelBuf = NULL;
	kernelBuf = new char[limit + 1]; //can cho chuoi terminal
	if (kernelBuf == NULL)
		return kernelBuf;
		
	memset(kernelBuf, 0, limit + 1);
	
	for (i = 0; i < limit; i++)
	{
		machine->ReadMem(virtAddr + i, 1, &oneChar);
		kernelBuf[i] = (char)oneChar;
		if (oneChar == 0)
			break;
	}
	return kernelBuf;
}


// Input: Khong gian vung nho User(int) - gioi han cua buffer(int) - bo nho dem buffer(char*)
// Output: So byte da sao chep(int)
// Chuc nang: Sao chep vung nho System sang vung nho User
int System2User(int virtAddr, int len, char* buffer)
{
	if (len < 0) return -1;
	if (len == 0)return len;
	int i = 0;
	int oneChar = 0;
	do{
		oneChar = (int)buffer[i];
		machine->WriteMem(virtAddr + i, 1, oneChar);
		i++;
	} while (i < len && oneChar != 0);
	return i;
}
void
ExceptionHandler(ExceptionType which)
{
    int type = machine->ReadRegister(2);

//    if ((which == SyscallException) && (type == SC_Halt)) {
//	DEBUG('a', "Shutdown, initiated by user program.\n");
//   	interrupt->Halt();
//    } else {
//	printf("Unexpected user mode exception %d %d\n", which, type);
//	ASSERT(FALSE);
//   }
	int op1, op2, result;
	int size, virtAddr;
	char* data;
	char c;
	switch(which){
		case  PageFaultException:    // No valid translation found
			printf("No valid translation is found %d %d\n", which, type);  
			ASSERT(FALSE);
			break;
		case  ReadOnlyException:     // Write attempted to page marked 
			printf("Write attempted tp page marked %d %d\n", which, type);
			ASSERT(FALSE);
			break;
		case  BusErrorException:     // Translation resulted in an 
			printf("Translaion resulted in an %d %d\n", which, type);
			ASSERT(FALSE);
			break;
		case  AddressErrorException: // Unaligned reference or one that
			printf("Unaligned reference or one that %d %d\n", which, type);
			ASSERT(FALSE);
			break;
		case  OverflowException:     // Integer overflow in add or sub.
			printf("Integer overflow in add or sub %d %d\n", which, type);
			ASSERT(FALSE);
			break;
		case  IllegalInstrException: // Unimplemented or reserved instr.
			printf("Unimplemented or reserved instr %d %d\n", which, type);
			ASSERT(FALSE);
			break;
		case NumExceptionTypes:
			printf("Number type exception is found %d %d\n", which, type);
			ASSERT(FALSE);
			break;
		case SyscallException:
			switch(type)
			{
				case SC_Halt:
					DEBUG('a', "Shutdown, initiated by user program.\n");
					interrupt->Halt();
					break;

				case SC_Sub:
					op1 = machine->ReadRegister (4);
					op2 = machine->ReadRegister (5);
					result = op1 - op2;
					machine->WriteRegister (2, result);
					interrupt->Halt();
					break;
				case SC_PrintChar:
					size = machine->ReadRegister (4);	//lay char
					data = User2System(size,1);		//chuyen sang System de print
					gSynchConsole->Write(data, 1);		//write
					interrupt->Halt();
					break;
					//c = (char)machine->ReadRegister(4); // Doc ki tu tu thanh ghi r4
					//gSynchConsole->Write(&c, 1);
					//interrupt->Halt();
					//break;
				case SC_ReadString:
					// Input: Buffer(char*), do dai toi da cua chuoi nhap vao(int)
					// Output: Khong co
					// Cong dung: Doc vao mot chuoi voi tham so la buffer va do dai toi da
					virtAddr = machine->ReadRegister(4); // Lay dia chi tham so buffer truyen vao tu thanh ghi so 4
					size = machine->ReadRegister(5); // Lay do dai toi da cua chuoi nhap vao tu thanh ghi so 5
					data = User2System(virtAddr, size); // Copy chuoi tu vung nho User Space sang System Space
					gSynchConsole->Read(data, size); // Goi ham Read cua SynchConsole de doc chuoi
					System2User(virtAddr, size, data); // Copy chuoi tu vung nho System Space sang vung nho User Space
					delete data; 
					IncreasePC(); // Tang Program Counter 
					return;
					//break;
				case SC_PrintInt:
					DEBUG('a', "Print integer to console");

					char s[MAX_LENGTH_INTEGER];

					int n = machine->ReadRegister(4);

					// Sign
					if (n < 0) {
						gSynchConsole->Write("-",1);
						n = -n;
					}

					// Value without Sign
					int i = 0;

					while (n > 0) {
						s[i] = n % 10 + '0';

						i++;
						n = n / 10;
					}


					int end = i;
					char tmp;

					for(int i = 0, j = end / 2; i < j; i++) {
						tmp = s[i];
						s[i] = s[end - i - 1];
						s[end - i - 1] = tmp;
					}

					s[end] = '\n';
					gSynchConsole->Write(s, end + 1);
					break;

				case SC_ReadChar:
					int len;
					char s[MAX_LENGTH_INTEGER];

					len = gSynchConsole->Read(s, MAX_LENGTH_INTEGER);
					machine->WriteRegister(2, s[len - 1]);

					interrupt->Halt();
					break;
			}
			break;
		default: 
			printf("Unexpected user mode exception %d %d \n", which, type);
			ASSERT(FALSE);
	}
}
