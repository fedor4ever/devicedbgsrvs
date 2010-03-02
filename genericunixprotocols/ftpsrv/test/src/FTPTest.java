//
// Copyright (c) 2005-2009 Nokia Corporation and/or its subsidiary(-ies).
// All rights reserved.
// This component and the accompanying materials are made available
// under the terms of "Eclipse Public License v1.0"
// which accompanies this distribution, and is available
// at the URL "http://www.eclipse.org/legal/epl-v10.html".
//
// Initial Contributors:
// Nokia Corporation - initial contribution.
//
// Contributors:
//
// Description:
//

import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.OutputStream;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import junit.framework.TestCase;

import org.apache.commons.net.ftp.FTP;
import org.apache.commons.net.ftp.FTPClient;
import org.apache.commons.net.ftp.FTPFile;
import org.apache.commons.net.ftp.FTPReply;




/** 
 * @author JavierM
 * 
 */
public abstract class FTPTest extends TestCase {

	/* (non-Javadoc)
	 * @see junit.framework.TestCase#setUp()
	 */
	FTPClient ftpClient = new FTPClient();
	
	protected void setUp() throws Exception {
		super.setUp();
		
		String address = System.getProperty("address");
		
		ftpClient.connect(/*"10.16.160.35"*/address);
		ftpClient.user("me");
		ftpClient.pass("friend");
		
		ftpClient.setBufferSize(1460*10);
		
		setPassive();
	}

	protected void setPassive()
	{
		ftpClient.enterLocalActiveMode();
	}
	
	/* (non-Javadoc)
	 * @see junit.framework.TestCase#tearDown()
	 */
	protected void tearDown() throws Exception {
		super.tearDown();
		
		ftpClient.logout();
		ftpClient.disconnect();
	}
	
	
//	public void testUploadFileInSmallChunks() throws IOException
//	{
//		
//		byte b[] = new byte[1];
//		
//		String sentence = new String(b);//"El veloz murciélago hind?comía feliz cardillo y kiwi\r\n";
//		int times =  10000;
//		
//		assertTrue(FTPReply.isPositiveCompletion(ftpClient.cwd("/C")));
//		
//		//BINARY
//		
//		assertTrue(ftpClient.setFileType(FTP.BINARY_FILE_TYPE));
//		
//		OutputStream out = ftpClient.storeFileStream("test.txt");
//		assertNotNull(out);
//		
//		for (int i = 0; i < times; i++) {
//			out.write(sentence.getBytes());	
//			out.flush();
//			System.out.print(".");
//		}
//		
//		out.close();
//		assertTrue(ftpClient.completePendingCommand());
//		
//		assertTrue(FTPReply.isPositiveCompletion(ftpClient.sendCommand("size /C/test.txt")));
//		assertEquals(times*sentence.length(), Integer.parseInt(ftpClient.getReplyString().substring(4).trim()));
//		
//		ftpClient.deleteFile("/C/test.txt");
//		
//		//ASCII
//		
//		assertTrue(ftpClient.setFileType(FTP.ASCII_FILE_TYPE));
//		
//		out = ftpClient.storeFileStream("test.txt");
//		assertNotNull(out);
//		
//		for (int i = 0; i < times; i++) {
//			out.write(sentence.getBytes());	
//		}
//		
//		out.close();
//		assertTrue(ftpClient.completePendingCommand());
//		
//		assertTrue(FTPReply.isPositiveCompletion(ftpClient.sendCommand("size /C/test.txt")));
//		assertEquals(times*sentence.length(), Integer.parseInt(ftpClient.getReplyString().substring(4).trim()));
//		
//		ftpClient.deleteFile("/C/test.txt");
//		
//	}
	
	
	public void testSyst()  throws IOException
	{
		String name = ftpClient.getSystemName();
		assertEquals(name,"UNIX Type: L8");
		
	}
	
	
	
	public void testListDrives()  throws IOException{
		
		assertTrue(FTPReply.isPositiveCompletion(ftpClient.cwd("/")));
		FTPFile[] drives = ftpClient.listFiles();
		
		assertNotNull(drives);
		
		for (int i = 0; i < drives.length; i++) {
			
			String driveName  = drives[i].getName();
			
			assertTrue(driveName.length()==1 && 
					   driveName.charAt(0) >= 'A' && 
					   driveName.charAt(0) <= 'Z');
		}
		
		String[] names = ftpClient.listNames();
		
		assertNotNull(names);
		
		for (int i = 0; i < drives.length; i++) {
			
			String driveName  = names[i];
			
			assertTrue(driveName.length()==1 && 
					   driveName.charAt(0) >= 'A' && 
					   driveName.charAt(0) <= 'Z');
		}
		
		
	}
	
