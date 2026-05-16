//=============================================================================
// Copyright (c) 2002 Radical Games Ltd.  All rights reserved.
//=============================================================================


//=============================================================================
//
// File:        namespace.cpp
//
// Subsystem:   FoundationTech - Radscript
//
// Description: This file contains code to implement a IRadNameSpace class.
//
// Revisions:   V1.00       Aug 10, 2001    Changed from linked list to BTree
//
//=============================================================================

//============================================================================
// Include Files
//============================================================================

#include "pch.hpp"

#include <radnamespace.hpp>




//============================================================================
// Component: RadNameSpace
//============================================================================



//=============================================================================
// Static variables
//=============================================================================

//
// By default there are no namespaces
//

// I comment this because i put in radnamespace.hpp
//one day later,i put here again
//template<> IRadNameSpace* radLinkedClass< IRadNameSpace >::s_pLinkedClassHead = NULL;
//template<> IRadNameSpace* radLinkedClass< IRadNameSpace >::s_pLinkedClassTail = NULL;

// Definición genérica de los estáticos del template:


//=============================================================================
// Public Functions
//=============================================================================

//=============================================================================
// Function:    radNameSpaceCreate
//-----------------------------------------------------------------------------
// Description: Factory function to create a namespace
//
// Paramters:   allocator - the allocator to use
//
// Return:      Returns a pointer to the newly created namespace
//
//=============================================================================

IRadNameSpace* radNameSpaceCreate
(
    unsigned int allocator
)
{
    return new( allocator ) IRadNameSpace( );
}

//=============================================================================
// Function:    radNameSpaceGet
//-----------------------------------------------------------------------------
// Description: Get the namespace with the given key
//
// Paramters:   key - the key to recognize the namespace by
//
// Return:      Returns a pointer to the appropriate namespace, or NULL if none
//              found.
//
//=============================================================================

IRadNameSpace* radNameSpaceGet
(
    radKey32 key
)
{
    IRadNameSpace* pSearch = IRadNameSpace::GetLinkedClassHead( );
    
    while( pSearch != NULL )
    {
        if( pSearch->m_Name == key )
        {
            return pSearch;
        }
        
        pSearch = pSearch->GetLinkedClassNext( );
    }
    
    return NULL;
}

//=============================================================================
// Function:    radNameSpaceGet
//-----------------------------------------------------------------------------
// Description: Get the namespace with the given name
//
// Paramters:   pName - the namespace's name
//
// Return:      Returns a pointer to the appropriate namespace, or NULL if none
//              found.
//
//=============================================================================

IRadNameSpace* radNameSpaceGet
(
    const char* pName
)
{
    return ::radNameSpaceGet( radMakeKey32( pName ) );
}

//=============================================================================
// Function:    radNameSpaceGetInstance
//-----------------------------------------------------------------------------
// Description: Get the object instance with the given key from the namespace
//              with the given key
//
// Paramters:   nameSpaceKey - the namespace's key
//              instanceKey - the object instance's key
//
// Return:      Returns a pointer to the appropriate object, or NULL if none
//              found.
//
//=============================================================================

IRefCount* radNameSpaceGetInstance
(
    radKey32 nameSpaceKey,
    radKey32 instanceKey
)
{
    ref< IRadNameSpace > xIRadNameSpace( ::radNameSpaceGet( nameSpaceKey ) );
    
    if ( xIRadNameSpace != NULL )
    {
        return xIRadNameSpace->GetInstance( instanceKey );
    }
    
    return NULL;
}

//=============================================================================
// Function:    radNameSpaceGetInstance
//-----------------------------------------------------------------------------
// Description: Get the object instance with the given name from the namespace
//              with the given name
//
// Paramters:   pNameSpaceName - the namespace's name
//              pInstanceName - the object instance's name
//
// Return:      Returns a pointer to the appropriate object, or NULL if none
//              found.
//
//=============================================================================

IRefCount* radNameSpaceGetInstance
(
    const char* pNameSpaceName,
    const char* pInstanceName
)
{
    return ::radNameSpaceGetInstance
    (
        radMakeKey32( pNameSpaceName ),
        radMakeKey32( pInstanceName )
    );
}

//=============================================================================
// Function:    radNameSpaceFindObjectNameSpace
//-----------------------------------------------------------------------------
// Description: Find the namespace containing the object with the given name
//
// Waring:      This is the slowest namespace activity of them all!
//
// Paramters:   pName - name of the object to find
//
// Return:      Returns a pointer to the appropriate namespace, or NULL
//              if none found.
//
//=============================================================================

IRadNameSpace* radNameSpaceFindObjectNameSpace
(
    const char* pName
)
{
    IRadNameSpace* pSearch = IRadNameSpace::GetLinkedClassHead( );
    
    while( pSearch != NULL )
    {
        if( pSearch->GetInstance( pName ) )
        {
            return pSearch;
        }
        
        pSearch = pSearch->GetLinkedClassNext( );
    }
    
    return NULL;
}


