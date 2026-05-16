//===========================================================================
// Copyright (C) 2000 Radical Entertainment Ltd.  All rights reserved.
//
// Component:   Scrooby Runtime Private Interface
//
// Description: This contains the Parent Base class.
//
// Authors:     Wilkin Ng, Ian Gipson
//
//===========================================================================

//===========================================================================
// Includes
//===========================================================================

#include "stdafx.h"

#include <raddebug.hpp>
#include "FeParent.h"
#include "FeGroup.h"
#include "FeLayer.h"
#include "FePolygon.h"
#include "FeSprite.h"
#include "FeText.h"
#include "FeMovie.h"
#include "FePure3dObject.h"
#include "FeEntity.h"

#include "Drawable.h"
#include "Group.h"
#include "Layer.h"
#include "Polygon.h"
#include "Sprite.h"
#include "Text.h"
#include "Movie.h"
#include "Pure3dObject.h"

#include "tLinearTable.h"


#if defined(RAD_ANDROID)
  #include <android/log.h>
  #define LOG_TAG "SimpsonsHitAndRun"
  #define LOGI(...) __android_log_print(ANDROID_LOG_INFO,  LOG_TAG, __VA_ARGS__)
  #define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

#elif defined(RAD_VITA)
  #include <psp2/kernel/clib.h>
  #define LOGI(...) do { sceClibPrintf(__VA_ARGS__); sceClibPrintf("\n"); } while(0)
  #define LOGE(...) do { sceClibPrintf(__VA_ARGS__); sceClibPrintf("\n"); } while(0)

#else
  #include <cstdio>
  #define LOGI(...) do { std::printf(__VA_ARGS__); std::printf("\n"); std::fflush(stdout); } while(0)
  #define LOGE(...) do { std::printf(__VA_ARGS__); std::printf("\n"); std::fflush(stdout); } while(0)
#endif

#ifdef RAD_ANDROID
#include <dlfcn.h>

static void LogCallerAddrs(void* ra0, void* ra1)
{
    Dl_info info0{}, info1{};
    const char* s0 = "(?)";
    const char* s1 = "(?)";
    const char* f0 = "(?)";
    const char* f1 = "(?)";

    if (ra0 && dladdr(ra0, &info0))
    {
        if (info0.dli_sname) s0 = info0.dli_sname;
        if (info0.dli_fname) f0 = info0.dli_fname;
    }
    if (ra1 && dladdr(ra1, &info1))
    {
        if (info1.dli_sname) s1 = info1.dli_sname;
        if (info1.dli_fname) f1 = info1.dli_fname;
    }

    LOGE("[FeParent] OOB caller ra0=%p (%s @ %s) ra1=%p (%s @ %s)",
         ra0, s0, f0, ra1, s1, f1);
}
#endif
//===========================================================================
// Local Constants, Typedefs, and Macros
//===========================================================================

//===========================================================================
// Global Data, Local Data, Local Classes
//===========================================================================

//===========================================================================
// Member Functions
//===========================================================================

//===========================================================================
// FeParent::FeParent() 
//===========================================================================
// Description: constructor
//
// Constraints:    NONE
//
// Parameters:    NONE
//
// Return:      NONE
//
//===========================================================================
FeParent::FeParent() 
{
    mChildren = new tLinearTable;
    mChildren->AddRef();

    mpChildIter = new tLinearTable::RawIterator( mChildren );
    mCurChildIndex = 0;
}


//===========================================================================
// FeParent::~FeParent()
//===========================================================================
// Description: destructor - reduces all the reference counts
//
// Constraints:    NONE
//
// Parameters:    NONE
//
// Return:      NONE
//
//===========================================================================
FeParent::~FeParent()
{
    mChildren->Release();
    delete mpChildIter;
}

//========================================================================
// FeParent::
//========================================================================
//
// Description: 
//
// Parameters:  None.
//
// Return:      None.
//
// Constraints: None.
//
//========================================================================
void FeParent::ResetIter() const
{
    mCurChildIndex = 0;
    mpChildIter->First();
}
//===========================================================================
// void FeParent::AddChild(FeEntity *s)
//===========================================================================
// Description: adds an entity to the group, and increments the reference 
//                count
//
// Constraints:    NONE
//
// Parameters:    s - pointer to an entity that's going to get stored
//
// Return:      NONE
//
//===========================================================================
void FeParent::AddChild( FeEntity* s )
{
    mChildren->Store( s );
    ResetIter();
}

