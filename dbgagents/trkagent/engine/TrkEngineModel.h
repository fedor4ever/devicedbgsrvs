/*
* Copyright (c) 2005 Nokia Corporation and/or its subsidiary(-ies).
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


#ifndef __TRKENGINEMODEL_H__
#define __TRKENGINEMODEL_H__

#include <e32base.h>
#include "TrkConnData.h"

//
// class CTrkEngineModel
//
// Provides the interface to the Trk engine data
//
class CTrkEngineModel : public CBase
{
public:

	static CTrkEngineModel* NewL();
	static CTrkEngineModel* NewLC();
	~CTrkEngineModel();
	
	void LoadDataL();
	void SaveDataL();
	
	TInt GetDefaultConnData(TTrkConnType aConnType, TTrkConnData& aTrkConnData);
	TInt GetConnData(TTrkConnData& aTrkConnData);
	TInt SetConnData(TTrkConnData& aTrkConnData);
	TBool GetPlugPlaySettingValue();
		
private:
	TTrkConnData iTrkConnData;	
	TBuf<KMaxFullName> iInitFilePath;
	TInt iLineNumber;
	TBool iLoaded;
	TInt iFileReadErr;
private:
	CTrkEngineModel();
	void    ConstructL();

};

#endif // __TRKENGINEMODEL_H__
