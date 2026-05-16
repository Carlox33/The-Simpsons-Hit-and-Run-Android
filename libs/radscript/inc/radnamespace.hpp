//=============================================================================
// Copyright (c) 2002 Radical Games Ltd.  All rights reserved.
//=============================================================================


//=============================================================================
//
// File: radnamespace.hpp
//
// Subsystem: RadScript
//
// Description:	A namespace stores lists of named objects.  Namespaces can be
//      linked to allow searches of multiple namespace.  Namespaces themselves
//      are named to allow global access.
//
//=============================================================================

#ifndef RADNAMESPACE_HPP
#define RADNAMESPACE_HPP

#include <radobject.hpp>
#include <radmemory.hpp>
#include <radkey.hpp>

#include <radobjectbtree.hpp>
#include <radlinkedclass.hpp>
//#include <radnamespace.hpp> //THis is weird a include of the same file 
#include <radtypeinfo.hpp>


//============================================================================
// Forward Declarations
//============================================================================

struct IRadNameSpace;

// i add this here for fix problems with android compilation
// 

//template<>
//IRadNameSpace* radLinkedClass< IRadNameSpace >::s_pLinkedClassHead = NULL;

//template<>
//IRadNameSpace* radLinkedClass< IRadNameSpace >::s_pLinkedClassTail = NULL;



//============================================================================
// Factories and functions
//============================================================================

IRadNameSpace * radNameSpaceCreate( unsigned int allocator = RADMEMORY_ALLOC_DEFAULT );

IRadNameSpace * radNameSpaceGet( const char * pName );
IRadNameSpace * radNameSpaceGet( radKey32 key );
IRadNameSpace * radNameSpaceFindObjectNameSpace( const char * pName );

IRefCount *     radNameSpaceGetInstance(  const char * pNameSpaceName, const char * pInstanceName );
IRefCount *     radNameSpaceGetInstance(  radKey32 nameSpaceKey, radKey32 instanceName );

