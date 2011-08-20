#include "Renderer.hpp"
#include "wipemalloc.h"
#include "math_neon.h"
#include "Common.hpp"
#include "KeyHandler.hpp"
#include "TextureManager.hpp"
#include <iostream>
#include <algorithm>
#include <cassert>
#include "omptl/omptl"
#include "omptl/omptl_algorithm"
#include "UserTexture.hpp"

class Preset;

Renderer::Renderer(int width, int height, int gx, int gy, int texsize, BeatDetect *beatDetect, std::string _presetURL,
		std::string _titlefontURL, std::string _menufontURL, bool enableFBO, bool enableFBOFast) :
	texsize(texsize), mesh(gx, gy), m_presetName("None"), vw(width), vh(height), title_fontURL(_titlefontURL), menu_fontURL(_menufontURL), presetURL(_presetURL)
{
	this->totalframes = 1;
	this->noSwitch = false;
	this->showfps = false;
	this->showtitle = false;
	this->showpreset = false;
	this->showhelp = false;
	this->showstats = false;
	this->studio = false;
	this->realfps = 0;

	this->drawtitle = 0;

	//this->title = "Unknown";

	/** Other stuff... */
	this->correction = true;
	this->aspect = (float) height / (float) width;;

	/// @bug put these on member init list
	this->renderTarget = new RenderTarget(texsize, width, height, enableFBO, enableFBOFast);
	this->textureManager = new TextureManager(presetURL);
	this->beatDetect = beatDetect;

#ifdef USE_FTGL
	/**f Load the standard fonts */

	title_font = new FTGLPixmapFont(title_fontURL.c_str());
	other_font = new FTGLPixmapFont(menu_fontURL.c_str());
	other_font->UseDisplayList(true);
	title_font->UseDisplayList(true);

	poly_font = new FTGLExtrdFont(title_fontURL.c_str());

	poly_font->UseDisplayList(true);
	poly_font->Depth(20);
	poly_font->FaceSize(72);

	poly_font->UseDisplayList(true);

#endif /** USE_FTGL */


	int size = (mesh.height - 1) *mesh.width * 5 * 2;
	p = ( float * ) wipemalloc ( size * sizeof ( float ) );


	for (int j = 0; j < mesh.height - 1; j++)
	{
		int base = j * mesh.width * 2 * 5;


		for (int i = 0; i < mesh.width; i++)
		{
			int index = j * mesh.width + i;
			int index2 = (j + 1) * mesh.width + i;

			int strip = base + i * 10;
			p[strip + 2] = mesh.identity[index].x;
			p[strip + 3] = mesh.identity[index].y;
			p[strip + 4] = 0;

			p[strip + 7] = mesh.identity[index2].x;
			p[strip + 8] = mesh.identity[index2].y;
			p[strip + 9] = 0;
		}
	}


#ifdef USE_CG
	shaderEngine.setParams(renderTarget->texsize, renderTarget->textureID[1], aspect, beatDetect, textureManager);
#endif

}

void Renderer::SetPipeline(Pipeline &pipeline)
{
	currentPipe = &pipeline;
#ifdef USE_CG
	shaderEngine.reset();
	shaderEngine.loadShader(pipeline.warpShader);
	shaderEngine.loadShader(pipeline.compositeShader);
#endif
}

void Renderer::ResetTextures()
{
	textureManager->Clear();

	bool enableFBO = renderTarget->enableFBO;
	bool enableFBOFast = renderTarget->enableFBOFast;
	delete (renderTarget);
	renderTarget = new RenderTarget(texsize, vw, vh, enableFBO, enableFBOFast);
	reset(vw, vh);

	textureManager->Preload();
}

void Renderer::SetupPass1(const Pipeline &pipeline, const PipelineContext &pipelineContext)
{
	//glMatrixMode(GL_PROJECTION);
	//glPushMatrix();
	//glMatrixMode(GL_MODELVIEW);
	//glPushMatrix();

	totalframes++;
	renderTarget->lock();
	glViewport(0, 0, renderTarget->texsize, renderTarget->texsize);

	glEnable(GL_TEXTURE_2D);

	//If using FBO, switch to FBO texture

	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

#ifdef USE_GLES1
	glOrthof(0.0, 1, 0.0, 1, -40, 40);
#else
	glOrtho(0.0, 1, 0.0, 1, -40, 40);
#endif

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

#ifdef USE_CG
	shaderEngine.RenderBlurTextures(pipeline, pipelineContext, renderTarget->texsize);
#endif
}

