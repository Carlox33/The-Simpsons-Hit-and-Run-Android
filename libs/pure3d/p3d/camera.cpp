//=============================================================================
// Copyright (c) 2002 Radical Games Ltd.  All rights reserved.
//=============================================================================


#include <p3d/camera.hpp>
#include <p3d/context.hpp>
#include <p3d/utility.hpp>

tCamera::tCamera()
{
    bool widescreen = p3d::display ? p3d::display->IsWidescreen() : false;
    float aspect = widescreen ? ((float)p3d::display->GetWidth() / (float)p3d::display->GetHeight()) : (4.0f / 3.0f);
    SetFOV(rmt::DegToRadian(90.0f), aspect);

    nearPlane = 1.0f;
    farPlane = 1000.0f;

    worldToCamera.Identity();
    cameraToWorld.Identity();

    updated = true;
}

tCamera::tCamera(tCamera* other) : tEntity(other)
{
    SetFOV(other->GetFieldOfView(), other->GetAspectRatio());

    SetNearPlane(other->GetNearPlane());
    SetFarPlane(other->GetFarPlane());

    worldToCamera.Identity();
    cameraToWorld.Identity();

    cameraToWorld = other->GetCameraToWorldMatrix();
    worldToCamera.Identity();
    updated = false;
}

tCamera::~tCamera()
{
    //
}

void tCamera::SetFOV(float newFov, float newAspect)
{
    fov = newFov;
    aspect = newAspect;

    float effectiveFov = newFov;
    const float refAspect = 16.0f / 9.0f;
    if (newAspect > refAspect)
    {
        effectiveFov = 2.0f * rmt::ATan(rmt::Tan(newFov * 0.5f) * (newAspect / refAspect));
    }

    fovScaleX = 1.0f / rmt::Tan(effectiveFov * 0.5f);
    fovScaleY = fovScaleX;
//  Note: Made fovScaleY = fovScaleX because it seems to mess up the world to device calculations.
//        To see this effect just draw a quad in world space and then transform the verticies into
//        device space using Context::WorldToDevice() and draw it using pddi->SetProjectionMode(PDDI_PROJECTION_DEVICE);
//        and they won't match up.
//  fovScaleY = fovScaleX * aspect;

    float fovy = rmt::ATan(rmt::Tan(effectiveFov/2.0f) / aspect);
    float fovx = effectiveFov/2;

    float sx = rmt::Sin(fovx);
    float cx = rmt::Cos(fovx);
    float sy = rmt::Sin(fovy);
    float cy = rmt::Cos(fovy);

    leftPlaneX = -cx;
    leftPlaneZ = -sx;
 
    rightPlaneX = cx;
    rightPlaneZ = -sx;

    upPlaneY = cy;
    upPlaneZ = -sy;

    downPlaneY = -cy;
    downPlaneZ = -sy;
}

void tCamera::GetFOV(float* fovOut, float* aspectOut)
{
    *fovOut = fov;
    *aspectOut = aspect;
}

void tCamera::SetNearPlane(float newNear)
{
    nearPlane = newNear;
}

void tCamera::SetFarPlane(float newFar)
{
    farPlane = newFar;
}

float tCamera::GetNearPlane()
{
    return nearPlane;
}

float tCamera::GetFarPlane()
{
    return farPlane;
}

void tCamera::GetCameraSpaceClipPlane(ClipCode which, rmt::Plane* plane)
{
    switch(which)
    { 
        case CLIP_NEAR:
            plane->normal.Set(0.0f,0.0f,-1.0f);
            plane->D = nearPlane;
            break;
        case CLIP_FAR:
            plane->normal.Set(0.0f,0.0f,1.0f);
            plane->D = -farPlane;
            break;
        case CLIP_TOP:
            plane->normal.Set(0.0f, upPlaneY, upPlaneZ);
            plane->D = 0.0f;
            break;
        case CLIP_BOTTOM:
            plane->normal.Set(0.0f, downPlaneY, downPlaneZ);
            plane->D = 0.0f;
            break;
        case CLIP_LEFT: 
            plane->normal.Set(leftPlaneX, 0.0f, leftPlaneZ);
            plane->D = 0.0f;
            break;
        case CLIP_RIGHT:
            plane->normal.Set(rightPlaneX, 0.0f, rightPlaneZ);
            plane->D = 0.0f;
            break;
        default:
            P3DASSERTMSG(0, "tCamera : Invalid clip plane specified","");
            break;
    }
}

void tCamera::GetWorldSpaceClipPlane(ClipCode which, rmt::Plane* plane)
{
    GetCameraSpaceClipPlane(which, plane);
    plane->Transform(GetCameraToWorldMatrix());
}

const rmt::Matrix& tCamera::GetWorldToCameraMatrix(void)
{
    if(!updated)
        Update();

    return worldToCamera;
}

const rmt::Matrix& tCamera::GetCameraToWorldMatrix(void)
{
    if(!updated)
        Update();

    return cameraToWorld;
}

