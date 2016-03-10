#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include "ShaderProgram.h"
#include <SDL_opengl.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include "Matrix.h"
#include "vector"
using namespace std;
#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

//code by Brian Kravets

SDL_Window* displayWindow;
class SheetSprite {
public:
	//SheetSprite();
	SheetSprite(ShaderProgram* program,unsigned int textureID, float u, float v, float width, float height, float
		size):program(program),textureID(textureID),u(u),v(v),width(width),height(height),size(size) {}
	void Draw();
	ShaderProgram* program;
	float size;
	unsigned int textureID;
	float u;
	float v;
	float width;
	float height;
};
void SheetSprite::Draw() {
	//glBindTexture(GL_TEXTURE_2D, textureID);
	GLfloat texCoords[] = {
		u, v + height,
		u + width, v,
		u, v,
		u + width, v,
		u, v + height,
		u + width, v + height
	};
	float aspect = width / height;
	float vertices[] = {
		-0.5f * size * aspect, -0.5f * size,
		0.5f * size * aspect, 0.5f * size,
		-0.5f * size * aspect, 0.5f * size,
		0.5f * size * aspect, 0.5f * size,
		-0.5f * size * aspect, -0.5f * size,
		0.5f * size * aspect, -0.5f * size };
	// draw our arrays
	glUseProgram(program->programID);
	glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
	glEnableVertexAttribArray(program->positionAttribute);
	glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
	glEnableVertexAttribArray(program->texCoordAttribute);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glDisableVertexAttribArray(program->positionAttribute);
	glDisableVertexAttribArray(program->texCoordAttribute);
}
class Entity {
public:
	Entity(float x, float y, float rotation, SheetSprite textureID, float width, float height/*, float speed*/) :x(x), y(y), rotation(rotation), textureID(textureID)/*,width(width),height(height)*/{}
	void Draw(){ textureID.Draw(); }
	float x;
	float y;
	float rotation;
	SheetSprite textureID;
	float width;//scaling done elsewhere
	float height;
	bool dead = false;
	bool collide(Entity ent){
		if (!((y  - .1) >= (ent.y + .1) || (y+ .1) <= (ent.y - .1) || (x - .1) >= (ent.x + .1) || (x + .1) <= (ent.x - .1))){
			return true;


		}
		else{ return false; }
	}
		
