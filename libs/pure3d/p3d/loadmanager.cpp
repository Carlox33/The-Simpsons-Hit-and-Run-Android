//=============================================================================
// Copyright (c) 2002 Radical Games Ltd.  All rights reserved.
//=============================================================================

#include <SDL.h>

#include <p3d/loadmanager.hpp>
#include <p3d/inventory.hpp>
#include <p3d/chunkfile.hpp>
#include <p3d/error.hpp>
#include <p3d/file.hpp>
#include <p3d/fileftt.hpp>
#include <p3d/utility.hpp>
#include <constants/chunkids.hpp>
#include <constants/chunks.h>
#include <p3d/queue.hpp>
#include <string.h>

#ifdef P3D_TRACK_LOAD_STATS
#include <radmemory.hpp>
#ifdef RAD_WIN32
#pragma warning(disable:4786)
#endif
#ifdef RAD_XBOX
#pragma warning(disable:4530)
#endif

// These two are for stats tracking
#include <radtime.hpp>
#include <map>

#endif // P3D_TRACK_LOAD_STATS

#ifdef LINUX
#define strcmpi strcasecmp
#endif


#ifdef RAD_ANDROID
#include <android/log.h>
#endif
#if defined(RAD_ANDROID)
  #include <unistd.h>
  #include <sys/syscall.h>
  static inline int mytid() { return (int)syscall(SYS_gettid); }
#else
  static inline int mytid() { return 0; }
#endif
#if defined(RAD_ANDROID)
    #define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "SimpsonsHitAndRun", __VA_ARGS__)
#endif

#ifdef RAD_GAMECUBE
#include <ctype.h>
int strcmpi(const char *a, const char *b)
{
     if ((a == NULL) && (b == NULL)) return 0;
     if ((a != NULL) && (b == NULL)) return -1;
     if ((a == NULL) && (b != NULL)) return 1;

     while ((*a != 0) && (*b != 0))
     {
          if (tolower(*a) < tolower(*b)) return -1;
          else if (tolower(*a) > tolower(*b)) return 1;
          ++a;
          ++b;
     }

     if (*a == *b) return 0;
     if (*a == 0) return -1;

     return 1;
}
#endif

class tTempStore : public tEntityStore
{
public:
    tTempStore() : entity(NULL) {}
    virtual void Store(tEntity* e)
    {
        // Only allow one object to be stored in this mock entity store.
        // This will also find chunk loaders which are storing more than
        // one object (which is not allowed with radLoad)
        rAssert(!entity);
        entity = e;
    }
    virtual void StoreHandlingCollisions( tEntity* e )
    {
        Store( e );
    }

    tEntity* GetEntity() {return entity;}
protected:
    tEntity* entity;
};

void tFileHandler::LoadFile(radLoadOptions* options, radLoadUpdatableRequest* request)
{
    tFile* file = static_cast<tFile*>(request->GetStream());
    radMemoryAllocator old = ::radMemorySetCurrentAllocator( RADMEMORY_ALLOC_TEMP );
    tEntityStore* store = new tEntityStore();
    ::radMemorySetCurrentAllocator( old );
    store->SetAutoCollisionTest(false);
    store->AddRef();
    
    if(!file)
    {
        radMemoryAllocator old = ::radMemorySetCurrentAllocator( RADMEMORY_ALLOC_TEMP );
        file = new tFileFTT(options->filename,options->syncLoad);
        ::radMemorySetCurrentAllocator( old );
        request->SetStream(file);
    }
    file->AddRef();
    Load(file,store);
    request->SetInventory(store);
    file->Release();
    store->Release();
    
}

IRefCount* tChunkHandler::LoadData(radLoadStream* stream, radMemoryAllocator alloc, radLoadInventory* resolver, IRefCount* originalObject)
{
    // Right now there's no re-loading any pure3d objects.
    if(originalObject)
    {
        return originalObject;
    }

    tTempStore* store = new tTempStore();
    Load( static_cast<tChunkFile*>(stream), store );
    return store->GetEntity();
}

tP3DFileHandler::tP3DFileHandler(int n)
{
    nExtensions = 1;
    strcpy(extensions[0],".p3d");

#ifdef P3D_TRACK_LOAD_STATS
    mTotalLoadTime = 0;
    mTotalTaskedOutTime = 0;
#endif
}

tP3DFileHandler::~tP3DFileHandler()
{
    RemoveAllHandlers();
}

bool tP3DFileHandler::CheckExtension(char* file)
{
    for(int i = 0; i < nExtensions; i++)
    {
        int len = strlen(file);
        int extLen = strlen(extensions[i]);
        
        if(len >= extLen)
        {
            if(SDL_strcasecmp(extensions[i], &file[len-extLen]) == 0)
                return true;
        }
        if(len == extLen-1)
        {
          if(strcmp(&extensions[i][1], &file[len-extLen+1]) == 0)
              return true;
          if(SDL_strcasecmp(&extensions[i][1], &file[len-extLen+1]) == 0)
              return true;
        }
    }
    return false;
}

struct LoadStats
{
    unsigned loadTime;
    unsigned taskedOutTime;
    unsigned loadCount;
    LoadStats(void) { loadTime = 0; taskedOutTime = 0; loadCount = 0; }
};


