//=============================================================================
// Copyright (c) 2002 Radical Games Ltd.  All rights reserved.
//=============================================================================

#include <pddi/gles/gl.hpp>
#include <pddi/gles/glcon.hpp>
#include <pddi/gles/gldev.hpp>
#include <pddi/gles/gldisplay.hpp>
#include <pddi/gles/gltex.hpp>
#include <pddi/gles/glmat.hpp>
#include <pddi/gles/glprog.hpp>

#include <pddi/base/debug.hpp>
#include <math.h>
#include <string.h>
#include <SDL.h>
#include <vector>

#include <microprofile.h>

// vertex arrays rendering
GLenum primTypeTable[5] =
{
    GL_TRIANGLES, //PDDI_PRIM_TRIANGLES
    GL_TRIANGLE_STRIP, //PDDI_PRIM_TRISTRIP
    GL_LINES, //PDDI_PRIM_LINES
    GL_LINE_STRIP, // PDDI_PRIM_LINESTRIP
    GL_POINTS, //PDDI_PRIM_POINTS
};

static inline void FillGLColour(pddiColour c, float* f)
{
    f[0] = float(c.Red()) / 255;
    f[1] = float(c.Green()) / 255;
    f[2] = float(c.Blue()) / 255;
    f[3] = float(c.Alpha()) / 255;
}

// extensions
class pglExtContext : public pddiExtGLContext 
{
public:
    pglExtContext(pglDisplay* d) : display(d) {}

    void BeginContext()
    {
        display->BeginContext();
    }

    void EndContext()
    {
        display->EndContext();
    }

private:
    pglDisplay* display;
};

class pglExtGamma : public pddiExtGammaControl
{
public:
    pglExtGamma(pglDisplay* d) { display = d;}

    void SetGamma(float r, float g, float b)     {display->SetGamma(r,g,b);}
    void GetGamma(float *r, float *g, float *b)  {display->GetGamma(r,g,b);}

protected:
    pglDisplay* display;
};

pglContext::pglContext(pglDevice* dev, pglDisplay* disp) : pddiBaseContext((pddiDisplay*)disp,(pddiDevice*)dev)
{
    device = dev;
    display = disp;
    currentProgram = nullptr;

    device->AddRef();
    display->AddRef();
    disp->SetContext(this);

    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTexSize);
    DefaultState();
    contextID = 0;

    extContext = new pglExtContext(display);
    extGamma = new pglExtGamma(display);

#ifdef RAD_CG
    CGprogram vertexShader = pglProgram::CompileShader(GL_VERTEX_SHADER,
        "typedef struct {\n"
        "    int enabled;\n"
        "    float4 position;\n"
        "    float4 colour;\n"
        "    float3 attenuation;\n"
        "} LightParams;\n"

        "float3 direction(float4 p1, float4 p2) { return normalize(p2.xyz * sign(p1.w) - p1.xyz * sign(p2.w)); }\n"
        "float power(float x, float y) { return y != 0.0 ? pow(x,y) : 1.0; }\n"
        "float product(float3 x, float3 y) { return max(dot(x,y), 0.0); }\n"

        "void main(float3 position      : ATTR0,\n"
        "          float3 normal        : ATTR1,\n"
        "          float2 texcoord      : ATTR2,\n"
        "          float4 color         : ATTR3,\n"
        "          out float4 oPosition : POSITION,\n"
        "          out float2 tc        : TEXCOORD0,\n"
        "          out float4 cpri      : COLOR0,\n"
        "          out float4 csec      : COLOR1,\n"
        "          uniform float4x4 projection,\n"
        "          uniform float4x4 modelview,\n"
        "          uniform float4x4 normalmatrix,\n"
        "          uniform float4 acs,\n"
        "          uniform float4 acm,\n"
        "          uniform float4 dcm,\n"
        "          uniform float4 scm,\n"
        "          uniform float4 ecm,\n"
        "          uniform float srm,\n"
        "          uniform LightParams lights[" PDDI_STRINGIZE( PDDI_MAX_LIGHTS ) "]) {\n"
        "    float4 V = mul(modelview, float4(position, 1.0));\n"
        "    float3 n = normalize(mul(float3x3(normalmatrix), normal));\n"

        "    float3 diff = ecm.rgb + acm.rgb * acs.rgb;\n"
        "    float3 spec = float3(0.0);\n"
        "    for (int i = 0; i < " PDDI_STRINGIZE(PDDI_MAX_LIGHTS) "; i++) {\n"
        "        if (lights[i].enabled == 0) continue;\n"

        "        float3 VP = direction(V, lights[i].position);\n"
        "        float f = product(n,VP) != 0.0 ? 1.0 : 0.0;\n"
        "        float3 h = normalize(VP + float3(0.0, 0.0, 1.0));\n"

        "        float3 k = lights[i].attenuation;\n"
        "        float d = distance(V.xyz, lights[i].position.xyz);\n"
        "        float att = lights[i].position.w != 0.0 ? 1.0 / (k[0] + k[1] * d + k[2] * d * d) : 1.0;\n"

        "        diff += att * product(n,VP) * dcm.rgb * lights[i].colour.rgb;\n"
        "        spec += att * f * power(product(n,h),srm) * scm.rgb * lights[i].colour.rgb;\n"
        "    }\n"

        "    tc = texcoord;\n"
        "    cpri = color * float4(diff, dcm.a);\n"
        "    csec = float4(spec, 0.0);\n"
        "    oPosition = mul(projection, V);\n"
        "}\n"
    );

    CGprogram fragmentShader = pglProgram::CompileShader( GL_FRAGMENT_SHADER,
        "void main(float2 tc        : TEXCOORD0,\n"
        "          float4 cpri      : COLOR0,\n"
        "          float4 csec      : COLOR1,\n"
        "          out float4 color : COLOR) {\n"
        "    color = cpri + csec;\n"
        "}\n"
    );

    CGprogram textureShader = pglProgram::CompileShader(GL_FRAGMENT_SHADER,
        "void main(float2 tc : TEXCOORD0,\n"
        "          float4 cpri : COLOR0,\n"
        "          float4 csec : COLOR1,\n"
        "          out float4 color : COLOR,\n"
        "          uniform sampler2D tex) {\n"
        "    color = tex2D(tex, tc) * cpri + csec;\n"
        "}\n"
    );

    CGprogram alphaTestShader = pglProgram::CompileShader(GL_FRAGMENT_SHADER,
        "void main(float2 tc : TEXCOORD0,\n"
        "          float4 cpri : COLOR0,\n"
        "          float4 csec : COLOR1,\n"
        "          out float4 color : COLOR,\n"
        "          uniform float alpharef,\n"
        "          uniform sampler2D tex) {\n"
        "    float4 c = tex2D(tex, tc) * cpri + csec;\n"
        "    if (c.a < alpharef) discard;\n"
        "    color = c;\n"
        "}\n"
    );
