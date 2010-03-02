/*
* Copyright (c) 2007-2009 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of "Eclipse Public License v1.0"
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
* Initial Contributors:
* Nokia Corporation - initial contribution.
*
* Contributors:
*
* Description: 
*
*/


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <symbianelfdefs.h>
#include <sys/stat.h>
#include <time.h>

#define ADDR(rtype, p, o) (rtype *)(((char *)p) + o)

FILE *core;
int ignoreSomeSections;
void hexdump_data(unsigned char *data,int aSize,int j)
{
	int i=0;
	int p=0;
    int m=0;
	while (i<aSize)	
		{
		int count=0;
		if(p==0)
		{
			fprintf(core,"\t0x%08x:\t\t\t",j);
		} // offset into section

		while (i<aSize && count<4)
			{ 
			fprintf(core,"%02X", *data);		// print 4 lots of %08x for the data expresed as 32-bit word 
			data++;
			i++;
			count++;
			j++;
			}

		fprintf(core,"  ");
		p++;
		if (p==4)
			{
			data=data-16;
			for (m=0;m<16;m++)			//print 16 bytes of memory interpreted 
				{							//as ASCII characters with all non-printing 
				if (*data>32 && *data <127)	//characters converted to dots
					{
					fprintf(core,"%1c",*data);
					}
				else
					{
					fprintf(core,".");
					}
					data++;
				}
			p=0; 
			fprintf(core,"\n "); 
			}
		}
		//fprintf(core,"\n"); 	   	
	}
void hexdump(unsigned char* data, int aSize, int offset)
	//  print hex dump of relevant sections
	{
	int i=0;
	int p=0;
    int m=0;
	while (i<aSize)	
		{
		int count=0;
		if(p==0){fprintf(core,"\t%06x   ",offset);} // offset into section
		while (i<aSize && count<4)
			{ 
			fprintf(core,"%02X", *data);		// print 4 lots of %08x for the data expresed as 32-bit word 
			data++;
			i++;
			count++;
			offset++;
			}

		fprintf(core,"  ");
		p++;
		if (p==4)
			{
			data=data-16;
			for (m=0;m<16;m++)			//print 16 bytes of memory interpreted 
				{							//as ASCII characters with all non-printing 
				if (*data>32 && *data <127)	//characters converted to dots
					{
					fprintf(core,"%1c",*data);
					}
				else
					{
					fprintf(core,".");
					}
					data++;
				}
			p=0; 
			fprintf(core," \n "); 
			}
		}
		fprintf(core," \n\n "); 	   	
	}

void print_directive(unsigned char* data, int size)
	// print formatted text of directive section
	{
    int i=0;
	printf ("\t");

	for (i=0; i<size; i++)
		{
		if ((char)data[i]>31 && (char)data[i]<127)
			{
			printf ("%c", (char)data[i]);
			}

		if ((char)data[i]=='\n')
			{
			printf ("\n\t");
			}
		}

	printf ("\n");
	}

void print_reloc(Elf32_Ehdr* eh, Elf32_Sym* symT, unsigned char* strtab)
	// print relocation section
	{
    int i=0;
    int j=0;
	Elf32_Shdr* shdr = ADDR(Elf32_Shdr, eh, eh->e_shoff);
	for (j=0;j< eh->e_shnum;j++)
		{
		char* sname = ADDR(char, eh, shdr[eh->e_shstrndx].sh_offset);
		if ( (shdr[j].sh_type==9) && 
		     ( (!ignoreSomeSections) || 
		       (strncmp(".rel.debug_", &sname[shdr[j].sh_name], 11))
		     )
		   )
			{
			unsigned char* data = ADDR(unsigned char, eh, shdr[j].sh_offset);
			Elf32_Rel* rl=(Elf32_Rel*)data;				// pointer to relocation section	
			int noOfReloc=shdr[j].sh_size / shdr[j].sh_entsize;
			fprintf(core,"\n\n\n\t\t\t%s\n", &sname[shdr[j].sh_name]);
			for (i=0;i<noOfReloc;i++)
				{
				unsigned char* symbolName = strtab;		// pointer to firest element of string											// table which holds symbol names
				Elf32_Sym*  sym = symT;					// pointer to symbol table
				int symTIndx= ELF32_R_SYM(rl->r_info);		// symbol Tableindex
				sym=sym+symTIndx;							
				symbolName=symbolName+sym->st_name;		// index into string table section 
															// with symbol names
				fprintf(core,"\t0x%08x \t", rl->r_offset);		// prints offset into relocation section
				fprintf(core,"%d", symTIndx);					// symbol table index
				fprintf(core,"\t%s\n",symbolName);				// symbol name
				rl++;			
				}
			}
		}
	}	

void print_GlSymbols(Elf32_Ehdr* eh, Elf32_Sym* symT, unsigned char* strtab)
	// print global symbols from Symbol Table
	{	
    int i=0;
    int l=0;
	Elf32_Shdr* shdr = ADDR(Elf32_Shdr, eh, eh->e_shoff);
	char* sname = ADDR(char, eh, shdr[eh->e_shstrndx].sh_offset);
	for (i=0;i< eh->e_shnum;i++)
		{
		if (!strcmp(".symtab", &sname[shdr[i].sh_name]))
			{
		  	int noOfSym=shdr[i].sh_size / shdr[i].sh_entsize; 	// number of symbols
		  	const char *symName =(const char *)strtab;
		  	int count = 1;										
		  	fprintf(core,"Global symbols:\n");
		  	fprintf(core,"=================\n\n");
		  	for (l=0;l< noOfSym ;l++)
				{
				symT=symT+1;
				if( ELF32_ST_BIND(symT->st_info) == 1)			// searching for global symbols
			 		{
			  		symName = symName + symT->st_name;			// index into string table section 
			 		fprintf(core,"%d	",count);
			  		fprintf(core,symName);
			  		fprintf(core,"\n");
			  		symName = symName - symT->st_name;			// back to pointing to first byte of string table
			  		count++;
					}
			
				}
			}
		}
	}
	