	public void testListFiles()  throws IOException{
		
		FTPFile[] files;
			
		//UNIX parseable contents on C	
			
		assertTrue(FTPReply.isPositiveCompletion(ftpClient.cwd("/C")));
		files = ftpClient.listFiles();
		
		assertNotNull(files);
		
		//UNIX parseable contents on Z	
		
		assertTrue(FTPReply.isPositiveCompletion(ftpClient.cwd("/Z")));
		files = ftpClient.listFiles();
		assertNotNull(files); 
		
	}
	
public void testListLocation() throws IOException{
		
		
		FTPFile[] files1;
		FTPFile[] files2;
		
		//UNIX parseable contents on /C/System	
			
		files1 = ftpClient.listFiles("/C/System");
		
		assertTrue(FTPReply.isPositiveCompletion(ftpClient.cwd("/C/System")));
		files2 = ftpClient.listFiles();
		
		assertEquals(files2.length, files1.length);

		
		
		//UNIX parseable contents on root	
		
 		files1 = ftpClient.listFiles("/");
		
		assertTrue(FTPReply.isPositiveCompletion(ftpClient.cwd("/")));
		files2 = ftpClient.listFiles();
		
		assertEquals(files2.length, files1.length);
		
		//UNIX parseable contents on /C	
		
		files1 = ftpClient.listFiles("/C");
		
		assertTrue(FTPReply.isPositiveCompletion(ftpClient.cwd("/C")));
		files2 = ftpClient.listFiles();
		
		assertEquals(files2.length, files1.length);

	
	}
	
	
	public void testCreateDeleteFolder() throws IOException
	{
		//clean up
		assertTrue(FTPReply.isPositiveCompletion(ftpClient.cwd("/C")));
		ftpClient.removeDirectory("testFolder");
		
		
		assertTrue(ftpClient.makeDirectory("testFolder"));
		ftpClient.removeDirectory("testFolder");
		
		assertTrue(FTPReply.isPositiveCompletion(ftpClient.cwd("/")));
		assertTrue(ftpClient.makeDirectory("/C/testFolder"));
		assertTrue(ftpClient.removeDirectory("/C/testFolder"));
		
		
		//clean up
		assertTrue(FTPReply.isPositiveCompletion(ftpClient.cwd("/C")));
		ftpClient.removeDirectory("testFolder");
		
		
	}
	
	
	public void testStoreBinaryFile() throws IOException
	{
		
		//for (int i = 0; i < 20; i++) {
			assertTrue(ftpClient.storeFile("/C/testDriver_TEF_0x10210D31.sis", new FileInputStream("d:\\testDriver_TEF_0x10210D32.sis")));
			ftpClient.deleteFile("/C/testDriver_TEF_0x10210D31.sis");
		//	System.out.println(i);
		//}
	}
	
	public void testStoreAsciiFile() throws IOException
	{
		
		//for (int i = 0; i < 20; i++) {
		//	assertTrue(ftpClient.storeFile("/C/donquixote.txt", new FileInputStream("./data/donquixote.txt")));
			ftpClient.deleteFile("/C/donquixote.txt");
		//	System.out.println(i);
		//}
	}
	
	
	public void testCreateDeleteFile() throws IOException
	{
		
		//clean possible previous failed tests
		ftpClient.deleteFile("/C/test.txt");
		
		//from same location
		
		OutputStream out = ftpClient.storeFileStream("/C/test.txt");
		assertNotNull(out);
		out.write("test file".getBytes());
		out.close();
		ftpClient.completePendingCommand();
		
		assertTrue(FTPReply.isPositiveCompletion(ftpClient.cwd("/C")));
		assertTrue(ftpClient.deleteFile("test.txt"));
		
		//from different location
		
		out = ftpClient.storeFileStream("/C/test.txt");
		assertNotNull(out);
		out.write("test file".getBytes());
		out.close();
		ftpClient.completePendingCommand();
		
		assertTrue(FTPReply.isPositiveCompletion(ftpClient.cwd("/")));
		assertTrue(ftpClient.deleteFile("/C/test.txt"));
		
	}
	
	public void testRename() throws IOException {
		
		OutputStream out = ftpClient.storeFileStream("/C/test.txt");
		assertNotNull(out);
		out.write("test file".getBytes());
		out.close();
		ftpClient.completePendingCommand();
		
		//from directory to same directory
		assertTrue(FTPReply.isPositiveCompletion(ftpClient.cwd("/C")));
		assertTrue(ftpClient.rename("test.txt", "test2.txt"));
		assertTrue(FTPReply.isPositiveCompletion(ftpClient.sendCommand("size test2.txt")));
		
		//from different directory
		assertTrue(FTPReply.isPositiveCompletion(ftpClient.cwd("/")));
		assertTrue(ftpClient.rename("/C/test2.txt", "/C/test.txt"));
		assertTrue(FTPReply.isPositiveCompletion(ftpClient.sendCommand("size test.txt")));
		
		assertTrue(ftpClient.deleteFile("/C/test.txt"));
		
	}
	
	public void testMove() throws IOException {
		
		OutputStream out = ftpClient.storeFileStream("/C/test.txt");
		assertNotNull(out);
		out.write("test file".getBytes());
		out.close();
		ftpClient.completePendingCommand();
		
		assertTrue(ftpClient.makeDirectory("/C/testFolder"));
		
		assertTrue(FTPReply.isPositiveCompletion(ftpClient.cwd("/")));
		assertTrue(ftpClient.rename("/C/test.txt", "/C/testFolder/test.txt"));
		
		assertTrue(FTPReply.isPositiveCompletion(ftpClient.cwd("/C/testFolder")));
		assertTrue(ftpClient.rename("test.txt", "/C/test.txt"));
		assertTrue(ftpClient.rename("/C/test.txt", "test.txt"));
		
		assertTrue(ftpClient.deleteFile("/C/testFolder/test.txt"));
		assertTrue(ftpClient.removeDirectory("/C/testFolder"));
		
	}
	
	public void testCdup() throws IOException {
		
		
		assertTrue(FTPReply.isPositiveCompletion(ftpClient.cwd("/C/system")));
		
		assertTrue(FTPReply.isPositiveCompletion(ftpClient.cdup()));
		assertEquals("/C", ftpClient.printWorkingDirectory());
		
		assertTrue(FTPReply.isPositiveCompletion(ftpClient.cdup()));
		assertEquals("/", ftpClient.printWorkingDirectory());
		
	}
	
