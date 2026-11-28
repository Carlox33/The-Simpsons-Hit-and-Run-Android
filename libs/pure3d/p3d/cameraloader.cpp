//=============================================================================
// Copyright (c) 2002 Radical Games Ltd.  All rights reserved.
//=============================================================================


#include <p3d/vectorcamera.hpp>
#include <constants/chunks.h>
#include <p3d/chunkfile.hpp>
#include <p3d/file.hpp>
#include <p3d/error.hpp>
#include <p3d/inventory.hpp>
#include <p3d/utility.hpp>

tCameraLoader::tCameraLoader() : tSimpleChunkHandler(P3D_CAMERA)
{
}

tEntity* tCameraLoader::LoadObject(tChunkFile* f, tEntityStore* store)
{
    char name[256];
    f->GetPString(name);

    tVectorCamera* camera = new tVectorCamera;
    camera->SetName(name);

    rmt::Vector pos, look, up;

    long version = f->GetLong();
    float fov = f->GetFloat();
    float a = f->GetFloat();
    float aspectratio = p3d::display->IsWidescreen() ? ((float)p3d::display->GetWidth() / (float)p3d::display->GetHeight()) : a;
    float nearclip = f->GetFloat();
    float farclip = f->GetFloat();

    pos.x = f->GetFloat();
    pos.y = f->GetFloat();
    pos.z = f->GetFloat();

    look.x = f->GetFloat();
    look.y = f->GetFloat();
    look.z = f->GetFloat();

    up.x = f->GetFloat();
    up.y = f->GetFloat();
    up.z = f->GetFloat();
    
    camera->SetFOV(fov, aspectratio);   
    camera->SetNearPlane(nearclip);
    camera->SetFarPlane(farclip);

    camera->SetPosition(pos);
    camera->SetDirection(look);
    camera->SetUpVector(up);

    return camera;
}

