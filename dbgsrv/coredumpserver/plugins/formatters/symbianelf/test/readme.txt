Directory for:
  coredump\plugins\formatters\symbian_elf\
test files

The tests at this level should be unit tests and should be run whenever the plugin is changed.

- test_crashdatasource is a dll called test_crashdatasource.dll. 

This dll's export creates a CTestCrashDataSource object (CTestCrashDataSource isa CCrashDataSource). 
CTestCrashDataSource supplies simulated, fixed crash data.


- t_symbian_elf_formatter is the unit test program. 

It creates a data source from the dll test_crashdatasource.dll.
It then lists the available Symbian ELF formatting plugins.
Then it instanciates each formatter, configures the CTestCrashDataSource as their data source, 
and then calls CrashEvent(). The formatter plugin will then call whatever it needs from the the
data source object to generate an elf dump.


- To build, 

At \coredump\plugins\formatters\symbian_elf\group> 
  abld build ARMV5 UDEB

At \epoc32\rom\tools> 
  buildrom h2 techview symbian_elf_formatter.iby t_symbian_el_formatter.iby test_crashdatasource.iby -D_FULL_DEBUG

It is advisable to have the serial output connected, for logging.

Then from e_shell, run
  t_symbian_elf_formatter.exe




Note: 
The reason for the dll, as opposed to a static library, is that at run 
time there is a check that fails when loading the formatter plugin at 
base\e32\memmodel\epoc\pcodeseg.cpp::DEpocCodeSeg::DoCreate() :: 

  if ( (DLL.RomInfo().iFlags & (KRomImageFlagExeInTree|KRomImageFlagDll)) == (KRomImageFlagExeInTree|KRomImageFlagDll))
    ....
    if ( NOT( Process->iXIP ) || ( &Process->RomInfo() != DLL.RomInfo().Dll Reference table[0].RomInfo() ))