	public void testBrowse() throws IOException {
	
		assertTrue(FTPReply.isPositiveCompletion(ftpClient.cwd("/")));
		assertEquals("/", ftpClient.printWorkingDirectory());
		
		assertTrue(FTPReply.isPositiveCompletion(ftpClient.cwd("C")));
		assertEquals("/C", ftpClient.printWorkingDirectory());
		
		assertTrue(FTPReply.isPositiveCompletion(ftpClient.cwd("system")));
		assertEquals("/C/system", ftpClient.printWorkingDirectory());
	
		assertTrue(FTPReply.isPositiveCompletion(ftpClient.cwd("/")));
		assertEquals("/", ftpClient.printWorkingDirectory());
		
		assertTrue(FTPReply.isPositiveCompletion(ftpClient.cwd("/C/system")));
		assertEquals("/C/system", ftpClient.printWorkingDirectory());
		
		assertTrue(FTPReply.isPositiveCompletion(ftpClient.cwd("/")));
		assertEquals("/", ftpClient.printWorkingDirectory());
		
		assertTrue(FTPReply.isPositiveCompletion(ftpClient.cwd("/C/system/.././system")));
		assertEquals("/C/system", ftpClient.printWorkingDirectory());
		
		assertTrue(FTPReply.isPositiveCompletion(ftpClient.cwd("..")));
		assertEquals("/C", ftpClient.printWorkingDirectory());
		
		assertTrue(FTPReply.isPositiveCompletion(ftpClient.cwd("..")));
		assertEquals("/", ftpClient.printWorkingDirectory());
		
			
		
	}
	
	public void testHelp() throws IOException {
		
		assertTrue(FTPReply.isPositiveCompletion(ftpClient.sendCommand("help")));
		
	}
	
	public void testStat() throws IOException {
		
		assertTrue(FTPReply.isPositiveCompletion(ftpClient.stat()));
		
	}
	
	public void testWrongCommands() throws IOException {
		
		assertTrue(FTPReply.isNegativePermanent(ftpClient.sendCommand("!\"?%^&*())__+")));
		assertTrue(FTPReply.isNegativePermanent(ftpClient.sendCommand("asdasdasdasdsdjkklahsdjkajkhsdkhajksdasjkdjkasjkdhasjkdhasdhjkashdjkhasjkdhasjdhjkashdjkhasjkdhjkashashdhasjdhjashdjhasjkhdjkashdhjkashdjkashdhasdkhasdjkahskdajkshdjkashdjkashdjkashdjkashjkdhasjkdhkashkdhasdjkahsjkdhasjkhdjkashdjkhaskdjhajkshdjkahsdjkahsdkjahksdh")));
		assertTrue(FTPReply.isNegativePermanent(ftpClient.sendCommand("asda sda sda sd sdj kk  l   \t	ahs \n \r")));
		
	}
	
	public void testGetSystemDrive() throws IOException{
		
		assertTrue(FTPReply.isPositiveCompletion(ftpClient.stat()));
		String s = ftpClient.getReplyString();
		assertNotNull(s);
		
		Pattern p = Pattern.compile(".*System Drive: (\\w).*", Pattern.DOTALL); //Pattern.DOTALL => '.' includes end of line
		Matcher m = p.matcher(s);
		assertTrue(m.matches());
		
	}
	
	public void testSize() throws IOException {
		
		//try{
		String testString = "test file";
		
		OutputStream out = ftpClient.storeFileStream("/C/test.txt");
		assertNotNull(out);
		out.write(testString.getBytes());
		out.close();
		ftpClient.completePendingCommand();
		
		//absolute
		//file
		assertTrue(FTPReply.isPositiveCompletion(ftpClient.sendCommand("size /C/test.txt")));
		assertEquals(testString.length(), Integer.parseInt(ftpClient.getReplyString().substring(4).trim()));
		
		//folder
		assertTrue(FTPReply.isNegativePermanent(ftpClient.sendCommand("size /C/system")));
		assertTrue(ftpClient.getReplyString().matches(".*not a plain file.*\r\n"));
		
		
		//relative
		//file
		assertTrue(FTPReply.isPositiveCompletion(ftpClient.cwd("/C")));
		assertTrue(FTPReply.isPositiveCompletion(ftpClient.sendCommand("size test.txt")));
		assertEquals(testString.length(), Integer.parseInt(ftpClient.getReplyString().substring(4).trim()));
		
		//folder
		assertTrue(FTPReply.isNegativePermanent(ftpClient.sendCommand("size system")));
		assertTrue(ftpClient.getReplyString().matches(".*not a plain file.*\r\n"));
		
		ftpClient.deleteFile("/C/test.txt");
	}
	
	
	//FTP&Telnet-File Transfer-001
	//To Transfer a file of size 0 from Pc to device (H2 /H4) from current working directory
	
	public void testFTP_Telnet_File_Transfer_001() throws IOException {
		
		String fileUnderTest = "emptyFile";
		
		File f = new File("./data/"+fileUnderTest);
		
		if(!f.exists())
		{
			f.createNewFile();
		}
		
		//ASCII
		assertTrue(ftpClient.setFileType(FTP.ASCII_FILE_TYPE));
		
		assertTrue(ftpClient.storeFile("/C/"+fileUnderTest, new FileInputStream(f)));
		assertTrue(FTPReply.isPositiveCompletion(ftpClient.sendCommand("size /C/"+fileUnderTest)));
		assertEquals(f.length(), Integer.parseInt(ftpClient.getReplyString().substring(4).trim()));
		ftpClient.deleteFile("/C/"+fileUnderTest);
		
		//BINARY
		assertTrue(ftpClient.setFileType(FTP.BINARY_FILE_TYPE));
		
		assertTrue(ftpClient.storeFile("/C/"+fileUnderTest, new FileInputStream(f)));
		assertTrue(FTPReply.isPositiveCompletion(ftpClient.sendCommand("size /C/"+fileUnderTest)));
		assertEquals(f.length(), Integer.parseInt(ftpClient.getReplyString().substring(4).trim()));
		ftpClient.deleteFile("/C/"+fileUnderTest);
		
	}
	
	//FTP&Telnet-File Transfer-002
	//To Transfer a file of normal size  from Pc to device (H2 /H4) from current working directory
	
