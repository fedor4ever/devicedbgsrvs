/*
* Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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
#ifndef _TrkSwInstall_H__
#define _TrkSwInstall_H__


class CTrkSwInstall : public CBase

{
public:
	CTrkSwInstall();

	static TInt SilentInstallL(const TDesC&, const TChar);
	static TInt Install(const TDesC&);

	static TInt UninstallL(const TDesC&);	
	
protected:
	
private:
};

#endif // __TrkSwInstall_H__
