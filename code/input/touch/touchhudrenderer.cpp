#include <input/touch/touchhudrenderer.h>
#include <input/touch/touchassetmanager.h>
#include <input/touch/touchhudsystem.h>
#include <input/touch/touchinputmodemanager.h>
#include <input/touch/touchcontextresolver.h>
#include <p3d/sprite.hpp>
#include <p3d/utility.hpp>
#include <pddi/pdditype.hpp>
#include <pddi/pddi.hpp>
#include <math.h>
#include <input/touch/touchcontrolsconfigurationmanager.h>
#if defined(RAD_ANDROID)
#include <android/log.h>

#define TOUCH_HUD_RENDERER_LOG_TAG "SimpsonsHitAndRun"
#define TOUCH_HUD_RENDERER_LOGI(...) __android_log_print(ANDROID_LOG_INFO, TOUCH_HUD_RENDERER_LOG_TAG, __VA_ARGS__)
#define TOUCH_HUD_RENDERER_LOGE(...) __android_log_print(ANDROID_LOG_ERROR, TOUCH_HUD_RENDERER_LOG_TAG, __VA_ARGS__)
#else
#define TOUCH_HUD_RENDERER_LOGI(...)
#define TOUCH_HUD_RENDERER_LOGE(...)
#endif

//=============================================================================
// Singleton
//=============================================================================

TouchHudRenderer& TouchHudRenderer::GetInstance()
{
    static TouchHudRenderer sInstance;
    return sInstance;
}

//=============================================================================
// Construction
//=============================================================================

TouchHudRenderer::TouchHudRenderer()
:
mInitialized( false ),
mEnabled( true ),
mOpacity( 0.25f ),
mPressedOpacity( 0.85f ),
mJoystickVisualScale( 1.25f )
{
}

TouchHudRenderer::~TouchHudRenderer()
{
    Shutdown();
}

//=============================================================================
// Public
//=============================================================================

bool TouchHudRenderer::Initialize()
{
    if ( mInitialized )
    {
        return true;
    }

    mInitialized = true;
    mEnabled = true;


    TouchControlsConfigurationManager& configManager =
    TouchControlsConfigurationManager::GetInstance();

    if ( configManager.IsInitialized() )
    {
        mOpacity = configManager.GetOpacity();
        mPressedOpacity = configManager.GetPressedOpacity();
    }
   

    return true;
}

void TouchHudRenderer::Shutdown()
{
    if ( !mInitialized )
    {
        return;
    }

    mInitialized = false;

}


void TouchHudRenderer::SetJoystickVisualScale( float scale )
{
    if ( scale < 0.5f )
    {
        scale = 0.5f;
    }

    if ( scale > 3.0f )
    {
        scale = 3.0f;
    }

    mJoystickVisualScale = scale;
}

float TouchHudRenderer::GetJoystickVisualScale() const
{
    return mJoystickVisualScale;
}

bool TouchHudRenderer::IsInitialized() const
{
    return mInitialized;
}

void TouchHudRenderer::SetEnabled( bool enabled )
{
    mEnabled = enabled;
}

bool TouchHudRenderer::IsEnabled() const
{
    return mEnabled;
}

void TouchHudRenderer::SetOpacity( float opacity )
{
    mOpacity = ClampOpacity( opacity );
}

float TouchHudRenderer::GetOpacity() const
{
    return mOpacity;
}

void TouchHudRenderer::SetPressedOpacity( float opacity )
{
    mPressedOpacity = ClampOpacity( opacity );
}

float TouchHudRenderer::GetPressedOpacity() const
{
    return mPressedOpacity;
}