//===========================================================================
// void FeParent::RemoveChild(FeEntity *s)
//===========================================================================
// Description: Removes an entity from the group and decrements the reference 
//                count
//
// Constraints:    NONE
//
// Parameters:    s - pointer to an entity that's going to get removed
//
// Return:      NONE
//
//===========================================================================
void FeParent::RemoveChild( FeEntity* s )
{
    mChildren->Remove(s);
    ResetIter();
}

//===========================================================================
// void FeParent::ReplaceChild
//===========================================================================
// Description: replaces an entity in the group
//
// Constraints:    NONE
//
// Parameters:    s - pointer to the original entity
//              newS - the pointer to the new entity
//
// Return:      NONE
//
//===========================================================================
void FeParent::ReplaceChild( FeEntity* s, FeEntity* newS )
{
    mChildren->Replace( s, newS );
    ResetIter();
}

//===========================================================================
// FeParent::Resize
//===========================================================================
// Description: resizes the container to the size specified
//
// Constraints: NONE
//
// Parameters:  size - how many elements should be in the container
//
// Return:      NONE
//
//===========================================================================
void FeParent::Resize( const size_t size )
{
    mChildren->Resize( size );
}

//===========================================================================
// void FeParent::DeleteChildren()
//===========================================================================
// Description: deletes and frees reference counts for all the children of 
//                this parent
//
// Constraints:    NONE
//
// Parameters:    NONE
//
// Return:      NONE
//
//===========================================================================
void FeParent::DeleteChildren()
{
    mChildren->RemoveAll();
    ResetIter();
}



//===========================================================================
// FeEntity* FeParent::GetChild( const char* name )
//===========================================================================
// Description: looks up a child entity by name
//
// Constraints:    no type checking is done at all, so there could be duplication
//                problems
//
// Parameters:    name - an ascii string representing the name of the object 
//                you wanted
//
// Return:      pointer to the child that you wanted
//
//===========================================================================
FeEntity* FeParent::GetChild( const char* name )
{
    return mChildren->Find(FeEntity::MakeUID(name));
}

//===========================================================================
// FeEntity* FeParent::GetChild( const tUID hashvalue )
//===========================================================================
// Description: gets a child object from the parent by its hash value
//
// Constraints:    NONE
//
// Parameters:    NONE
//
// Return:      NONE
//
//===========================================================================
FeEntity* FeParent::GetChild( const tUID hashvalue )
{
    return mChildren->Find( hashvalue );
}

//===========================================================================
// FeEntity* FeParent::GetChildIndex(int i) const
//===========================================================================
// Description: gets a child out of the group at a specific index in the array
//
// Constraints:    if i is beyond the end of the array, the last element is 
//                returned
//
// Parameters:    NONE
//
// Return:      a pointer to one of the children
//
//===========================================================================
/*
FeEntity* FeParent::GetChildIndex( int i ) 
{
    int childrenCount = GetChildrenCount();
    rAssert( i < childrenCount );
    
    if( i >= childrenCount ) 
    {
        i = childrenCount - 1;
    }

    if( i < mCurChildIndex )
    {
        ResetIter();
    }

    FeEntity* entity  = mpChildIter->Current();

    while ( entity && mCurChildIndex < i )
    {
        entity = mpChildIter->Next();
        mCurChildIndex++;
    }
    
    return( entity );
}
*/
/*
// NEW FUNCTION FOR ANDROID TEMPORAL, FOR DEBUG||| Esta funcion me permite pasar al menu principal(casa de homer) 
FeEntity* FeParent::GetChildIndex( int i )
{
    int childrenCount = GetChildrenCount();

    LOGI("[FeParent::GetChildIndex] ENTER this=%p i=%d childrenCount=%d curIndex=%d iter=%p",
         this, i, childrenCount, mCurChildIndex, mpChildIter);

    // --- Caso crítico: sin hijos ---
    if (childrenCount <= 0)
    {
        LOGE("[FeParent::GetChildIndex] ERROR: childrenCount=0 this=%p i=%d", this, i);
        return NULL;
    }

    // --- Índice fuera de rango ---
    if (i < 0 || i >= childrenCount)
    {
        LOGE("[FeParent::GetChildIndex] OOB index! this=%p i=%d children=%d curIndex=%d",
             this, i, childrenCount, mCurChildIndex);

        // Clamp temporal para seguir ejecución
        if (i < 0)
            i = 0;
        else
            i = childrenCount - 1;

        LOGI("[FeParent::GetChildIndex] Clamped i=%d", i);
    }

    // --- Si vamos hacia atrás, reseteamos iterador ---
    if (i < mCurChildIndex)
    {
        LOGI("[FeParent::GetChildIndex] ResetIter triggered (i=%d < curIndex=%d)",
             i, mCurChildIndex);

        ResetIter();
    }

    FeEntity* entity = mpChildIter->Current();

    LOGI("[FeParent::GetChildIndex] Starting iteration at curIndex=%d entity=%p",
         mCurChildIndex, entity);

    // --- Iteración ---
    while (entity && mCurChildIndex < i)
    {
        entity = mpChildIter->Next();
        mCurChildIndex++;

        LOGI("[FeParent::GetChildIndex] Iter step -> curIndex=%d entity=%p",
             mCurChildIndex, entity);
    }

    LOGI("[FeParent::GetChildIndex] RETURN entity=%p finalCurIndex=%d",
         entity, mCurChildIndex);

    return entity;
}
*/