tLoadStatus  tP3DFileHandler::Load(tFile* file, tEntityStore* store)
{
#ifdef P3D_TRACK_LOAD_STATS
    std::map<unsigned, LoadStats> chunkLoadTimes;
    tLoadManager::ResetTaskedOutStats();
    radTime64 totalstart = radTimeGetMicroseconds64();
    tLoadManager::SetLoadActive(true);
#endif
    radMemoryAllocator old = ::radMemorySetCurrentAllocator( RADMEMORY_ALLOC_TEMP );
    tChunkFile* chunkFile = new tChunkFile(file);
    ::radMemorySetCurrentAllocator( old );
    chunkFile->AddRef();
    tLoadStatus fileStatus = LOAD_OK;

    P3DASSERT(chunkFile->GetCurrentID() == Pure3D::DATA_FILE);
    while(chunkFile->ChunksRemaining())
    {

#ifdef P3D_TRACK_LOAD_STATS
        unsigned taskedoutstart = tLoadManager::TaskedOutTime();
        radTime64 chunkstart = radTimeGetMicroseconds64();
#endif
        chunkFile->BeginChunk();

        tChunkHandler* h = static_cast<tChunkHandler*>(radLoad->GetDataLoader(chunkFile->GetCurrentID()));
        if (h != NULL)
        {
            tLoadStatus status = h->Load(chunkFile, store);
            if(fileStatus == LOAD_ERROR) fileStatus = status;
        }
        else
        {
            p3d::printf("Unrecognized chunk (%x) in %s\n",  chunkFile->GetCurrentID(), file->GetFilename());
        }

#ifdef P3D_TRACK_LOAD_STATS
        // Stats stuff
        unsigned taskedouttime = tLoadManager::TaskedOutTime() - taskedoutstart;
        radTime64 chunkend = radTimeGetMicroseconds64();
        unsigned chunktime = ((unsigned) (chunkend - chunkstart)) - taskedouttime;

        LoadStats &ls = chunkLoadTimes[chunkFile->GetCurrentID()];
        ls.loadTime += chunktime;
        ls.taskedOutTime += taskedouttime;
        ls.loadCount++;
#endif
        chunkFile->EndChunk();
        P3DASSERT(chunkFile->GetCurrentID() == Pure3D::DATA_FILE);
    }
    chunkFile->Release();

#ifdef P3D_TRACK_LOAD_STATS
    tLoadManager::SetLoadActive(false);
    radTime64 totalend = radTimeGetMicroseconds64();
    unsigned thisloadtime = ((unsigned) (totalend - totalstart)) - tLoadManager::TaskedOutTime();
    mTotalLoadTime += thisloadtime;
    mTotalTaskedOutTime += tLoadManager::TaskedOutTime();

    p3d::printf("P3D Total File Load Time: %.1fms, Total Tasked Out Time: %.1fms\n", mTotalLoadTime / 1000.0F, mTotalTaskedOutTime / 1000.0F);
    p3d::printf("    This File Load Time: %.1fms, This File Tasked Out Time: %.1fms\n", thisloadtime / 1000.0F, tLoadManager::TaskedOutTime() / 1000.0F);

    std::map<unsigned, LoadStats>::iterator it;
    for (it = chunkLoadTimes.begin(); it != chunkLoadTimes.end(); it++)
    {
        p3d::printf("    Chunk %s Time: %.3fms  Tasked Out Time: %.3fms Count: %d\n", tChunkHandler::ChunkTypeName((*it).first), (*it).second.loadTime / 1000.0F, (*it).second.taskedOutTime / 1000.0F, (*it).second.loadCount);
    }
#endif

    return fileStatus;
}

tChunkHandler* tP3DFileHandler::AddHandler(tChunkHandler* l)
{
    radLoad->AddDataLoader(l, l->GetChunkID());
    return l;
}

tChunkHandler* tP3DFileHandler::AddHandler(tChunkHandler* l, unsigned chunkID)
{
    radLoad->AddDataLoader(l, chunkID);
    return l;
}

tChunkHandler* tP3DFileHandler::GetHandler(unsigned chunkID)
{
    return dynamic_cast<tChunkHandler*>(radLoad->GetDataLoader( chunkID ));
}

void tP3DFileHandler::RemoveHandler(unsigned chunkID)
{
    radLoad->RemoveDataLoader( chunkID );
}

void tP3DFileHandler::RemoveAllHandlers(void)
{
    // Stub
    // No way to do this right now.
}

void tP3DFileHandler::AddAlternateExtension(const char* ext)
{
    P3DASSERT(nExtensions < 8);
    P3DASSERT(strlen(ext) < 8);
    strcpy(extensions[nExtensions++], ext);
    radLoad->AddFileLoader( this, ext ); 
}

tLoadStatus tSimpleChunkHandler::Load(tChunkFile* file, tEntityStore* store)
{
    status = LOAD_OK;

    if(file->GetCurrentID() != _id)
        return LOAD_ERROR;

    tEntity* t = LoadObject(file, store);

    if(!t)
        return (status == LOAD_OK) ? LOAD_ERROR : status;

    tInventory* inv = dynamic_cast<tInventory*>(store);
    if(inv && inv->TestCollision(t))
    {
        HandleCollision(t);
        t = NULL;
        return LOAD_ERROR;
    }
    else
    {
        if( m_NameOverride )
        {
            t->SetName( m_NameOverride );
            m_NameOverride = 0;
        }
        store->StoreHandlingCollisions( t );
    }

    return LOAD_OK;
}

void tSimpleChunkHandler::HandleCollision(tEntity* e)
{
#ifdef P3D_DEBUG_MESSAGES
    char buf[256];
    p3d::ClassName(e, buf, 256);
#ifdef P3D_ALLOW_ENTITY_GETNAME
    p3d::printf("Pure3D : Inventory collision: %s ( %s )\n", e->GetName(), buf);
#else
    p3d::printf("Pure3D : Unknown Inventory collision: ( %s )\n", buf);
#endif
#endif
 
    e->AddRef(); // this is for the case when other objects Addref() in the loader
    e->Release();
}

bool tSimpleChunkHandler::CheckChunkID(unsigned id)
{
    return id == _id;
}

//----------------------------------------------------------------------------------
// Legacy classes
//----------------------------------------------------------------------------------

tLoadRequest::tLoadRequest( const char* n) : 
    options(new radLoadOptions),
    request(NULL),
    file(NULL),
    callback(NULL),
    section(0),
    store(NULL),
    priority(0),
    userData(NULL),
    dummy(false)
{
    radMemoryAllocator current = ::radMemoryGetCurrentAllocator();
    rAssert( current == RADMEMORY_ALLOC_TEMP );
    section = tEntity::MakeUID(P3D_DEFAULT_INV_SECTION);
    options->filename = new char[strlen(n)+1];
    strcpy(options->filename, n);
    // Pure3D uses a default sync loading method
    options->syncLoad = true;
}

tLoadRequest::tLoadRequest(tFile* f) :
    options(new radLoadOptions),
    request(NULL),
    file(f),
    callback(NULL),
    section(0),
    store(NULL),
    priority(0),
    userData(NULL),
    dummy(false)
{
    radMemoryAllocator current = ::radMemoryGetCurrentAllocator();
	//LOGI("[p3d] loadmanager before assert tid=%d current=%d", mytid(), (int)current);
    //LOGI("[p3d] current=%d RADMEMORY_ALLOC_TEMP=%d", 
     //(int)::radMemoryGetCurrentAllocator(),
     //(int)RADMEMORY_ALLOC_TEMP);
    rAssert( current == RADMEMORY_ALLOC_TEMP );
    // loads go into the default section (not the "current" section) unless requested otherwise
    section = tEntity::MakeUID(P3D_DEFAULT_INV_SECTION);

    if(file)
    {
        file->AddRef();
        options->filename = new char[strlen(file->GetFullFilename())+1];
        strcpy(options->filename,file->GetFullFilename());
        options->stream = file;
    }
    // Pure3D uses a default sync loading method
    options->syncLoad = true;
}