void Renderer::RenderItems(const Pipeline &pipeline, const PipelineContext &pipelineContext)
{
	renderContext.time = pipelineContext.time;
	renderContext.texsize = texsize;
	renderContext.aspectCorrect = correction;
	renderContext.aspectRatio = aspect;
	renderContext.textureManager = textureManager;
	renderContext.beatDetect = beatDetect;

	for (std::vector<RenderItem*>::const_iterator pos = pipeline.drawables.begin(); pos != pipeline.drawables.end(); ++pos)
    {
      if (*pos != NULL)
      {
		(*pos)->Draw(renderContext);
      }
    }
}

void Renderer::FinishPass1()
{
	draw_title_to_texture();
	/** Restore original view state */
	//glMatrixMode(GL_MODELVIEW);
	//glPopMatrix();

	//glMatrixMode(GL_PROJECTION);
	//glPopMatrix();

	renderTarget->unlock();

}

void Renderer::Pass2(const Pipeline &pipeline, const PipelineContext &pipelineContext)
{
	//BEGIN PASS 2
	//
	//end of texture rendering
	//now we copy the texture from the FBO or framebuffer to
	//video texture memory and render fullscreen.

	/** Reset the viewport size */
	if (renderTarget->renderToTexture)
	{
		glBindFramebufferOES(GL_FRAMEBUFFER_OES, this->renderTarget->fbuffer[1]);
		glViewport(0, 0, this->renderTarget->texsize, this->renderTarget->texsize);
	}
	else
		glViewport(0, 0, this->vw, this->vh);

	glBindTexture(GL_TEXTURE_2D, this->renderTarget->textureID[0]);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
#ifdef USE_GLES1
	glOrthof(-0.5, 0.5, -0.5, 0.5, -40, 40);
#else
	glOrtho(-0.5, 0.5, -0.5, 0.5, -40, 40);
#endif
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glLineWidth(this->renderTarget->texsize < 512 ? 1 : this->renderTarget->texsize / 512.0);

	CompositeOutput(pipeline, pipelineContext);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(-0.5, -0.5, 0);
#if 0
	// When console refreshes, there is a chance the preset has been changed by the user
	refreshConsole();
	draw_title_to_screen(false);
	if (this->showhelp % 2)
		draw_help();
	if (this->showtitle % 2)
		draw_title();
	if (this->showfps % 2)
		draw_fps(this->realfps);
	if (this->showpreset % 2)
		draw_preset();
	if (this->showstats % 2)
		draw_stats();
	glTranslatef(0.5, 0.5, 0);
#endif
	if (renderTarget->renderToTexture)
		glBindFramebufferOES(GL_FRAMEBUFFER_OES, this->renderTarget->framebuffer);
}

void Renderer::RenderFrame(const Pipeline &pipeline, const PipelineContext &pipelineContext)
{
	SetupPass1(pipeline, pipelineContext);

#ifdef USE_CG
	shaderEngine.enableShader(currentPipe->warpShader, pipeline, pipelineContext);
#endif
	Interpolation(pipeline);
#ifdef USE_CG
	shaderEngine.disableShader();
#endif

	RenderItems(pipeline, pipelineContext);

	FinishPass1();
	Pass2(pipeline, pipelineContext);
}

void Renderer::Interpolation(const Pipeline &pipeline)
{
	if (this->renderTarget->useFBO && !renderTarget->enableFBOFast)
		glBindTexture(GL_TEXTURE_2D, renderTarget->textureID[1]);
	else
		glBindTexture(GL_TEXTURE_2D, renderTarget->textureID[0]);

	//Texture wrapping( clamp vs. wrap)
	if (pipeline.textureWrap == 0)
	{
#ifdef USE_GLES1
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
#else
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
#endif
	}
	else
	{
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}

	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();

	glBlendFunc(GL_SRC_ALPHA, GL_ZERO);

	glColor4f(1.0, 1.0, 1.0, pipeline.screenDecay);

	glEnable(GL_TEXTURE_2D);


	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);


	//glVertexPointer(2, GL_FLOAT, 0, p);
	//glTexCoordPointer(2, GL_FLOAT, 0, t);
		glTexCoordPointer(2, GL_FLOAT, sizeof(GLfloat) * 5, (GLfloat*)p);
		glVertexPointer(3, GL_FLOAT, sizeof(GLfloat) * 5, (GLfloat*)p + 2);


	if (pipeline.staticPerPixel)
	{
		int base = 0;
		for (int j = 0; j < mesh.height - 1; j++, base += mesh.width * 2 * 5)
		{
			int strip = base;

			for (int i = 0; i < mesh.width; i++, strip += 10)
			{
				p[strip] = pipeline.x_mesh[i][j];
				p[strip + 1] = pipeline.y_mesh[i][j];

				p[strip + 5] = pipeline.x_mesh[i][j+1];
				p[strip + 6] = pipeline.y_mesh[i][j+1];
			}
		}

	}
	else
	{
		mesh.Reset();
		omptl::transform(mesh.p.begin(), mesh.p.end(), mesh.identity.begin(), mesh.p.begin(), &Renderer::PerPixel);
	int base = 0;
	int index = 0;
	for (int j = 0; j < mesh.height - 1; j++, base += mesh.width * 2 * 5)
	{
		int strip = base;
		int index2 = index + mesh.width;
		for (int i = 0; i < mesh.width; i++, strip += 10, index++, index2++)
		{
//			int index = j * mesh.width + i;
//			int index2 = (j + 1) * mesh.width + i;

			p[strip] = mesh.p[index].x;
			p[strip + 1] = mesh.p[index].y;

			p[strip + 5] = mesh.p[index2].x;
			p[strip + 6] = mesh.p[index2].y;


		}
	}

	}

	for (int j = 0; j < mesh.height - 1; j++)
		glDrawArrays(GL_TRIANGLE_STRIP,j* mesh.width* 2,mesh.width*2);


	glDisable(GL_TEXTURE_2D);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

}
Pipeline* Renderer::currentPipe;