/*
//===========================================================================
// FeParent::GetChildIndex
//===========================================================================
// Description: gets a child out of the group at a specific index in the array
//
// Constraints:    if i is beyond the end of the array, the last element is 
//                returned
//
// Parameters:    NONE
//
// Return:      a const pointer to one of the children
//
//===========================================================================
const FeEntity* FeParent::GetChildIndex( int i ) const
{
    int childrenCount = GetChildrenCount();
    rAssert( i < childrenCount );
    
    if( i >= childrenCount ) 
    {
        i = childrenCount - 1;
    }

    if( i < mCurChildIndex )
    {
        ResetIter();
    }

    FeEntity* entity  = mpChildIter->Current();

    while ( entity && mCurChildIndex < i )
    {
        entity = mpChildIter->Next();
        mCurChildIndex++;
    }
    
    return( entity );
}
*/

// NEW FUNCTIONS FOR ANDROID GetChildIndex(AMBAS) con "const " y sin const 

// ESTA VERSION ES LA "NUEVA Y MEJORADA EN CUANTO A QUE MANTENEMOS COMPORTAMIENTO ORIGINAL PERO HACIENDO RESET ITER EN LUGARES ADECUADOS"
/*
FeEntity* FeParent::GetChildIndex( int i )
{
    int childrenCount = GetChildrenCount();

    // 1) Caso real: sin hijos -> no hay nada que devolver.
    if( childrenCount <= 0 )
    {
        return NULL;
    }

    // 2) Contrato original (solo que ahora no explota cuando childrenCount==0)
    rAssert( i < childrenCount );

    // 3) Comportamiento original en release: clamp si OOB por arriba.
    if( i >= childrenCount )
    {
        i = childrenCount - 1;
    }

    // (Opcional pero MUY recomendado) Si hay posibilidad de i negativo:
    // rAssert( i >= 0 );
    // if( i < 0 ) i = 0;

    // 4) Reset recomendado: iterador en estado inválido aunque haya hijos
    if( mpChildIter->Current() == NULL )
    {
        ResetIter();
    }

    if( i < mCurChildIndex )
    {
        ResetIter();
    }

    FeEntity* entity = mpChildIter->Current();

    while( entity && mCurChildIndex < i )
    {
        entity = mpChildIter->Next();
        mCurChildIndex++;
    }

    return entity;
}

const FeEntity* FeParent::GetChildIndex( int i ) const
{
    int childrenCount = GetChildrenCount();

    if( childrenCount <= 0 )
    {
        return NULL;
    }

    rAssert( i < childrenCount );

    if( i >= childrenCount )
    {
        i = childrenCount - 1;
    }

    // rAssert( i >= 0 );
    // if( i < 0 ) i = 0;

    if( mpChildIter->Current() == NULL )
    {
        ResetIter();
    }

    if( i < mCurChildIndex )
    {
        ResetIter();
    }

    FeEntity* entity = mpChildIter->Current();

    while( entity && mCurChildIndex < i )
    {
        entity = mpChildIter->Next();
        mCurChildIndex++;
    }

    return entity;
}

*/


