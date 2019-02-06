#define _USE_MATH_DEFINES
#include <iostream>
#include <fstream>
#include "GL/glew.h"
#include "GL/3dgl.h"
#include "GL/glut.h"
#include "GL/freeglut_ext.h"

#pragma comment (lib, "glew32.lib")

using namespace std;
using namespace _3dgl;

// 3D Models
C3dglTerrain terrain, water;
C3dglSkyBox skybox;
C3dglSkyBox skybox1;
C3dglModel house;
C3dglModel streetlamp;
C3dglModel bulb;
C3dglModel bulb1;

// texture ids
GLuint idTexGrass;		// grass texture
GLuint idTexSand;		// sand texture
GLuint idTexWater;		// water texture
GLuint idTexLight;		// light texture
GLuint idTexNone;
// model textures
GLuint idTexParticle;	// rain drop texture
GLuint idTexHouse;      // house texture
GLuint idTexLamp;

// GLSL Objects (Shader Program)
C3dglProgram ProgramBasic;
C3dglProgram ProgramWater;
C3dglProgram ProgramTerrain;
C3dglProgram ProgramParticle;

// Particle System Params
const float PERIOD = 0.0000075f;
const float LIFETIME = 5.0;
const int NPARTICLES = (int)(LIFETIME / PERIOD);

// Particle Buffer ids
GLuint idBufferInitialPos;
GLuint idBufferVelocity;
GLuint idBufferStartTime;

// Water specific variables
float waterLevel = 4.6f;

// camera position (for first person type camera navigation)
float matrixView[16];		// The View Matrix
float angleTilt = 0;		// Tilt Angle
float deltaX = 0, deltaY = 0, deltaZ = 0;	// Camera movement values