void print_elf_header(Elf32_Ehdr* eh)
	{
	// print elf header
	if (eh->e_version==1)
		fprintf(core,"\tHeader version: EV_CURRENT (Current version)\n");
	else
		fprintf(core,"\tInvalid version: EV_NONE (Invalid version)\n");


	fprintf(core,"\tFile Type\t\t\t:");
	if (eh->e_type==0)
		fprintf(core,"ET_NONE (No file type) (0)\n");
	else if (eh->e_type==1)
		fprintf(core,"ET_REL (Relocatable object) (1)\n");
	else if (eh->e_type==2)
		fprintf(core,"ET_EXEC (Executable file) (2)\n"); 
	else if (eh->e_type==3)
		fprintf(core,"ET_DYN (Shared object file) (3)\n"); 
	else if (eh->e_type==4)
		fprintf(core,"ET_CORE (Core File) (4)\n"); 
	else if (eh->e_type==65280)
		fprintf(core,"ET_LOPROC (Precessor Specific) (ff00)\n");
	else	
		fprintf(core,"ET_HIPROC (Precessor Specific) (ffff)\n");

	if (eh->e_machine==40)
		fprintf(core,"\tMachine\t\t\t\t:EM_ARM (ARM)\n");
	else
		fprintf(core,"\tERROR:\tUnexpected machine\n");

	fprintf(core,"\tEntry offset (in SHF_ENTRYSECT section):0x%08x \n",eh->e_entry);
	fprintf(core,"\tProgram header entries\t\t:%d\n",eh->e_phnum); 
	fprintf(core,"\tSection header entries\t\t:%d\n",eh->e_shnum); 
  
	fprintf(core,"\tProgram header offset\t\t:%d",eh->e_phoff); 
	fprintf(core,"  bytes (0x%08X",eh->e_phoff);
	fprintf(core,")\n");
	fprintf(core,"\tSection header offset\t\t:%d",eh->e_shoff); 
	fprintf(core,"  bytes (0x%08X",eh->e_shoff);
	fprintf(core,")\n");

	fprintf(core,"\tProgram header entry size\t:%d",eh->e_phentsize); 
	fprintf(core,"  bytes (0x%02X",eh->e_phentsize);
	fprintf(core,")\n");
	fprintf(core,"\tSection header entry size\t:%d",eh->e_shentsize); 
	fprintf(core,"  bytes (0x%02X",eh->e_shentsize);
	fprintf(core,")\n");
	fprintf(core,"\tSection header string table index: %d \n", eh->e_shstrndx);
	fprintf(core,"\tHeader size\t\t\t:%d", eh->e_ehsize);
	fprintf(core,"  bytes (0x%02X",eh->e_ehsize);
	fprintf(core,")\n");
	}

void print_sect_header(char* sname, Elf32_Shdr* shdr, int count)
	// print section header names
	{
	static const char* KtypeName[]={"0","SHT_PROGBITS (1)","SHT_SYMTAB (2)","SHT_STRTAB (3)",
								  "SHT_RELA (4)","5",	"SHT_DINAMIC (6)","7","8","SHT_REL (9)",
								  "10","SHT_DINSYM (11)"};
						
	fprintf(core,"\n\n\tName\t\t:%1s\n ",&sname[shdr[count].sh_name]);
	fprintf(core,"\tType\t\t: %s\n",  KtypeName[shdr[count].sh_type]);
	fprintf(core,"\tAddr\t\t: 0x%08X\n",shdr[count].sh_addr);
	fprintf(core,"\tSize\t\t: %1d", shdr[count].sh_size);
	fprintf(core,"  bytes (0x%X",shdr[count].sh_size);
	fprintf(core,")\n");
	fprintf(core,"\tEntry Size\t: %1d\n",shdr[count].sh_entsize);
	fprintf(core,"\tAligment\t: %1d\n\n\n",shdr[count].sh_addralign);		 	
	}

unsigned char* findSymbolStringT(Elf32_Ehdr* eh)
	//calculate and return pointer to the first byte of string table(the one with symbol names)
	{
    int i=0;
	Elf32_Shdr* shdr = ADDR(Elf32_Shdr, eh, eh->e_shoff);
	char* sname = ADDR(char, eh, shdr[eh->e_shstrndx].sh_offset);
	for (i=0;i < eh->e_shnum; i++)
		{
		if (!strcmp(".strtab", &sname[shdr[i].sh_name]))
			{
			unsigned char* data = ADDR(unsigned char, eh, shdr[i].sh_offset); 
			return data;	//pointer to the first byte of string table section
			}
		}
	return NULL;	//if not found  
	}

Elf32_Sym* findSymbolT(Elf32_Ehdr* eh)
	//calculate and return pointer to the first element of symbol table	
	{
    int i=0;
	Elf32_Shdr* shdr = ADDR(Elf32_Shdr, eh, eh->e_shoff);
	for (i=0;i < eh->e_shnum;i++)
		{
		if (shdr[i].sh_type==2)
			{
			unsigned char* data = ADDR(unsigned char, eh, shdr[i].sh_offset);
			Elf32_Sym* sym=(Elf32_Sym*)data;
			return sym;		//pointer to the first element of symbol table.
			}
		}
	return NULL; // if not found
	}

void print_Summary(Elf32_Ehdr* eh)
	{
    int i=0;
	//print section names
	Elf32_Shdr* shdr = ADDR(Elf32_Shdr, eh, eh->e_shoff);
	char* sname = ADDR(char, eh, shdr[eh->e_shstrndx].sh_offset);
	fprintf(core,"\nSummary: \n");
	fprintf(core,"==========\n");
	for (i=0;i< eh->e_shnum;i++)
		{
		fprintf(core,&sname[shdr[i].sh_name]);
		fprintf(core,"\n");
		}
	}

int printAll;
/*char  *ctime( const time_t *date)
{
	 
}*/	
	

enum TCrashType { ECrashException, ECrashKill };