// NUEVAS FUNCIONES QUE ESTOY PROBANDO PARA DEBUG(ANDROID), cuando se solucione se supone que deberiamos volver a algo similar a las de justo arriba 
FeEntity* FeParent::GetChildIndex( int i )
{
    int childrenCount = GetChildrenCount();

    // 1) Caso real: sin hijos -> no hay nada que devolver.
    if( childrenCount <= 0 )
    {
//#ifdef RAD_ANDROID
  //      LOGE("[FeParent] GetChildIndex: childrenCount<=0 this=%p i=%d curIndex=%d iter=%p",
    //         this, i, mCurChildIndex, mpChildIter);
//#endif
        return NULL;
    }

    // 2) Si OOB (por arriba o por debajo), LOG ANTES del assert
    //    (así vemos quién llama mal). Luego el assert mantiene el contrato.
#ifdef RAD_ANDROID
    if( i < 0 || i >= childrenCount )
    {
#if defined(__clang__) || defined(__GNUC__)
        void* ra0 = __builtin_return_address(0);
        void* ra1 = __builtin_return_address(1);
#else
        void* ra0 = nullptr;
        void* ra1 = nullptr;
#endif
        //LOGE("[FeParent] GetChildIndex OOB this=%p i=%d childrenCount=%d curIndex=%d iter=%p cur=%p ra0=%p ra1=%p",
          //   this, i, childrenCount, mCurChildIndex, mpChildIter,
            // mpChildIter ? mpChildIter->Current() : (void*)0,
             //ra0, ra1);

             //LogCallerAddrs(ra0, ra1);
    }
#endif

    // 3) Contrato original (debug rompe aquí si i es inválido)
    rAssert( i < childrenCount );

    // 4) Comportamiento original: clamp si OOB por arriba (release)
    if( i >= childrenCount )
    {
        i = childrenCount - 1;
    }

    // (Opcional) si te preocupa i negativo en release:
    // if( i < 0 ) i = 0;

    // 5) Reset recomendado: iterador en estado inválido aunque haya hijos
    if( mpChildIter->Current() == NULL )
    {
//#ifdef RAD_ANDROID
  //      LOGI("[FeParent] GetChildIndex: iter current NULL -> ResetIter this=%p i=%d childrenCount=%d curIndex=%d iter=%p",
    //         this, i, childrenCount, mCurChildIndex, mpChildIter);
//#endif
        ResetIter();
    }

    if( i < mCurChildIndex )
    {
//#ifdef RAD_ANDROID
  //      LOGI("[FeParent] GetChildIndex: i < curIndex -> ResetIter this=%p i=%d curIndex=%d",
    //         this, i, mCurChildIndex);
//#endif
        ResetIter();
    }

    FeEntity* entity = mpChildIter->Current();

    while( entity && mCurChildIndex < i )
    {
        entity = mpChildIter->Next();
        mCurChildIndex++;
    }

    return entity;
}

const FeEntity* FeParent::GetChildIndex( int i ) const
{
    int childrenCount = GetChildrenCount();

    if( childrenCount <= 0 )
    {
//#ifdef RAD_ANDROID
  //      LOGE("[FeParent] GetChildIndex(const): childrenCount<=0 this=%p i=%d curIndex=%d iter=%p",
    //         this, i, mCurChildIndex, mpChildIter);
//#endif
        return NULL;
    }

#ifdef RAD_ANDROID
    if( i < 0 || i >= childrenCount )
    {
#if defined(__clang__) || defined(__GNUC__)
        void* ra0 = __builtin_return_address(0);
        void* ra1 = __builtin_return_address(1);
#else
        void* ra0 = nullptr;
        void* ra1 = nullptr;
#endif
        /*LOGE("[FeParent] GetChildIndex(const) OOB this=%p i=%d childrenCount=%d curIndex=%d iter=%p cur=%p ra0=%p ra1=%p",
             this, i, childrenCount, mCurChildIndex, mpChildIter,
             mpChildIter ? mpChildIter->Current() : (void*)0,
             ra0, ra1);
              LogCallerAddrs(ra0, ra1);
              */
    }
#endif

    rAssert( i < childrenCount );

    if( i >= childrenCount )
    {
        i = childrenCount - 1;
    }

    // if( i < 0 ) i = 0; // opcional

    if( mpChildIter->Current() == NULL )
    {
 /*     
#ifdef RAD_ANDROID
        LOGI("[FeParent] GetChildIndex(const): iter current NULL -> ResetIter this=%p i=%d childrenCount=%d curIndex=%d iter=%p",
             this, i, childrenCount, mCurChildIndex, mpChildIter);
#endif
*/  
        ResetIter();
    }

    if( i < mCurChildIndex )
    {
         /* 
#ifdef RAD_ANDROID
        LOGI("[FeParent] GetChildIndex(const): i < curIndex -> ResetIter this=%p i=%d curIndex=%d",
             this, i, mCurChildIndex);
#endif
*/  
        ResetIter();
    }

    FeEntity* entity = mpChildIter->Current();

    while( entity && mCurChildIndex < i )
    {
        entity = mpChildIter->Next();
        mCurChildIndex++;
    }

    return entity;
}