bool init()
{
	// rendering states
	glEnable(GL_DEPTH_TEST);	// depth test is necessary for most 3D scenes
	glEnable(GL_NORMALIZE);		// normalization is needed by AssImp library models
	glShadeModel(GL_SMOOTH);	// smooth shading mode is the default one; try GL_FLAT here!
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);	// this is the default one; try GL_LINE!
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// switch on: transparency/blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glPointSize(5);
	glEnable(GL_POINT_SPRITE);
	glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);

	// Initialise Shaders
	C3dglShader VertexShader;
	C3dglShader FragmentShader;

	// initialise basic shaders
	if (!VertexShader.Create(GL_VERTEX_SHADER)) return false;
	if (!VertexShader.LoadFromFile("shaders/basic.vert")) return false;
	if (!VertexShader.Compile()) return false;

	if (!FragmentShader.Create(GL_FRAGMENT_SHADER)) return false;
	if (!FragmentShader.LoadFromFile("shaders/basic.frag")) return false;
	if (!FragmentShader.Compile()) return false;

	if (!ProgramBasic.Create()) return false;
	if (!ProgramBasic.Attach(VertexShader)) return false;
	if (!ProgramBasic.Attach(FragmentShader)) return false;
	if (!ProgramBasic.Link()) return false;
	if (!ProgramBasic.Use(true)) return false;

	// Initialise Shader - Particle
	if (!VertexShader.Create(GL_VERTEX_SHADER)) return false;
	if (!VertexShader.LoadFromFile("shaders/particles.vert")) return false;
	if (!VertexShader.Compile()) return false;

	if (!FragmentShader.Create(GL_FRAGMENT_SHADER)) return false;
	if (!FragmentShader.LoadFromFile("shaders/particles.frag")) return false;
	if (!FragmentShader.Compile()) return false;

	if (!ProgramParticle.Create()) return false;
	if (!ProgramParticle.Attach(VertexShader)) return false;
	if (!ProgramParticle.Attach(FragmentShader)) return false;
	if (!ProgramParticle.Link()) return false;
	if (!ProgramParticle.Use(true)) return false;

	// Initialise Shader - Water
	if (!VertexShader.Create(GL_VERTEX_SHADER)) return false;
	if (!VertexShader.LoadFromFile("shaders/water.vert")) return false;
	if (!VertexShader.Compile()) return false;

	if (!FragmentShader.Create(GL_FRAGMENT_SHADER)) return false;
	if (!FragmentShader.LoadFromFile("shaders/water.frag")) return false;
	if (!FragmentShader.Compile()) return false;

	if (!ProgramWater.Create()) return false;
	if (!ProgramWater.Attach(VertexShader)) return false;
	if (!ProgramWater.Attach(FragmentShader)) return false;
	if (!ProgramWater.Link()) return false;
	if (!ProgramWater.Use(true)) return false;

	// Initialise Shader - Terrain
	if (!VertexShader.Create(GL_VERTEX_SHADER)) return false;
	if (!VertexShader.LoadFromFile("shaders/terrain.vert")) return false;
	if (!VertexShader.Compile()) return false;

	if (!FragmentShader.Create(GL_FRAGMENT_SHADER)) return false;
	if (!FragmentShader.LoadFromFile("shaders/terrain.frag")) return false;
	if (!FragmentShader.Compile()) return false;

	if (!ProgramTerrain.Create()) return false;
	if (!ProgramTerrain.Attach(VertexShader)) return false;
	if (!ProgramTerrain.Attach(FragmentShader)) return false;
	if (!ProgramTerrain.Link()) return false;
	if (!ProgramTerrain.Use(true)) return false;

	// glut additional setup
	glutSetVertexAttribCoord3(ProgramBasic.GetAttribLocation("aVertex"));
	glutSetVertexAttribNormal(ProgramBasic.GetAttribLocation("aNormal"));

	// load 3D models
	if (!terrain.loadHeightmap("models\\heightmap2.png", 10)) return false;
	if (!water.loadHeightmap("models\\watermap.png", 10)) return false;
	if (!bulb.load("models\\bulb.obj")) return false;
	if (!bulb1.load("models\\bulb.obj")) return false;
	if (!streetlamp.load("models\\StreetLight.obj")) return false;
	if (!house.load("models\\medievalhouse.obj")) return false;
	
	// load Sky Box
	if (!skybox.load("models\\FullMoon\\FullMoonFront2048.png",
		"models\\FullMoon\\FullMoonLeft2048.png",
		"models\\FullMoon\\FullMoonBack2048.png",
		"models\\FullMoon\\FullMoonRight2048.png",
		"models\\FullMoon\\FullMoonUp2048.png",
		"models\\FullMoon\\FullMoonDown2048.png")) return false;

	if (!skybox1.load("models\\TropicalSunnyDay\\TropicalSunnyDayFront1024.jpg",
		"models\\TropicalSunnyDay\\TropicalSunnyDayLeft1024.jpg",
		"models\\TropicalSunnyDay\\TropicalSunnyDayBack1024.jpg",
		"models\\TropicalSunnyDay\\TropicalSunnyDayRight1024.jpg",
		"models\\TropicalSunnyDay\\TropicalSunnyDayUp1024.jpg",
		"models\\TropicalSunnyDay\\TropicalSunnyDayDown1024.jpg")) return false;


	// Prepare the particle buffers
	std::vector<float> bufferVelocity;
	std::vector<float> bufferStartTime;
	std::vector<float> bufferInitialPos;
	float time = 0;
	for (int i = 0; i < NPARTICLES; i++)
	{
		float v = 2.5f + 0.2f * (float)rand() / (float)RAND_MAX;

		bufferVelocity.push_back(-0.5f * v);
		bufferVelocity.push_back(-v);
		bufferVelocity.push_back(0.25f * v);
		bufferStartTime.push_back(time);
		time += PERIOD;

		bufferInitialPos.push_back(80.f * (float)rand() / (float)RAND_MAX - 40);
		bufferInitialPos.push_back(12);
		bufferInitialPos.push_back(80.f * (float)rand() / (float)RAND_MAX - 40);
	}

	glGenBuffers(1, &idBufferInitialPos);
	glBindBuffer(GL_ARRAY_BUFFER, idBufferInitialPos);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * bufferInitialPos.size(), &bufferInitialPos[0], GL_STATIC_DRAW);

	glGenBuffers(1, &idBufferVelocity);
	glBindBuffer(GL_ARRAY_BUFFER, idBufferVelocity);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * bufferVelocity.size(), &bufferVelocity[0], GL_STATIC_DRAW);

	glGenBuffers(1, &idBufferStartTime);
	glBindBuffer(GL_ARRAY_BUFFER, idBufferStartTime);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * bufferStartTime.size(), &bufferStartTime[0], GL_STATIC_DRAW);

	// Setup the particle system
	ProgramParticle.SendUniform("gravity", 0.0, 0.0, 0.0);
	ProgramParticle.SendUniform("particleLifetime", LIFETIME);

	// create & load textures
	C3dglBitmap bm;
    glActiveTexture(GL_TEXTURE0);
	
	// Grass texture
	bm.Load("models/grass.png", GL_RGBA);
	glGenTextures(1, &idTexGrass);
	glBindTexture(GL_TEXTURE_2D, idTexGrass);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bm.GetWidth(), bm.GetHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, bm.GetBits());

	// Sand texture
	bm.Load("models/sand.png", GL_RGBA);
	glGenTextures(1, &idTexSand);
	glBindTexture(GL_TEXTURE_2D, idTexSand);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bm.GetWidth(), bm.GetHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, bm.GetBits());

	// Water texture
	bm.Load("models/water.png", GL_RGBA);
	glGenTextures(1, &idTexWater);
	glBindTexture(GL_TEXTURE_2D, idTexWater);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bm.GetWidth(), bm.GetHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, bm.GetBits());

	// Rain texture
	C3dglBitmap bmWater("models/water.png", GL_RGBA);
	glGenTextures(1, &idTexParticle);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, idTexParticle);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bmWater.GetWidth(), bmWater.GetHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, bmWater.GetBits());

	// House texture
	bm.Load("models/HouseDif.jpg", GL_RGBA);
	glGenTextures(1, &idTexHouse);
	glBindTexture(GL_TEXTURE_2D, idTexHouse);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bm.GetWidth(), bm.GetHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, bm.GetBits());

	// Streetlamp texture
	bm.Load("models/StreetLightDif.png", GL_RGBA);
	glGenTextures(1, &idTexLamp);
	glBindTexture(GL_TEXTURE_2D, idTexLamp);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bm.GetWidth(), bm.GetHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, bm.GetBits());

	// Light texture
	bm.Load("models/white.png", GL_RGBA);
	glGenTextures(1, &idTexLight);
	glBindTexture(GL_TEXTURE_2D, idTexLight);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bm.GetWidth(), bm.GetHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, bm.GetBits());

	// none (simple-white) texture
	glGenTextures(1, &idTexNone);
	glBindTexture(GL_TEXTURE_2D, idTexNone);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	BYTE bytes[] = { 255, 255, 255 };
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_BGR, GL_UNSIGNED_BYTE, &bytes);

	// setup the textures
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, idTexSand);
	ProgramTerrain.SendUniform("textureBed", 1);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, idTexGrass);
	ProgramTerrain.SendUniform("textureShore", 2);


	// Send the texture info to the shaders
	ProgramBasic.SendUniform("texture0", 0);

	ProgramTerrain.SendUniform("texture0", 0);
	ProgramTerrain.SendUniform("waterColor", 0.2f, 0.22f, 0.02f);
	ProgramTerrain.SendUniform("waterLevel", waterLevel);

	ProgramWater.SendUniform("texture0", 0);
	ProgramWater.SendUniform("waterColor", 0.2f, 0.22f, 0.02f);
	ProgramWater.SendUniform("skyColor", 0.0f, 0.21f, 0.47f);

	// setup lights (for basic and terrain programs only, water does not use these lights):
	ProgramBasic.SendUniform("lightAmbient.on", 1);
	ProgramBasic.SendUniform("lightAmbient.color", 0.1, 0.1, 0.1);
	ProgramBasic.SendUniform("lightDir.on", 1);
	ProgramBasic.SendUniform("lightDir.direction", 0.5, 0.5, 0.5);
	ProgramBasic.SendUniform("lightDir.diffuse", 1.0, 1.0, 1.0);
	ProgramTerrain.SendUniform("lightAmbient.on", 1);
	ProgramTerrain.SendUniform("lightAmbient.color", 0.1, 0.1, 0.1);
	ProgramTerrain.SendUniform("lightDir.on", 1);
	ProgramTerrain.SendUniform("lightDir.direction", 1.0, 0.5, 1.0);
	ProgramTerrain.SendUniform("lightDir.diffuse", 1.0, 1.0, 1.0);
	// spotlight 1
	ProgramBasic.SendUniform("lightSpot.on", 1);
	ProgramBasic.SendUniform("lightSpot.position", -18.5f, 7.1f, 2.0f);
	ProgramBasic.SendUniform("lightSpot.diffuse", 1.0, 1.0, 1.0);
	ProgramBasic.SendUniform("lightSpot.specular", 1.0, 1.0, 1.0);
	ProgramBasic.SendUniform("lightSpot.direction", 0.0, 1.0, 0.0);
	ProgramBasic.SendUniform("lightSpot.cutoff", 180.0);
	ProgramBasic.SendUniform("lightSpot.attenuation", 10.0);
	ProgramBasic.SendUniform("materialSpecular", 0.0, 0.0, 0.0);
	ProgramBasic.SendUniform("shininess", 3.0);
	// spotlight 2
	ProgramBasic.SendUniform("lightSpot1.on", 1);
	ProgramBasic.SendUniform("lightSpot1.position", -18.5f, 5.0f, 0.0f);
	ProgramBasic.SendUniform("lightSpot1.diffuse", 1.0, 1.0, 1.0);
	ProgramBasic.SendUniform("lightSpot1.specular", 1.0, 1.0, 1.0);
	ProgramBasic.SendUniform("lightSpot1.direction", 0.0, -1.0, 0.0);
	ProgramBasic.SendUniform("lightSpot1.cutoff", 45.0);
	ProgramBasic.SendUniform("lightSpot1.attenuation", 7.0);

	// setup the fog
	ProgramBasic.SendUniform("fogColour", 0.4f, 0.4f, 0.4f);
	ProgramBasic.SendUniform("fogDensity", 0.2);

	// setup materials (for basic and terrain programs only, water does not use these materials):
	ProgramBasic.SendUniform("materialAmbient", 0.5, 0.5, 0.5);		
	ProgramBasic.SendUniform("materialDiffuse", 0.75, 0.75, 0.75);
	ProgramTerrain.SendUniform("materialAmbient", 0.5, 0.5, 0.5);		
	ProgramTerrain.SendUniform("materialDiffuse", 0.75, 0.75, 0.75);

	// Initialise the View Matrix (initial position for the first-person camera)
	glMatrixMode(GL_MODELVIEW);
	angleTilt = 15;
	glLoadIdentity();
	glRotatef(angleTilt, 1, 0, 0);
	gluLookAt(4.0, 0.4, 30.0, 
	          4.0, 0.4, 0.0,
	          0.0, 1.0, 0.0);
	glGetFloatv(GL_MODELVIEW_MATRIX, matrixView);

	cout << endl;
	cout << "Use:" << endl;
	cout << "  WASD or arrow key to navigate" << endl;
	cout << "  QE or PgUp/Dn to move the camera up and down" << endl;
	cout << "  Drag the mouse to look around" << endl;
	cout << endl;

	// initialise animation clock
	time = GetTickCount();

	return true;
}