void TouchHudRenderer::Render()
{
    if ( !mInitialized || !mEnabled )
    {
        return;
    }

#if defined(RAD_ANDROID)
    TouchAssetManager& assetManager = TouchAssetManager::GetInstance();

    if ( !assetManager.IsInitialized() )
    {
        return;
    }

    TouchHudSystem& hudSystem = TouchHudSystem::GetInstance();

    if ( !hudSystem.IsEnabled() )
    {
        return;
    }

    if ( !TouchInputModeManager::GetInstance().ShouldShowTouchHud() )
    {
        return;
    }

    const TouchProfile currentProfile = hudSystem.GetCurrentProfile();

    if ( currentProfile == TOUCH_PROFILE_HIDDEN )
    {
        return;
    }

    if ( !BeginTouchHud2D() )
    {
        return;
    }

    float renderWidth = 0.0f;
    float renderHeight = 0.0f;

    if ( !GetRenderDimensions( renderWidth, renderHeight ) )
    {
        EndTouchHud2D();
        return;
    }

    RenderMovementJoystick(
    currentProfile,
    renderWidth,
    renderHeight
    );

    RenderVisibleControls(
    currentProfile,
    renderWidth,
    renderHeight
    );  

    EndTouchHud2D();
#endif
}

bool TouchHudRenderer::BeginTouchHud2D()
{
#if !defined(RAD_ANDROID)
    return false;
#else
    if ( p3d::pddi == 0 )
    {
        return false;
    }

    p3d::pddi->PushState(
        static_cast<pddiStateMask>(
            PDDI_STATE_RENDER |
            PDDI_STATE_VIEW |
            PDDI_STATE_FOG |
            PDDI_STATE_STENCIL
        )
    );

    p3d::pddi->SetProjectionMode( PDDI_PROJECTION_DEVICE );

    p3d::pddi->PushIdentityMatrix( PDDI_MATRIX_MODELVIEW );

    p3d::pddi->SetColourWrite( true, true, true, true );
    p3d::pddi->EnableZBuffer( false );
    p3d::pddi->SetZWrite( false );
    p3d::pddi->SetCullMode( PDDI_CULL_NONE );
    p3d::pddi->EnableFog( false );
    p3d::pddi->EnableStencilBuffer( false );

    return true;
#endif
}

void TouchHudRenderer::EndTouchHud2D()
{
#if defined(RAD_ANDROID)
    if ( p3d::pddi == 0 )
    {
        return;
    }

    p3d::pddi->PopMatrix( PDDI_MATRIX_MODELVIEW );

    p3d::pddi->PopState(
        static_cast<pddiStateMask>(
            PDDI_STATE_RENDER |
            PDDI_STATE_VIEW |
            PDDI_STATE_FOG |
            PDDI_STATE_STENCIL
        )
    );
#endif
}

bool TouchHudRenderer::GetRenderDimensions( float& width, float& height ) const
{
#if !defined(RAD_ANDROID)
    width = 0.0f;
    height = 0.0f;
    return false;
#else
    width = 0.0f;
    height = 0.0f;

    if ( p3d::pddi == 0 )
    {
        return false;
    }

    /*
     * Conservative approach:
     * use the current PDDI scissor as the active render area.
     * In the current FrontEndRenderLayer hook this should match the render frame.
     */
    pddiRect scissor;
    p3d::pddi->GetScissor( &scissor );

    width = static_cast<float>( scissor.right - scissor.left );
    height = static_cast<float>( scissor.bottom - scissor.top );

    if ( width <= 0.0f || height <= 0.0f )
    {
        return false;
    }

    return true;
#endif
}

