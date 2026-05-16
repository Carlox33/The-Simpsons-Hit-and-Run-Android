//=============================================================================
// Copyright (c) 2002 Radical Games Ltd.  All rights reserved.
//=============================================================================


#ifndef LINKEDCLASS_HPP
#define LINKEDCLASS_HPP

#include <stdlib.h>

template < class T > class radLinkedClass
{
	public:
		
		static T * GetLinkedClassHead( void )
		{
			return s_pLinkedClassHead;
		}

		T * GetLinkedClassNext( void )
		{
			return m_pLinkedClassNext;
		}

		T * GetLinkedClassPrev( void )
		{
			return m_pLinkedClassPrev;
		}

	protected:

		radLinkedClass( void )
		{
            m_pLinkedClassNext = NULL;

            if ( s_pLinkedClassTail != NULL )
            {
                s_pLinkedClassTail->m_pLinkedClassNext = static_cast< T * >( this );
                m_pLinkedClassPrev = s_pLinkedClassTail;
            }
            else
            {
                s_pLinkedClassHead = static_cast< T * >( this );
                m_pLinkedClassPrev = NULL;
            }

            s_pLinkedClassTail = static_cast< T * >( this );

		}

		~radLinkedClass( void )
		{
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
		}

		T * m_pLinkedClassNext;
		T * m_pLinkedClassPrev;

    inline static T* s_pLinkedClassHead = nullptr;
    inline static T* s_pLinkedClassTail = nullptr;

};

#endif // LINKED_CLASS