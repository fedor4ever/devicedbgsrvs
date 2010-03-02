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

#ifndef __TRKAPPCONTAINER_H__
#define __TRKAPPCONTAINER_H__

// INCLUDES
#include <coecntrl.h>

class CTrkAppView;

class CTrkAppContainer : public CCoeControl
{
    public: // Constructors and destructor
        /**
        * C++ constructor.
        * @since 2.1
        * @param aController reference to CCcorController
        */
        CTrkAppContainer(CTrkAppView* aTrkAppView);

        /**
        * Symbian OS default constructor.
        * @since 2.1
        */
        void ConstructL(const TRect& aRect);

        /**
        * Destructor.
        */
        virtual ~CTrkAppContainer();
        
    public: // Functions from base classes
        /**
        * From CCoeControl set the size and position of its component controls.
        */
        //void SizeChanged();
    
    public: // New functions
 
    protected: // Functions from base classes
	 	/**
	    * From CCoeControl,Draw.
	    */
    	void Draw(const TRect& aRect) const;
    
    private: //data
	
		CTrkAppView* iTrkAppView;
};

#endif  //__TRKAPPCONTAINER_H__

// End of File