enum TExcType
	{
	EExcGeneral=0,
	EExcIntegerDivideByZero=1,
	EExcSingleStep=2,
	EExcBreakPoint=3,
	EExcIntegerOverflow=4,
	EExcBoundsCheck=5,
	EExcInvalidOpCode=6,
	EExcDoubleFault=7,
	EExcStackFault=8,
	EExcAccessViolation=9,
	EExcPrivInstruction=10,
	EExcAlignment=11,
	EExcPageFault=12,
	EExcFloatDenormal=13,
	EExcFloatDivideByZero=14,
	EExcFloatInexactResult=15,
	EExcFloatInvalidOperation=16,
	EExcFloatOverflow=17,
	EExcFloatStackCheck=18,
	EExcFloatUnderflow=19,
	EExcAbort=20,
	EExcKill=21,
	EExcUserInterrupt=22,
	EExcDataAbort=23,
	EExcCodeAbort=24,
	EExcMaxNumber=25,
	EExcInvalidVector=26
	};

char * TExcTypeNames[EExcInvalidVector+1] = 
	{
	"EExcGeneral",
	"EExcIntegerDivideByZero",
	"EExcSingleStep",
	"EExcBreakPoint",
	"EExcIntegerOverflow",
	"EExcBoundsCheck",
	"EExcInvalidOpCode",
	"EExcDoubleFault",
	"EExcStackFault",
	"EExcAccessViolation",
	"EExcPrivInstruction",
	"EExcAlignment",
	"EExcPageFault",
	"EExcFloatDenormal",
	"EExcFloatDivideByZero",
	"EExcFloatInexactResult",
	"EExcFloatInvalidOperation",
	"EExcFloatOverflow",
	"EExcFloatStackCheck",
	"EExcFloatUnderflow",
	"EExcAbort",
	"EExcKill",
	"EExcUserInterrupt",
	"EExcDataAbort",
	"EExcCodeAbort",
	"EExcMaxNumber",
	"EExcInvalidVector"
	};

void print_symbian_info(Sym32_syminfod *syminfod)
{    
	  
	//fprintf(core,"\tDate and time of the crash\t:=0x%X",syminfod->sd_date_time );
	const time_t unix_time = (time_t)62168256000LL;//from 0AD to 1970 in seconds = 365*1971*86 400
	//fprintf(core,"\traw Date =0x%X\n", (syminfod->sd_date_time[1]<<32+syminfod->sd_date_time[0])/*/1000000*/ );
    time_t date = syminfod->sd_date_time/1000000; //convert to seconds
    date -= unix_time; //convert to unix time
	fprintf(core,"\tDate and time of the crash\t: %s\n", ctime(&date));

	if( SYM32_EXECID_SIZE != sizeof(Sym32_execid) )
		{
		fprintf(core,"\tWarning! : Expected Size of EXECUTABLE ID %d is different from sizeof operator %d\n\n", 
			SYM32_EXECID_SIZE, sizeof(Sym32_execid) );
		}

	fprintf(core,"\tExecutable Crc32 (first 1kb)\t:0x%X\n",syminfod->sd_execid.exec_crc );

	if( ECrashException == syminfod->sd_exit_type )
		{
		fprintf(core,"\tHardware Exception\t\t:%d\n",syminfod->sd_exit_reason);
		fprintf(core,"\tException Type\t\t\t:%s\n", TExcTypeNames[syminfod->sd_exit_reason] );
		}
	else if ( ECrashKill == syminfod->sd_exit_type )
		{
		fprintf(core,"\tExit Type\t\t\t:%d",syminfod->sd_exit_reason);
		switch(syminfod->sd_exit_reason)
			{
			case 0:fprintf(core,":EExitKill\n");		break;
			case 1:fprintf(core,":EExitTerminate\n");	break;
			case 2:fprintf(core,":EExitPanic\n");		break;
			case 3:fprintf(core,":EExitPending\n");	break;
			default:fprintf(core,":Unknown\n");			break;
			}    
		}
	else
		{
		fprintf(core,"\t\tUnknown Crash Type\n" );
		}
	

	fprintf(core,"\tCrashed Thread Id\t\t:0x%X\n",syminfod->sd_thread_id);
	fprintf(core,"\tOwning process\t\t\t:0x%X\n",syminfod->sd_proc_id);
}	

void print_thread_info(Sym32_thrdinfod *thrdinfod)
{
	
	//fprintf(core,"\tIndex into the CORE.SYMBIAN.STR note segment defining");
	//fprintf(core," the name of the thread or ESYM_STR_UNDEF :%d\n",thrdinfod->td_name);
	fprintf(core,"\tThread ID\t\t\t:0x%X\n",thrdinfod->td_id);
	fprintf(core,"\tOwning process\t\t\t:0x%X\n",thrdinfod->td_owning_process);
	fprintf(core,"\tThread Priority\t\t\t:%d\n",thrdinfod->td_priority);
	fprintf(core,"\tSupervisor Stack Pointer\t:0x%08X\n",thrdinfod->td_svc_sp);
	fprintf(core,"\tSupervisor Stack Address\t:0x%08X\n",thrdinfod->td_svc_stack);
	fprintf(core,"\tSupervisor Stack Size\t\t:%u",thrdinfod->td_svc_stacksz);
	fprintf(core,"  bytes (0x%08X",thrdinfod->td_svc_stacksz);
	fprintf(core,")\n");
    fprintf(core,"\tUser Stack Address\t\t:0x%08X\n",thrdinfod->td_usr_stack);
    fprintf(core,"\tUser Stack Size\t\t\t:%u",thrdinfod->td_usr_stacksz);
	fprintf(core,"  bytes (0x%08X)\n",thrdinfod->td_usr_stacksz);
	fprintf(core,"\tCPU id\t\t\t:%d\n\n",thrdinfod->td_last_cpu_id);	
}	

void print_lock_data(Sym32_lockdata* lockdata)
	{
	fprintf(core,"\tNum locks\t\t\t:%d\n", lockdata->lk_lock_count);
	fprintf(core,"\tmutex thread wait count\t\t\t:%d\n",lockdata->lk_mutex_thread_wait_count);
	fprintf(core,"\tmutex thread held count\t\t\t:%d\n",lockdata->lk_mutex_held_count);
	}