void done()
{
}

void render()
{
	// this global variable controls the animation
	static float theta = 0.0f;
	
	float matrix[16];

	// clear screen and buffers
	glClearColor(0.2f, 0.6f, 1.f, 1.0f);   // blue sky colour
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// setup the View Matrix (camera)
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glRotatef(angleTilt, 1, 0, 0);					// switch tilt off
	glTranslatef(deltaX, deltaY, deltaZ);			// animate camera motion (controlled by WASD keys)
	glRotatef(-angleTilt, 1, 0, 0);					// switch tilt on
	glMultMatrixf(matrixView);
	glGetFloatv(GL_MODELVIEW_MATRIX, matrixView);

	// set the camera height above the ground
	gluInvertMatrix(matrixView, matrix);
	glTranslatef(0, -max(terrain.getInterpolatedHeight(matrix[12], matrix[14]), waterLevel),  0);

	// setup View Matrix
	glGetFloatv(GL_MODELVIEW_MATRIX, matrix);
	ProgramBasic.SendUniform("matrixView", matrix);
	ProgramTerrain.SendUniform("matrixView", matrix);

	// render the skybox
	ProgramBasic.SendUniform("lightAmbient.color", 1.0, 1.0, 1.0);
	ProgramBasic.SendUniform("materialAmbient", 1.0, 1.0, 1.0);
	ProgramBasic.SendUniform("materialDiffuse", 0.0, 0.0, 0.0);
	skybox.render();
	ProgramBasic.SendUniform("lightAmbient.color", 0.05, 0.05, 0.1);
	ProgramBasic.SendUniform("materialDiffuse", 1.0, 1.0, 1.0);

	// render the streetlamp
	glPushMatrix();
	glTranslatef(-18.5f, 5.75f, 2.0f);
	glScalef(0.1f, 0.1f, 0.1f);
	glRotatef(90.0f, 0.0f, 90.0f, 0.0f);
	glBindTexture(GL_TEXTURE_2D, idTexLamp);
	streetlamp.render();
	glPopMatrix();

	// light bulb
	glPushMatrix();
	ProgramBasic.SendUniform("materialDiffuse", 0.2, 0.2, 0.2);
	ProgramBasic.SendUniform("materialAmbient", 0.2, 0.2, 0.2);
	ProgramBasic.SendUniform("lightSpot.on", 1);
	glTranslatef(-18.5f, 7.1f, 2.0f);
	glScalef(0.07f, 0.07f, 0.07f);
	glGetFloatv(GL_MODELVIEW_MATRIX, matrix);
	ProgramBasic.SendUniform("matrixModelView", matrix);
	glBindTexture(GL_TEXTURE_2D, idTexLight);
	bulb.render();
	ProgramBasic.SendUniform("lightSpot.on", 1);
	glPopMatrix();

	// light bulb 1
	glPushMatrix();
	ProgramBasic.SendUniform("materialDiffuse", 0.2, 0.2, 0.2);
	ProgramBasic.SendUniform("materialAmbient", 0.2, 0.2, 0.2);
	ProgramBasic.SendUniform("lightSpot1.on", 1);
	glTranslatef(-15.5f, 5.0f, 5.0f);
	glScalef(0.05f, 0.05f, 0.05f);
	glGetFloatv(GL_MODELVIEW_MATRIX, matrix);
	ProgramBasic.SendUniform("matrixModelView", matrix);
	glBindTexture(GL_TEXTURE_2D, idTexLight);
	bulb1.render();
	ProgramBasic.SendUniform("lightSpot1.on", 1);
	glPopMatrix();

	// render the house
	glPushMatrix();
	glTranslatef(-20.0f, 8.0f, 0.0f);
	glScalef(0.3f, 0.3f, 0.3f);
	glRotatef(90.0f, 0.0f, 90.0f, 0.0f);
	glBindTexture(GL_TEXTURE_2D, idTexHouse);
	house.render();
	glPopMatrix();

	// render the terrain
	ProgramTerrain.Use();
	glPushMatrix();
	glGetFloatv(GL_MODELVIEW_MATRIX, matrix);
	ProgramTerrain.SendUniform("matrixModelView", matrix);
	terrain.render();
	glPopMatrix();
	
	// setup the grass texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, idTexGrass);

	// setup the water texture
	glBindTexture(GL_TEXTURE_2D, idTexWater);

	// send the animation time to shaders
	ProgramWater.SendUniform("t", glutGet(GLUT_ELAPSED_TIME) / 1000.f);

	// render the water
	ProgramWater.Use();
	glPushMatrix();
	glTranslatef(0, waterLevel, 0);
	glScalef(0.5f, 1.0f, 0.5f);
	glGetFloatv(GL_MODELVIEW_MATRIX, matrix);
	ProgramWater.SendUniform("matrixModelView", matrix);
	water.render();
	glPopMatrix();

	// setup the rain drop texture
	glBindTexture(GL_TEXTURE_2D, idTexParticle);

	///////////////////////////////////
	// RENDER THE PARTICLE SYSTEM

	ProgramParticle.Use();
	glDepthMask(GL_FALSE);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, idTexParticle);

	glGetFloatv(GL_MODELVIEW_MATRIX, matrix);
	ProgramParticle.SendUniform("matrixModelView", matrix);

	ProgramParticle.SendUniform("time", glutGet(GLUT_ELAPSED_TIME) / 1000.f - 2);

	// render the buffer
	glEnableVertexAttribArray(0);	// initial position
	glEnableVertexAttribArray(1);	// velocity
	glEnableVertexAttribArray(2);	// start time
	glBindBuffer(GL_ARRAY_BUFFER, idBufferInitialPos);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glBindBuffer(GL_ARRAY_BUFFER, idBufferVelocity);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glBindBuffer(GL_ARRAY_BUFFER, idBufferStartTime);
	glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, 0, 0);
	glDrawArrays(GL_POINTS, 0, NPARTICLES);
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);

	// revert to normal
	glDepthMask(GL_TRUE);
	// END OF PARTICLE SYSTEM RENDERING
	///////////////////////////////////


	// Basic Shader is not currently in use...
	ProgramBasic.Use();

	
	// essential for double-buffering technique
	glutSwapBuffers();

	// proceed the animation
	glutPostRedisplay();
}

