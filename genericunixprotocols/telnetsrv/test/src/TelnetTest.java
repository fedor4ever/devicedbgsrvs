/*
* Copyright (c) 2005-2009 Nokia Corporation and/or its subsidiary(-ies). 
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

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

import junit.framework.TestCase;

import org.apache.commons.net.telnet.TelnetClient;


public class TelnetTest extends TestCase {
	
	/* (non-Javadoc)
	 * @see junit.framework.TestCase#setUp()
	 */
	TelnetClient telnetClient = new TelnetClient();
	
	InputStream is;
	OutputStream os;
	byte promptBuffer[] = new byte[11]; //"localhost# "
	String prompt = "localhost# ";
	
	
	//FTP&Telnet-Connection-001
	//To Establish a connection from remote host to Symbian OS device(H2/H4)
	
	protected void setUp() throws Exception {
		super.setUp();
		
		String address = System.getProperty("address");
		
		telnetClient.connect(/*"172.0.0.3"*/address);
		
		is = telnetClient.getInputStream();
		os = telnetClient.getOutputStream();
		
		is.read(promptBuffer);
		
		
	}
	
	/* (non-Javadoc)
	 * @see junit.framework.TestCase#tearDown()
	 */
	
	//FTP&Telnet-Connection-002
	//To Close a connection from remote host to Symbian OS device(H2/H4)
	
	protected void tearDown() throws Exception {
		super.tearDown();
		
		os.write("exit\r\n".getBytes());
		os.flush();
	
		telnetClient.disconnect();
	}
	
	private String readUntil(InputStream in, String pattern) throws IOException {
		StringBuffer sb = new StringBuffer();
		while(!(sb.toString().endsWith(prompt)))
		{
			sb.append((char)in.read());
		}
		return sb.toString();
	}
	
	public void testAYT() throws IOException, IllegalArgumentException, InterruptedException
	{
		assertTrue(telnetClient.sendAYT(5000));
	}
	
	public void testCd() throws IOException
	{
		os.write("cd c:\r\n".getBytes());
		os.flush();
		is.read(promptBuffer);	//"localhost# "
		assertEquals(prompt, new String(promptBuffer));
	}
	
	public void testPwd() throws IOException
	{
		os.write("pwd\r\n".getBytes());
		os.flush();
		
		String s = readUntil(is,prompt);
		s = s.substring(1, s.indexOf(prompt));
		
		assertEquals(":\\Private\\2000cd0f\\\r\n", s);
		
	}
	
	public void testCdPwd() throws IOException
	{
		
		os.write("cd c:\r\n".getBytes());
		os.flush();
		is.read(promptBuffer);	//"localhost# "
		assertEquals(new String(promptBuffer),prompt);
		
		os.write("pwd\r\n".getBytes());
		os.flush();
		
		String s = readUntil(is,prompt);
		s = s.substring(0, s.indexOf(prompt));
		
		assertEquals("c:\r\n", s);
	}
	
	//DEF117414
	//zsh cannot run external commands after running 82 commands
	//TODO: change maxIter to 100 when fixed
	
	public void testPs() throws IOException
	{
	
		int maxIter = 2;
		
		for(int i=0; i<maxIter; i++)
		{

		os.write("ps\r\n".getBytes());
		os.flush();
		
		String s = readUntil(is,prompt);
		
		s = s.substring(0, s.indexOf(prompt));
		
		System.out.println(s);
		
		System.out.println("iteration "+(i+1)+" of "+maxIter);
		
		Pattern p = Pattern.compile(".*ps.EXE\\[2000ee91\\]\\d\\d\\d\\d\r\n$", Pattern.DOTALL | Pattern.CASE_INSENSITIVE); //Pattern.DOTALL => '.' includes end of line
		Matcher m = p.matcher(s);
		assertTrue(m.matches());
		}
		
	}
	
	
	//DEF116570
	//after few successful ps and grep, when you do ps and grep on non exsiting string the emulator hangs
	
	public void testPsGrepNonExistingString() throws IOException
	{
		int maxIter = 2;
		
		for(int i=0; i<maxIter; i++)
		{
		
		os.write("ps | grep nonexisting\r\n".getBytes());
		os.flush();
		
		String s = readUntil(is,prompt);
		
		s = s.substring(0, s.indexOf(prompt));
		
		assertEquals(0,s.length());
		}
	}
	
	//FTP&Telnet-Remote Execution-001
	//To Launch a process on a remote Symbian OS device(H2/H4) from PC
	
	public void testFTP_Telnet_Remote_Execution_001() throws IOException
	{
		os.write("testexecute c:\\nonexisting.script\r\n".getBytes());
		os.flush();
		
		String s = readUntil(is,prompt);
		
		assertEquals(11, s.length());
	}
	
	
	//FTP&Telnet-Remote Execution-002
	//To pool all the process on the symbian OS device(H2/H4) from PC
	
	public void testFTP_Telnet_Remote_Execution_002() throws IOException
	{
		os.write("ps\r\n".getBytes());
		os.flush();
		
		String s = readUntil(is,prompt);
		
		s = s.substring(0, s.indexOf(prompt));
		
	//	System.out.println(s);
		
		Pattern p = Pattern.compile(".*ps.EXE\\[2000ee91\\]\\d\\d\\d\\d\r\n$", Pattern.DOTALL | Pattern.CASE_INSENSITIVE); //Pattern.DOTALL => '.' includes end of line
		Matcher m = p.matcher(s);
		assertTrue(m.matches());
	}
	
	
	//FTP&Telnet-Remote Execution-003
	// Kill any process on Symbian OS device(H2/H4) from PC
	
	public void testFTP_Telnet_Remote_Execution_003() throws IOException
	{
		os.write("testexecute\r\n".getBytes());
		os.flush();
		
		//this shell will lock, open another one and kill TEF
		
		TelnetClient tc = new TelnetClient();
		tc.connect(telnetClient.getRemoteAddress());
		
		InputStream is2 = tc.getInputStream();
		OutputStream os2 = tc.getOutputStream();
		
		readUntil(is2, prompt);
		
		os2.write("ps | grep TestExecute\r\n".getBytes());
		os2.flush();
		
		String s = readUntil(is2, prompt);
		
		Pattern p = Pattern.compile(".*\\D+(\\d+)\\s[\\s:\\d]+TestExecute Script Engine.*", Pattern.DOTALL | Pattern.CASE_INSENSITIVE); //Pattern.DOTALL => '.' includes end of line
		Matcher m = p.matcher(s);
		
		assertTrue(m.matches());
				
		String s1 = m.group(1);
		
		int pid = Integer.parseInt(s1);
		
		os2.write(("kill "+pid+"\r\n").getBytes());
		os2.flush();
		readUntil(is2, prompt);
		os2.write("bye\r\n".getBytes());
		os2.flush();
		
		//we should be able now to close the other shell
		
		readUntil(is, prompt);
		
		
	}
	
	//FTP&Telnet-Remote Execution-004
	//To verify the output stream of any Symbian OS process(H2/H4) to PC
	public void testFTP_Telnet_Remote_Execution_004() throws IOException
	{
		
		os.write("helloworld $PIPE\r\n".getBytes());
		os.flush();
		
		String s = readUntil(is, prompt);
		
		assertTrue(s.length()>0); //should print "ABCDEFG...".length() > 0
		
	}
	
	//FTP&Telnet-Remote Execution-005
	//To verify the Standard Error display on Symbian OS device(H2/H4) to PC
	public void testFTP_Telnet_Remote_Execution_005() throws IOException
	{
		os.write("aaa\r\n".getBytes());
		os.flush();
		
		String s = readUntil(is, prompt);
		
		assertEquals("zsh: command not found: aaa",s.substring(0, s.lastIndexOf("\r\n"))); 
	}
	
	
	//FTP&Telnet-Software Install-001
	//Install a SIS file on the Symbian OS device (H2/H4) from PC
	public void testFTP_Telnet_Install_001() throws IOException
	{
		
		os.write("install c:\\10210d02.sis\r\n".getBytes());
		os.flush();
		
		String s = readUntil(is, prompt);
		
		assertEquals("Return Code: 0",s.substring(0, s.lastIndexOf("\r\n"))); 
	}
	
	//FTP&Telnet-Software Install-002
	//Uninstall the SIS file on the symbian OS device (H2/H4) from PC
	public void testFTP_Telnet_Install_002() throws IOException
	{
		
		os.write("uninstall 10210d02\r\n".getBytes());
		os.flush();
		
		String s = readUntil(is, prompt);
		
		assertEquals("Return Code: 0",s.substring(0, s.lastIndexOf("\r\n"))); 
	}
	
	
	
	
}