void TouchHudRenderer::RenderSpriteInPixelRect
(
    tSprite* sprite,
    float rectX,
    float rectY,
    float rectW,
    float rectH,
    float opacity
)
{
#if defined(RAD_ANDROID)
    if ( sprite == 0 )
    {
        return;
    }

    if ( rectW <= 0.0f || rectH <= 0.0f )
    {
        return;
    }

    int alpha = static_cast<int>( 255.0f * ClampOpacity( opacity ) );

    if ( alpha < 0 )
    {
        alpha = 0;
    }
    else if ( alpha > 255 )
    {
        alpha = 255;
    }

    sprite->SetColour(
        tColour(
            255,
            255,
            255,
            alpha
        )
    );

    float drawX = rectX;
    float drawY = rectY;
    float drawW = rectW;
    float drawH = rectH;

    ComputeAspectFitRect(
        sprite,
        rectX,
        rectY,
        rectW,
        rectH,
        drawX,
        drawY,
        drawW,
        drawH
    );

    sprite->DisplayAt(
        drawX,
        drawY,
        drawW,
        drawH
    );
#else
    (void)sprite;
    (void)rectX;
    (void)rectY;
    (void)rectW;
    (void)rectH;
    (void)opacity;
#endif
}


void TouchHudRenderer::RenderControlDefinition
(
    const TouchHudControlDefinition& control,
    TouchProfile currentProfile,
    float renderWidth,
    float renderHeight
)
{
#if defined(RAD_ANDROID)
    if ( !control.enabled || !control.visibleByDefault )
    {
        return;
    }

    TouchHudSystem& hudSystem = TouchHudSystem::GetInstance();

    if ( !hudSystem.ShouldControlBeUsedForProfile( control.id, currentProfile ) )
    {
        return;
    }

    if ( control.rect.width <= 0.0f || control.rect.height <= 0.0f )
    {
        return;
    }

    TouchAssetManager& assetManager = TouchAssetManager::GetInstance();

    tSprite* sprite = 0;

    if ( control.id == TOUCH_HUD_CONTROL_EDITOR_NEXT_LAYOUT )
    {
        sprite = assetManager.GetSpriteForEditorNextLayout(
            hudSystem.GetCurrentEditableLayout()
        );
    }
    else if ( control.id == TOUCH_HUD_CONTROL_CHARACTER_CONTEXT_ACTION )
    {
        sprite = assetManager.GetSpriteForInteractionIcon(
            hudSystem.GetCurrentInteractionIcon()
        );
    }
    else
    {
        sprite = assetManager.GetSpriteForControl( control.id );
    }

    if ( sprite == 0 )
    {
        return;
    }


    if ( !hudSystem.IsControlVisible( control.id ) )
    {
        return;
    }
    
    const bool pressed = hudSystem.IsControlPressed( control.id );
    float opacity = pressed ? mPressedOpacity : mOpacity;

    if ( control.id == TOUCH_HUD_CONTROL_EDITOR_ENTER_IN_GAME || control.id == TOUCH_HUD_CONTROL_EDITOR_ENTER_MAIN_MENU || control.id == TOUCH_HUD_CONTROL_EDITOR_ENTER_MINIGAME )
    {
        opacity = pressed ? 0.5f : 0.85f;
    }
    else if ( hudSystem.IsTouchControlsEditModeActive() )
    {
        opacity = 1.0f;
    }

    const TouchRect effectiveRect = hudSystem.GetEffectiveControlRect( control.id );

    const float rectX = effectiveRect.x * renderWidth;
    const float rectY = effectiveRect.y * renderHeight;
    const float rectW = effectiveRect.width * renderWidth;
    const float rectH = effectiveRect.height * renderHeight;

    RenderSpriteInPixelRect(
        sprite,
        rectX,
        rectY,
        rectW,
        rectH,
        opacity
    );
#else
    (void)control;
    (void)currentProfile;
    (void)renderWidth;
    (void)renderHeight;
#endif
}