//===========================================================================
// FeParent::GetChildDrawable
//===========================================================================
// Description: returns a pointer to the scrooby::Drawable stored at a 
//              specific index.  Good for iterating through groups
//
// Constraints:   
//
// Parameters:    NONE
//
// Return:      a const pointer to one of the children
//
//===========================================================================
Scrooby::Drawable* FeParent::GetChildDrawable( const unsigned int index )
{
    FeEntity* entity = GetChildIndex( index );
    Scrooby::Drawable* returnMe = dynamic_cast< Scrooby::Drawable* >( entity );
    rAssert( returnMe != NULL );
    //IAN - get rid of this dynamic cast if you can
    return returnMe;
}

//===========================================================================
// int FeParent::GetChildrenCount() const
//===========================================================================
// Description: figures out how many children this parent has
//
// Constraints:    NONE
//
// Parameters:    NONE
//
// Return:      NONE
//
//===========================================================================
int FeParent::GetChildrenCount() const
{
    return mChildren->GetElementCount();
}

//===========================================================================
// Scrooby::Group* FeParent::GetGroup( const char* name );
//===========================================================================
// Description: gets a group object from within the page by name
//
// Constraints:    NONE
//
// Parameters:    NONE
//
// Return:      NONE
//
//===========================================================================
Scrooby::Group* FeParent::GetGroup( const char* name )
{
    tUID hashValue = FeEntity::MakeUID( name );
    return this->GetGroup( hashValue );
}

//===========================================================================
// Scrooby::Group* GetGroup( const tUID hashValue );
//===========================================================================
// Description: gets a group object from within the page by hash value
//
// Constraints:    NONE
//
// Parameters:    NONE
//
// Return:      NONE
//
//===========================================================================
Scrooby::Group* FeParent::GetGroup( const tUID hashValue )
{
    int childrenCount = this->GetChildrenCount();
    for( int i = 0; i < childrenCount; i++ )
    {
        FeEntity* child = this->GetChildIndex( i );
        tUID uid = child->GetUID();

        if( uid == hashValue )
        {
            //check if the child is a group
            FeGroup* element = dynamic_cast< FeGroup* >( child );
            if( element != NULL )
            {
               return element;
            }
        }

        //check if it's a parent
        if( child->IsOwner() )
        {
            FeParent* parent = dynamic_cast< FeParent* >( child );
            if( parent != NULL )
            {
                Scrooby::Group* element = parent->GetGroup( hashValue );
                if( element != NULL )
                {
                    return element;
                }
            }
        }
    }

    return NULL;
}

//===========================================================================
// Scrooby::Layer* FeParent::GetLayer( const char* name );
//===========================================================================
// Description: gets a layer object from within the page by name
//
// Constraints:    NONE
//
// Parameters:    NONE
//
// Return:      NONE
//
//===========================================================================
Scrooby::Layer* FeParent::GetLayer( const char* name )
{
    tUID hashValue = FeEntity::MakeUID( name );
    return this->GetLayer( hashValue );
}

//===========================================================================
// Scrooby::Layer* FeParent::GetLayer( const tUID hashValue )
//===========================================================================
// Description: gets a text object from within the page by name
//
// Constraints:    NONE
//
// Parameters:    NONE
//
// Return:      NONE
//
//===========================================================================
Scrooby::Layer* FeParent::GetLayer( const tUID hashValue )
{
    int childrenCount = this->GetChildrenCount();
    for( int i = 0; i < childrenCount; i++ )
    {
        FeEntity* child = this->GetChildIndex( i );
        tUID uid = child->GetUID();

        //check if the child is a layer
        if( uid == hashValue )
        {
            Scrooby::Layer* element = dynamic_cast< Scrooby::Layer* >( child );
            if( element != NULL )
            {
                return element;
            }
        }

        //check if it's a parent
        if( child->IsOwner() )
        {
            FeParent* parent = dynamic_cast< FeParent* >( child );
            if( parent != NULL )
            {
                Scrooby::Layer* element = parent->GetLayer( hashValue );
                if( element != NULL )
                {
                    return element;
                }
            }
        }
    }
    return NULL;
}

