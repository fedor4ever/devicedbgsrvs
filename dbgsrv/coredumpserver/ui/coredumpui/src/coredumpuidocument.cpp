// Copyright (c) 2007-2009 Nokia Corporation and/or its subsidiary(-ies).
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

#include "coredumpui.h" 

//             The constructor of the document class just passes the
//             supplied reference to the constructor initialisation list.
//             The document has no real work to do in this application.
//
CResourceDocument::CResourceDocument(CEikApplication& aApp)
		: CEikDocument(aApp)
	{
	}


//             This is called by the UI framework as soon as the 
//             document has been created. It creates an instance
//             of the ApplicationUI. The Application UI class is
//             an instance of a CEikAppUi derived class.
//
CEikAppUi* CResourceDocument::CreateAppUiL()
	{
    return new(ELeave) CResourceAppUi;
	}