void TouchHudRenderer::RenderMovementJoystick
(
    TouchProfile currentProfile,
    float renderWidth,
    float renderHeight
)
{
#if defined(RAD_ANDROID)
    if ( currentProfile != TOUCH_PROFILE_CHARACTER )
    {
        return;
    }

    TouchHudSystem& hudSystem = TouchHudSystem::GetInstance();

    const TouchHudMovementState& movement =
        hudSystem.GetMovementState();

    if ( !movement.active )
    {
        return;
    }

    if ( movement.radius <= 0.0f )
    {
        return;
    }

    TouchAssetManager& assetManager = TouchAssetManager::GetInstance();

    tSprite* baseSprite =
        assetManager.GetSprite( TOUCH_ASSET_CHARACTER_JOYSTICK_BASE );

    tSprite* knobSprite =
        assetManager.GetSprite( TOUCH_ASSET_CHARACTER_JOYSTICK_KNOB );

    if ( baseSprite == 0 || knobSprite == 0 )
    {
        return;
    }

    const float originX = movement.origin.x * renderWidth;
    const float originY = movement.origin.y * renderHeight;

    const float currentX = movement.current.x * renderWidth;
    const float currentY = movement.current.y * renderHeight;


    const float joystickVisualScale = 1.25f;
    const float outerRadius = movement.radius * renderHeight * mJoystickVisualScale;

    // Tamaño visual total del sprite del knob.
    const float innerRadius = outerRadius * 0.8f;

    /*
    * Si el PNG del knob tiene transparencia alrededor, la parte visible real
    * es menor que el rectángulo completo.
    *
    * 1.00f = el PNG visible ocupa todo el rectángulo.
    * 0.85f = asumimos que hay un 15% de padding/transparencia.
    * 0.75f = el knob se acercará aún más al borde.
    * Valores mas bajos se acercan 
    * Acercamiento knoib
    */
    const float knobVisibleRadiusFactor = 0.55f;

    const float effectiveKnobRadius = innerRadius * knobVisibleRadiusFactor;

    float maxKnobDistance = outerRadius - effectiveKnobRadius;

    if ( maxKnobDistance < 0.0f )
    {
        maxKnobDistance = 0.0f;
    }
    
    if ( maxKnobDistance < 0.0f )
    {
        maxKnobDistance = 0.0f;
    }

    float dx = currentX - originX;
    float dy = currentY - originY;

    const float distanceSq = ( dx * dx ) + ( dy * dy );
    const float maxDistanceSq = maxKnobDistance * maxKnobDistance;

    if ( distanceSq > maxDistanceSq && maxKnobDistance > 0.0f )
    {
        const float distance = sqrtf( distanceSq );

        if ( distance > 0.0f )
        {
            const float scale = maxKnobDistance / distance;

            dx *= scale;
            dy *= scale;
        }
    }

    const float knobCenterX = originX + dx;
    const float knobCenterY = originY + dy;

    const float outerDiameter = outerRadius * 2.0f;
    const float innerDiameter = innerRadius * 2.0f;

    /*
     * Base: normal opacity.
     * Knob: pressed opacity because it is only visible while actively dragging.
     */
    RenderSpriteInPixelRect(
        baseSprite,
        originX - outerRadius,
        originY - outerRadius,
        outerDiameter,
        outerDiameter,
        mOpacity
    );

    RenderSpriteInPixelRect(
        knobSprite,
        knobCenterX - innerRadius,
        knobCenterY - innerRadius,
        innerDiameter,
        innerDiameter,
        mPressedOpacity
    );
#else
    (void)currentProfile;
    (void)renderWidth;
    (void)renderHeight;
#endif
}

void TouchHudRenderer::RenderVisibleControls(TouchProfile currentProfile,float renderWidth,float renderHeight)
{
#if defined(RAD_ANDROID)
    TouchHudSystem& hudSystem = TouchHudSystem::GetInstance();

    const unsigned int controlCount = hudSystem.GetControlCount();

    for ( unsigned int i = 0; i < controlCount; ++i )
    {
        const TouchHudControlDefinition* control =
            hudSystem.GetControlByIndex( i );

        if ( control == 0 )
        {
            continue;
        }

        RenderControlDefinition(
            *control,
            currentProfile,
            renderWidth,
            renderHeight
        );
    }
#else
    (void)currentProfile;
    (void)renderWidth;
    (void)renderHeight;
#endif
}