//===========================================================================
// Scrooby::Polygon* FeParent::GetPolygon( const char* name );
//===========================================================================
// Description: gets a polygon object from within the page by name
//
// Constraints:    NONE
//
// Parameters:    NONE
//
// Return:      NONE
//
//===========================================================================
Scrooby::Polygon* FeParent::GetPolygon( const char* name )
{
    tUID hashValue = FeEntity::MakeUID( name );
    return this->GetPolygon( hashValue );
}

//===========================================================================
// Scrooby::Polygon* GetPolygon( const tUID hashValue );
//===========================================================================
// Description: gets a polygon from within the page by hashValue
//
// Constraints:    NONE
//
// Parameters:    NONE
//
// Return:      NONE
//
//===========================================================================
Scrooby::Polygon* FeParent::GetPolygon( const tUID hashValue )
{
    int childrenCount = this->GetChildrenCount();
    for( int i = 0; i < childrenCount; i++ )
    {
        FeEntity* child = this->GetChildIndex( i );

        //check if the child is a sprite
        Scrooby::Polygon* element = dynamic_cast< Scrooby::Polygon* >( child );
        if( element != NULL )
        {
            //check if it's a sprite with the correct name
            if( child->GetUID() == hashValue )
            {
                return element;
            }
        }

        //check if it's a parent
        FeParent* parent = dynamic_cast< FeParent* >( child );
        if( parent != NULL )
        {
            Scrooby::Polygon* element = parent->GetPolygon( hashValue );
            if( element != NULL )
            {
                return element;
            }
        }
    }
    return NULL;
}

//===========================================================================
// Sprite* FeParent::GetSprite( const char* name )
//===========================================================================
// Description: gets a sprite out of the page by name
//
// Constraints:    NONE
//
// Parameters:    NONE
//
// Return:      NONE
//
//===========================================================================
Scrooby::Sprite* FeParent::GetSprite( const char* name )
{
    tUID hashValue = FeEntity::MakeUID( name );
    return this->GetSprite( hashValue );
}

//===========================================================================
// Scrooby::Sprite* GetSprite( const tUID hashValue );
//===========================================================================
// Description: gets a sprite from within the page by hashValue
//
// Constraints:    NONE
//
// Parameters:    NONE
//
// Return:      NONE
//
//===========================================================================
Scrooby::Sprite* FeParent::GetSprite( const tUID hashValue )
{
    int childrenCount = this->GetChildrenCount();
    for( int i = 0; i < childrenCount; i++ )
    {
        FeEntity* child = this->GetChildIndex( i );
        tUID uid = child->GetUID();

        //check if the child is a sprite
        if( uid == hashValue )
        {
            FeSprite* element = dynamic_cast< FeSprite* >( child );
            if( element != NULL )
            {
                //check if it's a sprite with the correct name
                return element;
            }
        }
        //check if it's a parent
        if( child->IsOwner() )
        {
            FeParent* parent = dynamic_cast< FeParent* >( child );
            if( parent != NULL )
            {
                Scrooby::Sprite* element = parent->GetSprite( hashValue );
                if( element != NULL )
                {
                    return element;
                }
            }
        }
    }
    return NULL;
}

//===========================================================================
// Sprite* FeParent::GetText( const char* name )
//===========================================================================
// Description: gets a text object from within the page by name
//
// Constraints:    NONE
//
// Parameters:    NONE
//
// Return:      NONE
//
//===========================================================================
Scrooby::Text* FeParent::GetText( const char* name )
{
    tUID hashValue = FeEntity::MakeUID( name );
    return this->GetText( hashValue );
}


//===========================================================================
// Scrooby::Text* GetText( const tUID hashValue );
//===========================================================================
// Description: gets a text object from within the page by name
//
// Constraints:    NONE
//
// Parameters:    NONE
//
// Return:      NONE
//
//===========================================================================
Scrooby::Text* FeParent::GetText( const tUID hashValue )
{
    int childrenCount = this->GetChildrenCount();
    for( int i = 0; i < childrenCount; i++ )
    {
        FeEntity* child = this->GetChildIndex( i );
        tUID childId = child->GetUID();
        if( childId == hashValue )
        {
            //check if the child is a text object
            FeText* element = dynamic_cast< FeText* >( child );
            if( element != NULL )
            {
                return element;
            }
        }

        //check if it's a parent
        if( child->IsOwner() )
        {

            FeParent* parent = dynamic_cast< FeParent* >( child );
            if( parent != NULL )
            {
                Scrooby::Text* element = parent->GetText( hashValue );
                if( element != NULL )
                {
                    return element;
                }
            }
        }
    }
    return NULL;
}