#else
    GLuint vertexShader = pglProgram::CompileShader(GL_VERTEX_SHADER,
        "precision highp float;\n"

        "attribute vec3 position;\n"
        "attribute vec3 normal;\n"
        "attribute vec2 texcoord;\n"
        "attribute vec4 color;\n"

        "uniform mat4 projection;\n"
        "uniform mat4 modelview;\n"
        "uniform mat4 normalmatrix;\n"

        // Lights
        "uniform struct LightParams {\n"
        "    int enabled;\n"
        "    vec4 position;\n"
        "    vec4 colour;\n"
        "    vec3 attenuation;\n"
        "} lights[" PDDI_STRINGIZE(PDDI_MAX_LIGHTS) "];\n"

        // Scene
        "uniform vec4 acs;\n"

        // Material
        "uniform vec4 acm;\n"
        "uniform vec4 dcm;\n"
        "uniform vec4 scm;\n"
        "uniform vec4 ecm;\n"
        "uniform float srm;\n"

        "varying vec2 tc;\n"
        "varying vec4 cpri;\n"
        "varying vec4 csec;\n"

        "vec3 direction(vec4 p1, vec4 p2) { return normalize(p2.xyz * sign(p1.w) - p1.xyz * sign(p2.w)); }\n"
        "float power(float x, float y) { return y != 0.0 ? pow(x,y) : 1.0; }\n"
        "float product(vec3 x, vec3 y) { return max(dot(x,y), 0.0); }\n"

        "void main() {\n"
        "    vec4 V = modelview * vec4(position, 1.0);\n"
        "    vec3 n = normalize(mat3(normalmatrix) * normal);\n"

        "    vec3 diff = ecm.rgb + acm.rgb * acs.rgb;\n"
        "    vec3 spec = vec3(0.0);\n"
        "    for (int i = 0; i < " PDDI_STRINGIZE(PDDI_MAX_LIGHTS) "; i++) {\n"
        "        if (lights[i].enabled == 0) continue;\n"

        "        vec3 VP = direction(V, lights[i].position);\n"
        "        float f = product(n,VP) != 0.0 ? 1.0 : 0.0;\n"
        "        vec3 h = normalize(VP + vec3(0.0, 0.0, 1.0));\n"

        "        vec3 k = lights[i].attenuation;\n"
        "        float d = distance(V.xyz, lights[i].position.xyz);\n"
        "        float att = lights[i].position.w != 0.0 ? 1.0 / (k[0] + k[1] * d + k[2] * d * d) : 1.0;\n"

        "        diff += att * product(n,VP) * dcm.rgb * lights[i].colour.rgb;\n"
        "        spec += att * f * power(product(n,h),srm) * scm.rgb * lights[i].colour.rgb;\n"
        "    }\n"

        "    tc = texcoord;\n"
        "    cpri = color * vec4(diff, dcm.a);\n"
        "    csec = vec4(spec, 0.0);\n"
        "    gl_Position = projection * V;\n"
        "}\n"
    );

    GLuint fragmentShader = pglProgram::CompileShader( GL_FRAGMENT_SHADER,
    "precision mediump float;\n"
    "varying vec2 tc;\n"
    "varying vec4 cpri;\n"
    "varying vec4 csec;\n"
    // uniform gamma: recibe el exponente de corrección por canal (1/r, 1/g, 1/b)
    "uniform vec3 gamma;\n"

    "void main() {\n"
    "    vec4 c = cpri + csec;\n"
    // aplico pow() canal por canal para corregir el gamma antes de mostrar el píxel
    "    gl_FragColor = vec4(pow(c.rgb, gamma), c.a);\n"
    "}\n"
);

    GLuint textureShader = pglProgram::CompileShader(GL_FRAGMENT_SHADER,
    "precision mediump float;\n"
    "varying vec2 tc;\n"
    "varying vec4 cpri;\n"
    "varying vec4 csec;\n"

    "uniform sampler2D tex;\n"
    // mismo uniform gamma que en el fragmentShader
    "uniform vec3 gamma;\n"

    "void main() {\n"
    "    vec4 c = texture2D(tex, tc) * cpri + csec;\n"
    // aplico la corrección gamma al color final con textura
    "    gl_FragColor = vec4(pow(c.rgb, gamma), c.a);\n"
    "}\n"
);

    GLuint alphaTestShader = pglProgram::CompileShader(GL_FRAGMENT_SHADER,
    "precision mediump float;\n"
    "varying vec2 tc;\n"
    "varying vec4 cpri;\n"
    "varying vec4 csec;\n"

    "uniform float alpharef;\n"
    "uniform sampler2D tex;\n"
    // mismo uniform gamma
    "uniform vec3 gamma;\n"

    "void main() {\n"
    "    vec4 c = texture2D(tex, tc) * cpri + csec;\n"
    // el alpha test se hace antes de aplicar gamma porque el alpha no se corrige
    "    if (c.a < alpharef) discard;\n"
    "    gl_FragColor = vec4(pow(c.rgb, gamma), c.a);\n"
    "}\n"
);
#endif

    colorProgram = pglProgram::CreateProgram(vertexShader, fragmentShader);

    textureProgram = pglProgram::CreateProgram(vertexShader, textureShader);

    alphaTestProgram = pglProgram::CreateProgram(vertexShader, alphaTestShader);

    // Don't leak shaders