Renderer::~Renderer()
{
	if (renderTarget)
		delete (renderTarget);
	if (textureManager)
		delete (textureManager);

	//std::cerr << "grid assign end" << std::endl;

	free(p);

#ifdef USE_FTGL
	//	std::cerr << "freeing title fonts" << std::endl;
	if (title_font)
		delete title_font;
	if (poly_font)
		delete poly_font;
	if (other_font)
		delete other_font;
	//	std::cerr << "freeing title fonts finished" << std::endl;
#endif
	//	std::cerr << "exiting destructor" << std::endl;
}

void Renderer::reset(int w, int h)
{
	aspect = (float) h / (float) w;
	this -> vw = w;
	this -> vh = h;

#if USE_CG
	shaderEngine.setAspect(aspect);
#endif

	glShadeModel(GL_SMOOTH);

	glCullFace(GL_BACK);
	//glFrontFace( GL_CCW );

	glClearColor(0, 0, 0, 0);

	glViewport(0, 0, w, h);

	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

#ifndef USE_GLES1
	glDrawBuffer(GL_BACK);
	glReadBuffer(GL_BACK);
#endif
	glEnable(GL_BLEND);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_LINE_SMOOTH);

	glEnable(GL_POINT_SMOOTH);
	glClear(GL_COLOR_BUFFER_BIT);

#ifndef USE_GLES1
	glLineStipple(2, 0xAAAA);
#endif

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	//glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	//glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
	//glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	if (!this->renderTarget->useFBO)
	{
		this->renderTarget->fallbackRescale(w, h);
	}
}

GLuint Renderer::initRenderToTexture()
{
	return renderTarget->initRenderToTexture();
}

void Renderer::draw_title_to_texture()
{
}

void Renderer::draw_title_to_screen(bool flip)
{
}

void Renderer::draw_title()
{
}

void Renderer::draw_preset()
{
}

void Renderer::draw_help()
{
}

void Renderer::draw_stats()
{
}
void Renderer::draw_fps(float realfps)
{
}

void Renderer::CompositeOutput(const Pipeline &pipeline, const PipelineContext &pipelineContext)
{

	glMatrixMode(GL_TEXTURE);
	glLoadIdentity();

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	//Overwrite anything on the screen
	glBlendFunc(GL_ONE, GL_ZERO);
	glColor4f(1.0, 1.0, 1.0, 1.0f);

	glEnable(GL_TEXTURE_2D);

#ifdef USE_CG
	shaderEngine.enableShader(currentPipe->compositeShader, pipeline, pipelineContext);
#endif

	float tex[4][2] =
	{
	{ 0, 1 },
	{ 0, 0 },
	{ 1, 0 },
	{ 1, 1 } };

	float points[4][2] =
	{
	{ -0.5, -0.5 },
	{ -0.5, 0.5 },
	{ 0.5, 0.5 },
	{ 0.5, -0.5 } };

	glEnableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

#ifndef FBO_HACK
	glVertexPointer(2, GL_FLOAT, 0, points);
	glTexCoordPointer(2, GL_FLOAT, 0, tex);

	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
#endif

	glDisable(GL_TEXTURE_2D);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

#ifdef USE_CG
	shaderEngine.disableShader();
#endif

	for (std::vector<RenderItem*>::const_iterator pos = pipeline.compositeDrawables.begin(); pos
			!= pipeline.compositeDrawables.end(); ++pos)
		(*pos)->Draw(renderContext);

}