// called before window opened or resized - to setup the Projection Matrix
void reshape(int w, int h)
{
	// find screen aspect ratio
	float ratio =  w * 1.0f / h;      // we hope that h is not zero

	// setup the projection matrix
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glViewport(0, 0, w, h);
	gluPerspective(60.0, ratio, 0.02, 1000.0);

	float matrix[16];
	glGetFloatv(GL_PROJECTION_MATRIX, matrix);
	ProgramBasic.SendUniform("matrixProjection", matrix);
	ProgramTerrain.SendUniform("matrixProjection", matrix);
	ProgramWater.SendUniform("matrixProjection", matrix);

	ProgramParticle.SendUniform("matrixProjection", matrix);
	ProgramParticle.SendUniform("scaleFactor", (float)h / 720.f);
}

// Handle WASDQE keys
void onKeyDown(unsigned char key, int x, int y)
{
	switch (tolower(key))
	{
	case 'w': deltaZ = max(deltaZ * 1.05f, 0.01f); break;
	case 's': deltaZ = min(deltaZ * 1.05f, -0.01f); break;
	case 'a': deltaX = max(deltaX * 1.05f, 0.01f); break;
	case 'd': deltaX = min(deltaX * 1.05f, -0.01f); break;
	case 'e': deltaY = max(deltaY * 1.05f, 0.01f); break;
	case 'q': deltaY = min(deltaY * 1.05f, -0.01f); break; 
	}
	// speed limit
	deltaX = max(-0.15f, min(0.15f, deltaX));
	deltaY = max(-0.15f, min(0.15f, deltaY));
	deltaZ = max(-0.15f, min(0.15f, deltaZ));
}