void TouchHudRenderer::ComputeAspectFitRect
(
    tSprite* sprite,
    float sourceX,
    float sourceY,
    float sourceWidth,
    float sourceHeight,
    float& drawX,
    float& drawY,
    float& drawWidth,
    float& drawHeight
) const
{
    drawX = sourceX;
    drawY = sourceY;
    drawWidth = sourceWidth;
    drawHeight = sourceHeight;

    if ( sprite == 0 )
    {
        return;
    }

    if ( sourceWidth <= 0.0f || sourceHeight <= 0.0f )
    {
        return;
    }

    const int spriteWidth = sprite->GetWidth();
    const int spriteHeight = sprite->GetHeight();

    if ( spriteWidth <= 0 || spriteHeight <= 0 )
    {
        return;
    }

    const float spriteAspect =
        static_cast<float>( spriteWidth ) /
        static_cast<float>( spriteHeight );

    const float sourceAspect = sourceWidth / sourceHeight;

    if ( sourceAspect > spriteAspect )
    {
        /*
         * The available rect is wider than the image.
         * Fit by height and center horizontally.
         */
        drawHeight = sourceHeight;
        drawWidth = drawHeight * spriteAspect;

        drawX = sourceX + ( sourceWidth - drawWidth ) * 0.5f;
        drawY = sourceY;
    }
    else
    {
        /*
         * The available rect is taller than the image.
         * Fit by width and center vertically.
         */
        drawWidth = sourceWidth;
        drawHeight = drawWidth / spriteAspect;

        drawX = sourceX;
        drawY = sourceY + ( sourceHeight - drawHeight ) * 0.5f;
    }
}

void TouchHudRenderer::RenderCinematicSkip()
{
#if defined(RAD_ANDROID)
    /*
     * Only render this overlay when touch HUD is allowed.
     * If the player is using a physical gamepad, do not draw touch UI.
     */
    if ( !TouchInputModeManager::GetInstance().ShouldShowTouchHud() )
    {
        return;
    }

    /*
     * This render path is exclusive for cinematics.
     * Normal character/vehicle/frontend HUD is rendered from FrontEndRenderLayer.
     */
    if ( TouchContextResolver::GetInstance().Resolve() != TOUCH_PROFILE_CINEMATIC )
    {
        return;
    }

    tSprite* sprite = TouchAssetManager::GetInstance().GetSprite(
        TOUCH_ASSET_FRONTEND_RIGHT
    );

    if ( sprite == 0 )
    {
        return;
    }

    float screenWidth = 0.0f;
    float screenHeight = 0.0f;

    if ( !GetRenderDimensions( screenWidth, screenHeight ) )
    {
        return;
    }

    /*
     * Same normalized area as the cinematic skip touch control.
     */
    const float normalizedX = 0.90f;
    const float normalizedY = 0.02f;
    const float normalizedW = 0.09f;
    const float normalizedH = 0.18f;

    float x = normalizedX * screenWidth;
    float y = normalizedY * screenHeight;
    float w = normalizedW * screenWidth;
    float h = normalizedH * screenHeight;

    const bool pressed =
    TouchHudSystem::GetInstance().IsControlPressed(TOUCH_HUD_CONTROL_CINEMATIC_SKIP);

    const float opacity = pressed ? mPressedOpacity : mOpacity;
    
    BeginTouchHud2D();

    RenderSpriteInPixelRect(
        sprite,
        x,
        y,
        w,
        h,
        opacity
    );


    EndTouchHud2D();
#endif
}
//=============================================================================
// Private
//=============================================================================

float TouchHudRenderer::ClampOpacity( float value ) const
{
    if ( value < 0.0f )
    {
        return 0.0f;
    }

    if ( value > 1.0f )
    {
        return 1.0f;
    }

    return value;
}