struct IRadNameSpace
    : 
    public radLinkedClass< IRadNameSpace >,
    public radRefCount
{
public:
    
    IMPLEMENT_REFCOUNTED( "IRadNameSpace" )
    
    //============================================================================
    // Manipulate the namespace's name
    //============================================================================
    
    void SetName( const char * pName )
    {
        m_Name = ::radMakeKey32( pName );
        ::radTypeInfoSystemGet( )->AddString
        (
            static_cast< unsigned int >( m_Name ),
            pName
        );
    }
    
    void SetName( radKey32 key )
    {
        m_Name = key;
    }
    
    radKey32 GetName( void )
    {
        return m_Name;
    }
    
    //============================================================================
    // Add an object instance to the namespace
    //============================================================================
    
    bool AddInstance( radKey32 key, IRefCount* pIRefCount )
    {
        rAssert( pIRefCount != NULL );
        
        return m_pInstanceTree->AddObject( key, pIRefCount );
    }

    bool AddInstance( const char * pName, IRefCount* pIRefCount )
    {
        return AddInstance( ::radMakeKey32( pName ), pIRefCount );
    }
    
    //============================================================================
    // Find an object instance in the namespace
    //============================================================================
   
    IRefCount* GetInstance( radKey32 key )
    {
        return m_pInstanceTree->FindObject( key );
    }

    IRefCount* GetInstance( const char* pName )
    {
        IRefCount* pIRefCount = GetInstance( ::radMakeKey32( pName )  );
        return pIRefCount;
    }
       
    //============================================================================
    // Remove an object instance from the namespace
    //============================================================================
    bool RemoveInstance( IRefCount* pIRefCount )
    {
        rAssert( pIRefCount != NULL );
        return m_pInstanceTree->RemoveObject( pIRefCount );
    }

    bool RemoveInstance( radKey32 key )
    {
        return m_pInstanceTree->RemoveObject( key );
    }

    bool RemoveInstance( const char * pName )
    {
        return RemoveInstance( ::radMakeKey32( pName ) );
    }
            
    void RemoveAll( void )
    {
        m_pInstanceTree->RemoveAll( );
    }
    
    //============================================================================
    // Iteration procedures
    //============================================================================

    IRefCount* GetFirst( radKey32* pKey )
    {
        // Always an inorder traversal.
        // Beware of other operations modifying the traversal status
        return m_pInstanceTree->GetFirst( radObjectBTree::InOrder, pKey );
    }

    IRefCount* GetNext( radKey32* pKey )
    {
        // Beware of other operations modifying the traversal status
        return m_pInstanceTree->GetNext( pKey );
    }

    //============================================================================
    // Get the name of a particular object instance already in the namespace
    //============================================================================
    
    radKey32 GetInstanceName( IRefCount* pIRefCount )
    {
        rAssert( pIRefCount != NULL );

        // Warning: This is an incredibly slow and potentially memory abusive
        //          command!

        radKey32 key = 0;
        IRefCount* pObject = NULL;
        
        pObject = m_pInstanceTree->GetFirst
        (
            radObjectBTree::InOrder,
            &key
        );

        while( pObject != NULL )
        {
            if( pObject == pIRefCount )
            {
                return key;
            }

            pObject = m_pInstanceTree->GetNext( &key );
        }

        return 0;         
    }
    
    //============================================================================
    // Link namespaces
    //============================================================================
    
    void Link( IRadNameSpace* pIRadNameSpace ) 
    {
        rAssert( pIRadNameSpace != NULL );

        // Create the link
        LinkedNameSpaceListNode* pNew =
            new ( GetThisAllocator( ) ) LinkedNameSpaceListNode;

        pNew->m_pINameSpace_Linked = pIRadNameSpace;
        pNew->m_pINameSpace_Linked->AddRef( );

        // Add to our linked namespace list
        pNew->m_pNext = m_pLinkedNameSpaceListHead;
        m_pLinkedNameSpaceListHead = pNew;
    }
    
    bool UnLink( IRadNameSpace* pIRadNameSpace )
    {
        LinkedNameSpaceListNode* pPrev = NULL;
        LinkedNameSpaceListNode* pSearch = m_pLinkedNameSpaceListHead;
        
        while( pSearch != NULL )
        {
            if( pSearch->m_pINameSpace_Linked == pIRadNameSpace )
            {
                if ( pPrev != NULL )
                {
                    pPrev->m_pNext = pSearch->m_pNext;
                }
                else
                {
                    m_pLinkedNameSpaceListHead = pSearch->m_pNext;
                }
                
                pSearch->m_pINameSpace_Linked->Release( );
                delete pSearch;
                return true;
            }
            
            pPrev = pSearch;
            pSearch = pSearch->m_pNext;
        }
        
        return false;
    }
    
    void UnLinkAll( void )
    {
        LinkedNameSpaceListNode* pTemp = NULL;
        LinkedNameSpaceListNode* pSearch = m_pLinkedNameSpaceListHead;
        
        while( pSearch != NULL )
        {
            pTemp = pSearch->m_pNext;
            
            pSearch->m_pINameSpace_Linked->Release( );
            delete pSearch;
            pSearch = pTemp;
        }
        
        m_pLinkedNameSpaceListHead = NULL;
    }
    
    //====================================================================
    // Get the number of objects in the namespace
    //====================================================================
    
    unsigned int GetSize( void )
    {
        return m_pInstanceTree->GetSize( );
    }

    //====================================================================
    // SIMPSONS 2: Move namespace to the front of the list for object 
    // searches, needed because RadTuner doesn't expose a namespace view.
    //====================================================================

    void MoveToFront( void )
    {
        if( GetLinkedClassHead() != this )
        {
            //
            // Remove from list
            //
			if ( m_pLinkedClassPrev != NULL )
			{
				m_pLinkedClassPrev->m_pLinkedClassNext = m_pLinkedClassNext;
			}
			else
			{
				s_pLinkedClassHead = m_pLinkedClassNext;
			}

			if ( m_pLinkedClassNext != NULL )
			{
				m_pLinkedClassNext->m_pLinkedClassPrev = m_pLinkedClassPrev;
			}
            else
            {
                s_pLinkedClassTail = m_pLinkedClassPrev;
            }

            //
            // Re-add in front
            //
            m_pLinkedClassPrev = NULL;

            if ( s_pLinkedClassHead != NULL )
            {
                s_pLinkedClassHead->m_pLinkedClassPrev = static_cast< IRadNameSpace * >( this );
                m_pLinkedClassNext = s_pLinkedClassHead;
            }
            else
            {
                s_pLinkedClassTail = static_cast< IRadNameSpace * >( this );
                m_pLinkedClassNext = NULL;
            }

            s_pLinkedClassHead = static_cast< IRadNameSpace * >( this );
        }
    }

    //====================================================================
    // CONSTRUCTOR
    //====================================================================
    
    IRadNameSpace( )
        :
        radRefCount( 0 ),
        m_Name( 0 ),
        m_pLinkedNameSpaceListHead( NULL )
    {
        m_pInstanceTree = new ( GetThisAllocator( ) )
            radObjectBTree( );
        m_pInstanceTree->AddRef( );
    }
    
    //====================================================================
    // DESTRUCTOR
    //====================================================================
    
    virtual ~IRadNameSpace( void )
    {
        RemoveAll( );
        UnLinkAll( );

        // Release the instance tree
        m_pInstanceTree->Release( );
    }
    
    //====================================================================
    // Data Members
    //====================================================================
    
    
    //
    // The name of this namespace (hashed)
    //
    radKey32                          m_Name;
    
    //
    // These are the actual objects contained in this namespace
    //
    radObjectBTree*                m_pInstanceTree;

    //
    // The link to other namespaces
    //
    struct LinkedNameSpaceListNode : public radObject
    {
        IMPLEMENT_BASEOBJECT( "LinkedNameSpaceListNode" )
        
        IRadNameSpace*              m_pINameSpace_Linked;
        LinkedNameSpaceListNode*    m_pNext;
    };
    LinkedNameSpaceListNode*        m_pLinkedNameSpaceListHead;
};

#endif // RADNAMESPACE_HPP