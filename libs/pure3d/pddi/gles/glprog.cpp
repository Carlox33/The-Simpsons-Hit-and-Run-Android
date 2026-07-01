//=============================================================================
// Copyright (c) 2002 Radical Games Ltd.  All rights reserved.
//=============================================================================

#include <pddi/gles/gl.hpp>
#include <pddi/gles/glprog.hpp>
#include <pddi/gles/glmat.hpp>

#include <string>
#include <vector>
#include <SDL.h>

#ifdef RAD_CG
#define CGGL_NO_OPENGL
#include <Cg/cg.h>    /* Can't include this?  Is Cg Toolkit installed! */
#include <Cg/cgGL.h>

void pglProgram::checkForCgError()
{
    CGerror error;
    const char* string = cgGetLastErrorString(&error);

    if (error != CG_NO_ERROR) {
        SDL_Log("%s\n", string);
        if (error == CG_COMPILER_ERROR) {
            printf("%s\n", cgGetLastListing(pglProgram::context));
        }
        __debugbreak();
    }
}

CGcontext pglProgram::context = nullptr;
CGprofile pglProgram::vertexProfile, pglProgram::fragmentProfile;
#endif

#ifdef RAD_CG
static inline void UniformColour(CGparameter param, pddiColour c)
{
    cgSetParameter4f(param, float(c.Red()) / 255, float(c.Green()) / 255, float(c.Blue()) / 255, float(c.Alpha()) / 255);
}
#else
static inline void UniformColour(GLint loc, pddiColour c)
{
    glUniform4f(loc, float(c.Red()) / 255, float(c.Green()) / 255, float(c.Blue()) / 255, float(c.Alpha()) / 255);
}
#endif

pglProgram::pglProgram()
{
#ifdef RAD_CG
    program = nullptr;
    projection = modelview = normalmatrix = alpharef = sampler = acs = nullptr;
#else
    program = 0;
    projection = modelview = normalmatrix = alpharef = sampler = acs = -1;
#endif
}

pglProgram::~pglProgram()
{
#ifdef RAD_CG
    if (program)
        cgDestroyProgram(program);
#else
    if (program)
        glDeleteProgram(program);
#endif
}

void pglProgram::SetProjectionMatrix(const pddiMatrix* matrix)
{
#ifdef RAD_CG
    if (projection)
        cgSetMatrixParameterfc(projection, matrix->m[0]);
    checkForCgError();
#else
    if (projection >= 0)
        glUniformMatrix4fv(projection, 1, GL_FALSE, matrix->m[0]);
#endif
}

void pglProgram::SetModelViewMatrix(const pddiMatrix* matrix)
{
#ifdef RAD_CG
    if (modelview)
        cgSetMatrixParameterfc(modelview, matrix->m[0]);
    checkForCgError();

    if (normalmatrix)
    {
        pddiMatrix inverse;
        inverse.Invert(*matrix);
        inverse.Transpose();
        cgSetMatrixParameterfc(normalmatrix, inverse.m[0]);
    }
    checkForCgError();
#else
    if (modelview >= 0)
        glUniformMatrix4fv(modelview, 1, GL_FALSE, matrix->m[0]);
    if (normalmatrix >= 0)
    {
        pddiMatrix inverse;
        inverse.Invert(*matrix);
        inverse.Transpose();
        glUniformMatrix4fv(normalmatrix, 1, GL_FALSE, inverse.m[0]);
    }
#endif
}

void pglProgram::SetTextureEnvironment(const pglTextureEnv* texEnv)
{
#ifdef RAD_CG
    if (texEnv->lit)
    {
        UniformColour(acm, texEnv->ambient);
        UniformColour(ecm, texEnv->emissive);
        UniformColour(dcm, texEnv->diffuse);
        UniformColour(scm, texEnv->specular);
        cgSetParameter1f(srm, texEnv->shininess);
    }
    else
    {
        UniformColour(acm, pddiColour(-1));
        UniformColour(ecm, pddiColour(-1));
        UniformColour(dcm, pddiColour(-1));
        UniformColour(scm, pddiColour(-1));
        cgSetParameter1f(srm, 0.0f);
    }
    checkForCgError();

    if (texEnv->alphaTest && alpharef >= 0)
    {
        PDDIASSERT(texEnv->alphaCompareMode == PDDI_COMPARE_GREATER ||
            texEnv->alphaCompareMode == PDDI_COMPARE_GREATEREQUAL);
        cgSetParameter1f(alpharef, texEnv->alphaTest ? texEnv->alphaRef : 0.0f);
    }
    checkForCgError();
#else
    if (sampler >= 0)
        glUniform1i(sampler, 0);

    if (texEnv->lit)
    {
        UniformColour(acm, texEnv->ambient);
        UniformColour(ecm, texEnv->emissive);
        UniformColour(dcm, texEnv->diffuse);
        UniformColour(scm, texEnv->specular);
        glUniform1f(srm, texEnv->shininess);
    }
    else
    {
        UniformColour(acm, pddiColour(-1));
        UniformColour(ecm, pddiColour(-1));
        UniformColour(dcm, pddiColour(-1));
        UniformColour(scm, pddiColour(-1));
        glUniform1f(srm, 0.0f);
    }

    if (texEnv->alphaTest && alpharef >= 0)
    {
        PDDIASSERT(texEnv->alphaCompareMode == PDDI_COMPARE_GREATER ||
            texEnv->alphaCompareMode == PDDI_COMPARE_GREATEREQUAL);
        glUniform1f(alpharef, texEnv->alphaTest ? texEnv->alphaRef : 0.0f);
    }
#endif
}