#ifdef RAD_CG
    cgDestroyProgram(vertexShader);
    cgDestroyProgram(fragmentShader);
    cgDestroyProgram(textureShader);
    cgDestroyProgram(alphaTestShader);
#else
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    glDeleteShader(textureShader);
    glDeleteShader(alphaTestShader);
#endif

    defaultShader = new pglMat(this);
    defaultShader->AddRef();
    SetShaderProgram(colorProgram);

    // gamma a 1.0 en los 3 shaders (sin corrección)
    // se actualizará cuando el juego llame a SetGamma
    colorProgram->SetGamma(1.0f, 1.0f, 1.0f);
    textureProgram->SetGamma(1.0f, 1.0f, 1.0f);
    alphaTestProgram->SetGamma(1.0f, 1.0f, 1.0f);
}

pglContext::~pglContext()
{
    defaultShader->Release();
    currentProgram->Release();
    colorProgram->Release();
    textureProgram->Release();
    alphaTestProgram->Release();

    delete extContext;
    delete extGamma;

    display->SetContext(NULL);
    display->Release();
    device->Release();
}

// frame synchronisation
void pglContext::BeginFrame()
{
    pddiBaseContext::BeginFrame();

    SDL_GL_SetSwapInterval(display->GetForceVSync() ? 1 : 0);

    if(display->HasReset())
    {
        contextID++;

        glEnable(GL_CULL_FACE);
        glCullFace(GL_FRONT);
#ifndef RAD_VITAGL
        glEnable(GL_DITHER);
#endif

        SyncState(0xffffffff);
    }

    projection.Identity();
}

void pglContext::EndFrame()
{
    pddiBaseContext::EndFrame();
}

// buffer clearing
void pglContext::Clear(unsigned bufferMask)
{
    pddiBaseContext::Clear(bufferMask);

    int myClearMask = 0;
    myClearMask |= (bufferMask & PDDI_BUFFER_COLOUR) ? GL_COLOR_BUFFER_BIT : 0;
    myClearMask |= (bufferMask & PDDI_BUFFER_DEPTH) ? GL_DEPTH_BUFFER_BIT : 0;
    //myClearMask |= (bufferMask & PDDI_BUFFER_STENCIL) ? GL_STENCIL_BUFFER_BIT : 0;

    glClearDepthf(state.viewState->clearDepth);
    glClearColor(float(state.viewState->clearColour.Red())/255.0f, 
                     float(state.viewState->clearColour.Green())/255.0f, 
                     float(state.viewState->clearColour.Blue())/255.0f,
                     float(state.viewState->clearColour.Alpha())/255.0f);
    glClearStencil(state.viewState->clearStencil);
    glClear(myClearMask);
}