		//float speed;speed made global for use by all enemies
		//float direction_x;
		//float direction_y;
};
	float speed;
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

	void DrawText(ShaderProgram *program, int fontTexture, std::string text, float size, float spacing) {
		float texture_size = 1.0 / 16.0f;
		std::vector<float> vertexData;
		std::vector<float> texCoordData;
		for (int i = 0; i < text.size(); i++) {
			float texture_x = (float)(((int)text[i]) % 16) / 16.0f;
			float texture_y = (float)(((int)text[i]) / 16) / 16.0f;
			vertexData.insert(vertexData.end(), {
				((size + spacing) * i) + (-0.5f * size), 0.5f * size,
				((size + spacing) * i) + (-0.5f * size), -0.5f * size,
				((size + spacing) * i) + (0.5f * size), 0.5f * size,
				((size + spacing) * i) + (0.5f * size), -0.5f * size,
				((size + spacing) * i) + (0.5f * size), 0.5f * size,
				((size + spacing) * i) + (-0.5f * size), -0.5f * size,
			});
			texCoordData.insert(texCoordData.end(), {
				texture_x, texture_y,
				texture_x, texture_y + texture_size,
				texture_x + texture_size, texture_y,
				texture_x + texture_size, texture_y + texture_size,
				texture_x + texture_size, texture_y,
				texture_x, texture_y + texture_size,
			});
		}
		glUseProgram(program->programID);
		glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertexData.data());
		glEnableVertexAttribArray(program->positionAttribute);
		glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordData.data());
		glEnableVertexAttribArray(program->texCoordAttribute);
		glBindTexture(GL_TEXTURE_2D, fontTexture);
		glDrawArrays(GL_TRIANGLES, 0, text.size() * 6);
		glDisableVertexAttribArray(program->positionAttribute);
		glDisableVertexAttribArray(program->texCoordAttribute);
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
		ShaderProgram program(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
		speed = 0.1;
		//music initilization
		Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
		Mix_Chunk *laserSound;
		laserSound = Mix_LoadWAV("laser1.wav");
		Mix_Chunk *explosionSound;
		explosionSound = Mix_LoadWAV("explosion1.wav");
		Mix_Music *music;
		music = Mix_LoadMUS("BoxCat_Games_-_10_-_Epic_Song.mp3");
		Mix_PlayMusic(music, -1);

		//texture initilization
		GLuint text = LoadTexture("font1.png");
		GLuint spriteSheetTexture = LoadTexture("sheet.png");
		int enemycount = 36;
		//player sprite
		SheetSprite mySprite(&program, spriteSheetTexture, 112.0f / 1024.0f, 791.0f / 1024.0f, 112.0f / 1024.0f, 75.0f /
			1024.0f, 0.3);
		SheetSprite enemyt1(&program, spriteSheetTexture, 423.0f / 1024.0f, 728.0f / 1024.0f, 93.0f / 1024.0f, 84.0f /
			1024.0f, 0.25);
		SheetSprite enemyt2(&program, spriteSheetTexture, 120.0f / 1024.0f, 604.0f / 1024.0f, 104.0f / 1024.0f, 84.0f /
			1024.0f, 0.25);
		SheetSprite enemyt3(&program, spriteSheetTexture, 144.0f / 1024.0f, 156.0f / 1024.0f, 103.0f / 1024.0f, 84.0f /
			1024.0f, 0.25);
		SheetSprite playerlaser(&program, spriteSheetTexture, 856.0f / 1024.0f, 421.0f / 1024.0f, 9.0f / 1024.0f, 54.0f /
			1024.0f, 0.2);
		SheetSprite enemylaser(&program, spriteSheetTexture, 858.0f / 1024.0f, 230.0f / 1024.0f, 9.0f / 1024.0f, 54.0f /
			1024.0f, 0.2);
		float lastFrameTicks = 0.0f;
		float pxpos = 0;
		float enemyydisp = 0;//enemy y displacement
		float distanceToTravel = 1.2;
		float bulletSpeed = 2.7;
		bool canShoot = true;
		bool enemyShoot = true;
		Matrix projectionMatrix;
		Matrix modelMatrix;
		Matrix viewMatrix;
		Entity playerBullet(100, 100, 0, playerlaser, 1, 1);
		Entity enemyBullet(-100, -100, 0, enemylaser, 1, 1);
	
		vector<Entity> enemies;
		for (int i = 0; i < 36; i++){
			SheetSprite* enemysprite = &enemyt1;
			if (i > 11 && i < 24){ enemysprite = &enemyt2; }
			else if (i > 23){ enemysprite = &enemyt3; }
			Entity enemy(-3.2 + ((i % 12) / 2.0f), 1.8 - (i / 12) / 3.0f, M_PI, *enemysprite, 10, 10);
			enemies.push_back(enemy);
		}
		projectionMatrix.setOrthoProjection(-3.55f, 3.55f, -2.0f, 2.0f, -1.0f, 1.0f);
		glUseProgram(program.programID);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		enum{ START_SCREEN, GAME_SCREEN,WEND_SCREEN,LEND_SCREEN };
		int state = START_SCREEN;
		while (!done) {
			while (SDL_PollEvent(&event)) {
				if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
					done = true;
				}


			}
			glClear(GL_COLOR_BUFFER_BIT);
			const Uint8 *keys = SDL_GetKeyboardState(NULL);
			float vertices[] = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5 };
			float texCoords[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };
			program.setProjectionMatrix(projectionMatrix);
			program.setViewMatrix(viewMatrix);
			switch (state){

			case START_SCREEN:
				//text
				modelMatrix.identity();
				//modelMatrix.Scale(.6, .6, 0);
				modelMatrix.Translate(-1.5, 1, 0);
				program.setModelMatrix(modelMatrix);



				//glBindTexture(GL_TEXTURE_2D, ship2texB);

				glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
				glEnableVertexAttribArray(program.positionAttribute);

				glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
				glEnableVertexAttribArray(program.texCoordAttribute);
				DrawText(&program, text, "SPACE INVADERS", .5f, -.27f);
				modelMatrix.identity();

				modelMatrix.Translate(-1.5, -0.33, 0);
				program.setModelMatrix(modelMatrix);



				DrawText(&program, text, "press Enter to begin", .4f, -.25f);


				//glDrawArrays(GL_TRIANGLES, 0, 6);
				glDisableVertexAttribArray(program.positionAttribute);
				glDisableVertexAttribArray(program.texCoordAttribute);
				if (keys[SDL_SCANCODE_RETURN]){
					lastFrameTicks = (float)SDL_GetTicks() / 1000.0f;//contains start time of the program so that ticks starts at zero rather than actual time since start of the program
					state = GAME_SCREEN;
					
				}
				break;
			case WEND_SCREEN:

				//text
				modelMatrix.identity();
				//modelMatrix.Scale(.6, .6, 0);
				modelMatrix.Translate(-1.5, 1, 0);
				program.setModelMatrix(modelMatrix);




				glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
				glEnableVertexAttribArray(program.positionAttribute);

				glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
				glEnableVertexAttribArray(program.texCoordAttribute);
				DrawText(&program, text, "YOU WIN!", .5f, -.27f);
				modelMatrix.identity();

				modelMatrix.Translate(-1.5, -0.33, 0);
				program.setModelMatrix(modelMatrix);



				DrawText(&program, text, "press Enter to close", .4f, -.25f);


				//glDrawArrays(GL_TRIANGLES, 0, 6);
				glDisableVertexAttribArray(program.positionAttribute);
				glDisableVertexAttribArray(program.texCoordAttribute);
				if (keys[SDL_SCANCODE_RETURN]){
					done = true;
				}
				break;
			case LEND_SCREEN:

				//text
				modelMatrix.identity();
				//modelMatrix.Scale(.6, .6, 0);
				modelMatrix.Translate(-1.5, 1, 0);
				program.setModelMatrix(modelMatrix);




				glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
				glEnableVertexAttribArray(program.positionAttribute);

				glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
				glEnableVertexAttribArray(program.texCoordAttribute);
				DrawText(&program, text, "YOU LOST.", .5f, -.27f);
				modelMatrix.identity();

				modelMatrix.Translate(-1.5, -0.33, 0);
				program.setModelMatrix(modelMatrix);



				DrawText(&program, text, "press Enter to close", .4f, -.25f);


				//glDrawArrays(GL_TRIANGLES, 0, 6);
				glDisableVertexAttribArray(program.positionAttribute);
				glDisableVertexAttribArray(program.texCoordAttribute);
				if (keys[SDL_SCANCODE_RETURN]){
					done = true;
				}
				break;

			case GAME_SCREEN:
				float ticks = (float)SDL_GetTicks() / 1000.0f ;
				float elapsed = ticks - lastFrameTicks;
				lastFrameTicks = ticks;

				if (keys[SDL_SCANCODE_RIGHT]){
					if (pxpos < 3.3){
						pxpos += elapsed*distanceToTravel;
					}
				}
				if (keys[SDL_SCANCODE_LEFT]){
					if (pxpos > -3.3){
						pxpos -= elapsed*distanceToTravel;
					}

				}
				if (keys[SDL_SCANCODE_SPACE]){
					if (canShoot){
						canShoot = false;
						playerBullet.x = pxpos;
						playerBullet.y = -1.8;
						Mix_PlayChannel(-1, laserSound, 0);

						/*playerBullet.x = enemies[3].x;
						playerBullet.y = enemies[3].y; */
					}

				}
				//player rendering
				modelMatrix.identity();

				modelMatrix.Translate(pxpos, -1.8, 0);
				//modelMatrix.Scale(1.5, 1.5, 0);
				program.setModelMatrix(modelMatrix);
				mySprite.Draw();

				//enemy rendering
				for (int j = 0; j < 36; j++){
					if (!enemies[j].dead){
						modelMatrix.identity();
						enemies[j].x += elapsed * speed;
						if (enemies[j].x > 3.3){
							speed *= -1;
							for (int k = 0; k < enemies.size(); k++){
								enemies[k].y -= .2;
							}
						}
						else if (enemies[j].x < -3.3){
							speed *= -1;
							for (int k = 0; k < enemies.size(); k++){
								enemies[k].y -= .2;
							}
						}
						if (!((enemies[j].y - .1) >= (-1.8 + .1) || (enemies[j].y + .1) <= (-1.8 - .1) || (enemies[j].x - .1) >= (pxpos + .1) || (enemies[j].x + .1) <= (pxpos - .1))){
							state = LEND_SCREEN;
						}
						if (rand() % 500 == 1 && enemyShoot== true){ 
							enemyShoot = false;
							enemyBullet.x = enemies[j].x;
							enemyBullet.y = enemies[j].y;
						}
						modelMatrix.Translate(enemies[j].x, enemies[j].y, 0);
						modelMatrix.Rotate(enemies[j].rotation);
						program.setModelMatrix(modelMatrix);
						enemies[j].Draw();
					}
				}
				//player bullet rendering
				playerBullet.y += elapsed * bulletSpeed;
				if (playerBullet.y > 2){
					canShoot = true;
				}
				for (int j = 0; j < enemies.size(); j++){
					if (playerBullet.collide(enemies[j])&& enemies[j].dead == false){
						playerBullet.x = 100;
						playerBullet.y = 100;
						enemies[j].dead = true;
						enemycount--;
						Mix_PlayChannel(-1, explosionSound, 0);

						if (speed > 0){ speed += 0.02; }
						else{ speed -= 0.02; }
						canShoot = true;
						//done = true;
					}
					
				}
				if (playerBullet.collide(enemyBullet)){
					playerBullet.x = 100;
					playerBullet.y = 100;
					enemyBullet.x = -100;
					enemyBullet.y = -100;
					canShoot = true;
					enemyShoot = true;
				}
				if (enemycount == 0){
					state = WEND_SCREEN;
				}
				modelMatrix.identity();

				modelMatrix.Translate(playerBullet.x, playerBullet.y , 0);
				program.setModelMatrix(modelMatrix);
				playerBullet.Draw();


				//enemy bullet
				enemyBullet.y -= bulletSpeed* elapsed;
				if (enemyBullet.y < -2){
					enemyBullet.y =  -100;
					enemyShoot = true;
				}
				if (!((enemyBullet.y - .1) >= (-1.8 + .1) || (enemyBullet.y + .1) <= (-1.8 - .1) || (enemyBullet.x - .1) >= (pxpos + .1) || (enemyBullet.x + .1) <= (pxpos - .1))){
					Mix_PlayChannel(-1, explosionSound, 0);
					state = LEND_SCREEN;
				}
				modelMatrix.identity();

				modelMatrix.Translate(enemyBullet.x, enemyBullet.y, 0);
				modelMatrix.Rotate(M_PI);
				program.setModelMatrix(modelMatrix);
				enemyBullet.Draw();




				break;

				
			}

			



			SDL_GL_SwapWindow(displayWindow);
		}

		SDL_Quit();
		return 0;
	}
