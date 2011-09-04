/**
 * projectM -- Milkdrop-esque visualisation SDK
 * Copyright (C)2003-2004 projectM Team
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 * See 'LICENSE.txt' included within this release
 *
 */
/**
 * $Id: FBO.c,v 1.1.1.1 2005/12/23 18:05:00 psperl Exp $
 *
 * Render this methods
 */

#include <stdio.h>
#include <iostream>
#include "Common.hpp"
#include "FBO.hpp"

#include <android/log.h> 
#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, "libprojectm", 	__VA_ARGS__) 
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG  , "libprojectm", 	__VA_ARGS__) 
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO   , "libprojectm", 		__VA_ARGS__) 
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN   , "libprojectm", 		__VA_ARGS__) 
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR  , "libprojectm", 		__VA_ARGS__) 

static bool checkOpenGL(const std::string& msg)
{
	GLenum errorCode = glGetError();
	if (errorCode != GL_NO_ERROR) {
		LOGE("OpenGL Error %d: %s", errorCode, msg.c_str());
		return true;
	}
	return false;
}

RenderTarget::~RenderTarget() {


	glDeleteTextures( 1, &this->textureID[0]);

	if (useFBO) 
         {
		glDeleteTextures( 1, &this->textureID[1] );
		glDeleteRenderbuffersOES(1,  &this->depthb[0]);
		glDeleteFramebuffersOES(1, &this->fbuffer[0]);
		if(renderToTexture)
		  {
		    glDeleteTextures( 1, &this->textureID[2] );
		    glDeleteRenderbuffersOES(1,  &this->depthb[1]);
		    glDeleteFramebuffersOES(1, &this->fbuffer[1]);
		  }
         }
}

GLuint RenderTarget::initRenderToTexture()
{
  if (this->useFBO)
    {
      this->renderToTexture=1;
      
      GLuint   fb2, depth_rb2;
      glGenFramebuffersOES(1, &fb2);
      glBindFramebufferOES(GL_FRAMEBUFFER_OES, fb2);
      glGenRenderbuffersOES(1, &depth_rb2);
      glBindRenderbufferOES(GL_RENDERBUFFER_OES, depth_rb2);
      
      glRenderbufferStorageOES(GL_RENDERBUFFER_OES, GL_DEPTH_COMPONENT16_OES, this->texsize, this->texsize);
      glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_DEPTH_ATTACHMENT_OES, GL_RENDERBUFFER_OES, depth_rb2);
      this->fbuffer[1] = fb2;
      this->depthb[1]=  depth_rb2;
      glGenTextures(1, &this->textureID[2]);
      glBindTexture(GL_TEXTURE_2D, this->textureID[2]); 
      glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, texsize, texsize, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL );
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      glFramebufferTexture2DOES( GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES, GL_TEXTURE_2D, this->textureID[2], 0 );
      return this->textureID[2];
    }
return -1;
      
}

/** Creates new pbuffers */
RenderTarget::RenderTarget(int texsize, int width, int height, bool enableFBO, bool enableFBOFast) : useFBO(false), enableFBO(enableFBO), enableFBOFast(enableFBOFast) {

   int mindim = 0;
   int origtexsize = 0;

   this->renderToTexture = 0;
   this->texsize = texsize;
   this->framebuffer = 0;

    mindim = width < height ? width : height;
    origtexsize = this->texsize;
    texsize = this->texsize = nearestPower2( mindim, SCALE_MINIFY );

	if (enableFBO) {
	  GLuint   fb,  depth_rb, rgba_tex,  other_tex;
	  
	glGetError();
	  glGenTextures(1, &other_tex);
		checkOpenGL("RenderTarget:genTexture");
	  glBindTexture(GL_TEXTURE_2D,other_tex);
		checkOpenGL("RenderTarget:bindTexture");
	  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	  //glGenerateMipmapOES(GL_TEXTURE_2D);
	  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		checkOpenGL("RenderTarget:texParams");
	  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texsize, texsize, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		checkOpenGL("RenderTarget:TexImage2D");
	  glBindTexture(GL_TEXTURE_2D, 0);
	  
	  glGenTextures(1, &rgba_tex);
		checkOpenGL("RenderTarget:genTexture");
	  glBindTexture(GL_TEXTURE_2D, rgba_tex); 
	  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	  //glGenerateMipmapOES(GL_TEXTURE_2D);
	  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		checkOpenGL("RenderTarget:texParams");
	  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texsize, texsize, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		checkOpenGL("RenderTarget:TexImage2D");
	  glBindTexture(GL_TEXTURE_2D, 0);
		checkOpenGL("RenderTarget:bindTexture");
	
	  glGenRenderbuffersOES(1, &depth_rb);
		checkOpenGL("RenderTarget:genRenderbuffer");
	  glBindRenderbufferOES(GL_RENDERBUFFER_OES, depth_rb);
		checkOpenGL("RenderTarget:bindRenderbuffer");
	  glRenderbufferStorageOES(GL_RENDERBUFFER_OES, GL_DEPTH_COMPONENT16_OES, this->texsize, this->texsize);
		checkOpenGL("RenderTarget:renderbufferStorage");
	  
	  glGenFramebuffersOES(1, &fb);
		checkOpenGL("RenderTarget:genFramebuffer");
	  glBindFramebufferOES(GL_FRAMEBUFFER_OES, fb);
		checkOpenGL("RenderTarget:bindFramebuffer");
	  glFramebufferTexture2DOES(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES, GL_TEXTURE_2D, rgba_tex, 0);
		checkOpenGL("RenderTarget:genFramebufferTexture");
	  
	  glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_DEPTH_ATTACHMENT_OES, GL_RENDERBUFFER_OES, depth_rb);
		checkOpenGL("RenderTarget:framebufferRenderbuffer");
	  
	  this->fbuffer[0] = fb;
	  this->depthb[0] = depth_rb;
	  this->textureID[0] = rgba_tex;
#if defined(FBO_HACK) || defined(FBO_HACK2)
	  this->textureID[1] = rgba_tex;
#else
	  this->textureID[1] = other_tex;
#endif
	  
	  GLenum status = glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES);
	  if (status == GL_FRAMEBUFFER_COMPLETE_OES) {
		  useFBO = true;
	    return;
	  }	
	  std::cerr << "[projecM] warning: FBO support not detected. Using fallback." << std::endl;

	}