	public void testFTP_Telnet_File_Transfer_002_ASCII() throws IOException {
		
		String fileUnderTest = "smallfile.txt";
		File f = new File("./data/"+fileUnderTest);
		
		//ASCII
		assertTrue(ftpClient.setFileType(FTP.ASCII_FILE_TYPE));
		
		assertTrue(ftpClient.storeFile("/C/"+fileUnderTest, new FileInputStream(f)));
		assertTrue(FTPReply.isPositiveCompletion(ftpClient.sendCommand("size /C/"+fileUnderTest)));
		assertEquals(f.length(), Integer.parseInt(ftpClient.getReplyString().substring(4).trim()));
		assertTrue(ftpClient.deleteFile("/C/"+fileUnderTest));
		
		//BINARY
		assertTrue(ftpClient.setFileType(FTP.BINARY_FILE_TYPE));
		
		assertTrue(ftpClient.storeFile("/C/"+fileUnderTest, new FileInputStream(f)));
		assertTrue(FTPReply.isPositiveCompletion(ftpClient.sendCommand("size /C/"+fileUnderTest)));
		assertEquals(f.length(), Integer.parseInt(ftpClient.getReplyString().substring(4).trim()));
		assertTrue(ftpClient.deleteFile("/C/"+fileUnderTest));
		
	}
	
	//FTP&Telnet-File Transfer-002
	//To Transfer a file of normal size  from Pc to device (H2 /H4) from current working directory
	
	public void testFTP_Telnet_File_Transfer_002_BINARY() throws IOException {
		
		String fileUnderTest = "smallfile.bin";
		File f = new File("./data/"+fileUnderTest);
		
		//ASCII
		assertTrue(ftpClient.setFileType(FTP.ASCII_FILE_TYPE));
		
		assertTrue(ftpClient.storeFile("/C/"+fileUnderTest, new FileInputStream(f)));
		assertTrue(FTPReply.isPositiveCompletion(ftpClient.sendCommand("size /C/"+fileUnderTest)));
		assertTrue(f.length() != Integer.parseInt(ftpClient.getReplyString().substring(4).trim()));
		assertTrue(ftpClient.deleteFile("/C/"+fileUnderTest));
		
		//BINARY
		assertTrue(ftpClient.setFileType(FTP.BINARY_FILE_TYPE));
		
		assertTrue(ftpClient.storeFile("/C/"+fileUnderTest, new FileInputStream(f)));
		assertTrue(FTPReply.isPositiveCompletion(ftpClient.sendCommand("size /C/"+fileUnderTest)));
		assertEquals(f.length(), Integer.parseInt(ftpClient.getReplyString().substring(4).trim()));
		assertTrue(ftpClient.deleteFile("/C/"+fileUnderTest));
		
	}
	
	
	//FTP&Telnet-File Transfer-003
	//To Transfer a file of Large size  from Pc to device (H2 /H4) from current working directory
	
	public void testFTP_Telnet_File_Transfer_003_ASCII() throws IOException {
		
		String fileUnderTest = "bigfile.txt";
		File f = new File("./data/"+fileUnderTest);
		
		//ASCII
		assertTrue(ftpClient.setFileType(FTP.ASCII_FILE_TYPE));
		
		assertTrue(ftpClient.storeFile("/C/"+fileUnderTest, new FileInputStream(f)));
		assertTrue(FTPReply.isPositiveCompletion(ftpClient.sendCommand("size /C/"+fileUnderTest)));
		assertEquals(f.length(), Integer.parseInt(ftpClient.getReplyString().substring(4).trim()));
		assertTrue(ftpClient.deleteFile("/C/"+fileUnderTest));
		
		//BINARY
		assertTrue(ftpClient.setFileType(FTP.BINARY_FILE_TYPE));
		
		assertTrue(ftpClient.storeFile("/C/"+fileUnderTest, new FileInputStream(f)));
		assertTrue(FTPReply.isPositiveCompletion(ftpClient.sendCommand("size /C/"+fileUnderTest)));
		assertEquals(f.length(), Integer.parseInt(ftpClient.getReplyString().substring(4).trim()));
		assertTrue(ftpClient.deleteFile("/C/"+fileUnderTest));
		
	}
	
	public void testFTP_Telnet_File_Transfer_003_BINARY() throws IOException {
		
		String fileUnderTest = "bigfile.bin";
		File f = new File("./data/"+fileUnderTest);
		
		//ASCII
		assertTrue(ftpClient.setFileType(FTP.ASCII_FILE_TYPE));
		
		assertTrue(ftpClient.storeFile("/C/"+fileUnderTest, new FileInputStream(f)));
		assertTrue(FTPReply.isPositiveCompletion(ftpClient.sendCommand("size /C/"+fileUnderTest)));
		assertTrue(f.length() != Integer.parseInt(ftpClient.getReplyString().substring(4).trim()));
		assertTrue(ftpClient.deleteFile("/C/"+fileUnderTest));
		
		//BINARY
		assertTrue(ftpClient.setFileType(FTP.BINARY_FILE_TYPE));
		
		assertTrue(ftpClient.storeFile("/C/"+fileUnderTest, new FileInputStream(f)));
		assertTrue(FTPReply.isPositiveCompletion(ftpClient.sendCommand("size /C/"+fileUnderTest)));
		assertEquals(f.length(), Integer.parseInt(ftpClient.getReplyString().substring(4).trim()));
		assertTrue(ftpClient.deleteFile("/C/"+fileUnderTest));
		
	}
	
	//FTP&Telnet-Connection-003
	//To change the mode of transfer from remote host to Device (H2/H4)
	public void testFTP_Telnet_Connection_Execution_003() throws IOException
	{
		assertTrue(ftpClient.setFileType(FTP.ASCII_FILE_TYPE));
		assertTrue(ftpClient.setFileType(FTP.BINARY_FILE_TYPE));
	}
	
	//FTP&Telnet-File Transfer-007
	//To Transfer a file from Pc to device (H2 /H4) from  working directory to non Existence directory
	public void testFTP_Telnet_File_Transfer_007_ASCII_Mode() throws IOException
	{
		assertTrue(ftpClient.setFileType(FTP.ASCII_FILE_TYPE));
		assertFalse(ftpClient.storeFile("/C/nonexistingfolder/smallfile.txt", new FileInputStream("./data/smallfile.txt")));
	}
	
