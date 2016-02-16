#ifdef _WINDOWS
	#include <GL/glew.h>
#endif
#include <SDL.h>
#include "ShaderProgram.h"
#include <SDL_opengl.h>
#include <SDL_image.h>
#include "Matrix.h"

#ifdef _WINDOWS
	#define RESOURCE_FOLDER ""
#else
	#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

//code by Brian Kravets



SDL_Window* displayWindow;
GLuint LoadTexture(const char *image_path){
	SDL_Surface *surface = IMG_Load(image_path);
	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface->w, surface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, surface->pixels);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	SDL_FreeSurface(surface);
	return textureID;
}



int main(int argc, char *argv[])
{
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);
	#ifdef _WINDOWS
		glewInit();
	#endif
		
	SDL_Event event;
	//glClearColor(0.4f, 0.2f, 0.4f, 1.0f);
	bool done = false;


	glViewport(0, 0, 640, 360);
	ShaderProgram program(RESOURCE_FOLDER"vertex.glsl", RESOURCE_FOLDER"fragment.glsl");


	float lastFrameTicks = 0.0f;
	float yPos1 = 0;//y pos player 1
	float yPos2 = 0;//y pos player 2
	float xbpos = 0;// ball x position
	float ybpos = 0;//ball y position
	float Speed = -10;//initial speed of ball
	float angle = 0;//balls direction vector
	float pi = 3.14159265359;
	float distanceToTravel = 2.2;
	Matrix projectionMatrix;
	Matrix modelMatrix;
	Matrix viewMatrix;

	projectionMatrix.setOrthoProjection(-3.55f, 3.55f, -2.0f, 2.0f, -1.0f, 1.0f);
	glUseProgram(program.programID);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	while (!done) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
				done = true;
			}


		}
		glClear(GL_COLOR_BUFFER_BIT);
		float ticks = (float)SDL_GetTicks() / 1000.0f;
		float elapsed = ticks - lastFrameTicks;
		lastFrameTicks = ticks;
		//player 1
		const Uint8 *keys = SDL_GetKeyboardState(NULL);
		if (keys[SDL_SCANCODE_W]){
			if (yPos1 < 2.2){
				yPos1 += elapsed*distanceToTravel;
			}
		}
		if (keys[SDL_SCANCODE_S]){
			if (yPos1 > -2.2){
				yPos1 -= elapsed*distanceToTravel;
			}

		}

		modelMatrix.identity();
		modelMatrix.Scale(.1, .7, 0);
		modelMatrix.Translate(-32, yPos1, 0);
		program.setModelMatrix(modelMatrix);
		program.setProjectionMatrix(projectionMatrix);
		program.setViewMatrix(viewMatrix);

		float vertices[] = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5 };
		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
		glEnableVertexAttribArray(program.positionAttribute);

		glDrawArrays(GL_TRIANGLES, 0, 6);
		glDisableVertexAttribArray(program.positionAttribute);
		//player 2
		if (keys[SDL_SCANCODE_UP]){
			if (yPos2 < 2.2){
				yPos2 += elapsed*distanceToTravel;
			}
		}
		if (keys[SDL_SCANCODE_DOWN]){
			if (yPos2 > -2.2){
				yPos2 -= elapsed*distanceToTravel;
			}

		}

		modelMatrix.identity();
		modelMatrix.Scale(.1, .7, 0);
		modelMatrix.Translate(32, yPos2, 0);
		program.setModelMatrix(modelMatrix);
		program.setProjectionMatrix(projectionMatrix);
		program.setViewMatrix(viewMatrix);

		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
		glEnableVertexAttribArray(program.positionAttribute);

		glDrawArrays(GL_TRIANGLES, 0, 6);
		glDisableVertexAttribArray(program.positionAttribute);
		//top wall
		modelMatrix.identity();
		modelMatrix.Scale(6.5, .1, 0);
		modelMatrix.Translate(0,20, 0);
		program.setModelMatrix(modelMatrix);
		program.setProjectionMatrix(projectionMatrix);
		program.setViewMatrix(viewMatrix);

		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
		glEnableVertexAttribArray(program.positionAttribute);

		glDrawArrays(GL_TRIANGLES, 0, 6);
		glDisableVertexAttribArray(program.positionAttribute);

		//bottom wall
		modelMatrix.identity();
		modelMatrix.Scale(6.5, .1, 0);
		modelMatrix.Translate(0, -20, 0);
		program.setModelMatrix(modelMatrix);
		program.setProjectionMatrix(projectionMatrix);
		program.setViewMatrix(viewMatrix);

		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
		glEnableVertexAttribArray(program.positionAttribute);

		glDrawArrays(GL_TRIANGLES, 0, 6);
		glDisableVertexAttribArray(program.positionAttribute);
		 
		
		//left paddle collision
		//each paddle's x cooridinate is fixed 32 units away from the center
		if (!((ybpos/7 - .1) >= (yPos1 + .5) || (ybpos/7 + .1) <= (yPos1 - .5) || (xbpos - .5) >= (-32 + .5) || (xbpos + .5) <= (-32 - .5))){
			float distFromCen = (ybpos/7) - yPos1;//times 7 to handle the different scaleing
				angle = (distFromCen)/.7   * 85 * (pi / 180) +pi;// ball bounces away from the center of the paddle. 85 degree cap, to prevent ball from going vertically.
				Speed *= 1.1; //speed increases with every bounce,to make the game get harder over time.
			
			
			

		}
	

		//right paddle collision
		if (!((ybpos/7 - .1) >= (yPos2 + .5) || (ybpos/7 + .1) <= (yPos2 - .5) || (xbpos - .5) >= (32 + .5) || (xbpos + .5) <= (32 - .5))){
			float distFromCen = (ybpos / 7) - yPos2;//divded by 7 to handle the different scaleing
				angle = (distFromCen) / .7 * -85 * (pi / 180);// ball bounces away from the center of the paddle. 85 degree cap, to prevent ball from going vertically.
				Speed *= 1.1;
			
		}
		//top wall collision
		if (!((ybpos - .5) >= (20 + .5) || (ybpos + .5) <= (20 - .5) || (xbpos - .5) >= (31) || (xbpos + .5) <= (-31))){
			//Speed = -10;
			//ybpos = 0;
			angle = 2*pi - angle ;
		}
		//bottom wall collision
		if (!((ybpos - .5) >= (-20 + .5) || (ybpos + .05) <= (-20 - .05) || (xbpos - .5) >= (30) || (xbpos + .5) <= (-30))){
			//Speed = 10;
			//ybpos = 0;
			angle = 2*pi - angle;
		}
		//wins on first point

		if (xbpos < -35){
			done = 1;
			
		}
		if (xbpos >35){
			done = 1;

		}
	
		xbpos +=cos(angle)*elapsed*Speed;
		ybpos +=sin(angle)*elapsed*Speed;
		modelMatrix.identity();
		modelMatrix.Scale(.1, .1, 0);
		modelMatrix.Translate(xbpos, ybpos, 0);
		program.setModelMatrix(modelMatrix);
		program.setProjectionMatrix(projectionMatrix);
		program.setViewMatrix(viewMatrix);

		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
		glEnableVertexAttribArray(program.positionAttribute);

		glDrawArrays(GL_TRIANGLES, 0, 6);
		glDisableVertexAttribArray(program.positionAttribute);

		// dotted center line
		int doty = 19;//y pos of dot in the dotted line

		for (int i = 0; i < 16; i++){//16 dots, as it provided a nice spacing
			
			modelMatrix.identity();
			modelMatrix.Scale(.1, .1, 0);
			modelMatrix.Translate(0, doty-i*(2.5), 0);//spacing of each dot
			program.setModelMatrix(modelMatrix);
			program.setProjectionMatrix(projectionMatrix);
			program.setViewMatrix(viewMatrix);

			glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
			glEnableVertexAttribArray(program.positionAttribute);

			glDrawArrays(GL_TRIANGLES, 0, 6);
			glDisableVertexAttribArray(program.positionAttribute);
		}
		SDL_GL_SwapWindow(displayWindow);
	}

	SDL_Quit();
	return 0;
}