void tCamera::CameraToWorld(const rmt::Vector& cam, rmt::Vector* world)
{
    if(!updated)
        Update();

    cameraToWorld.Transform(cam,world);
}

void tCamera::WorldToCamera(const rmt::Vector& world, rmt::Vector* camera)
{
    if(!updated)
        Update();

    worldToCamera.Transform(world,camera);
}

bool tCamera::CameraToView(const rmt::Vector& cam, rmt::Vector* view)
{
    if(cam.z != 0.0f)
    {
        float ooz = 1.0f / cam.z;
        view->x = cam.x * ooz * fovScaleX * 0.5f;
        view->y = cam.y * ooz * fovScaleY * 0.5f;
        view->z = cam.z;
        return true;
    }
    return false;
}

bool tCamera::WorldToView(const rmt::Vector& world, rmt::Vector* view)
{
    rmt::Vector tmp;
    WorldToCamera(world, &tmp);
    return CameraToView(tmp, view);
}

void tCamera::ViewToCamera(const rmt::Vector& view, rmt::Vector* line1, rmt::Vector* line2)
{
    float ooz_near = 1.0f / nearPlane;
    float ooz_far = 1.0f / farPlane;

    line1->x = view.x / (ooz_near * fovScaleX * 0.5f);
    line1->y = view.y / (ooz_near * fovScaleY * 0.5f);
    line1->z = nearPlane;

    line2->x = view.x / (ooz_far * fovScaleX * 0.5f);
    line2->y = view.y / (ooz_far * fovScaleY * 0.5f);
    line2->z = farPlane;
}

void tCamera::ViewToWorld(const rmt::Vector& view, rmt::Vector* line1, rmt::Vector* line2)
{
    rmt::Vector tmp1, tmp2;
    ViewToCamera(view, &tmp1, &tmp2);
    CameraToWorld(tmp1, line1);
    CameraToWorld(tmp2, line2);
}

bool tCamera::PointVisibleCamera(const rmt::Vector& camera)
{
    return (PointClipCode(camera) == 0);
}

bool tCamera::SphereVisibleCamera(const rmt::Vector& camera, float radius)
{
    if((camera.z + radius) < nearPlane) return false;
    if((camera.z - radius) > farPlane) return false;

    if((camera.x * leftPlaneX + camera.z * leftPlaneZ) > radius) return false;
    if((camera.x * rightPlaneX + camera.z * rightPlaneZ) > radius) return false;

    if((camera.y * upPlaneY + camera.z * upPlaneZ) > radius) return false;
    if((camera.y * downPlaneY + camera.z * downPlaneZ) > radius) return false;

    return true;
}

bool tCamera::PointVisible(const rmt::Vector& world)
{
    rmt::Vector camera;
    WorldToCamera(world,&camera);

    return PointVisibleCamera(camera);
}

bool tCamera::SphereVisible(const rmt::Vector& c, float radius)
{
    rmt::Vector centre;
    WorldToCamera(c,&centre);

    return SphereVisibleCamera(centre, radius);
}

unsigned tCamera::PointClipCode(const rmt::Vector& cameraSpacePoint)
{
    unsigned clip = 0;

    //fovScaleX = rmt::DegToRadian(90.0f) / fov;
    //fovScaleY = rmt::DegToRadian(90.0f) / fov;

    float x = cameraSpacePoint.x * fovScaleX;
    float y = cameraSpacePoint.y * fovScaleY;
    float z = cameraSpacePoint.z;

    if(z > farPlane) clip |= CLIP_FAR;
    else if(z < nearPlane) clip |= CLIP_NEAR;
    
    if((y * aspect) > z) clip |= CLIP_TOP;
    else if((-y * aspect) > z) clip |= CLIP_BOTTOM;

    if(x > z) clip |= CLIP_RIGHT;
    else if(-x > z) clip |= CLIP_LEFT;

    return clip;
}

void tCamera::GetWorldLookAtDirection(rmt::Vector *v)
{
    if(!updated)
        Update();

    *v = cameraToWorld.Row(2);
}

void tCamera::GetWorldPosition(rmt::Vector *v)
{
    if(!updated)
        Update();

    *v = cameraToWorld.Row(3);
}

void tCamera::SetCameraMatrix(rmt::Matrix* CameraMatrix)
{
    cameraToWorld = *CameraMatrix;

    // do the update immediatly, so if this is 
    // a derived class, it's update won't get called
    tCamera::Update();
}

void tCamera::SetState(void)
{
    if(!updated)
        Update();

    float effectiveFov = fov;
    const float refAspect = 16.0f / 9.0f;
    if (aspect > refAspect)
    {
        effectiveFov = 2.0f * rmt::ATan(rmt::Tan(fov * 0.5f) * (aspect / refAspect));
    }

    p3d::pddi->SetCamera(nearPlane,farPlane,effectiveFov,aspect);
}

void tCamera::Update()
{
    worldToCamera = cameraToWorld;
    worldToCamera.InvertOrtho();
}