	//FTP&Telnet-File Transfer-007
	//To Transfer a file from Pc to device (H2 /H4) from  working directory to non Existence directory
	public void testFTP_Telnet_File_Transfer_007_BINARY_Mode() throws IOException
	{
		assertTrue(ftpClient.setFileType(FTP.BINARY_FILE_TYPE));
		assertFalse(ftpClient.storeFile("/C/nonexistingfolder/smallfile.txt", new FileInputStream("./data/smallfile.txt")));
	}
	
	//FTP&Telnet-File Retrieve-001
	//To Retrieve a file of size 0 from device to PC
	
	public void testFTP_Telnet_File_Retrieve_001() throws IOException {
		
		String fileUnderTest = "emptyFile";
		
		File f = new File("./data/"+fileUnderTest);
		
		if(!f.exists())
			f.createNewFile();
		
		f.deleteOnExit();
		
		//upload and verify correctly uploaded
		assertTrue(ftpClient.setFileType(FTP.BINARY_FILE_TYPE));
		
		assertTrue(ftpClient.storeFile("/C/"+fileUnderTest, new FileInputStream(f)));
		assertTrue(FTPReply.isPositiveCompletion(ftpClient.sendCommand("size /C/"+fileUnderTest)));
		assertEquals(f.length(), Integer.parseInt(ftpClient.getReplyString().substring(4).trim()));
		
		
		//download file
		
		File f2 = File.createTempFile("FTP_Telnet_File_Retrieve_001",".txt");
		File f3 = File.createTempFile("FTP_Telnet_File_Retrieve_001",".bin");
		f2.deleteOnExit();
		f3.deleteOnExit();
		
		//ASCII download
		
		assertTrue(ftpClient.setFileType(FTP.ASCII_FILE_TYPE));
		assertTrue(ftpClient.retrieveFile("/C/"+fileUnderTest, new FileOutputStream(f2)));
		assertEquals(f.length(), f2.length());
		
		//BINARY download
		
		assertTrue(ftpClient.setFileType(FTP.BINARY_FILE_TYPE));
		assertTrue(ftpClient.retrieveFile("/C/"+fileUnderTest, new FileOutputStream(f3)));
		assertEquals(f.length(), f3.length());
		
		assertTrue(ftpClient.deleteFile("/C/"+fileUnderTest));
		
	}
	
	//FTP&Telnet-File Retrieve-002
	//To Retrieve a file of normal size  from device to Pc 
	
	public void testFTP_Telnet_File_Retrieve_002_ASCII() throws IOException {
		
		String fileUnderTest = "smallfile.txt";
		File f = new File("./data/"+fileUnderTest);
		
		//ASCII
		assertTrue(ftpClient.setFileType(FTP.ASCII_FILE_TYPE));
		
		assertTrue(ftpClient.storeFile("/C/"+fileUnderTest, new FileInputStream(f)));
		assertTrue(FTPReply.isPositiveCompletion(ftpClient.sendCommand("size /C/"+fileUnderTest)));
		assertEquals(f.length(), Integer.parseInt(ftpClient.getReplyString().substring(4).trim()));
		
		//download file
		
		File f2 = File.createTempFile("FTP_Telnet_File_Retrieve_002_ASCII",".txt");
		File f3 = File.createTempFile("FTP_Telnet_File_Retrieve_002_ASCII",".bin");
		f3.deleteOnExit();
		f2.deleteOnExit();
		
		//ASCII download
		
		assertTrue(ftpClient.setFileType(FTP.ASCII_FILE_TYPE));
		assertTrue(ftpClient.retrieveFile("/C/"+fileUnderTest, new FileOutputStream(f2)));
		assertEquals(f.length(), f2.length());
		
		//BINARY download
		
		assertTrue(ftpClient.setFileType(FTP.BINARY_FILE_TYPE));
		assertTrue(ftpClient.retrieveFile("/C/"+fileUnderTest, new FileOutputStream(f3)));
		assertEquals(f.length(), f3.length());
		
		assertTrue(ftpClient.deleteFile("/C/"+fileUnderTest));
		
	}
	
	//FTP&Telnet-File Retrieve-002
	//To Transfer a file of normal size  from  device to pc
	
	public void testFTP_Telnet_File_Retrieve_002_BINARY() throws IOException {
		
		String fileUnderTest = "smallfile.bin";
		File f = new File("./data/"+fileUnderTest);
		
		//BINARY
		assertTrue(ftpClient.setFileType(FTP.BINARY_FILE_TYPE));
		
		assertTrue(ftpClient.storeFile("/C/"+fileUnderTest, new FileInputStream(f)));
		assertTrue(FTPReply.isPositiveCompletion(ftpClient.sendCommand("size /C/"+fileUnderTest)));
		assertEquals(f.length(), Integer.parseInt(ftpClient.getReplyString().substring(4).trim()));
		
		//download file
		
		File f2 = File.createTempFile("FTP_Telnet_File_Retrieve_002_BINARY",".txt");
		File f3 = File.createTempFile("FTP_Telnet_File_Retrieve_002_BINARY",".bin");
		f2.deleteOnExit();
		f3.deleteOnExit();
		
		//ASCII download
		
		assertTrue(ftpClient.setFileType(FTP.ASCII_FILE_TYPE));
		assertTrue(ftpClient.retrieveFile("/C/"+fileUnderTest, new FileOutputStream(f2)));
		assertEquals(f.length() , f2.length());
		
		//BINARY download
		
		assertTrue(ftpClient.setFileType(FTP.BINARY_FILE_TYPE));
		assertTrue(ftpClient.retrieveFile("/C/"+fileUnderTest, new FileOutputStream(f3)));
		assertEquals(f.length(), f3.length());
		
		assertTrue(ftpClient.deleteFile("/C/"+fileUnderTest));
		
		
		
	}
	
	
	//FTP&Telnet-File Retrieve-003
	//To Transfer a file of Large size  from device to pc
	