void pglContext::SetupHardwareProjection(void)
{
    switch(state.viewState->projectionMode)
    {
        case PDDI_PROJECTION_DEVICE :
            projection.Identity();
            projection.SetOrthographic(0, display->GetWidth(),
                      display->GetHeight(), 0,
                      (state.viewState->camera.nearPlane),(state.viewState->camera.farPlane));
            glViewport(0, 0, display->GetWidth(), display->GetHeight());
            break;

        case PDDI_PROJECTION_ORTHOGRAPHIC :
            projection.Identity();
            projection.SetOrthographic(-0.5,  0.5,
                      -((1/state.viewState->camera.aspect)/2),  ((1/state.viewState->camera.aspect)/2),
                      (state.viewState->camera.nearPlane),(state.viewState->camera.farPlane));
            glViewport(int(state.viewState->viewWindow.left * display->GetWidth()), 
                              int((1.0f - state.viewState->viewWindow.bottom) * display->GetHeight() ),
                              int((state.viewState->viewWindow.right - state.viewState->viewWindow.left) * display->GetWidth()), 
                              int((state.viewState->viewWindow.bottom - state.viewState->viewWindow.top) * display->GetHeight()));
            break;

        case PDDI_PROJECTION_PERSPECTIVE :
            projection.Identity();
            projection.SetPerspective(state.viewState->camera.fov,state.viewState->camera.aspect,state.viewState->camera.nearPlane,state.viewState->camera.farPlane);
            glViewport(int(state.viewState->viewWindow.left * display->GetWidth()), 
                            int((1.0f - state.viewState->viewWindow.bottom) * display->GetHeight() ),
                            int((state.viewState->viewWindow.right - state.viewState->viewWindow.left) * display->GetWidth()), 
                            int((state.viewState->viewWindow.bottom - state.viewState->viewWindow.top) * display->GetHeight()));
            break;
        default:
            PDDIASSERTMSG(0, "Bad projection mode","");
            break;
    }

    if(currentProgram)
        currentProgram->SetProjectionMatrix(&projection);
}

void pglContext::LoadHardwareMatrix(pddiMatrixType id)
{
    switch(id)
    {
        case PDDI_MATRIX_MODELVIEW :
        {
            if(currentProgram)
                currentProgram->SetModelViewMatrix(state.matrixStack[id]->Top());
        }
        break;
        default :
            PDDIASSERTMSG(0, "Invalid matrix load","");
            break;
    }
}

// viewport clipping
void pglContext::SetScissor(pddiRect* rect)
{
    pddiBaseContext::SetScissor(rect);
    if(!rect)
    {
        glDisable(GL_SCISSOR_TEST);
    }
    else
    {
        glScissor(rect->left, display->GetHeight() - rect->bottom, rect->right - rect->left, rect->bottom - rect->top);
        glEnable(GL_SCISSOR_TEST);
    }
}

#include <vector>
class pglPrimStream : public pddiPrimStream
{
public:
    std::vector<pddiVector> coords;
    std::vector<pddiVector> normals;
    std::vector<GLubyte> colours;
    std::vector<pddiVector2> uvs;

    GLenum primitive;
    unsigned vertexType;

    void Coord(float x, float y, float z)  
    {
        coords.push_back( pddiVector{ x, y, z } );
    }

    void Normal(float x, float y, float z) 
    {
        normals.push_back( pddiVector{ x, y, z } );
    }

    void Colour(pddiColour colour, int channel = 0)
    {
        colours.push_back( colour.Red() );
        colours.push_back( colour.Green() );
        colours.push_back( colour.Blue() );
        colours.push_back( colour.Alpha() );
    }

    void UV(float u, float v, int channel = 0) 
    { 
        if(channel == 0)
        {
            uvs.push_back( pddiVector2{ u, v } );
        }
    }

    void Specular(pddiColour colour) 
    {
        //
    }

    void Vertex(pddiVector* v, pddiColour c) 
    {
        colours.push_back( c.Red() );
        colours.push_back( c.Green() );
        colours.push_back( c.Blue() );
        colours.push_back( c.Alpha() );
        coords.push_back( *v );
    }

    void Vertex(pddiVector* v, pddiVector* n)
    {
        normals.push_back( *n );
        coords.push_back( *v );
    }

    void Vertex(pddiVector* v, pddiVector2* uv)
    {
        uvs.push_back( *uv );
        coords.push_back( *v );
    }

    void Vertex(pddiVector* v, pddiColour c, pddiVector2* uv)
    {
        colours.push_back( c.Red() );
        colours.push_back( c.Green() );
        colours.push_back( c.Blue() );
        colours.push_back( c.Alpha() );
        uvs.push_back( *uv );
        coords.push_back( *v );
    }

