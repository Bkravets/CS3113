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
#include <math.h>

#include <fstream>
#include <string>
#include <iostream>
#include <sstream>
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
enum EntityType {
	ENTITY_PLAYER, ENTITY_ENEMY,
	ENTITY_COIN
};
float friction_x = 0.5;
float friction_y = 0.0;
float gravity_x = 0;
float gravity_y = 0.0f;
float FIXED_TIMESTEP = 0.0166666f;
int MAX_TIMESTEPS = 6;
float lerp(float v0, float v1, float t) {
	return (1.0 - t)*v0 + t*v1;
}
class Entity {
public:
	Entity(SheetSprite sprite, float x, float y, float width, float height, float velocity_x, float  velocity_y, float acceleration_x, float acceleration_y, bool isStatic , EntityType entityType) :sprite(sprite), x(x),y(y),width(width),height(height),velocity_x(velocity_x),velocity_y(velocity_y),acceleration_x(acceleration_x),acceleration_y(acceleration_y),isStatic(isStatic),entityType(entityType){}
	void Update(float elapsed);
	void Render(ShaderProgram *program){
		Matrix projectionMatrix;
		Matrix modelMatrix;
		Matrix viewMatrix;
		modelMatrix.identity();

		modelMatrix.Translate(x, y , 0);
		program->setModelMatrix(modelMatrix);
		

		sprite.Draw();
	
	
	}
	bool collidesWith(Entity *ent){
		if (!((y - height / 2) >= (ent->y + ent->height / 2) || (y + height / 2) <= (ent->y - ent->height / 2) || (x - width / 2) >= (ent->x + ent->width / 2) || (x + width / 2) <= (ent->x - ent->width / 2))){
			
			return true;
		}
		else{ return false; }
	}
	SheetSprite sprite;
	float x;
	float y;
	float width;
	float height;
	float velocity_x;
	float velocity_y;
	float acceleration_x;
	float acceleration_y;
	bool isStatic;
	EntityType entityType;
	bool collidedTop = false;
	bool collidedBottom = false;
	bool collidedLeft = false;
	bool collidedRight = false;
};
	float speed;
	vector <Entity> entities;

	void Entity::Update(float elapsed){
		collidedTop = false;
		collidedBottom = false;
		collidedLeft = false;
		collidedRight = false;
		if (entityType == ENTITY_PLAYER){

			velocity_x += gravity_x * elapsed;
			velocity_y += gravity_y * elapsed;
			velocity_x = lerp(velocity_x, 0.0f, elapsed * friction_x);
			velocity_y = lerp(velocity_y, 0.0f, elapsed * friction_y);
			velocity_x += acceleration_x * elapsed;
			velocity_y += acceleration_y * elapsed;

			y += velocity_y * elapsed;
			for (int i = 0; i < entities.size(); i++){
				if (&entities[i] != this){
					bool collide =collidesWith(&entities[i]);
					
					if (collide){
						velocity_y = 0;
						if (y >= entities[i].y){
							collidedBottom = true;
							y += fabs((y - entities[i].y) - height / 2 - entities[i].height / 2) + 0.0001;
						}
						else if (y < entities[i].y){
							collidedTop = true;
							y -= fabs(( entities[i].y - y) - height / 2 - entities[i].height / 2) + .0001;
						}
					}
					
				}
			}
			x += velocity_x * elapsed;
			for (int i = 0; i < entities.size(); i++){
				if (&entities[i] != this){
					bool collide = collidesWith(&entities[i]);

					if (collide){
						velocity_x = 0;
						if (x >= entities[i].x){
							collidedLeft = true;
							x += fabs((x - entities[i].x) - width / 2 - entities[i].width / 2) + 0.0001;
						}
						else if (x < entities[i].x){
							collidedRight = true;
							x -= fabs(( entities[i].x - x) - width / 2 - entities[i].width / 2) + 0.0001;
						}
					}
				}
			}



		}
	}
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


	float TILE_SIZE = .5;
	int SPRITE_COUNT_X = 14;
	int SPRITE_COUNT_Y = 7;