	public void testFTP_Telnet_File_Retrieve_003_ASCII() throws IOException {
		
		String fileUnderTest = "bigfile.txt";
		File f = new File("./data/"+fileUnderTest);
		
		//ASCII
		assertTrue(ftpClient.setFileType(FTP.ASCII_FILE_TYPE));
		
		assertTrue(ftpClient.storeFile("/C/"+fileUnderTest, new FileInputStream(f)));
		assertTrue(FTPReply.isPositiveCompletion(ftpClient.sendCommand("size /C/"+fileUnderTest)));
		assertEquals(f.length(), Integer.parseInt(ftpClient.getReplyString().substring(4).trim()));
	
		//download file
		
		File f2 = File.createTempFile("testFTP_Telnet_File_Retrieve_003_ASCII",".txt");
		File f3 = File.createTempFile("testFTP_Telnet_File_Retrieve_003_ASCII",".bin");
		f2.deleteOnExit();
		
		//ASCII download
		
		assertTrue(ftpClient.setFileType(FTP.ASCII_FILE_TYPE));
		assertTrue(ftpClient.retrieveFile("/C/"+fileUnderTest, new FileOutputStream(f2)));
		assertEquals(f.length(), f2.length());
		
		//BINARY download
		
		assertTrue(ftpClient.setFileType(FTP.BINARY_FILE_TYPE));
		assertTrue(ftpClient.retrieveFile("/C/"+fileUnderTest, new FileOutputStream(f3)));
		assertEquals(f.length(), f3.length());
		
		assertTrue(ftpClient.deleteFile("/C/"+fileUnderTest));
		
	}
	
	//FTP&Telnet-File Retrieve-003
	//To Transfer a file of Large size from device to pc
	
	
	public void testFTP_Telnet_File_Retrieve_003_BINARY() throws IOException {
		
		String fileUnderTest = "bigfile.bin";
		File f = new File("./data/"+fileUnderTest);
		
		//BINARY
		assertTrue(ftpClient.setFileType(FTP.BINARY_FILE_TYPE));
		
		assertTrue(ftpClient.storeFile("/C/"+fileUnderTest, new FileInputStream(f)));
		assertTrue(FTPReply.isPositiveCompletion(ftpClient.sendCommand("size /C/"+fileUnderTest)));
		assertEquals(f.length(), Integer.parseInt(ftpClient.getReplyString().substring(4).trim()));
		
		//download file
		
		File f2 = File.createTempFile("FTP_Telnet_File_Retrieve_003_BINARY",".txt");
		File f3 = File.createTempFile("FTP_Telnet_File_Retrieve_003_BINARY",".bin");
		f2.deleteOnExit();
		f3.deleteOnExit();
		
		//ASCII download
		
		assertTrue(ftpClient.setFileType(FTP.ASCII_FILE_TYPE));
		assertTrue(ftpClient.retrieveFile("/C/"+fileUnderTest, new FileOutputStream(f2)));
		assertEquals(f.length() , f2.length());
		
		//BINARY download
		
		assertTrue(ftpClient.setFileType(FTP.BINARY_FILE_TYPE));
		assertTrue(ftpClient.retrieveFile("/C/"+fileUnderTest, new FileOutputStream(f3)));
		assertEquals(f.length(), f3.length());
		
		
		assertTrue(ftpClient.deleteFile("/C/"+fileUnderTest));
		
	}
	
  	//FTP&Telnet-File Retrieve-009
	//retrieve from non-existing directory
	
	public void testFTP_Telnet_File_Retrieve_009() throws IOException {
		
		assertFalse(ftpClient.retrieveFile("/C/nonexisting", System.out));
		
	}
	
	//FTP&Telnet-File Retrieve-016
	//To Retrieve  a Directory from  device (H2 /H4) to PC
	
	public void testFTP_Telnet_File_Retrieve_016() throws IOException {
		
		assertFalse(ftpClient.retrieveFile("/C/system", System.out));
		
	}
	
	//FTP&Telnet-File Delete-001
	//To Delete  a file of size 0 from Pc
	
	public void testFTP_Telnet_File_Delete_001() throws IOException {
		
		String fileUnderTest = "emptyFile";
		
		File f = new File("./data/"+fileUnderTest);
		
		//upload and verify correctly uploaded
		assertTrue(ftpClient.setFileType(FTP.BINARY_FILE_TYPE));
		
		assertTrue(ftpClient.storeFile("/C/"+fileUnderTest, new FileInputStream(f)));
		assertTrue(FTPReply.isPositiveCompletion(ftpClient.sendCommand("size /C/"+fileUnderTest)));
		assertEquals(f.length(), Integer.parseInt(ftpClient.getReplyString().substring(4).trim()));
		
		assertTrue(ftpClient.deleteFile("/C/"+fileUnderTest));
		
		assertTrue(FTPReply.isNegativePermanent(ftpClient.sendCommand("size /C/"+fileUnderTest)));
	}
	
	//FTP&Telnet-File Delete-003
	//To Delete a file of normal size on Device (H2/H4)  from Pc
	public void testFTP_Telnet_File_Delete_003() throws IOException {
		
		String fileUnderTest = "smallfile.bin";
		File f = new File("./data/"+fileUnderTest);
		
		//BINARY
		assertTrue(ftpClient.setFileType(FTP.BINARY_FILE_TYPE));
		
		assertTrue(ftpClient.storeFile("/C/"+fileUnderTest, new FileInputStream(f)));
		assertTrue(FTPReply.isPositiveCompletion(ftpClient.sendCommand("size /C/"+fileUnderTest)));
		assertEquals(f.length(), Integer.parseInt(ftpClient.getReplyString().substring(4).trim()));
		
		assertTrue(ftpClient.deleteFile("/C/"+fileUnderTest));
		
		assertTrue(FTPReply.isNegativePermanent(ftpClient.sendCommand("size /C/"+fileUnderTest)));
	}
	