    void Vertex(pddiVector* v, pddiVector* n, pddiVector2* uv)
    {
        normals.push_back( *n );
        uvs.push_back( *uv );
        coords.push_back( *v );
    }

} thePrimStream;

pddiPrimStream* pglContext::BeginPrims(pddiShader* mat, pddiPrimType primType, unsigned vertexType, int vertexCount, unsigned pass)
{
    if(!mat)
        mat = defaultShader;

    pddiBaseContext::BeginPrims(mat, primType, vertexType, vertexCount);
    pddiBaseShader* material = (pddiBaseShader*)mat;
    ADD_STAT( PDDI_STAT_MATERIAL_OPS, !material->IsCurrent() );
    material->SetMaterial();
    thePrimStream.primitive = primTypeTable[primType];
    thePrimStream.vertexType = vertexType;
    return &thePrimStream;
}

void pglContext::EndPrims(pddiPrimStream* stream)
{
    MICROPROFILE_SCOPEI("SRR2", "pglContext::EndPrims", MP_RED);

    pddiBaseContext::EndPrims(stream);
    pglPrimStream* glstream = (pglPrimStream*)stream;

    glBindVertexArrayOES( 0 );
    glBindBuffer( GL_ARRAY_BUFFER, 0 );
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
    glEnableVertexAttribArray( 0 );
    glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 0, glstream->coords.data() );

    if( !glstream->normals.empty() )
    {
        glEnableVertexAttribArray( 1 );
        glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, 0, glstream->normals.data() );
    }
    else
    {
        glDisableVertexAttribArray( 1 );
        glVertexAttrib3f( 1, 0.0f, 0.0f, 0.0f );
    }

    if( !glstream->uvs.empty() )
    {
        glEnableVertexAttribArray( 2 );
        glVertexAttribPointer( 2, 2, GL_FLOAT, GL_FALSE, 0, glstream->uvs.data() );
    }
    else
    {
        glDisableVertexAttribArray( 2 );
        glVertexAttrib2f( 2, 0.0f, 0.0f );
    }

    if( !glstream->colours.empty() )
    {
        glEnableVertexAttribArray( 3 );
        glVertexAttribPointer( 3, 4, GL_UNSIGNED_BYTE, GL_TRUE, 0, glstream->colours.data() );
    }
    else
    {
        glDisableVertexAttribArray( 3 );
        glVertexAttrib4f( 3, 1.0f, 1.0f, 1.0f, 1.0f );
    }

    glDrawArrays( glstream->primitive, 0, glstream->coords.size() );

    glstream->coords.clear();
    glstream->normals.clear();
    glstream->colours.clear();
    glstream->uvs.clear();
}

class pglPrimBufferStream : public pddiPrimBufferStream
{
public:
    pglPrimBuffer* buffer;

    pglPrimBufferStream(pglPrimBuffer* b)
    {
        buffer = b;
    }

    void Next(void)  
    {
        if(buffer->coord)
            buffer->coord = (float*)((char*)buffer->coord + buffer->stride);

        if(buffer->normal)
            buffer->normal = (float*)((char*)buffer->normal + buffer->stride);

        if(buffer->uv)
            buffer->uv = (float*)((char*)buffer->uv + buffer->stride);

        if(buffer->colour)
            buffer->colour += buffer->stride;

        buffer->total++;
        PDDIASSERT(buffer->total <= buffer->allocated);
    }

    void Position(float x, float y, float z)  
    { 
        buffer->coord[0] = x;
        buffer->coord[1] = y;
        buffer->coord[2] = z;
        Next();
    }

    void Normal(float x, float y, float z) 
    { 
        buffer->normal[0] = x;
        buffer->normal[1] = y;
        buffer->normal[2] = z;
    }

    void Colour(pddiColour colour, int channel = 0)         
    {
        // HBW: Multiple CBVs not yet implemented.  For now just ignore channel.
        buffer->colour[0] = colour.Red();
        buffer->colour[1] = colour.Green();
        buffer->colour[2] = colour.Blue();
        buffer->colour[3] = colour.Alpha();
    }

    void TexCoord1(float u, int channel = 0) {}

    void TexCoord2(float u, float v, int channel = 0) 
    { 
        if(channel == 0)
        {
            buffer->uv[0] = u;
            buffer->uv[1] = v;
        }
    }

    void TexCoord3(float u, float v, float s, int channel = 0) {}
    void TexCoord4(float u, float v, float s, float t, int channel = 0) {}

    void Specular(pddiColour colour) 
    {
        //
    }

    void SkinIndices(unsigned, unsigned, unsigned, unsigned)
    {
    }

    void SkinWeights(float, float, float)
    {
    }

    void Vertex(pddiVector* v, pddiColour c) 
    {
        buffer->colour[0] = c.Red();
        buffer->colour[1] = c.Green();
        buffer->colour[2] = c.Blue();
        buffer->colour[3] = c.Alpha();
        buffer->coord[0] = v->x;
        buffer->coord[1] = v->y;
        buffer->coord[2] = v->z;
        Next();
    }