//===========================================================================
// FeParent::GetMovie( const char* name )
//===========================================================================
// Description: gets a movie object by name
//
// Constraints:    NONE
//
// Parameters:    name - the name assigned to the object in the builder
//
// Return:      A pointer to the object if found, else NULL.
//
//===========================================================================
Scrooby::Movie* FeParent::GetMovie
(
    const char* name 
)
{
    tUID hashValue = FeEntity::MakeUID( name );
    
    return( this->GetMovie( hashValue ) );
}


//===========================================================================
// FeParent::GetMovie( const tUID hashValue );
//===========================================================================
// Description: gets a movie object by hash value
//
// Constraints:    NONE
//
// Parameters:    hashValue - the hash value of the name assigned to the object
//                          in the builder.
//                          
// Return:      A pointer to the object if found, else NULL.
//
//===========================================================================
Scrooby::Movie* FeParent::GetMovie( const tUID hashValue )
{
    int childrenCount = this->GetChildrenCount();

    for( int i = 0; i < childrenCount; ++i )
    {
        FeEntity* child = this->GetChildIndex( i );

        // Check if the child is a movie.
        //
        FeMovie* element = dynamic_cast<FeMovie*>( child );
        
        if( element != NULL )
        {
            // Check if it's a movie with the correct name.
            //
            if( child->GetUID() == hashValue )
            {
                return( element );
            }
        }

        // Check if it's a parent.
        //
        FeParent* parent = dynamic_cast<FeParent*>( child );
        
        if( parent != NULL )
        {
            Scrooby::Movie* element = parent->GetMovie( hashValue );
            
            if( element != NULL )
            {
                return( element );
            }
        }
    }
    
    return( NULL );
}


//===========================================================================
// FeParent::GetPure3dObject( const char* name )
//===========================================================================
// Description: Gets a Pure3d object by name
//
// Constraints:    NONE
//
// Parameters:    name - the name assigned to the object in the builder
//
// Return:      A pointer to the object if found, else NULL.
//
//===========================================================================
Scrooby::Pure3dObject* FeParent::GetPure3dObject
(
    const char* name 
)
{
    tUID hashValue = FeEntity::MakeUID( name );
    
    return( this->GetPure3dObject( hashValue ) );
}


//===========================================================================
// FeParent::GetPure3dObject( const tUID hashValue );
//===========================================================================
// Description: Gets a Pure3d object by hash value
//
// Constraints:    NONE
//
// Parameters:    hashValue - the hash value of the name assigned to the object
//                          in the builder.
//                          
// Return:      A pointer to the object if found, else NULL.
//
//===========================================================================
Scrooby::Pure3dObject* FeParent::GetPure3dObject( const tUID hashValue )
{
    int childrenCount = this->GetChildrenCount();

    for( int i = 0; i < childrenCount; ++i )
    {
        FeEntity* child = this->GetChildIndex( i );

        // Check if the child is a movie.
        //
        FePure3dObject* element = dynamic_cast<FePure3dObject*>( child );
        
        if( element != NULL )
        {
            // Check if it's a movie with the correct name.
            //
            if( child->GetUID() == hashValue )
            {
                return( element );
            }
        }

        // Check if it's a parent.
        //
        FeParent* parent = dynamic_cast<FeParent*>( child );
        
        if( parent != NULL )
        {
            Scrooby::Pure3dObject* element = parent->GetPure3dObject( hashValue );
            
            if( element != NULL )
            {
                return( element );
            }
        }
    }
    
    return( NULL );
}

//===========================================================================
// FeParent::Size()
//===========================================================================
// Description: returns the size of the group
//
// Constraints:    NONE
//
// Parameters:    NONE
//                          
// Return:      unsigned int representing the size
//
//===========================================================================
unsigned int FeParent::Size() const
{
    int childrenCount = GetChildrenCount();
    rAssert( childrenCount >= 0 );
    return childrenCount;
}