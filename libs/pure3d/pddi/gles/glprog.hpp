//=============================================================================
// Copyright (c) 2002 Radical Games Ltd.  All rights reserved.
//=============================================================================


#ifndef _GLPROG_HPP_
#define _GLPROG_HPP_

#include <pddi/pddi.hpp>
#include <pddi/base/basecontext.hpp>
#include <pddi/gles/gl.hpp>
struct pglTextureEnv;

#ifdef RAD_CG
#include <Cg/cg.h>    /* Can't include this?  Is Cg Toolkit installed! */
#endif

class pglProgram : public pddiObject
{
public:
    pglProgram();
    ~pglProgram();

#ifdef RAD_CG
    void UseProgram();
    static CGprogram CompileShader(GLenum type, const char* source);
    static pglProgram* CreateProgram(CGprogram vertexShader, CGprogram fragmentShader);
#else
    void UseProgram() { glUseProgram(program); }
    static GLuint CompileShader(GLenum type, const char* source);
    static pglProgram* CreateProgram(GLuint vertexShader, GLuint fragmentShader);
#endif

    void SetProjectionMatrix(const pddiMatrix* matrix);
    void SetModelViewMatrix(const pddiMatrix* matrix);
    void SetTextureEnvironment(const pglTextureEnv* texEnv);
    void SetLightState(int handle, const pddiLight* lightState);
    void SetAmbientLight(pddiColour ambient);
    // setea el valor de gamma en el shader via uniform
    void SetGamma(float r, float g, float b);

    inline bool SupportsLighting() { return acs >= 0; }
    inline bool SupportsTextures() { return sampler >= 0; }

protected:
#ifdef RAD_CG
    bool LinkProgram(CGprogram vertexShader, CGprogram fragmentShader);

    static void checkForCgError();
    static CGcontext context;
    static CGprofile vertexProfile, fragmentProfile;

    CGprogram program;

    // Uniform locations
    CGparameter projection, modelview, normalmatrix, alpharef, sampler;
    struct {
        CGparameter enabled, position, colour, attenuation;
    } lights[PDDI_MAX_LIGHTS];
    CGparameter acs, acm, dcm, scm, ecm, srm;
#else
    bool LinkProgram(GLuint vertexShader, GLuint fragmentShader);

    GLuint program;

    // Uniform locations
    GLint projection, modelview, normalmatrix, alpharef, sampler;
    struct {
        GLint enabled, position, colour, attenuation;
    } lights[PDDI_MAX_LIGHTS];
    GLint acs, acm, dcm, scm, ecm, srm;
    // ubicación del uniform gamma en el shader
    GLint gamma;
    // indica al shader si el material tiene iluminación o no
    GLint lit;
#endif
};

#endif