// Can reach here via two code paths: 
// (1) useFBO was set to false externally by cmake / system setting / etc.
// (2) useFBO was true but forced to false as it failed to pass all the GLU extension checks.

    /** Fallback pbuffer creation via teximage hack */
    /** Check the texture size against the viewport size */
    /** If the viewport is smaller, then we'll need to scale the texture size down */
    /** If the viewport is larger, scale it up */
	glGetError();
        glGenTextures(1, &this->textureID[0] );
		checkOpenGL("RenderTarget:genTexture");

        glBindTexture(GL_TEXTURE_2D, this->textureID[0] );
		checkOpenGL("RenderTarget:bindTexture");
        //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		checkOpenGL("RenderTarget:texParams");
        glTexImage2D(GL_TEXTURE_2D,
		    0,
		    GL_RGB,
		    this->texsize, this->texsize,
		    0,
		    GL_RGBA,
		    GL_UNSIGNED_BYTE,
		    NULL);
		checkOpenGL("RenderTarget:TexImage2D");
      

   
    return;
  }

  void RenderTarget::fallbackRescale(int width, int height)
  {
	int mindim = width < height ? width : height;
    //int origtexsize = this->texsize;
    this->texsize = nearestPower2( mindim, SCALE_MINIFY );      

    /* Create the texture that will be bound to the render this */
    /*

        if ( this->texsize != origtexsize ) {

            glDeleteTextures( 1, &this->textureID[0] );
          }
    */

        glGenTextures(1, &this->textureID[0] );

        glBindTexture(GL_TEXTURE_2D, this->textureID[0] );
        //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        glTexImage2D(GL_TEXTURE_2D,
		    0,
		    GL_RGB,
		    this->texsize, this->texsize,
		    0,
		    GL_RGBA,
		    GL_UNSIGNED_BYTE,
		    NULL);
      

  }

/** Destroys the pbuffer */

/** Locks the pbuffer */
void RenderTarget::lock() {


  if(this->useFBO)
    { 
//		glEnable(GL_DEPTH_TEST);
#if defined(FBO_HACK) || defined(FBO_HACK2)
      glBindFramebufferOES(GL_FRAMEBUFFER_OES, framebuffer);
#else
      glBindFramebufferOES(GL_FRAMEBUFFER_OES, this->fbuffer[0]);
#endif
    } else {
      glBindFramebufferOES(GL_FRAMEBUFFER_OES, framebuffer);
	  glBindRenderbufferOES(GL_RENDERBUFFER_OES, 0);
	}
    }