// Handle WASDQE keys (key up)
void onKeyUp(unsigned char key, int x, int y)
{
	switch (tolower(key))
	{
	case 'w':
	case 's': deltaZ = 0; break;
	case 'a':
	case 'd': deltaX = 0; break;
	case 'q':
	case 'e': deltaY = 0; break;
	case ' ': deltaY = 0; break;	
	}
}

// Handle arrow keys and Alt+F4
void onSpecDown(int key, int x, int y)
{
	switch (key)
	{
	case GLUT_KEY_F4:		if ((glutGetModifiers() & GLUT_ACTIVE_ALT) != 0) exit(0); break;
	case GLUT_KEY_UP:		onKeyDown('w', x, y); break;
	case GLUT_KEY_DOWN:		onKeyDown('s', x, y); break;
	case GLUT_KEY_LEFT:		onKeyDown('a', x, y); break;
	case GLUT_KEY_RIGHT:	onKeyDown('d', x, y); break;
	case GLUT_KEY_PAGE_UP:	onKeyDown('q', x, y); break;
	case GLUT_KEY_PAGE_DOWN:onKeyDown('e', x, y); break;
	case GLUT_KEY_F11:		glutFullScreenToggle();
	}
}

// Handle arrow keys (key up)
void onSpecUp(int key, int x, int y)
{
	switch (key)
	{
	case GLUT_KEY_UP:		onKeyUp('w', x, y); break;
	case GLUT_KEY_DOWN:		onKeyUp('s', x, y); break;
	case GLUT_KEY_LEFT:		onKeyUp('a', x, y); break;
	case GLUT_KEY_RIGHT:	onKeyUp('d', x, y); break;
	case GLUT_KEY_PAGE_UP:	onKeyUp('q', x, y); break;
	case GLUT_KEY_PAGE_DOWN:onKeyUp('e', x, y); break;
	}
}