tLoadRequest::~tLoadRequest()
{
    if(file)
    {
        file->Release();
    }

    tRefCounted::Release(callback);
    tRefCounted::Release(request);
    delete [] options->filename;
    delete options;
}

void tLoadRequest::SetCallback(Callback* c)
{
    P3DASSERT(GetState() == CREATED);
    tRefCounted::Assign(callback,c);
    tRefCounted::Assign(callback->request, this);
}

void tLoadRequest::SetAsync(bool b)
{
    P3DASSERT(GetState() == CREATED);
    options->syncLoad = !b;
}

void tLoadRequest::SetMemoryAllocator(int a)
{
    P3DASSERT(GetState() == CREATED);
    options->allocator = a;
}

void tLoadRequest::SetInventorySection(const char* n)
{
    P3DASSERT(GetState() == CREATED);
    section = tEntity::MakeUID(n);
}

void tLoadRequest::SetInventorySection(tUID u)
{
    P3DASSERT(GetState() == CREATED);
    section = u;
}

void tLoadRequest::SetStore(tEntityStore* s)
{
    P3DASSERT(GetState() == CREATED);
    store = s;
}

void tLoadRequest::SetPriority(int p)
{
    P3DASSERT(GetState() == CREATED);
    priority = p;
}

void tLoadRequest::SetUserData(void* u)
{
    P3DASSERT(GetState() == CREATED);
    userData = u;
}

void tLoadRequest::SetDummy(bool d)
{
    P3DASSERT(GetState() == CREATED);
    dummy = d;
}

bool tLoadRequest::IsDummy(void)
{
    return dummy;
}

const char* tLoadRequest::GetFilename(void)
{
	return options->filename;
}

float tLoadRequest::GetPortionLoaded(void)
{
	return request->GetPercentDone();
}

radLoadState tLoadRequest::GetState()
{
    if( request )
    {
        return request->GetState();
    }
    return CREATED;
}

void tLoadRequest::Complete()
{
    if( request )
    {
        while (!request->IsComplete())
        {
            radLoadService();
        }
    }
}

void tLoadRequest::Cancel()
{
    if( request )
    {
        request->Cancel();
    }
}

void tLoadRequest::InternalCallback::Done()
{
    tEntityStore* inventory = static_cast<tEntityStore*>(request->request->GetInventory());
    if(inventory)
    {
        if(request->store)
        {
            inventory->Dump(request->store);
        }
        else
        {
            tEntityTable* sectionstore = p3d::context->GetInventory()->GetSection(request->section);
            if(sectionstore)
            {
                inventory->Dump(sectionstore);
            }
            else
            {
                inventory->Dump(p3d::context->GetInventory());
            }
        }
    }

    tRefCounted::Release(request);
}


//----------------------------------------------------------------------
#ifdef P3D_TRACK_LOAD_STATS
unsigned tLoadManager::mTaskedOutTime = 0;
int      tLoadManager::mTaskedOutMemoryDelta = 0;
int      tLoadManager::mTaskedOutAllocationsDelta = 0;
bool     tLoadManager::mLoadActive = false;
#endif

tLoadManager::tLoadManager(int n)
{
}

tLoadManager::~tLoadManager()
{
    TriggerCallbacks();
    RemoveAllHandlers();
}

void tLoadManager::SwitchTask(void)
{
#ifdef P3D_TRACK_LOAD_STATS
    radTime64 start = 0;
    int freememorystart = 0, allocationsstart = 0;

    if (mLoadActive)
    {
        start = radTimeGetMicroseconds64();
        GetMemoryStats(&freememorystart, &allocationsstart);
    }
    
    radLoad->Service();
    
    if (mLoadActive)
    {
        radTime64 end = radTimeGetMicroseconds64();
        mTaskedOutTime += (unsigned) (end - start);

        int freememoryend, allocationsend;
        GetMemoryStats(&freememoryend, &allocationsend);
        mTaskedOutMemoryDelta += (freememoryend - freememorystart);
        mTaskedOutAllocationsDelta += (allocationsend - allocationsstart);
    }
#else
    radLoad->Service();
#endif
}

void tLoadManager::CancelAll(void)
{
    radLoad->Cancel();
}

void tLoadManager::CompleteAll(void)
{
    while( radLoad->IsLoadPending() )
    {
        radLoad->Service();
    }
}

tLoadStatus tLoadManager::Load(tLoadRequest* request)
{
    request->AddRef();

    if(!request->IsDummy())
    {
        radLoad->Load(request->options, &request->request);
        radMemoryAllocator old = ::radMemorySetCurrentAllocator( RADMEMORY_ALLOC_TEMP );
        tLoadRequest::InternalCallback* newRequest = new tLoadRequest::InternalCallback(request);
        ::radMemorySetCurrentAllocator( old );
        radLoad->AddCallback( newRequest );
    }
    if(request->callback)
    {
        radLoad->AddCallback(request->callback);
    }

    request->Release();
    return LOAD_OK;
}

void tLoadManager::TriggerCallbacks(void)
{
}

tFileHandler* tLoadManager::AddHandler(tFileHandler* l, const char* ext)
{
    radLoad->AddFileLoader(l, ext);
    return l;
}

tFileHandler* tLoadManager::GetHandler(const char* ext)
{
    return dynamic_cast<tFileHandler*>(radLoad->GetFileLoader(ext));
}

void tLoadManager::RemoveHandler(const char* ext)
{
    radLoad->RemoveFileLoader(ext);
}

void tLoadManager::RemoveAllHandlers()
{
    // Stub
    // No way to do this right now
}


tP3DFileHandler* tLoadManager::GetP3DHandler(void)
{
    return dynamic_cast<tP3DFileHandler*>(GetHandler(".p3d"));
}



//-----------------------------------------------------------------------------------
#ifdef P3D_TRACK_LOAD_STATS
void tLoadManager::GetMemoryStats(int *freeMemory, int *allocations)
{
    IRadMemoryAllocator *allocator = radMemoryGetAllocator(radMemoryGetCurrentAllocator());
    unsigned unused;
    unsigned totalFree, numAllocations;
    allocator->GetStatus(&totalFree, &unused, &numAllocations, &unused);       

    *freeMemory  = (int) totalFree;
    *allocations = (int) numAllocations;
}