    void Vertex(pddiVector* v, pddiVector* n)
    {
        buffer->normal[0] = n->x;
        buffer->normal[1] = n->y;
        buffer->normal[2] = n->z;
        buffer->coord[0] = v->x;
        buffer->coord[1] = v->y;
        buffer->coord[2] = v->z;
        Next();
    }

    void Vertex(pddiVector* v, pddiVector2* uv)
    {
        buffer->uv[0] = uv->u;
        buffer->uv[1] = uv->v;
        buffer->coord[0] = v->x;
        buffer->coord[1] = v->y;
        buffer->coord[2] = v->z;
        Next();
    }

    void Vertex(pddiVector* v, pddiColour c, pddiVector2* uv)
    {
        buffer->colour[0] = c.Red();
        buffer->colour[1] = c.Green();
        buffer->colour[2] = c.Blue();
        buffer->colour[3] = c.Alpha();
        buffer->uv[0] = uv->u;
        buffer->uv[1] = uv->v;
        buffer->coord[0] = v->x;
        buffer->coord[1] = v->y;
        buffer->coord[2] = v->z;
        Next();
    }

    void Vertex(pddiVector* v, pddiVector* n, pddiVector2* uv)
    {
        buffer->normal[0] = n->x;
        buffer->normal[1] = n->y;
        buffer->normal[2] = n->z;
        buffer->uv[0] = uv->u;
        buffer->uv[1] = uv->v;
        buffer->coord[0] = v->x;
        buffer->coord[1] = v->y;
        buffer->coord[2] = v->z;
        Next();
    }

    bool CheckMemImageVersion(int version) { return false; }
    void* GetMemImagePtr()                 { return NULL; }
    unsigned GetMemImageLength()           { return 0; }

};

pglPrimBuffer::pglPrimBuffer(pglContext* c, pddiPrimType type, unsigned vertexFormat, int nVertex, int nIndex) : context(c)
{
    stream = new pglPrimBufferStream(this);

    total = allocated = stride = nStrips = 0;
    coord = normal = uv = NULL;
    colour = NULL;
    strips = NULL;
    indices = NULL;

    valid = false;
    vertexBuffer = indexBuffer = vertexArray = 0;

    primType = type;
    vertexType = vertexFormat;

    allocated = nVertex;
    
    stride = 36;

    mem = stride * nVertex;
    buffer = new unsigned char[mem];

    unsigned char* ptr = buffer;
    coord = (float*)ptr;
    ptr += 12;
    
    if(vertexFormat & PDDI_V_NORMAL)
    {
        normal = (float*)ptr;
        ptr += 12;
    }
    
    if(vertexFormat & 0xf)
    {
        uv = (float*)ptr;
        ptr += 8;
    }
    
    if(vertexFormat & PDDI_V_COLOUR)
    {
        colour = ptr;
        ptr += 4;
    }

    indexCount = nIndex;
    if(indexCount) 
        indices = new unsigned short[indexCount];

    nStrips = 0;
    strips = NULL;

    context->ADD_STAT(PDDI_STAT_BUFFERED_COUNT, 1);
    context->ADD_STAT(PDDI_STAT_BUFFERED_ALLOC, mem / 1024.0f);
}

pglPrimBuffer::~pglPrimBuffer()
{
    delete stream;

    delete [] strips;
    delete [] indices;
    delete [] buffer;

    context->ADD_STAT(PDDI_STAT_BUFFERED_COUNT, -1);
    context->ADD_STAT(PDDI_STAT_BUFFERED_ALLOC, -mem / 1024.0f);

    GLuint buffers[] = { vertexBuffer, indexBuffer };
    glDeleteBuffers(2, buffers);
    glDeleteVertexArraysOES(1, &vertexArray);
}

pddiPrimBufferStream* pglPrimBuffer::Lock()
{
    total = 0;
    return stream;
}

void pglPrimBuffer::Unlock(pddiPrimBufferStream* stream)
{
    if(coord)
        coord = (float*)((char*)coord - total * stride);

    if(normal)
        normal = (float*)((char*)normal - total * stride);

    if(uv)
        uv = (float*)((char*)uv - total * stride);

    if(colour)
        colour -= total * stride;

    valid = false;
}

unsigned char* pglPrimBuffer::LockIndexBuffer()
{
    PDDIASSERT(0);
    return NULL;
}

void pglPrimBuffer::UnlockIndexBuffer(int count)
{
    PDDIASSERT(0);
}

void pglPrimBuffer::SetIndices(unsigned short* i, int count)
{
    PDDIASSERT(count <= (int)indexCount);
    memcpy(indices, i, count * sizeof(unsigned short));
    valid = false;
}