#define LEVEL_HEIGHT 25
#define LEVEL_WIDTH 25
	unsigned char** levelData;
	int mapWidth = 0;
	int mapHeight = 0;


	bool readHeader(std::ifstream &stream) {
		string line;
		mapWidth = -1;
		mapHeight = -1;
		while (getline(stream, line)) {
			if (line == "") { break; }
			istringstream sStream(line);
			string key, value;
			getline(sStream, key, '=');
			getline(sStream, value);
			if (key == "width") {
				mapWidth = atoi(value.c_str());
			}
			else if (key == "height"){
				mapHeight = atoi(value.c_str());
			}
		}
		if (mapWidth == -1 || mapHeight == -1) {
			return false;
		}
		else { // allocate our map data, done elsewhere
			levelData = new unsigned char*[mapHeight];
			for (int i = 0; i < mapHeight; ++i) {
				levelData[i] = new unsigned char[mapWidth];
			}
			return true;
		}
	}



	bool readLayerData(std::ifstream &stream) {
		string line;
		while (getline(stream, line)) {
			if (line == "") { break; }
			istringstream sStream(line);
			string key, value;
			getline(sStream, key, '=');
			getline(sStream, value);
			if (key == "data") {
				for (int y = 0; y < mapHeight; y++) {
					getline(stream, line);
					istringstream lineStream(line);
					string tile;
					for (int x = 0; x < mapWidth; x++) {
						getline(lineStream, tile, ',');
						unsigned char val = (unsigned char)atoi(tile.c_str());
						if (val > 0) {
							// be careful, the tiles in this format are indexed from 1 not 0
							levelData[y][x] = val - 1;
						}
						else {
							levelData[y][x] = 0;
						}
					}
				}
			}
		}
		return true;
	}


	bool readEntityData(std::ifstream &stream) {
		string line;
		string type;
		while (getline(stream, line)) {
			if (line == "") { break; }
			istringstream sStream(line);
			string key, value;
			getline(sStream, key, '=');
			getline(sStream, value);
			if (key == "type") {
				type = value;
			}
			else if (key == "location") {
				istringstream lineStream(value);
				string xPosition, yPosition;
				getline(lineStream, xPosition, ',');
				getline(lineStream, yPosition, ',');
					float placeX = atoi(xPosition.c_str()) / 16 * TILE_SIZE;
				float placeY = atoi(yPosition.c_str()) / 16 * -TILE_SIZE;
				//entities[0].x = 1;
				//entities[0].y = -11.7;

			}
		}
		return true;
	}

	void worldToTileCoordinates(float worldX, float worldY, int *gridX, int *gridY) {
		*gridX = (int)(worldX / TILE_SIZE);
		*gridY = (int)(-worldY / TILE_SIZE);
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
		
		

		//music initilization
		Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
		
		Mix_Music *music;
		music = Mix_LoadMUS("BoxCat_Games_-_10_-_Epic_Song.mp3");
		Mix_PlayMusic(music, -1);

		//texture initilization
		GLuint text = LoadTexture("font1.png");
		GLuint playerTexture = LoadTexture("alienBlue_stand.png");
		GLuint spriteSheetTexture = LoadTexture("sheet.png");

		//int enemycount = 36;
		////player sprite
		SheetSprite mySprite(&program, playerTexture, 0, 0, 1, 1, 0.2);
		//SheetSprite enemyt1(&program, spriteSheetTexture, 423.0f / 1024.0f, 728.0f / 1024.0f, 93.0f / 1024.0f, 84.0f /
		//	1024.0f, 0.25);
		//SheetSprite enemyt2(&program, spriteSheetTexture, 120.0f / 1024.0f, 604.0f / 1024.0f, 104.0f / 1024.0f, 84.0f /
		//	1024.0f, 0.25);
		//SheetSprite enemyt3(&program, spriteSheetTexture, 144.0f / 1024.0f, 156.0f / 1024.0f, 103.0f / 1024.0f, 84.0f /
		//	1024.0f, 0.25);
		//SheetSprite playerlaser(&program, spriteSheetTexture, 856.0f / 1024.0f, 421.0f / 1024.0f, 9.0f / 1024.0f, 54.0f /
		//	1024.0f, 0.2);
		//SheetSprite enemylaser(&program, spriteSheetTexture, 858.0f / 1024.0f, 230.0f / 1024.0f, 9.0f / 1024.0f, 54.0f /
		//	1024.0f, 0.2);
		float lastFrameTicks = 0.0f;
		
		Matrix projectionMatrix;
		Matrix modelMatrix;
		Matrix viewMatrix;
		
		Entity player(mySprite,0,0,.45,.3,0,0,0,0,false,ENTITY_PLAYER);
		entities.push_back(player);
		entities[0].x = 10;
		entities[0].y = -11.7;
		//Entity test(mySprite, 1, 0, .45, .3, 0, 0, 0, 0, true, ENTITY_PLAYER);
		//entities.push_back(test);
		projectionMatrix.setOrthoProjection(-3.55f, 3.55f, -2.0f, 2.0f, -1.0f, 1.0f);

		//file read
		ifstream infile("platformtiles.txt");
		string line;
		while (getline(infile, line)) {
			if (line == "[header]") {
				if (!readHeader(infile)) {
					return 1;
				}
			}
			else if (line == "[layer]") {
				readLayerData(infile);
			}
			else if (line == "[ObjectLayer]") {
				readEntityData(infile);//no object layer included, as player is the only entity
			}
		}
		//reading map into vector
		int count = 0;
		std::vector<float> vertexDat;
		std::vector<float> texCoordDat;
		for (int y = 0; y < LEVEL_HEIGHT; y++) {
			for (int x = 0; x < LEVEL_WIDTH; x++) {
				if (levelData[y][x] != 0) {
					count++;
					float u = (float)(((int)levelData[y][x]) % SPRITE_COUNT_X) / (float)SPRITE_COUNT_X;
					float v = (float)(((int)levelData[y][x]) / SPRITE_COUNT_X) / (float)SPRITE_COUNT_Y;
					float spriteWidth = 1.0f / (float)SPRITE_COUNT_X;
					float spriteHeight = 1.0f / (float)SPRITE_COUNT_Y;
					vertexDat.insert(vertexDat.end(), {
						TILE_SIZE * x, -TILE_SIZE * y,
						TILE_SIZE * x, (-TILE_SIZE * y) - TILE_SIZE,
						(TILE_SIZE * x) + TILE_SIZE, (-TILE_SIZE * y) - TILE_SIZE,
						TILE_SIZE * x, -TILE_SIZE * y,
						(TILE_SIZE * x) + TILE_SIZE, (-TILE_SIZE * y) - TILE_SIZE,
						(TILE_SIZE * x) + TILE_SIZE, -TILE_SIZE * y
					});
					texCoordDat.insert(texCoordDat.end(), {
						u, v,
						u, v + (spriteHeight),
						u + spriteWidth, v + (spriteHeight),
						u, v,
						u + spriteWidth, v + (spriteHeight),
						u + spriteWidth, v
					});
				}
			}
		}
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
				DrawText(&program, text, "PLATFORMER", .5f, -.27f);
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
				entities[0].acceleration_x = 0;
				entities[0].acceleration_y = 0;

				if (keys[SDL_SCANCODE_RIGHT]){
					
					entities[0].acceleration_x = 1;
				}
				if (keys[SDL_SCANCODE_LEFT]){
					
					entities[0].acceleration_x = -1;
				}
				if (keys[SDL_SCANCODE_UP]){
					
					entities[0].acceleration_y = 1;

				}
				if (keys[SDL_SCANCODE_DOWN]){

					entities[0].acceleration_y = -1;

				}

				


				//tiles
				modelMatrix.identity();

				//modelMatrix.Translate(-3.4, -1, 0);
				////modelMatrix.Scale(1.5, 1.5, 0);
				program.setModelMatrix(modelMatrix);
				glUseProgram(program.programID);
				glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertexDat.data());
				glEnableVertexAttribArray(program.positionAttribute);
				glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordDat.data());
				glEnableVertexAttribArray(program.texCoordAttribute);
				glBindTexture(GL_TEXTURE_2D, spriteSheetTexture);
				glDrawArrays(GL_TRIANGLES, 0, count*6);
				glDisableVertexAttribArray(program.positionAttribute);
				glDisableVertexAttribArray(program.texCoordAttribute);


				
				for (int i = 0; i < entities.size(); i++){
					if (entities[i].isStatic == false){
						entities[i].Update(elapsed);
					}


					int botx = entities[i].x ;
					int boty = entities[i].y - entities[i].height / 2;
					worldToTileCoordinates(botx, boty, &botx, &boty);
					/*if (botx > 25){ botx = 20; }
					if (botx < 0){ botx = 0; }
					if (boty < 0){ boty = 0; }*/
					if (levelData[boty][botx] != levelData[2][2]){
						entities[i].y += 0.001;
						entities[i].velocity_y = 0;
						entities[i].collidedBottom = true;
					}
					int topx = entities[i].x ;
					int topy = entities[i].y + entities[i].height / 2;
					worldToTileCoordinates(topx, topy, &topx, &topy);
					if (levelData[topy][topx] != levelData[2][2]){
						entities[i].y -= 0.001;
						entities[i].velocity_y = 0;
						entities[i].collidedTop = true;
					}
					topx = entities[i].x + entities[i].width / 2;//rightx
					topy = entities[i].y ;
					worldToTileCoordinates(topx, topy, &topx, &topy);
					if (levelData[topy][topx] != levelData[2][2]){
						entities[i].x -= 0.001;
						entities[i].velocity_x = 0;
						entities[i].collidedRight = true;
					}
					topx = entities[i].x - entities[i].width / 2;//leftx
					topy = entities[i].y;
					worldToTileCoordinates(topx, topy, &topx, &topy);
					if (levelData[topy][topx] != levelData[2][2]){
						entities[i].x += 0.001;
						entities[i].velocity_x = 0;
						entities[i].collidedLeft = true;
						if (levelData[topy][topx] == levelData[11][1]){//door
							state = WEND_SCREEN;
						}
					}
					//int rightx = entities[i].x + entities[i].width / 2;
					//int righty = entities[i].y + entities[i].width / 2;
					//worldToTileCoordinates(rightx, righty, &rightx, &righty);
					//if (rightx > 20){ rightx = 20; }
					//if (rightx < 0){ rightx = 0; }
					//if (righty < 0){ righty = 0; }
					//if (levelData[righty][rightx] != 0){
					//	entities[i].x -= (entities[i].x + entities[i].width / 2 - (rightx* TILE_SIZE));
					//	entities[i].velocity_x = 0;
					//	entities[i].collidedRight = true;
					//	//entities[0].y = -12;
					//}

					entities[i].Render(&program);
				}
				viewMatrix.identity();
				viewMatrix.Translate(-entities[0].x, -entities[0].y, 0);
				program.setViewMatrix(viewMatrix);



				break;

				
			}

			



			SDL_GL_SwapWindow(displayWindow);
		}

		SDL_Quit();
		return 0;
	}