void print_process_info(Sym32_procinfod *procinfod)
{
	
	//fprintf(core,"\t\t\tIndex into the CORE.SYMBIAN.STR note segment defining");
	//fprintf(core," the name of the process or ESYM_STR_UNDEF:%d\n",procinfod->pd_name);
	fprintf(core,"\tProcess ID\t\t\t:0x%X\n",procinfod->pd_id);
	fprintf(core,"\tProcess Priority\t\t:%d\n",procinfod->pd_priority);

}

void print_executable_info(Sym32_execinfod *execinfod)
{
	
	fprintf(core,"\tExecutable ID\t\t\t:0x%08X\n",execinfod->ed_execid.exec_id);
	fprintf(core,"\tExecutable Crc32 (first 1kb)\t:0x%X\n",execinfod->ed_execid.exec_crc );

	if(execinfod->ed_XIP == 1)
	{
		fprintf(core,"\tXIP ROM\t\t\t\t:TRUE\n");
    }
    else if(execinfod->ed_XIP == 0)
    {
	     fprintf(core,"\tXIP ROM\t\t\t\t:FALSE\n");
    }

	fprintf(core,"\tSize of executable code segment\t\t\t:%d",execinfod->ed_codesize);
	fprintf(core,"  bytes (0x%08X)\n",execinfod->ed_codesize);

	fprintf(core,"\tExecution address of the code segment\t\t:0x%08X\n",execinfod->ed_coderunaddr);

	fprintf(core,"\tBuild address of the code segment\t\t:0x%08X\n",execinfod->ed_codeloadaddr);

	fprintf(core,"\tSize of the executable read only data segment\t:%d",execinfod->ed_rodatasize);
	fprintf(core,"  bytes (0x%08X)\n",execinfod->ed_rodatasize);

	fprintf(core,"\tExecution address of the read only data segment\t:0x%08X\n",execinfod->ed_rodatarunaddr);

    fprintf(core,"\tBuild address of the read only data segment\t:0x%08X\n",execinfod->ed_rodataloadaddr);

    fprintf(core,"\tSize of the executable data segment\t\t:%d", execinfod->ed_datasize);
	fprintf(core,"  bytes (0x%08X)\n",execinfod->ed_datasize);

    fprintf(core,"\tExecution address of the data segment\t\t:0x%08X\n",execinfod->ed_datarunaddr);
	
    fprintf(core,"\tBuild address of the data segment\t\t:0x%08X\n",execinfod->ed_dataloadaddr);
}

//void print_register_info(Sym32_reginfod *reginfod,unsigned int nreg,Sym32_regdatad *regdatad,char *array,unsigned int elenum)
void print_register_info( Sym32_reginfod *reginfod, Elf32_Ehdr* eh, char *array )
{   
    unsigned int i=0;
	Sym32_regdatad *regdatad = ADDR(Sym32_regdatad, reginfod, sizeof (Sym32_reginfod) );
	fprintf(core,"\tVersion of the register data info descriptor\t:%s\n",&array[reginfod->rid_version]);
	fprintf(core,"\tThread ID\t\t\t:0x%X\n",reginfod->rid_thread_id);
	fprintf(core,"\tNumber of registers\t\t:%d\n",reginfod->rid_num_registers);

	fprintf(core,"\tRegister Class\t\t\t:%d",reginfod->rid_class);
	switch( reginfod->rid_class )
		{
		case ESYM_REG_CORE:
			fprintf(core, " : ESYM_REG_CORE\n" );
			break;
		case ESYM_REG_COPRO:
			fprintf(core, " : ESYM_REG_COPRO\n" );
			break;
		default:
			fprintf(core, " : Unknown Register Class\n" );
		}

	fprintf(core,"\tRegister Representation\t\t:%d",reginfod->rid_repre );
	switch( reginfod->rid_repre )
		{
		case ESYM_REG_8:
			fprintf(core, " : ESYM_REG_8\n" );
			break;
		case ESYM_REG_16:
			fprintf(core, " : ESYM_REG_16\n" );
			break;
		case ESYM_REG_32:
			fprintf(core, " : ESYM_REG_32\n" );
			break;
		case ESYM_REG_64:
			fprintf(core, " : ESYM_REG_64\n" );
			break;
		default:
			fprintf(core, "\n" );
		}

	fprintf(core, "\n" );


	for( i = 0; i < reginfod->rid_num_registers; i++ )
	{
		fprintf(core,"\tRegister ID\t\t\t:0x%X ", regdatad->rd_id);

		if( ESYM_REG_CORE == reginfod->rid_class )
		{
			switch(regdatad->rd_id)
			{
				case 0x00000000: fprintf(core,"ARM REGISTER R0\n"); break;
				case 0x00000100: fprintf(core,"ARM REGISTER R1\n"); break;
				case 0x00000200: fprintf(core,"ARM REGISTER R2\n"); break;
				case 0x00000300: fprintf(core,"ARM REGISTER R3\n"); break;
				case 0x00000400: fprintf(core,"ARM REGISTER R4\n"); break;
				case 0x00000500: fprintf(core,"ARM REGISTER R5\n"); break;
				case 0x00000600: fprintf(core,"ARM REGISTER R6\n"); break;
				case 0x00000700: fprintf(core,"ARM REGISTER R7\n"); break;
				case 0x00000800: fprintf(core,"ARM REGISTER R8\n"); break;
				case 0x00000900: fprintf(core,"ARM REGISTER R9\n"); break;
				case 0x00000a00: fprintf(core,"ARM REGISTER R10\n"); break;
				case 0x00000b00: fprintf(core,"ARM REGISTER R11\n"); break;
				case 0x00000c00: fprintf(core,"ARM REGISTER R12\n"); break;
				case 0x00000d00: fprintf(core,"ARM REGISTER R13\n"); break;
				case 0x00000e00: fprintf(core,"ARM REGISTER R14\n"); break;
				case 0x00000f00: fprintf(core,"ARM REGISTER R15\n"); break;
				case 0x00001000: fprintf(core,"ARM REGISTER CPSR\n"); break;
				case 0x00001100: fprintf(core,"ARM REGISTER R13_SVC\n"); break;
				case 0x00001200: fprintf(core,"ARM REGISTER R14_SVC\n"); break;
				case 0x00001300: fprintf(core,"ARM REGISTER SPSR_SVC\n"); break;
				case 0x00001400: fprintf(core,"ARM REGISTER R13_ABT\n"); break;
				case 0x00001500: fprintf(core,"ARM REGISTER R14_ABT\n"); break;
				case 0x00001600: fprintf(core,"ARM REGISTER SPSR_ABT\n"); break;
				case 0x00001700: fprintf(core,"ARM REGISTER R13_UND\n"); break;
				case 0x00001800: fprintf(core,"ARM REGISTER R14_UND\n"); break;
				case 0x00001900: fprintf(core,"ARM REGISTER SPSR_UND\n"); break;
				case 0x00001a00: fprintf(core,"ARM REGISTER R13_IRQ\n"); break;
				case 0x00001b00: fprintf(core,"ARM REGISTER R14_IRQ\n"); break;
				case 0x00001c00: fprintf(core,"ARM REGISTER SPSR_IRQ\n"); break;
				case 0x00001d00: fprintf(core,"ARM REGISTER R8_FIQ\n"); break;
				case 0x00001e00: fprintf(core,"ARM REGISTER R9_FIQ\n"); break;
				case 0x00001f00: fprintf(core,"ARM REGISTER R10_FIQ\n"); break;
				case 0x00002000: fprintf(core,"ARM REGISTER R11_FIQ\n"); break;
				case 0x00002100: fprintf(core,"ARM REGISTER R12_FIQ\n"); break;
				case 0x00002200: fprintf(core,"ARM REGISTER R13_FIQ\n"); break;
				case 0x00002300: fprintf(core,"ARM REGISTER R14_FIQ\n"); break; 
				case 0x00002400: fprintf(core,"ARM REGISTER SPSR_FIQ\n"); break;
				default:fprintf(core,"Unknown Core Register\n"); break;
			} // switch
		} // if CORE
		else
		{
			fprintf(core,"\n\tRegister SubId\t\t\t:0x%X\n",regdatad->rd_sub_id);
		}

		switch( reginfod->rid_repre )
		{
		case ESYM_REG_8:
            {
			Elf32_Byte * val8;
			val8 = ADDR( Elf32_Byte, eh, regdatad->rd_data );
			fprintf(core, "\tESYM_REG_8 Value\t\t:0x%02X\n", *val8 );
			break;
            }
		case ESYM_REG_16:
            {
			Elf32_Half * val16;
			val16 = ADDR( Elf32_Half, eh, regdatad->rd_data );
			fprintf(core, "\tESYM_REG_16 Value\t\t:0x%04X\n", *val16 );
			break;
            }
		case ESYM_REG_32:
            {
			Elf32_Word * val32;
			val32 = ADDR( Elf32_Word, eh, regdatad->rd_data );
			fprintf(core, "\tESYM_REG_32 Value\t\t:0x%08X\n", *val32 );
			break;
            }
		case ESYM_REG_64:
            {
			// We need to split the printing of a 64 bit number since the 
			// printf is not working correctly for this size.
			Elf32_Word * val64_0;
			Elf32_Word * val64_1;
			val64_0 = ADDR( Elf32_Word, eh, regdatad->rd_data );
			val64_1 = ADDR( Elf32_Word, eh, regdatad->rd_data + 4 );
			fprintf(core, "\tESYM_REG_64 Value\t\t:0x%X%X\n", *val64_1, *val64_0 );
			break;
	        }	
		default:
			fprintf(core, "\n" );
		}

		fprintf(core, "\n" );

		regdatad++;

	} // for
 }