// Handle mouse click
void onMouse(int button, int state, int x, int y)
{
	int cx = glutGet(GLUT_WINDOW_WIDTH) / 2;
	int cy = glutGet(GLUT_WINDOW_HEIGHT) / 2;

	if (state == GLUT_DOWN)
	{
		glutSetCursor(GLUT_CURSOR_CROSSHAIR);
		glutWarpPointer(cx, cy);
	}
	else
		glutSetCursor(GLUT_CURSOR_INHERIT);
}

// handle mouse move
void onMotion(int x, int y)
{
	int cx = glutGet(GLUT_WINDOW_WIDTH) / 2;
	int cy = glutGet(GLUT_WINDOW_HEIGHT) / 2;
	if (x == cx && y == cy) 
		return;	// caused by glutWarpPointer

	float amp = 0.25;
	float deltaTilt = amp * (y - cy);
	float deltaPan  = amp * (x - cx);

	glutWarpPointer(cx, cy);

	// handle camera tilt (mouse move up & down)
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glRotatef(deltaTilt, 1, 0, 0);
	glMultMatrixf(matrixView);
	glGetFloatv(GL_MODELVIEW_MATRIX, matrixView);

	angleTilt += deltaTilt;

	// handle camera pan (mouse move left & right)
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glRotatef(angleTilt, 1, 0, 0);
	glRotatef(deltaPan, 0, 1, 0);
	glRotatef(-angleTilt, 1, 0, 0);
	glMultMatrixf(matrixView);
	glGetFloatv(GL_MODELVIEW_MATRIX, matrixView);
}

int main(int argc, char **argv)
{
	// init GLUT and create Window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(1080, 720);
	glutCreateWindow("Alfie B. Taylor - CI5520 3D Graphics Programming");

	// init glew
	GLenum err = glewInit();
	if (GLEW_OK != err)
	{
		cerr << "GLEW Error: " << glewGetErrorString(err) << endl;
		return 0;
	}
	cout << "Using GLEW " << glewGetString(GLEW_VERSION) << endl;

	// register callbacks
	glutDisplayFunc(render);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(onKeyDown);
	glutSpecialFunc(onSpecDown);
	glutKeyboardUpFunc(onKeyUp);
	glutSpecialUpFunc(onSpecUp);
	glutMouseFunc(onMouse);
	glutMotionFunc(onMotion);

	cout << "Vendor: " << glGetString(GL_VENDOR) << endl;
	cout << "Renderer: " << glGetString(GL_RENDERER) << endl;
	cout << "Version: " << glGetString(GL_VERSION) << endl;

	// init light and everything – not a GLUT or callback function!
	if (!init())
	{
		cerr << "Application failed to initialise" << endl;
		return 0;
	}

	// enter GLUT event processing cycle
	glutMainLoop();

	done();

	return 1;
}