void pglProgram::SetLightState(int handle, const pddiLight* lightState)
{
    if( handle >= PDDI_MAX_LIGHTS )
        return;

    float dir[4];
    switch(lightState->type)
    {
        case PDDI_LIGHT_DIRECTIONAL :
            dir[0] = -lightState->worldDirection.x;
            dir[1] = -lightState->worldDirection.y;
            dir[2] = -lightState->worldDirection.z;
            dir[3] = 0.0f;
            break;

        case PDDI_LIGHT_POINT :
            dir[0] = lightState->worldPosition.x;
            dir[1] = lightState->worldPosition.y;
            dir[2] = lightState->worldPosition.z;
            dir[3] = 1.0f;
            break;

        case PDDI_LIGHT_SPOT :
            PDDIASSERT(0);
            break;
    }

#ifdef RAD_CG
    if (lights[handle].enabled)
    {
        cgSetParameter1i(lights[handle].enabled, lightState->enabled ? 1 : 0);
        cgSetParameter4fv(lights[handle].position, dir);
        UniformColour(lights[handle].colour, lightState->colour);
        cgSetParameter3f(lights[handle].attenuation, lightState->attenA, lightState->attenB, lightState->attenC);
    }
    checkForCgError();
#else
    glUniform1i(lights[handle].enabled, lightState->enabled ? 1 : 0);
    glUniform4fv(lights[handle].position, 1, dir);
    UniformColour(lights[handle].colour, lightState->colour);
    glUniform3f(lights[handle].attenuation, lightState->attenA, lightState->attenB, lightState->attenC);
#endif
}

void pglProgram::SetAmbientLight(pddiColour ambient)
{
    if (acs)
        UniformColour(acs, ambient);
}