void pglPrimBuffer::Display(void)
{
    MICROPROFILE_SCOPEI("PDDI", "pglPrimBuffer::Display", MP_RED);

    if(!valid)
    {
        if(!vertexArray)
            glGenVertexArraysOES(1, &vertexArray);
        glBindVertexArrayOES(vertexArray);

        if(!vertexBuffer)
            glGenBuffers(1, &vertexBuffer);
        glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, mem, buffer, GL_STATIC_DRAW);

        if(indexCount && indices)
        {
            if(!indexBuffer)
                glGenBuffers(1, &indexBuffer);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,indexBuffer);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER,indexCount*sizeof(unsigned short),indices,GL_STATIC_DRAW);
        }
        else
        {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);
        }

        GLintptr offset = 0;
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,stride,(void*)offset);
        offset += 12;

        if(vertexType & PDDI_V_NORMAL)
        {
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,stride,(void*)offset);
            offset += 12;
        }
        else
        {
            glDisableVertexAttribArray(1);
            glVertexAttrib3f(1, 0.0f, 0.0f, 0.0f);
        }

        if(vertexType & 0xf)
        {
            glEnableVertexAttribArray(2);
            glVertexAttribPointer(2,2,GL_FLOAT,GL_FALSE,stride,(void*)offset);
            offset += 8;
        }
        else
        {
            glDisableVertexAttribArray(2);
            glVertexAttrib2f(2, 0.0f, 0.0f);
        }

        if(vertexType & PDDI_V_COLOUR)
        {
            glEnableVertexAttribArray(3);
            glVertexAttribPointer(3,4,GL_UNSIGNED_BYTE,GL_TRUE,stride,(void*)offset);
            offset += 4;
        }
        else
        {
            glDisableVertexAttribArray(3);
            glVertexAttrib4f(3, 1.0f, 1.0f, 1.0f, 1.0f);
        }
        valid = true;
    }
    else
    {
        glBindVertexArrayOES(vertexArray);
    }

    if(indexCount && indices)
    {
        glDrawElements(primTypeTable[primType],indexCount,GL_UNSIGNED_SHORT,0);
    }
    else
    {
        glDrawArrays(primTypeTable[primType], 0, total);
    }
}

/*
protected:
    float* coord;
    float* normal;
    float* uv;
    unsigned char* colour;

    unsigned allocated;
    unsigned total;

};
*/

void pglContext::DrawPrimBuffer(pddiShader* mat, pddiPrimBuffer* buffer)
{
    if(!mat)
        mat = defaultShader;

    pddiBaseShader* material = (pddiBaseShader*)mat;
    ADD_STAT(PDDI_STAT_MATERIAL_OPS, !material->IsCurrent());
    material->SetMaterial();
    ((pglPrimBuffer*)buffer)->Display();
}

// lighting

int pglContext::GetMaxLights(void)
{
    return PDDI_MAX_LIGHTS;
}

void pglContext::SetupHardwareLight(int handle)
{
    if(currentProgram)
        currentProgram->SetLightState(handle, &state.lightingState->light[handle]);
}

void pglContext::SetAmbientLight(pddiColour col)
{
    pddiBaseContext::SetAmbientLight(col);
    if(currentProgram)
        currentProgram->SetAmbientLight(col);
}


// backface culling
GLenum cullModeTable[3] =
{
    GL_FRONT, // PDDI_CULL_NONE (disabled using glDisable())
    GL_FRONT, // PDDI_CULL_NORMAL
    GL_BACK   // PDDI_CULL_INVERTED
};
    
void pglContext::SetCullMode(pddiCullMode mode)
{
    pddiBaseContext::SetCullMode(mode);

    if(mode == PDDI_CULL_NONE)
    {
        glDisable(GL_CULL_FACE);
    }
    else
    {
        glEnable(GL_CULL_FACE);
        glCullFace(cullModeTable[mode]);
    }
}

// z-buffer control
GLenum compTable[8] = {
    GL_NEVER,
    GL_ALWAYS,  
    GL_LESS,
    GL_LEQUAL,
    GL_GREATER,    
    GL_GEQUAL,  
    GL_EQUAL,
    GL_NOTEQUAL,
};

void pglContext::SetColourWrite( bool red, bool green, bool blue, bool alpha )
{
    pddiBaseContext::SetColourWrite(red, green, blue, alpha);
    glColorMask(red, green, blue, alpha);
}

void pglContext::EnableZBuffer(bool enable)
{
    pddiBaseContext::EnableZBuffer(enable);
    if(enable)
    {
        glEnable(GL_DEPTH_TEST);
    }
    else
    {
        glDisable(GL_DEPTH_TEST);
    }
}


void pglContext::SetZCompare(pddiCompareMode compareMode)
{
    pddiBaseContext::SetZCompare(compareMode);
    glDepthFunc(compTable[compareMode]);
}

void pglContext::SetZWrite(bool b)
{
    pddiBaseContext::SetZWrite(b);
    glDepthMask(b);
}