/** Unlocks the pbuffer */
void RenderTarget::unlock() {
	glGetError();

  if(this->useFBO)
    {
//		glDisable(GL_DEPTH_TEST);
      glBindTexture( GL_TEXTURE_2D, this->textureID[1] );
	checkOpenGL("unlock:bindTexture");
#ifndef FBO_HACK
	if (!enableFBOFast) {
      glCopyTexSubImage2D( GL_TEXTURE_2D,
                         0, 0, 0, 0, 0, 
                         this->texsize, this->texsize );
	if (checkOpenGL("unlock:copyTexSubImage"))
		useFBO = false;
	}
#endif
      glBindFramebufferOES(GL_FRAMEBUFFER_OES, framebuffer);
	checkOpenGL("unlock:bindFramebuffer");
      return;
    }
    /** Fallback texture path */
    
    glBindTexture( GL_TEXTURE_2D, this->textureID[0] );
	checkOpenGL("unlock:bindTexture");
	//return;
    
	glCopyTexSubImage2D( GL_TEXTURE_2D, 0, 0, 0, 0, 0, this->texsize, this->texsize );
	if (checkOpenGL("unlock:copyTexSubImage")) {
	if (enableFBO) {
      glBindFramebufferOES(GL_FRAMEBUFFER_OES, framebuffer);
	  glBindRenderbufferOES(GL_RENDERBUFFER_OES, 0);
	  GLuint   fb,  depth_rb, rgba_tex,  other_tex;
	  
	  glGenTextures(1, &other_tex);
		checkOpenGL("unlock:genTexture");
	  glBindTexture(GL_TEXTURE_2D,other_tex);
		checkOpenGL("unlock:bindTexture");
	  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	  //glGenerateMipmapOES(GL_TEXTURE_2D);
	  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		checkOpenGL("unlock:texParams");
	  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texsize, texsize, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		checkOpenGL("unlock:TexImage2D");
	  glBindTexture(GL_TEXTURE_2D, 0);
	  
	  glGenTextures(1, &rgba_tex);
		checkOpenGL("unlock:genTexture");
	  glBindTexture(GL_TEXTURE_2D, rgba_tex); 
	  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	  //glGenerateMipmapOES(GL_TEXTURE_2D);
	  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		checkOpenGL("unlock:texParams");
	  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texsize, texsize, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
		checkOpenGL("unlock:TexImage2D");
	  glBindTexture(GL_TEXTURE_2D, 0);
		checkOpenGL("unlock:bindTexture");
	
	  glGenRenderbuffersOES(1, &depth_rb);
		checkOpenGL("unlock:genRenderbuffer");
	  glBindRenderbufferOES(GL_RENDERBUFFER_OES, depth_rb);
		checkOpenGL("unlock:bindRenderbuffer");
	  glRenderbufferStorageOES(GL_RENDERBUFFER_OES, GL_DEPTH_COMPONENT16_OES, this->texsize, this->texsize);
		checkOpenGL("unlock:renderbufferStorage");
	  
	  glGenFramebuffersOES(1, &fb);
		checkOpenGL("unlock:genFramebuffer");
	  glBindFramebufferOES(GL_FRAMEBUFFER_OES, fb);
		checkOpenGL("unlock:bindFramebuffer");
	  glFramebufferTexture2DOES(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES, GL_TEXTURE_2D, rgba_tex, 0);
		checkOpenGL("unlock:genFramebufferTexture");
	  
	  glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_DEPTH_ATTACHMENT_OES, GL_RENDERBUFFER_OES, depth_rb);
		checkOpenGL("unlock:framebufferRenderbuffer");
	  
	  this->fbuffer[0] = fb;
	  this->depthb[0] = depth_rb;
	  this->textureID[0] = rgba_tex;
#if defined(FBO_HACK) || defined(FBO_HACK2)
	  this->textureID[1] = rgba_tex;
#else
	  this->textureID[1] = other_tex;
#endif
	  
	  GLenum status = glCheckFramebufferStatusOES(GL_FRAMEBUFFER_OES);
	  if (status == GL_FRAMEBUFFER_COMPLETE_OES) {
		  useFBO = true;
	    return;
	  }	
	  std::cerr << "[projecM] warning: FBO support not detected. Using fallback." << std::endl;
      glBindFramebufferOES(GL_FRAMEBUFFER_OES, framebuffer);
	  glBindRenderbufferOES(GL_RENDERBUFFER_OES, 0);
	}

		glGenTextures(1, &this->textureID[0]);
		checkOpenGL("unlock:genTexture");
		glBindTexture(GL_TEXTURE_2D, this->textureID[0]);
		checkOpenGL("unlock:bindTexture");

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		checkOpenGL("unlock:texParams");

        glTexImage2D(GL_TEXTURE_2D,
		    0,
		    GL_RGB,
		    this->texsize, this->texsize,
		    0,
		    GL_RGB,
		    GL_UNSIGNED_BYTE,
		    NULL);
		checkOpenGL("unlock:genTexImage2D");
	}
  }

/** 
 * Calculates the nearest power of two to the given number using the
 * appropriate rule
 */
int RenderTarget::nearestPower2( int value, TextureScale scaleRule ) {

    int x = value;
    int power = 0;

    while ( ( x & 0x01 ) != 1 ) {
        x >>= 1;
      }

    if ( x == 1 ) {
        return value;
      } else {
        x = value;
        while ( x != 0 ) {
            x >>= 1;
            power++;
          }
        switch ( scaleRule ) {
            case SCALE_NEAREST:
                if ( ( ( 1 << power ) - value ) <= ( value - ( 1 << ( power - 1 ) ) ) ) {
                    return 1 << power;
                  } else {
                    return 1 << ( power - 1 );
                  }
            case SCALE_MAGNIFY:
                return 1 << power;
            case SCALE_MINIFY:
                return 1 << ( power - 1 );
            default:
                break;
          }
      }
    return 0;
  }