#ifdef RAD_CG
bool pglProgram::LinkProgram(CGprogram vertexShader, CGprogram fragmentShader)
{
    program = cgCombinePrograms2(vertexShader, fragmentShader);
    checkForCgError();
    if (!program)
        return false;

    cgGLLoadProgram(program);
    checkForCgError();

    projection = cgGetNamedParameter(program, "projection");
    modelview = cgGetNamedParameter(program, "modelview");
    normalmatrix = cgGetNamedParameter(program, "normalmatrix");
    alpharef = cgGetNamedParameter(program, "alpharef");
    sampler = cgGetNamedParameter(program, "tex");
    checkForCgError();

    for (int i = 0; i < PDDI_MAX_LIGHTS; i++)
    {
        std::string prefix = std::string("lights[") + char('0' + i) + "].";
        lights[i].enabled = cgGetNamedParameter(program, (prefix + "enabled").c_str());
        lights[i].position = cgGetNamedParameter(program, (prefix + "position").c_str());
        lights[i].colour = cgGetNamedParameter(program, (prefix + "colour").c_str());
        lights[i].attenuation = cgGetNamedParameter(program, (prefix + "attenuation").c_str());
        checkForCgError();
    }

    acs = cgGetNamedParameter(program, "acs");
    acm = cgGetNamedParameter(program, "acm");
    dcm = cgGetNamedParameter(program, "dcm");
    scm = cgGetNamedParameter(program, "scm");
    ecm = cgGetNamedParameter(program, "ecm");
    srm = cgGetNamedParameter(program, "srm");
    checkForCgError();
    return true;
}
#else
bool pglProgram::LinkProgram(GLuint vertexShader, GLuint fragmentShader)
{
    program = glCreateProgram();

    if(vertexShader)
        glAttachShader(program, vertexShader);
    if(fragmentShader)
        glAttachShader(program, fragmentShader);

    glBindAttribLocation(program, 0, "position");
    glBindAttribLocation(program, 1, "normal");
    glBindAttribLocation(program, 2, "texcoord");
    glBindAttribLocation(program, 3, "color");

    glLinkProgram(program);

    GLint isLinked = 0;
    glGetProgramiv(program, GL_LINK_STATUS, (int *)&isLinked);
    if (isLinked == GL_FALSE)
    {
        GLint maxLength = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

        // The maxLength includes the NULL character
        std::vector<GLchar> infoLog(maxLength);
        glGetProgramInfoLog(program, maxLength, &maxLength, &infoLog[0]);

        SDL_Log("Program linking error: %s", infoLog.data());
        return false;
    }
    
    projection = glGetUniformLocation(program, "projection");
    modelview = glGetUniformLocation(program, "modelview");
    normalmatrix = glGetUniformLocation(program, "normalmatrix");
    alpharef = glGetUniformLocation(program, "alpharef");
    sampler = glGetUniformLocation(program, "tex");

    for (int i = 0; i < PDDI_MAX_LIGHTS; i++)
    {
        std::string prefix = std::string("lights[") + char('0' + i) + "].";
        lights[i].enabled = glGetUniformLocation(program, (prefix + "enabled").c_str());
        lights[i].position = glGetUniformLocation(program, (prefix + "position").c_str());
        lights[i].colour = glGetUniformLocation(program, (prefix + "colour").c_str());
        lights[i].attenuation = glGetUniformLocation( program, (prefix + "attenuation").c_str() );
    }

    acs = glGetUniformLocation(program, "acs");
    acm = glGetUniformLocation(program, "acm");
    dcm = glGetUniformLocation(program, "dcm");
    scm = glGetUniformLocation(program, "scm");
    ecm = glGetUniformLocation(program, "ecm");
    srm = glGetUniformLocation(program, "srm");
    // registro la ubicación del uniform gamma
    gamma = glGetUniformLocation(program, "gamma");

#ifndef RAD_VITAGL
    // Always detach shaders after a successful link
    if(vertexShader)
        glDetachShader(program, vertexShader);
    if(fragmentShader)
        glDetachShader(program, fragmentShader);
#endif
    return true;
}
#endif

#ifdef RAD_CG
CGprogram pglProgram::CompileShader(GLenum type, const char* source)
{
    if(!context)
    {
        context = cgCreateContext();
        cgGLSetDebugMode(CG_FALSE);
        cgSetParameterSettingMode(context, CG_IMMEDIATE_PARAMETER_SETTING);
        vertexProfile = cgGetProfile("glslv");
        assert(cgGLIsProfileSupported(vertexProfile));
        cgGLSetOptimalOptions(vertexProfile);
        fragmentProfile = cgGetProfile("glslf");
        assert(cgGLIsProfileSupported(fragmentProfile));
        cgGLSetOptimalOptions(fragmentProfile);
    }
    checkForCgError();

    const char* args[] = { "version=330", NULL };
    CGprofile profile = type == GL_VERTEX_SHADER ?
        vertexProfile : fragmentProfile;
    cgGLSetOptimalOptions(profile);
    CGprogram program = cgCreateProgram(
        context,
        CG_SOURCE,
        source,
        profile,
        "main",
        args);
    checkForCgError();
    return program;
}
#else
GLuint pglProgram::CompileShader(GLenum type, const char* source)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, 0);
    glCompileShader(shader);

    GLint isCompiled = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
    if(isCompiled == GL_FALSE)
    {
        GLint maxLength = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

        // The maxLength includes the NULL character
        std::vector<GLchar> infoLog(maxLength);
        glGetShaderInfoLog(shader, maxLength, &maxLength, &infoLog[0]);

        // We don't need the shader anymore.
        glDeleteShader(shader);

        SDL_Log("Shader compilation error: %s", infoLog.data());
        return 0;
    }
    return shader;
}
#endif

#ifdef RAD_CG
void pglProgram::UseProgram()
{
    cgUpdateProgramParameters(program);
    cgGLBindProgram(program);
    checkForCgError();
}
#endif

#ifdef RAD_CG
pglProgram* pglProgram::CreateProgram(CGprogram vertexShader, CGprogram fragmentShader)
#else
pglProgram* pglProgram::CreateProgram(GLuint vertexShader, GLuint fragmentShader)
#endif
{
    pglProgram* program = new pglProgram();
    program->AddRef();
    if(!program->LinkProgram(vertexShader, fragmentShader))
    {
        program->Release();
        return nullptr;
    }
    return program;
}
void pglProgram::SetGamma(float r, float g, float b)
{
    // seteo el uniform gamma en el shader activo
    // gamma >= 0 significa que el uniform existe en este shader
    if(gamma >= 0)
        glUniform3f(gamma, r, g, b);
}