void print_trace_info( Sym32_tracedata *aTraceData, Elf32_Ehdr* aElfHdr, char *aArray)
	{	
	fprintf(core, "\tVersion of the trace data info descriptor\t:%s\n", &aArray[aTraceData->tr_version]);
	fprintf(core, "\tSize of trace buffer\t\t:%d bytes\n", aTraceData->tr_size);	
			
	if(aTraceData->tr_data == 0)
		{
		fprintf(core, "\tNo trace data present\n");
		}
	else
		{
		unsigned char* data = ADDR(unsigned char, aElfHdr, aTraceData->tr_data);	
		fprintf(core, "\tTrace Data starts at\t\t:0x%X\n\n", data);
		}	

	fprintf(core, "\n\n");
	}
 

int do_elf_file(char* buffer, char* name)
	{
    int i=0;
    int j=0;
    int k=0;
    char *array = NULL;
	Elf32_Ehdr* eh=(Elf32_Ehdr *)buffer;	//elf header
	int phnum = eh->e_phnum;
	int phoff =eh->e_phoff;  
	Elf32_Phdr* phdr = ADDR(Elf32_Phdr,eh,phoff);

    int shoff = eh->e_shoff;							    // offset of section header table
	Elf32_Shdr* shdr = ADDR(Elf32_Shdr, eh, shoff);			// calculating pointer to Secton Header Table
															// Elf32_Shdr * shdr = (Elf32_Shdr *)(buffer+shoff); 
	int shnum = eh->e_shnum;							    // number of section headers
	int shstrndx = eh->e_shstrndx;
	int snameoffset = shdr[shstrndx].sh_offset;				// offset in file of sections' names
	char* sname = ADDR(char, eh, snameoffset);				// pointer to String Table which holds section names
	Elf32_Sym* symT= findSymbolT(eh);	// pointer to Symbol table
	unsigned char* strtab=findSymbolStringT(eh);	// pointer to String table which holds symbol names


	if (eh->e_ident[EI_MAG0] !=0x7f || eh->e_ident[EI_MAG1] != 0x45 || eh->e_ident[EI_MAG2] !=0x4c || eh->e_ident[EI_MAG3] != 0x46)
		{
		// EI_MAG0 to EI_MAG3 - A files' first 4 bytes hold a 'magic number', identifying the file as an ELF object file. 
        fprintf(core,"Error: %s is not a valid ELF file", name);
		return 1;
		}
	if (eh->e_ident[EI_DATA] == 2)							
		{
		// ELF Header size should be 52 bytes or converted into Big-Endian system 13312
		if (eh->e_ehsize != 13312)
			{
			fprintf(core,"\tERROR:\tELF Header contains invalid file type\n");
			exit(1);
			}
		// e_ident[EI_DATA] specifies the data encoding of the processor-specific data in the object file.
		fprintf(core,"\tERROR:\tData encoding ELFDATA2MSB (Big-Endian) not supported\n");
		exit(1);
		}
	if (eh->e_ehsize != 52)
		{
		// ELF Header size should be 52 bytes
        fprintf(core,"\tERROR:\tELF Header contains invalid file type\n");
        exit(1);
        }
    
	fprintf(core,"ELF HEADER INFORMATION\n");  
	print_elf_header(eh);									// print Elf Header

    //segments start here
	for(j = 0; j < phnum; j++)
		{
		if(phdr[j].p_type == PT_NOTE)
			{
			Sym32_dhdr *dhdr = ADDR(Sym32_dhdr, eh, phdr[j].p_offset);
			if(dhdr->d_type == ESYM_NOTE_STR)
				{
                array = (char*)dhdr + sizeof(Sym32_dhdr);
				}
		    }	
		}   
    
    for(i = 0; i < phnum; i++)
		{
	    unsigned int data = phdr[i].p_offset;
        Sym32_dhdr *dhdr = ADDR(Sym32_dhdr,eh,data);
	    unsigned int  flag = phdr[i].p_flags;

		if( SYM32_DESCHDR_SIZE != sizeof(Sym32_dhdr) )
			{
			fprintf(core,"\n\tWarning! : Expected Size of SYM32_DESCHDR_SIZE %d is different from sizeof(Sym32_dhdr) %d\n\n", 
				 SYM32_DESCHDR_SIZE, sizeof(Sym32_dhdr) );
			}

	    fprintf(core,"\nPROGRAM HEADER ENTRY  %d INFORMATION \n",i);
	    fprintf(core,"\tHeader offset\t\t\t:%d",phdr[i].p_offset); 
	    fprintf(core,"  bytes (0x%08X",phdr[i].p_offset);
	    fprintf(core,")\n");
	    fprintf(core,"\tVirtual address\t\t\t:0x%08X\n",phdr[i].p_vaddr);
	    
	    fprintf(core,"\tSize of mapping from the file\t:0x%X (%d bytes)\n",phdr[i].p_filesz, phdr[i].p_filesz);
	    fprintf(core,"\tSize of mapping in memory\t:0x%X (%d bytes)\n",phdr[i].p_memsz, phdr[i].p_memsz);
	    
	    switch(flag)
			{
		    case 1 :
		             fprintf(core,"\tFlag\t\t\t\t:PF_X : (1) \n");
		              break;
		    case 2 :
		             fprintf(core,"\tFlag\t\t\t\t:PF_W: (2) \n");
		             break;
		    case 4 :
		             fprintf(core,"\tFlag\t\t\t\t:PF_R : (4) \n");
		             break;
		    case 5:
		             fprintf(core,"\tFlag\t\t\t\t:PF_X|PF_R : (5)\n");
		             break;
		    case 6:
		             fprintf(core,"\tFlag\t\t\t\t:PF_W|PF_R : (6)\n");
		             break;
			}

		fprintf(core,"\tAlignment to word boundary      :%d \n",phdr[i].p_align);
		
	    if(phdr[i].p_type == PT_LOAD)
			{
		 	unsigned char* data = ADDR(unsigned char, eh, phdr[i].p_offset);
            const int scope = 128;
		 	fprintf(core,"\tLOADABLE CODE/DATA SEGMENT\n"); //load  
		 	 //fprintf(core,"\tDatasegment starts from here:%p, size:%d\n",data, phdr[i].p_memsz);
             if(phdr[i].p_filesz == 0) continue;
             if(phdr[i].p_filesz < 2*scope)
		        hexdump_data(data,phdr[i].p_memsz,phdr[i].p_vaddr);									
             else
                 {
                 hexdump_data(data,scope,phdr[i].p_vaddr);
                 fprintf(core,"\t...\n"); 
                 hexdump_data(data+phdr[i].p_filesz-scope,scope,phdr[i].p_vaddr+phdr[i].p_filesz-scope);
                 }
             if(phdr[i].p_filesz%16) fprintf(core,"\n");
			}
	    else if(phdr[i].p_type == PT_NOTE)
			{
            fprintf(core,"\tName of the descriptor\t\t:%s\n",&array[dhdr->d_name]);
            fprintf(core,"\tSize of single descriptor element:%d \n",dhdr->d_descrsz);
	        fprintf(core,"\tVersion string\t\t\t:%s\n",&array[dhdr->d_version]);
	        fprintf(core,"\tNumber of descriptor elements\t:%d\n",dhdr->d_elemnum); 
			fprintf(core,"\tSegment type\t\t\t:");

	        if( dhdr->d_type == ESYM_NOTE_SYM )
				{
				Sym32_syminfod *syminfod = ADDR(Sym32_syminfod,eh,data+sizeof(Sym32_dhdr));
				fprintf(core,"SYMBIAN INFO SEGMENT\n\n");

				if( sizeof(Sym32_syminfod) != dhdr->d_descrsz )
					{
					fprintf(core,"\tWarning! : sizeof(Sym32_syminfod) %d is different from descriptor size %d\n\n", 
						 sizeof(Sym32_syminfod), dhdr->d_descrsz );
					}

				if( SYM32_SYMINFO_SIZE != dhdr->d_descrsz )
					{
					fprintf(core,"\tWarning! : Expected Size of SYMBIAN INFO SEGMENT %d is different from descriptor size %d\n\n", 
						 SYM32_SYMINFO_SIZE, dhdr->d_descrsz );
					}

				if( syminfod->sd_exit_cat > 0 )
					{
					fprintf(core,"\tCrash reason\t\t\t:%s\n",&array[syminfod->sd_exit_cat]);
					}
				print_symbian_info(syminfod);
	            }                 
			else if( dhdr->d_type == ESYM_NOTE_THRD)
				{
		        Sym32_thrdinfod *thrdinfod = ADDR(Sym32_thrdinfod,eh,data+sizeof(Sym32_dhdr));
		        fprintf(core,"THREAD INFO SEGMENT\n\n");

				if( sizeof(Sym32_thrdinfod) != dhdr->d_descrsz )
					{
					fprintf(core,"\tWarning! : sizeof(Sym32_thrdinfod) %d is different from descriptor size %d\n\n", 
						sizeof(Sym32_thrdinfod), dhdr->d_descrsz );
					}

				if( SYM32_THRINFO_SIZE!= dhdr->d_descrsz )
					{
					fprintf(core,"\tWarning! : Expected Size of THREAD INFO SEGMENT %d is different from descriptor size %d\n\n", 
						 SYM32_THRINFO_SIZE, dhdr->d_descrsz );
					}

				for(j = 0; j < dhdr->d_elemnum; j++ )
					{
					fprintf(core,"\tThread Name\t\t\t:%s\n",&array[thrdinfod->td_name]);
					print_thread_info(thrdinfod);
					thrdinfod ++;
					}
				}                      
			else if(dhdr->d_type == ESYM_NOTE_PROC )
				{
		        Sym32_procinfod *procinfod = ADDR(Sym32_procinfod,eh,data+sizeof(Sym32_dhdr));
		        fprintf(core,"PROCESS INFO SEGMENT\n\n");

				if( sizeof(Sym32_procinfod) != dhdr->d_descrsz )
					{
					fprintf(core,"\tWarning! :  sizeof(Sym32_procinfod) %d is different from descriptor size %d\n\n", 
						sizeof(Sym32_procinfod), dhdr->d_descrsz );
					}

				if( SYM32_PROCINFO_SIZE != dhdr->d_descrsz )
					{
					fprintf(core,"\tWarning! : Expected Size of PROCESS INFO SEGMENT %d is different from descriptor size %d\n\n", 
						 SYM32_PROCINFO_SIZE, dhdr->d_descrsz );
					}

				
				for(j = 0; j < dhdr->d_elemnum; j++ )
					{
					fprintf(core,"\tProcess Name\t\t\t:%s\n",&array[procinfod->pd_name]);
					print_process_info(procinfod);
					procinfod ++;
					fprintf(core,"\n");
					}
				}                        
			else if(dhdr->d_type == ESYM_NOTE_EXEC)
				{ 
		        Sym32_execinfod *execinfod = ADDR(Sym32_execinfod,eh,data+sizeof(Sym32_dhdr));
		        fprintf(core,"EXECUTABLE INFO SEGMENT\n\n");
				
				if( sizeof(Sym32_execinfod) != dhdr->d_descrsz )
					{
					fprintf(core,"\tWarning! : sizeof(Sym32_execinfod) %d is different from descriptor size %d\n\n", 
						sizeof(Sym32_execinfod), dhdr->d_descrsz );
					}

				if( SYM32_EXECINFO_SIZE != dhdr->d_descrsz )
					{
					fprintf(core,"\tWarning! : Expected Size of EXECUTABLE INFO SEGMENT %d is different from descriptor size %d\n\n", 
						 SYM32_EXECINFO_SIZE, dhdr->d_descrsz );
					}

                for(j = 0; j < dhdr->d_elemnum; j++)
                    {
                    if(j) fprintf(core,"\n");
		            fprintf(core,"\tCrashed Executable Name\t\t:%s\n",&array[execinfod->ed_name]);
			        print_executable_info(execinfod);
                    execinfod++;
                    }
				}                      
			else if(dhdr->d_type == ESYM_NOTE_REG)
				{
			    Sym32_reginfod *reginfod = ADDR(Sym32_reginfod,eh,data+sizeof(Sym32_dhdr));
			    fprintf(core,"REGISTER INFO SEGMENT\n\n");

				if( SYM32_REGINFO_SIZE != sizeof(Sym32_reginfod) )
					{
					fprintf(core,"\tWarning! : Expected Size of REGISTER INFO SEGMENT %d is different from sizeof operator %d\n\n", 
						SYM32_REGINFO_SIZE, sizeof(Sym32_reginfod) );
					}

				if( SYM32_REGDATA_SIZE != sizeof(Sym32_regdatad) )
					{
					fprintf(core,"\tWarning! : Expected Size of REGISTER INFO DATA SEGMENT %d is different from sizeof operator %d\n\n", 
						SYM32_REGDATA_SIZE, sizeof(Sym32_regdatad) );
					}

			    
				for( j = 0; j < dhdr->d_elemnum; j++ )
					{
				    print_register_info(reginfod, eh, array);
					reginfod ++;
					}
			    
				}   
			else if(dhdr->d_type == ESYM_NOTE_TRACE)
				{
				Sym32_tracedata *traceInfo = ADDR(Sym32_tracedata, eh, data+sizeof(Sym32_dhdr));
				int cnt = 0;
				fprintf(core, "TRACE INFO SEGMENT\n\n");
				
				if( SYM32_TRACEDATA_SIZE != sizeof(Sym32_tracedata) )
					{
					fprintf(core, "\tWarning! : Expected Size of TRACE INFO SEGMENT %d is different from sizeof operator %d\n\n", 
							SYM32_TRACEDATA_SIZE, sizeof(Sym32_tracedata) );
					}

				if( SYM32_TRACEDATA_SIZE != sizeof(Sym32_tracedata) )
					{
					fprintf(core, "\tWarning! : Expected Size of TRACE INFO DATA SEGMENT %d is different from sizeof operator %d\n\n", 
							SYM32_TRACEDATA_SIZE, sizeof(Sym32_regdatad) );
					}														
				
				for( cnt = 0; cnt < dhdr->d_elemnum; cnt++ )
					{
					print_trace_info(traceInfo, eh, array);
					traceInfo++;
					}
				
				}
			else if(dhdr->d_type == ESYM_NOTE_STR)
				{
			    char *temp=array+dhdr->d_descrsz;
				fprintf(core,"STRING INFO SEGMENT\n\n");
			    fprintf(core,"\t");
			    while(array<=temp)
					{
				    fprintf(core,"%c",*array);
			        array++;
		            }
		        }
			else if(dhdr->d_type == ESYM_NOTE_LOCKDATA)
				{

				Sym32_lockdata* lockdata = ADDR(Sym32_lockdata,eh,data+sizeof(Sym32_dhdr));
		        fprintf(core,"LOCK DATA SEGMENT\n\n");

				if( sizeof(Sym32_lockdata) != dhdr->d_descrsz )
					{
					fprintf(core,"\tWarning! :  sizeof(Sym32_lockdata) %d is different from descriptor size %d\n\n", 
						sizeof(Sym32_lockdata), dhdr->d_descrsz );
					}

				if( SYM32_LOCKDATA_SIZE != dhdr->d_descrsz )
					{
					fprintf(core,"\tWarning! : Expected Size of LOCK DATA SEGMENT %d is different from descriptor size %d\n\n", 
							SYM32_LOCKDATA_SIZE, dhdr->d_descrsz );
					}				
	            fprintf(core,"\tLock Data\n");
	            print_lock_data(lockdata);
				}
			}    
		else
			{
		 	//fprintf(core,"\t other p_types\n"); //unknown 
		 	continue;  
			}
		}
	
    //sections start here
   	if (symT==NULL)
		{
		fprintf(core,"\nSymbol table not found\n");
		}

	if (strtab==NULL)
		{	
		fprintf(core,"\nString table holding symbol names not found\n");
		}

	print_reloc(eh,symT, strtab);	// print relocation info showing symbol names and
					        // and the name of section in which the relocaton occurs.														
	for(k = 0; k < shnum; k++)
		{    
		unsigned char* data = ADDR(unsigned char, eh, shdr[k].sh_offset);	//pointer to the first byte in the section
															//unsigned char * data = (unsigned char * )(buffer+shdr[k].sh_offset);
		int size = shdr[k].sh_size;	// section size in bytes
	
		//print directive section
		if (!strcmp(".directive", &sname[shdr[k].sh_name])) 
			{ 
			print_sect_header(sname, shdr, k);
			print_directive(data,size);
			}

		if (!strcmp(".symtab", &sname[shdr[k].sh_name])) 
			{
			 print_sect_header(sname, shdr, k);	
			 // print global symbols
			 print_GlSymbols(eh,symT, strtab);								
			}

		//print relevant section header names
 		//print hex dump of relevant sections
		if (shdr[k].sh_type==1 || shdr[k].sh_type==4 || shdr[k].sh_type==6 ||
		    shdr[k].sh_type==9 || shdr[k].sh_type==11)
			{
			if (strcmp(".comment", &sname[shdr[k].sh_name])&&
				strcmp(".line", &sname[shdr[k].sh_name])   &&
				strcmp(".hash", &sname[shdr[k].sh_name])   &&
				strcmp(".note", &sname[shdr[k].sh_name])   &&
				strcmp(".directive", &sname[shdr[k].sh_name]) &&
				strncmp(".debug",&sname[shdr[k].sh_name] ,6))
				{
				if ( ! ( (ignoreSomeSections) &&
					 (strncmp(".rel.debug_", &sname[shdr[k].sh_name], 11)==0)
				       )
				   )
					{
					print_sect_header(sname, shdr, k);			
			 	    hexdump(data,size,k);
					}
				}
			}
		if (printAll)		// displays extra information
			{ 	
			if(k!=0)
			 	{
		 	 	print_sect_header(sname, shdr, k);					
		 		hexdump(data,size,k);				
		 		}
		 	}		
		}
	print_Summary(eh);	// print section names
	return 0;
}