	//FTP&Telnet-File Delete-004
	//To Delete a file of Large size on Device (H2/H4)  from Pc
	public void testFTP_Telnet_File_Delete_004() throws IOException {
		
		String fileUnderTest = "bigfile.bin";
		File f = new File("./data/"+fileUnderTest);
		
		//BINARY
		assertTrue(ftpClient.setFileType(FTP.BINARY_FILE_TYPE));
		
		assertTrue(ftpClient.storeFile("/C/"+fileUnderTest, new FileInputStream(f)));
		assertTrue(FTPReply.isPositiveCompletion(ftpClient.sendCommand("size /C/"+fileUnderTest)));
		assertEquals(f.length(), Integer.parseInt(ftpClient.getReplyString().substring(4).trim()));
		
		assertTrue(ftpClient.deleteFile("/C/"+fileUnderTest));
		
		assertTrue(FTPReply.isNegativePermanent(ftpClient.sendCommand("size /C/"+fileUnderTest)));
	}
	
	//FTP&Telnet-File Delete-006
	//To Delete a non existent  file on Device (H2/H4)  from Pc
	public void testFTP_Telnet_File_Delete_006() throws IOException {
		
		assertFalse(ftpClient.deleteFile("/C/nonexistingfile"));
	}
	
	//FTP&Telnet-File Retrieve-018
	//To Retrieve a drive listing on Device (H2/H4)  from Pc
	public void testFTP_Telnet_File_Retrieve_018() throws IOException {

		assertTrue(FTPReply.isPositiveCompletion(ftpClient.cwd("/")));
		FTPFile[] drives = ftpClient.listFiles();

		assertNotNull(drives);

		for (int i = 0; i < drives.length; i++) {

			String driveName = drives[i].getName();

			assertTrue(driveName.length() == 1 && driveName.charAt(0) >= 'A'
					&& driveName.charAt(0) <= 'Z');
		}

	}
	
	
	// FTP&Telnet-File Retrieve-019
	//To Retrieve a directory listing on Device (H2/H4)  from Pc
	
	public void testFTP_Telnet_File_Retrieve_019()  throws IOException{
		
		FTPFile[] files;
			
		//UNIX parseable contents on C	
			
		assertTrue(FTPReply.isPositiveCompletion(ftpClient.cwd("/C")));
		files = ftpClient.listFiles();
		
		assertNotNull(files);
		
		//UNIX parseable contents on Z	
		
		assertTrue(FTPReply.isPositiveCompletion(ftpClient.cwd("/Z")));
		files = ftpClient.listFiles();
		assertNotNull(files); 
		
	}
	
	//FTP&Telnet-File Retrieve-020
	//To check a particular file/Folder on Device (H2/H4)  from Pc
	
	public void testFTP_Telnet_File_Retrieve_020()  throws IOException{
		
		FTPFile[] files;
		String[] names;
		
		boolean found = false;
		
		//folder
		//List C:\System
		
		assertTrue(FTPReply.isPositiveCompletion(ftpClient.cwd("/C")));
		files = ftpClient.listFiles();
		
		for (int i = 0; i < files.length; i++) {
			if(files[i].getName().equalsIgnoreCase("System") && files[i].isDirectory())
				found = true;
		}
		
		assertTrue(found);
		
		found = false;
		
		assertTrue(FTPReply.isPositiveCompletion(ftpClient.cwd("/C")));
		
		names = ftpClient.listNames();
		
		for (int i = 0; i < names.length; i++) {
			if(names[i].equalsIgnoreCase("System"))
				found = true;
		}
		
		assertTrue(found);
		
		found = false;
		
		assertTrue(FTPReply.isPositiveCompletion(ftpClient.cwd("/")));
		
		files = ftpClient.listFiles("/C");
		
		for (int i = 0; i < files.length; i++) {
			if(files[i].getName().equalsIgnoreCase("System") && files[i].isDirectory())
				found = true;
		}
		
		assertTrue(found);
		
		
		found = false;
		
		assertTrue(FTPReply.isPositiveCompletion(ftpClient.cwd("/")));
		
		names = ftpClient.listNames("/C");
		
		for (int i = 0; i < names.length; i++) {
			if(names[i].equalsIgnoreCase("System"))
				found = true;
		}
		
		assertTrue(found);
		
		
		found = false;
		
		//file
		//List C:\System\System.ini	
		
		assertTrue(FTPReply.isPositiveCompletion(ftpClient.cwd("/C/System")));
		files = ftpClient.listFiles();
		
		for (int i = 0; i < files.length; i++) {
			if(files[i].getName().equalsIgnoreCase("System.ini") && files[i].isFile())
				found = true;
		}
		
		assertTrue(found);
		
		found = false;
		
		assertTrue(FTPReply.isPositiveCompletion(ftpClient.cwd("/C/System")));
		
		names = ftpClient.listNames();
		
		for (int i = 0; i < names.length; i++) {
			if(names[i].equalsIgnoreCase("System.ini"))
				found = true;
		}
		
		assertTrue(found);
		
		found = false;
		
		assertTrue(FTPReply.isPositiveCompletion(ftpClient.cwd("/")));
		files = ftpClient.listFiles("/C/System");
		
		for (int i = 0; i < files.length; i++) {
			if(files[i].getName().equalsIgnoreCase("System.ini") && files[i].isFile())
				found = true;
		}
		
		assertTrue(found);
		
		
		found = false;
		
		assertTrue(FTPReply.isPositiveCompletion(ftpClient.cwd("/")));
		
		names = ftpClient.listNames("/C/System");
		
		for (int i = 0; i < names.length; i++) {
			if(names[i].equalsIgnoreCase("System.ini"))
				found = true;
		}
		
		assertTrue(found);
		
		
	}
	