//-----------------------------------------------------------------------------------
const char *tChunkHandler::ChunkTypeName(unsigned id)
{
    switch (id)
    {
        case Pure3D::Mesh::MESH:                            return "Mesh::MESH";
        case Pure3D::Mesh::SKIN:                            return "Mesh::SKIN";
        case Pure3D::Mesh::PRIMGROUP:                       return "Mesh::PRIMGROUP";
        case Pure3D::Mesh::BOX:                             return "Mesh::BOX";
        case Pure3D::Mesh::SPHERE:                          return "Mesh::SPHERE";
        case Pure3D::Mesh::POSITIONLIST:                    return "Mesh::POSITIONLIST";
        case Pure3D::Mesh::NORMALLIST:                      return "Mesh::NORMALLIST";
        case Pure3D::Mesh::UVLIST:                          return "Mesh::UVLIST";
        case Pure3D::Mesh::COLOURLIST:                      return "Mesh::COLOURLIST";
        case Pure3D::Mesh::STRIPLIST:                       return "Mesh::STRIPLIST";
        case Pure3D::Mesh::INDEXLIST:                       return "Mesh::INDEXLIST";
        case Pure3D::Mesh::MATRIXLIST:                      return "Mesh::MATRIXLIST";
        case Pure3D::Mesh::WEIGHTLIST:                      return "Mesh::WEIGHTLIST";
        case Pure3D::Mesh::MATRIXPALETTE:                   return "Mesh::MATRIXPALETTE";
        case Pure3D::Mesh::OFFSETLIST:                      return "Mesh::OFFSETLIST";
        case Pure3D::Mesh::INSTANCEINFO:                    return "Mesh::INSTANCEINFO";
        case Pure3D::Mesh::PACKEDNORMALLIST:                return "Mesh::PACKEDNORMALLIST";
        case Pure3D::Mesh::VERTEXSHADER:                    return "Mesh::VERTEXSHADER";
        case Pure3D::Mesh::MEMORYIMAGEVERTEXLIST:           return "Mesh::MEMORYIMAGEVERTEXLIST";
        case Pure3D::Mesh::MEMORYIMAGEINDEXLIST:            return "Mesh::MEMORYIMAGEINDEXLIST";
        case Pure3D::Mesh::MEMORYIMAGEVERTEXDESCRIPTIONLIST: return "Mesh::MEMORYIMAGEVERTEXDESCRIPTIONLIST";
        case Pure3D::Mesh::TANGENTLIST:                     return "Mesh::TANGENTLIST";
        case Pure3D::Mesh::BINORMALLIST:                    return "Mesh::BINORMALLIST";
        case Pure3D::Mesh::RENDERSTATUS:                    return "Mesh::RENDERSTATUS";
        case Pure3D::Mesh::EXPRESSIONOFFSETS:               return "Mesh::EXPRESSIONOFFSETS";
        case Pure3D::Mesh::SHADOWSKIN:                      return "Mesh::SHADOWSKIN";
        case Pure3D::Mesh::SHADOWMESH:                      return "Mesh::SHADOWMESH";
        case Pure3D::Mesh::TOPOLOGY:                        return "Mesh::TOPOLOGY";

        case Pure3D::Shader::SHADER:                        return "Shader::SHADER";
        case Pure3D::Shader::SHADER_DEFINITION:             return "Shader::SHADER_DEFINITION";
        case Pure3D::Shader::TEXTURE_PARAM:                 return "Shader::TEXTURE_PARAM";
        case Pure3D::Shader::INT_PARAM:                     return "Shader::INT_PARAM";
        case Pure3D::Shader::FLOAT_PARAM:                   return "Shader::FLOAT_PARAM";
        case Pure3D::Shader::COLOUR_PARAM:                  return "Shader::COLOUR_PARAM";
        case Pure3D::Shader::VECTOR_PARAM:                  return "Shader::VECTOR_PARAM";
        case Pure3D::Shader::MATRIX_PARAM:                  return "Shader::MATRIX_PARAM";

        case Pure3D::GameAttr::GAME_ATTR:                   return "GameAttr::GAME_ATTR";
        case Pure3D::GameAttr::INT_PARAM:                   return "GameAttr::INT_PARAM";
        case Pure3D::GameAttr::FLOAT_PARAM:                 return "GameAttr::FLOAT_PARAM";
        case Pure3D::GameAttr::COLOUR_PARAM:                return "GameAttr::COLOUR_PARAM";
        case Pure3D::GameAttr::VECTOR_PARAM:                return "GameAttr::VECTOR_PARAM";
        case Pure3D::GameAttr::MATRIX_PARAM:                return "GameAttr::MATRIX_PARAM";

        case Pure3D::Light::LIGHT:                          return "Light::LIGHT";
        case Pure3D::Light::DIRECTION:                      return "Light::DIRECTION";
        case Pure3D::Light::POSITION:                       return "Light::POSITION";
        case Pure3D::Light::CONE_PARAM:                     return "Light::CONE_PARAM";
        case Pure3D::Light::SHADOW:                         return "Light::SHADOW";
        case Pure3D::Light::PHOTON_MAP:                     return "Light::PHOTON_MAP";
        case Pure3D::Light::DECAY_RANGE:                    return "Light::DECAY_RANGE";

        case Pure3D::Locator::LOCATOR:                      return "Locator::Locator";

        case Pure3D::ParticleSystem::SYSTEM_FACTORY:           return "ParticleSystem::SYSTEM_FACTORY";
        case Pure3D::ParticleSystem::SYSTEM:                   return "ParticleSystem::SYSTEM";
        case Pure3D::ParticleSystem::BASE_PARTICLE_ARRAY:      return "ParticleSystem::BASE_PARTICLE_ARRAY";
        case Pure3D::ParticleSystem::SPRITE_PARTICLE_ARRAY:    return "ParticleSystem::SPRITE_PARTICLE_ARRAY";
        case Pure3D::ParticleSystem::DRAWABLE_PARTICLE_ARRAY:  return "ParticleSystem::DRAWABLE_PARTICLE_ARRAY";
        case Pure3D::ParticleSystem::BASE_EMITTER_FACTORY:     return "ParticleSystem::BASE_EMITTER_FACTORY";
        case Pure3D::ParticleSystem::SPRITE_EMITTER_FACTORY:   return "ParticleSystem::SPRITE_EMITTER_FACTORY";
        case Pure3D::ParticleSystem::DRAWABLE_EMITTER_FACTORY: return "ParticleSystem::DRAWABLE_EMITTER_FACTORY";
        case Pure3D::ParticleSystem::PARTICLE_ANIMATION:       return "ParticleSystem::PARTICLE_ANIMATION";
        case Pure3D::ParticleSystem::EMITTER_ANIMATION:        return "ParticleSystem::EMITTER_ANIMATION";
        case Pure3D::ParticleSystem::GENERATOR_ANIMATION:      return "ParticleSystem::GENERATOR_ANIMATION";
        case Pure3D::ParticleSystem::INSTANCING_INFO:          return "ParticleSystem::INSTANCING_INFO";

        case Pure3D::OpticEffect::CORONA_V14:                  return "OpticEffect::CORONA_V14";
        case Pure3D::OpticEffect::LENS_FLARE_PARENT_V14:       return "OpticEffect::LENS_FLARE_PARENT_V14";
        case Pure3D::OpticEffect::LENS_FLARE_V14:              return "OpticEffect::LENS_FLARE_V14";
        case Pure3D::OpticEffect::VECTOR_V14:                  return "OpticEffect::VECTOR_V14";
        case Pure3D::OpticEffect::LENS_FLARE_GROUP:            return "OpticEffect::LENS_FLARE_GROUP";
        case Pure3D::OpticEffect::LENS_FLARE:                  return "OpticEffect::LENS_FLARE";

        case Pure3D::BillboardObject::QUAD_V14:                return "BillboardObject::QUAD_V14";
        case Pure3D::BillboardObject::QUAD:                    return "BillboardObject::QUAD";
        case Pure3D::BillboardObject::QUAD_GROUP:              return "BillboardObject::QUAD_GROUP";
        case Pure3D::BillboardObject::DISPLAY_INFO:            return "BillboardObject::DISPLAY_INFO";
        case Pure3D::BillboardObject::PERSPECTIVE_INFO:        return "BillboardObject::PERSPECTIVE_INFO";

        case Pure3D::Scrooby::PROJECT:                         return "Scrooby::PROJECT";
        case Pure3D::Scrooby::SCREEN:                          return "Scrooby::SCREEN";
        case Pure3D::Scrooby::PAGE:                            return "Scrooby::PAGE";
        case Pure3D::Scrooby::LAYER:                           return "Scrooby::LAYER";
        case Pure3D::Scrooby::GROUP:                           return "Scrooby::GROUP";
        case Pure3D::Scrooby::MOVIE:                           return "Scrooby::MOVIE";
        case Pure3D::Scrooby::MULTISPRITE:                     return "Scrooby::MULTISPRITE";
        case Pure3D::Scrooby::MULTITEXT:                       return "Scrooby::MULTITEXT";
        case Pure3D::Scrooby::P3DOBJECT:                       return "Scrooby::P3DOBJECT";
        case Pure3D::Scrooby::POLYGON:                         return "Scrooby::POLYGON";
        case Pure3D::Scrooby::SPRITE:                          return "Scrooby::SPRITE";
        case Pure3D::Scrooby::STRINGTEXTBIBLE:                 return "Scrooby::STRINGTEXTBIBLE";
        case Pure3D::Scrooby::STRINGHARDCODED:                 return "Scrooby::STRINGHARDCODED";
        case Pure3D::Scrooby::TEXTBIBLE:                       return "Scrooby::TEXTBIBLE";
        case Pure3D::Scrooby::LANGUAGE:                        return "Scrooby::LANGUAGE";
        case Pure3D::Scrooby::RESOURCEIMAGE:                   return "Scrooby::RESOURCEIMAGE";
        case Pure3D::Scrooby::RESOURCEPURE3D:                  return "Scrooby::RESOURCEPURE3D";
        case Pure3D::Scrooby::OLDRESOURCETEXTSTYLE:            return "Scrooby::OLDRESOURCETEXTSTYLE";
        case Pure3D::Scrooby::OLDRESOURCETEXTBIBLE:            return "Scrooby::OLDRESOURCETEXTBIBLE";
        case Pure3D::Scrooby::RESOURCETEXTSTYLE:               return "Scrooby::RESOURCETEXTSTYLE";
        case Pure3D::Scrooby::RESOURCETEXTBIBLE:               return "Scrooby::RESOURCETEXTBIBLE";

        case Pure3D::Texture::TEXTURE:                         return "Texture::TEXTURE";
        case Pure3D::Texture::IMAGE:                           return "Texture::IMAGE";
        case Pure3D::Texture::IMAGE_DATA:                      return "Texture::IMAGE_DATA";
        case Pure3D::Texture::IMAGE_FILENAME:                  return "Texture::IMAGE_FILENAME";
        case Pure3D::Texture::VOLUME_IMAGE:                    return "Texture::VOLUME_IMAGE";
        case Pure3D::Texture::SPRITE:                          return "Texture::SPRITE";

        case Pure3D::AnimatedObject::FACTORY:                  return "AnimatedObject::FACTORY";
        case Pure3D::AnimatedObject::OBJECT:                   return "AnimatedObject::OBJECT";
        case Pure3D::AnimatedObject::ANIMATION:                return "AnimatedObject::ANIMATION";

        case Pure3D::Expression::VERTEX_EXPRESSION:            return "Expression::VERTEX_EXPRESSION";
        case Pure3D::Expression::VERTEX_EXPRESSION_GROUP:      return "Expression::VERTEX_EXPRESSION_GROUP";
        case Pure3D::Expression::VERTEX_EXPRESSION_MIXER:      return "Expression::VERTEX_EXPRESSION_MIXER";

        case Pure3D::Font::TEXTURE_FONT:                       return "Font::TEXTURE_FONT";
        case Pure3D::Font::TEXTURE_GLYPH_LIST:                 return "Font::TEXTURE_GLYPH_LIST";
        case Pure3D::Font::IMAGE_FONT:                         return "Font::IMAGE_FONT";
        case Pure3D::Font::IMAGE_GLYPH_LIST:                   return "Font::IMAGE_GLYPH_LIST";

        case Pure3D::SceneGraph::SCENEGRAPH:                   return "SceneGraph::SCENEGRAPH";
        case Pure3D::SceneGraph::ROOT:                         return "SceneGraph::ROOT";
        case Pure3D::SceneGraph::BRANCH:                       return "SceneGraph::BRANCH";
        case Pure3D::SceneGraph::TRANSFORM:                    return "SceneGraph::TRANSFORM";
        case Pure3D::SceneGraph::VISIBILITY:                   return "SceneGraph::VISIBILITY";
        case Pure3D::SceneGraph::ATTACHMENT:                   return "SceneGraph::ATTACHMENT";
        case Pure3D::SceneGraph::ATTACHMENTPOINT:              return "SceneGraph::ATTACHMENTPOINT";
        case Pure3D::SceneGraph::DRAWABLE:                     return "SceneGraph::DRAWABLE";
        case Pure3D::SceneGraph::CAMERA:                       return "SceneGraph::CAMERA";
        case Pure3D::SceneGraph::LIGHTGROUP:                   return "SceneGraph::LIGHTGROUP";
        case Pure3D::SceneGraph::SORTORDER:                    return "SceneGraph::SORTORDER";

        case Pure3D::Animation::AnimationData::ANIMATION:               return "Animation::AnimationData::ANIMATION";
        case Pure3D::Animation::AnimationData::GROUP:                   return "Animation::AnimationData::GROUP";
        case Pure3D::Animation::AnimationData::GROUP_LIST:              return "Animation::AnimationData::GROUP_LIST";
        case Pure3D::Animation::AnimationData::SIZE:                    return "Animation::AnimationData::SIZE"; 
        case Pure3D::Animation::ChannelData::FLOAT_1:                   return "Animation::ChannelData::FLOAT_1";
        case Pure3D::Animation::ChannelData::FLOAT_2:                   return "Animation::ChannelData::FLOAT_2";
        case Pure3D::Animation::ChannelData::VECTOR_1DOF:               return "Animation::ChannelData::VECTOR_1DOF";
        case Pure3D::Animation::ChannelData::VECTOR_2DOF:               return "Animation::ChannelData::VECTOR_2DOF";
        case Pure3D::Animation::ChannelData::VECTOR_3DOF:               return "Animation::ChannelData::VECTOR_3DOF";
        case Pure3D::Animation::ChannelData::QUATERNION:                return "Animation::ChannelData::QUATERNION"; 
        case Pure3D::Animation::ChannelData::STRING:                    return "Animation::ChannelData::STRING";
        case Pure3D::Animation::ChannelData::ENTITY:                    return "Animation::ChannelData::ENTITY";
        case Pure3D::Animation::ChannelData::BOOL:                      return "Animation::ChannelData::BOOL";
        case Pure3D::Animation::ChannelData::COLOUR:                    return "Animation::ChannelData::COLOUR";
        case Pure3D::Animation::ChannelData::EVENT:                     return "Animation::ChannelData::EVENT";
        case Pure3D::Animation::ChannelData::EVENT_OBJECT:              return "Animation::ChannelData::EVENT_OBJECT";
        case Pure3D::Animation::ChannelData::EVENT_OBJECT_DATA:         return "Animation::ChannelData::EVENT_OBJECT_DATA";
        case Pure3D::Animation::ChannelData::EVENT_OBJECT_DATA_IMAGE:   return "Animation::ChannelData::EVENT_OBJECT_DATA_IMAGE";
        case Pure3D::Animation::ChannelData::INT:                       return "Animation::ChannelData::INT";
        case Pure3D::Animation::ChannelData::QUATERNION_FORMAT:         return "Animation::ChannelData::QUATERNION_FORMAT";
        case Pure3D::Animation::ChannelData::INTERPOLATION_MODE:        return "Animation::ChannelData::INTERPOLATION_MODE";
        case Pure3D::Animation::FrameControllerData::FRAME_CONTROLLER:  return "Animation::FrameControllerData::FRAME_CONTROLLER";
        case Pure3D::Animation::VertexAnim::OffsetList::COLOUR:         return "Animation::VertexAnim::OffsetList::COLOUR";
        case Pure3D::Animation::VertexAnim::OffsetList::VECTOR:         return "Animation::VertexAnim::OffsetList::VECTOR";
        case Pure3D::Animation::VertexAnim::OffsetList::VECTOR2:        return "Animation::VertexAnim::OffsetList::VECTOR2";
        case Pure3D::Animation::VertexAnim::OffsetList::INDEX:          return "Animation::VertexAnim::OffsetList::INDEX";
        case Pure3D::Animation::VertexAnim::KEY:                        return "Animation::VertexAnim::KEY";

        case Simulation::Collision::OBJECT:                    return "Simulation::Collision::OBJECT";
        case Simulation::Collision::VOLUME:                    return "Simulation::Collision::VOLUME";
        case Simulation::Collision::SPHERE:                    return "Simulation::Collision::SPHERE";
        case Simulation::Collision::CYLINDER:                  return "Simulation::Collision::CYLINDER";
        case Simulation::Collision::OBBOX:                     return "Simulation::Collision::OBBOX";
        case Simulation::Collision::WALL:                      return "Simulation::Collision::WALL";
        case Simulation::Collision::BBOX:                      return "Simulation::Collision::BBOX";
        case Simulation::Collision::VECTOR:                    return "Simulation::Collision::VECTOR";
        case Simulation::Collision::SELFCOLLISION:             return "Simulation::Collision::SELFCOLLISION";
        case Simulation::Collision::OWNER:                     return "Simulation::Collision::OWNER";
        case Simulation::Collision::OWNERNAME:                 return "Simulation::Collision::OWNERNAME";
        case Simulation::Collision::ATTRIBUTE:                 return "Simulation::Collision::ATTRIBUTE";

        case Simulation::Physics::OBJECT:                      return "Simulation::Physics::OBJECT";
        case Simulation::Physics::IMAT:                        return "Simulation::Physics::IMAT";
        case Simulation::Physics::VECTOR:                      return "Simulation::Physics::VECTOR";
        case Simulation::Physics::JOINT:                       return "Simulation::Physics::JOINT";
        case Simulation::Physics::JOINT_DOF:                   return "Simulation::Physics::JOINT_DOF";

        case Simulation::Flexible::OBJECT:                     return "Simulation::Flexible::OBJECT";
        case Simulation::Flexible::OBJECT_PARAMETERS:          return "Simulation::Flexible::OBJECT_PARAMETERS";
        case Simulation::Flexible::FIX_PARTICLE:               return "Simulation::Flexible::FIX_PARTICLE";
        case Simulation::Flexible::MAP_VL:                     return "Simulation::Flexible::MAP_VL";
        case Simulation::Flexible::TRI_MAP:                    return "Simulation::Flexible::TRI_MAP";
        case Simulation::Flexible::EDGE_MAP:                   return "Simulation::Flexible::EDGE_MAP";
        case Simulation::Flexible::EDGE_LEN:                   return "Simulation::Flexible::EDGE_LEN";
        case Simulation::Flexible::OBJECT_LAMBDA:              return "Simulation::Flexible::OBJECT_LAMBDA";
        case Simulation::Flexible::JOINT:                      return "Simulation::Flexible::JOINT";
        case Simulation::Flexible::JOINT_PARAMETERS:           return "Simulation::Flexible::JOINT_PARAMETERS";
        case Simulation::Flexible::JOINT_DEFINITION:           return "Simulation::Flexible::JOINT_DEFINITION";
        case Simulation::Flexible::JOINT_LAMBDA:               return "Simulation::Flexible::JOINT_LAMBDA";

        case Simulation::Link::IK:                             return "Simulation::Link::IK";
        case Simulation::Link::REACH:                          return "Simulation::Link::REACH";
        case Simulation::Link::TRACKER:                        return "Simulation::Link::TRACKER";
        case Simulation::Link::TARGET:                         return "Simulation::Link::TARGET";
        case Simulation::Link::TARGET_NODE:                    return "Simulation::Link::TARGET_NODE";
        case Simulation::Link::TARGET_POSE:                    return "Simulation::Link::TARGET_POSE";

        case Simulation::Parameters::ENVIRONMENT:              return "Simulation::Parameters::ENVIRONMENT";
        case Simulation::Parameters::PHYSICS:                  return "Simulation::Parameters::PHYSICS";

        case SmartProp::SMARTPROP:                             return "SmartProp::SMARTPROP";
        case SmartProp::SMARTPROPSTATEDATA:                    return "SmartProp::SMARTPROPSTATEDATA";
        case SmartProp::SMARTPROPVISIBILITYDATA:               return "SmartProp::SMARTPROPVISIBILITYDATA";
        case SmartProp::SMARTPROPFRAMECONTROLLERDATA:          return "SmartProp::SMARTPROPFRAMECONTROLLERDATA";
        case SmartProp::SMARTPROPEVENTDATA:                    return "SmartProp::SMARTPROPEVENTDATA";
        case SmartProp::SMARTPROPCALLBACKDATA:                 return "SmartProp::SMARTPROPCALLBACKDATA";
        case SmartProp::SMARTPROPAPPLIEDFORCE:                 return "SmartProp::SMARTPROPAPPLIEDFORCE";
        case SmartProp::SMARTPROPBREAKABLE:                    return "SmartProp::SMARTPROPBREAKABLE";
        case SmartProp::SMARTPROPEXTRAATTRIBUTE:               return "SmartProp::SMARTPROPEXTRAATTRIBUTE";

        case MemorySection::MEMORYSECTION:                     return "MemorySection::MEMORYSECTION";


        // Olsd style chunk ID's
        case P3D_IMAGE:                           return "Old style P3D_IMAGE";
        case P3D_IMAGE_DATA:                      return "Old style P3D_IMAGE_DATA";
        case P3D_IMAGE_FILENAME:                  return "Old style P3D_IMAGE_FILENAME";

        case P3D_SKELETON:                        return "Old style P3D_SKELETON";
        case P3D_SKELETON_JOINT:                  return "Old style P3D_SKELETON_JOINT";
        case P3D_SKELETON_JOINT_PHYSICS:          return "Old style P3D_SKELETON_JOINT_PHYSICS";
        case P3D_SKELETON_JOINT_MIRROR_MAP:       return "Old style P3D_SKELETON_JOINT_MIRROR_MAP";
        case P3D_SKELETON_JOINT_FIX_FLAG:         return "Old style P3D_SKELETON_JOINT_FIX_FLAG";

        case P3D_COMPOSITE_DRAWABLE:              return "Old style P3D_COMPOSITE_DRAWABLE";
        case P3D_COMPOSITE_DRAWABLE_SKIN_LIST:    return "Old style P3D_COMPOSITE_DRAWABLE_SKIN_LIST";
        case P3D_COMPOSITE_DRAWABLE_PROP_LIST:    return "Old style P3D_COMPOSITE_DRAWABLE_PROP_LIST";
        case P3D_COMPOSITE_DRAWABLE_SKIN:         return "Old style P3D_COMPOSITE_DRAWABLE_SKIN";
        case P3D_COMPOSITE_DRAWABLE_PROP:         return "Old style P3D_COMPOSITE_DRAWABLE_PROP";
        case P3D_COMPOSITE_DRAWABLE_EFFECT_LIST:  return "Old style P3D_COMPOSITE_DRAWABLE_EFFECT_LIST";
        case P3D_COMPOSITE_DRAWABLE_EFFECT:       return "Old style P3D_COMPOSITE_DRAWABLE_EFFECT";
        case P3D_COMPOSITE_DRAWABLE_SORTORDER:    return "Old style P3D_COMPOSITE_DRAWABLE_SORTORDER";

        case P3D_FRAME_CONTROLLER:                return "Old style P3D_FRAME_CONTROLLER";

        case P3D_MULTI_CONTROLLER:                return "Old style P3D_MULTI_CONTROLLER";
        case P3D_MULTI_CONTROLLER_TRACKS:         return "Old style P3D_MULTI_CONTROLLER_TRACKS";
        case P3D_MULTI_CONTROLLER_TRACK:          return "Old style P3D_MULTI_CONTROLLER_TRACK";

        case P3D_CAMERA:                          return "Old style P3D_CAMERA";
        case P3D_LIGHT_GROUP:                     return "Old style P3D_LIGHT_GROUP";
        case P3D_HISTORY:                         return "Old style P3D_HISTORY";
        case P3D_ALIGN:                           return "Old style P3D_ALIGN";

        case P3D_EXPORT_INFO:                     return "Old style P3D_EXPORT_INFO";
        case P3D_EXPORT_NAMED_STRING:             return "Old style P3D_EXPORT_NAMED_STRING";
        case P3D_EXPORT_NAMED_INT:                return "Old style P3D_EXPORT_NAMED_INT";

        //case P3D_EXPRESSION_PRESET:               return "Old style P3D_EXPRESSION_PRESET";
        //case P3D_EXPRESSION_GROUP:                return "Old style P3D_EXPRESSION_GROUP";
        //case P3D_EXPRESSION_ANIM:                 return "Old style P3D_EXPRESSION_ANIM";
        //case P3D_EXPRESSION_ANIM_CHANNEL:         return "Old style P3D_EXPRESSION_ANIM_CHANNEL";
        //case P3D_EXPRESSION_MIXER:                return "Old style P3D_EXPRESSION_MIXER";

        case P3D_VERTEXOFFSET:                    return "Old style P3D_VERTEXOFFSET";
        case P3D_VERTEXOFFSET_ANIM:               return "Old style P3D_VERTEXOFFSET_ANIM";
        case P3D_VERTEX_OFFSET_EXPRESSION:        return "Old style P3D_VERTEX_OFFSET_EXPRESSION";

        case P3D_SG_TRANSFORM_ANIM:               return "Old style P3D_SG_TRANSFORM_ANIM";

        case P3D_VISIBILITY_ANIM:                 return "Old style P3D_VISIBILITY_ANIM";
        case P3D_VISIBILITY_ANIM_CHANNEL:         return "Old style P3D_VISIBILITY_ANIM_CHANNEL";

        case P3D_TEXTURE_ANIM:                    return "Old style P3D_TEXTURE_ANIM";
        case P3D_TEXTURE_ANIM_CHANNEL:            return "Old style P3D_TEXTURE_ANIM_CHANNEL";

        case P3D_POSE_ANIM:                       return "Old style P3D_POSE_ANIM";
        case P3D_JOINT_LIST:                      return "Old style P3D_JOINT_LIST";
        case P3D_ANIM_CHANNEL:                    return "Old style P3D_ANIM_CHANNEL";
        case P3D_POSE_ANIM_MIRRORED:              return "Old style P3D_POSE_ANIM_MIRRORED";

        case P3D_CHANNEL_1DOF:                    return "Old style P3D_CHANNEL_1DOF";
        case P3D_CHANNEL_3DOF:                    return "Old style P3D_CHANNEL_3DOF";
        case P3D_CHANNEL_1DOF_ANGLE:              return "Old style P3D_CHANNEL_1DOF_ANGLE";
        case P3D_CHANNEL_3DOF_ANGLE:              return "Old style P3D_CHANNEL_3DOF_ANGLE";
        case P3D_CHANNEL_STATIC:                  return "Old style P3D_CHANNEL_STATIC";
        case P3D_CHANNEL_STATIC_ANGLE:            return "Old style P3D_CHANNEL_STATIC_ANGLE";
        case P3D_CHANNEL_QUATERNION:              return "Old style P3D_CHANNEL_QUATERNION";
        case P3D_CHANNEL_STATIC_QUATERNION:       return "Old style P3D_CHANNEL_STATIC_QUATERNION";

        case P3D_CAMERA_ANIM:                     return "Old style P3D_CAMERA_ANIM";
        case P3D_CAMERA_ANIM_CHANNEL:             return "Old style P3D_CAMERA_ANIM_CHANNEL";
        case P3D_CAMERA_ANIM_POS_CHANNEL:         return "Old style P3D_CAMERA_ANIM_POS_CHANNEL";
        case P3D_CAMERA_ANIM_LOOK_CHANNEL:        return "Old style P3D_CAMERA_ANIM_LOOK_CHANNEL";
        case P3D_CAMERA_ANIM_UP_CHANNEL:          return "Old style P3D_CAMERA_ANIM_UP_CHANNEL";
        case P3D_CAMERA_ANIM_FOV_CHANNEL:         return "Old style P3D_CAMERA_ANIM_FOV_CHANNEL";

        case P3D_CAMERA_ANIM_NEARCLIP_CHANNEL:    return "Old style P3D_CAMERA_ANIM_NEARCLIP_CHANNEL";
        case P3D_CAMERA_ANIM_FARCLIP_CHANNEL:     return "Old style P3D_CAMERA_ANIM_FARCLIP_CHANNEL";

        case P3D_LIGHT_ANIM:                      return "Old style P3D_LIGHT_ANIM";
        case P3D_LIGHT_ANIM_CHANNEL:              return "Old style P3D_LIGHT_ANIM_CHANNEL";
        case P3D_LIGHT_ANIM_COLOUR_CHANNEL:       return "Old style P3D_LIGHT_ANIM_COLOUR_CHANNEL";
        case P3D_LIGHT_ANIM_PARAM_CHANNEL:        return "Old style P3D_LIGHT_ANIM_PARAM_CHANNEL";
        case P3D_LIGHT_ANIM_ENABLE_CHANNEL:       return "Old style P3D_LIGHT_ANIM_ENABLE_CHANNEL";

        case P3D_ENTITY_ANIM_CHANNEL:             return "Old style P3D_ENTITY_ANIM_CHANNEL";
        case P3D_KEYLIST_COLOUR:                  return "Old style P3D_KEYLIST_COLOUR";

        case P3D_VERTEX_ANIM:                     return "Old style P3D_VERTEX_ANIM";
        case P3D_VERTEX_ANIM_CHANNEL:             return "Old style P3D_VERTEX_ANIM_CHANNEL";

        default:
        {
            static char s[80];
            switch (id & 0xFF000000)
            {
                case 0x00000000: sprintf(s, "Unknown Pure3D Chunk (%08X)", id);     break;
                case 0x01000000: sprintf(s, "Unknown Foundation Chunk (%08X)", id); break;
                case 0x02000000: sprintf(s, "Unknown Hairclub Chunk (%08X)", id);   break;
                case 0x03000000: sprintf(s, "Unknown Simpsons Chunk (%08X)", id);   break;
                case 0x04000000: sprintf(s, "Unknown M2 Chunk (%08X)", id);         break;
                case 0x05000000: sprintf(s, "Unknown Squidney Chunk (%08X)", id);   break;
                case 0x06000000: sprintf(s, "Unknown Penthouse Chunk (%08X)", id);  break;
                case 0x07000000: sprintf(s, "Unknown Simulation Chunk (%08X)", id); break;
                case 0x08000000: sprintf(s, "Unknown SmartProp Chunk (%08X)", id);  break;

                default:         sprintf(s, "Unknown Reserved Chunk (%08X)", id);
            }
            return s;
        }
    }
    return "Oh shit!";
}

#endif //P3D_TRACK_LOAD_STATS