void pglContext::SetZBias(float bias)
{
    pddiBaseContext::SetZBias(bias);
//TODO : Figure out how ro do this
}

void pglContext::SetZRange(float n, float f)
{
    pddiBaseContext::SetZRange(n,f);
    glDepthRangef(n,f);
}

// stencil buffer control
GLenum stencilTable[6] = {
    GL_KEEP,
    GL_ZERO,
    GL_REPLACE,
    GL_INCR,
    GL_DECR,
    GL_INVERT
};

void pglContext::EnableStencilBuffer(bool enable)
{
    pddiBaseContext::EnableStencilBuffer(enable);
    if(enable)
        glEnable(GL_STENCIL_TEST);
    else
        glDisable(GL_STENCIL_TEST);
}
        
void pglContext::SetStencilCompare(pddiCompareMode compare)
{
    pddiBaseContext::SetStencilCompare(compare);
    glStencilFunc(compTable[compare], state.stencilState->ref, state.stencilState->mask);
}

void pglContext::SetStencilRef(int ref)
{
    pddiBaseContext::SetStencilRef(ref);
    glStencilFunc(compTable[state.stencilState->compare], ref, state.stencilState->mask);
}

void pglContext::SetStencilMask(unsigned mask)
{
    pddiBaseContext::SetStencilMask(mask);
    glStencilFunc(compTable[state.stencilState->compare], state.stencilState->ref, mask);
}

void pglContext::SetStencilWriteMask(unsigned mask)
{
    pddiBaseContext::SetStencilWriteMask(mask);
    glStencilMask(mask);
}

void pglContext::SetStencilOp(pddiStencilOp failOp, pddiStencilOp zFailOp, pddiStencilOp zPassOp)
{
    pddiBaseContext::SetStencilOp(failOp, zFailOp, zPassOp);
    glStencilOp(stencilTable[failOp],stencilTable[zFailOp],stencilTable[zPassOp]);
}

void pglContext::SetFillMode(pddiFillMode mode)
{
    pddiBaseContext::SetFillMode(mode);
}

// fog
void pglContext::EnableFog(bool enable)
{
    pddiBaseContext::EnableFog(enable);
}

void pglContext::SetFog(pddiColour colour, float start, float end)
{
    pddiBaseContext::SetFog(colour,start,end);

    float fog[4];
    fog[0] = float(colour.Red()) / 255;
    fog[1] = float(colour.Green()) / 255;
    fog[2] = float(colour.Blue()) / 255;
    fog[3] = float(colour.Alpha()) / 255;
}

int pglContext::GetMaxTextureDimension(void)
{
    return maxTexSize;
}

pddiExtension* pglContext::GetExtension(unsigned extID)
{ 
    switch(extID)
    {
        case PDDI_EXT_GL_CONTEXT :
            return extContext;
        case PDDI_EXT_GAMMACONTROL :
            return extGamma;
    }

    return pddiBaseContext::GetExtension(extID);
}

bool pglContext::VerifyExtension(unsigned extID)
{ 
    switch(extID)
    {
        case PDDI_EXT_GL_CONTEXT :
        case PDDI_EXT_GAMMACONTROL :
            return true;
    }

    return pddiBaseContext::VerifyExtension(extID);
}

void  pglContext::BeginTiming(void)
{
    display->BeginTiming();
}

float pglContext::EndTiming(void)
{
    return display->EndTiming();
}

void pglContext::SetShaderProgram(pglProgram* program)
{
    if(program == currentProgram)
        return;

    if(currentProgram)
        currentProgram->Release();
    currentProgram = program;
    if(!currentProgram)
        return;

    currentProgram->AddRef();
    currentProgram->UseProgram();
    currentProgram->SetProjectionMatrix(&projection);

    LoadHardwareMatrix(PDDI_MATRIX_MODELVIEW);
    if(currentProgram->SupportsLighting())
    {
        for (int i = 0; i < PDDI_MAX_LIGHTS; i++)
            SetupHardwareLight(i);
        SetAmbientLight(state.lightingState->ambient);
    }
}

void pglContext::SetTextureEnvironment(const pglTextureEnv* texEnv)
{
    if(texEnv->texture)
        SetShaderProgram(texEnv->alphaTest ? alphaTestProgram : textureProgram);
    else
        SetShaderProgram(colorProgram);
    currentProgram->SetTextureEnvironment(texEnv);
}
void pglContext::SetGammaUniform(float r, float g, float b)
{
    // activamos cada programa y seteamos el uniform gamma
    // hay que activar el programa antes de poder setear su uniform
    colorProgram->UseProgram();
    colorProgram->SetGamma(r, g, b);

    textureProgram->UseProgram();
    textureProgram->SetGamma(r, g, b);

    alphaTestProgram->UseProgram();
    alphaTestProgram->SetGamma(r, g, b);

    // restauramos el programa que estaba activo antes
    if(currentProgram)
        currentProgram->UseProgram();
}