	//FTP&Telnet-Directory-001 + 002
	//To create a  Directory on device(H2/H4) from PC Side
	//To Remove a  Directory on device(H2/H4) from PC Side
	
	public void testFTP_Telnet_Directory_001()  throws IOException{
		
		boolean found = false;
		
		//create folder
		assertTrue(ftpClient.makeDirectory("/C/newFolder"));
		
		assertTrue(FTPReply.isPositiveCompletion(ftpClient.cwd("/C")));
		FTPFile[] files = ftpClient.listFiles();
		
		for (int i = 0; i < files.length; i++) {
			if(files[i].getName().equalsIgnoreCase("newFolder") && files[i].isDirectory())
				found = true;
		}
		
		assertTrue(found);
		
		found = false;
		
		//delete folder
		assertTrue(ftpClient.removeDirectory("/C/newFolder"));
		
		assertTrue(FTPReply.isPositiveCompletion(ftpClient.cwd("/C")));
		files = ftpClient.listFiles();
		
		for (int i = 0; i < files.length; i++) {
			if(files[i].getName().equalsIgnoreCase("newFolder") && files[i].isDirectory())
				found = true;
		}
		
		assertFalse(found);
				
	}
	
	//FTP&Telnet-Directory-003
	//To Rename a  file on device(H2/H4) from PC Side
	
	public void testFTP_Telnet_Directory_003()  throws IOException{
		
		OutputStream out = ftpClient.storeFileStream("/C/test.txt");
		assertNotNull(out);
		out.write("test file".getBytes());
		out.close();
		ftpClient.completePendingCommand();
		
		//from directory to same directory
		assertTrue(FTPReply.isPositiveCompletion(ftpClient.cwd("/C")));
		assertTrue(ftpClient.rename("test.txt", "test2.txt"));
		assertTrue(FTPReply.isPositiveCompletion(ftpClient.sendCommand("size test2.txt")));
		
		//from different directory
		assertTrue(FTPReply.isPositiveCompletion(ftpClient.cwd("/")));
		assertTrue(ftpClient.rename("/C/test2.txt", "/C/test.txt"));
		assertTrue(FTPReply.isPositiveCompletion(ftpClient.sendCommand("size test.txt")));
		
		assertTrue(ftpClient.deleteFile("/C/test.txt"));
	}
	
	//FTP&Telnet-Directory-004
	//To Move a  file on device(H2/H4) from PC Side
	
	public void testFTP_Telnet_Directory_004()  throws IOException{
		OutputStream out = ftpClient.storeFileStream("/C/test.txt");
		assertNotNull(out);
		out.write("test file".getBytes());
		out.close();
		ftpClient.completePendingCommand();
		
		assertTrue(ftpClient.makeDirectory("/C/testFolder"));
		
		assertTrue(FTPReply.isPositiveCompletion(ftpClient.cwd("/")));
		assertTrue(ftpClient.rename("/C/test.txt", "/C/testFolder/test.txt"));
		
		assertTrue(FTPReply.isPositiveCompletion(ftpClient.cwd("/C/testFolder")));
		assertTrue(ftpClient.rename("test.txt", "/C/test.txt"));
		assertTrue(ftpClient.rename("/C/test.txt", "test.txt"));
		
		assertTrue(ftpClient.deleteFile("/C/testFolder/test.txt"));
		assertTrue(ftpClient.removeDirectory("/C/testFolder"));
	}
	
	//FTP&Telnet-Directory-006
	//To Change the current directory  on device(H2/H4) from PC Side
	public void testFTP_Telnet_Directory_006()  throws IOException{
		
		assertTrue(FTPReply.isPositiveCompletion(ftpClient.cwd("/")));
		assertEquals("/", ftpClient.printWorkingDirectory());
		
		assertTrue(FTPReply.isPositiveCompletion(ftpClient.cwd("C")));
		assertEquals("/C", ftpClient.printWorkingDirectory());
		
		assertTrue(FTPReply.isPositiveCompletion(ftpClient.cwd("system")));
		assertEquals("/C/system", ftpClient.printWorkingDirectory());
	
		assertTrue(FTPReply.isPositiveCompletion(ftpClient.cwd("/")));
		assertEquals("/", ftpClient.printWorkingDirectory());
		
		assertTrue(FTPReply.isPositiveCompletion(ftpClient.cwd("/C/system")));
		assertEquals("/C/system", ftpClient.printWorkingDirectory());
		
		assertTrue(FTPReply.isPositiveCompletion(ftpClient.cwd("/")));
		assertEquals("/", ftpClient.printWorkingDirectory());
		
		assertTrue(FTPReply.isPositiveCompletion(ftpClient.cwd("/C/system/.././system")));
		assertEquals("/C/system", ftpClient.printWorkingDirectory());
		
		assertTrue(FTPReply.isPositiveCompletion(ftpClient.cwd("..")));
		assertEquals("/C", ftpClient.printWorkingDirectory());
		
		assertTrue(FTPReply.isPositiveCompletion(ftpClient.cwd("..")));
		assertEquals("/", ftpClient.printWorkingDirectory());

	}
	
	//FTP&Telnet-Directory-007
	//To change a current working directory from a valid directory to a non existence directory from PC to Device(H2/H4)
	public void testFTP_Telnet_Directory_007()  throws IOException{
		assertTrue(FTPReply.isNegativePermanent(ftpClient.cwd("/ajdaskldjklasdjlasjdlasldjklasjdl")));
		assertTrue(FTPReply.isNegativePermanent(ftpClient.cwd("/C/ajdaskldjklasdjlasjdlasldjklasjdl")));
	}
	
	//FTP&Telnet-Directory-008
	//To change a current working directory from a valid directory to a non existence directory of different drive from PC to Device(H2/H4)
	
	
}