int read_ar_element_header(char* ptr)
	{
	int length = strtol(ptr+48,0,10);

	if (strncmp(ptr+58, "\x60\x0A", 2) != 0)
		{
		return -1;
		}
	return length;
	}
	
int main(int argc, char* argv[])
	{
	struct stat results;
	FILE *elffile;
    int i=0;
	char* arg = NULL;
	int numberOfOptions=2;
    int remainder = 0;
    char *buffer = NULL;
    char *nextfile = NULL;

	printAll=0;
	ignoreSomeSections=0;

	if (argc<2)
		{
		fprintf(stderr,"File not specified");
		exit(1);
		}
	else if (argc>numberOfOptions+2)
		{
		fprintf(stderr,"Too many arguments");
		exit(1);
		}
	else
		{
		for (i=1;i<=argc-2;i++)
			{
			if ( strcmp("-i", argv[i]) ==0 )
				{
				ignoreSomeSections=1;
				}
			else if ( strcmp("-a", argv[i]) ==0 )
				{
				printAll=1;
				}
			}
		arg=argv[argc-1];
		}

    
    if((core = fopen("c:\\core", "w")) == NULL)
    {
        fprintf(stderr, "Error opening core\n");
        exit(1);
    }

	stat(arg, &results); 
	if((elffile  = fopen(arg, "rb" )) == NULL)
   		{
   		fprintf(stderr,"Error opening file %s", arg);
		exit (1); 
   		}

    buffer = (char*) calloc(results.st_size, sizeof(char));
	remainder = fread( buffer, sizeof( char ), results.st_size, elffile);
	fclose(elffile);						
	
	if (strncmp(buffer, "!<arch>\x0A", 8) != 0)
		{
		// plain ELF file
		if (do_elf_file(buffer, arg) != 0)
			{
			return 1;
			}
		return 0;
		}

    fclose(core);
	// library file
	nextfile = buffer;
	remainder = results.st_size;

#define ADVANCE(n)	nextfile+=(n); remainder-=(n);

	ADVANCE(8);
		
	while (remainder > 0)
		{
		int element_length = read_ar_element_header(nextfile);
		ADVANCE(60);
		
		if (element_length < 0 || element_length > remainder)
			{
			fprintf(stderr,"Error: archive file corrupt");
			return 1;
			}
		
		if (strncmp(nextfile, "\x7F\x45\x4C\x46",4) == 0)
			{
			if (do_elf_file(nextfile, "archive_element") != 0)
				{
				return 1;
				}
			}
		element_length += element_length&1;	// round up to a multiple of 2
		ADVANCE(element_length);
		}
		
    free(buffer);
	return 0;
	}

