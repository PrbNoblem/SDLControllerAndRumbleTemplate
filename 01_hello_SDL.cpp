/*This source code copyrighted by Lazy Foo' Productions (2004-2015)
and may not be redistributed without written permission.*/

//Using SDL, SDL_image, standard IO, math, and strings
#include <SDL.h>
#include <SDL_image.h>
#include <stdio.h>
#include <string>
#include <cmath>
#include <SDL_ttf.h>
#include <stdlib.h>
#include <SDL_mixer.h>

//Screen dimension constants
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

//Analog joystick dead zone
const int JOYSTICK_DEAD_ZONE = 8000;

//Texture wrapper class
class LTexture
{
	public:
		//Initializes variables
		LTexture();

		//Deallocates memory
		~LTexture();

		//Loads image at specified path
		bool loadFromFile( std::string path );

		#ifdef _SDL_TTF_H
		//Creates image from font string
		bool loadFromRenderedText( std::string textureText, SDL_Color textColor );
		#endif

		//Deallocates texture
		void free();

		//Set color modulation
		void setColor( Uint8 red, Uint8 green, Uint8 blue );

		//Set blending
		void setBlendMode( SDL_BlendMode blending );

		//Set alpha modulation
		void setAlpha( Uint8 alpha );

		//Renders texture at given point
		void render( int x, int y, SDL_Rect* clip = NULL, double angle = 0.0, SDL_Point* center = NULL, SDL_RendererFlip flip = SDL_FLIP_NONE );

		//Gets image dimensions
		int getWidth();
		int getHeight();

	private:
		//The actual hardware texture
		SDL_Texture* mTexture;

		//Image dimensions
		int mWidth;
		int mHeight;
};

//Starts up SDL and creates window
bool init();

//Loads media
bool loadMedia();

//Frees media and shuts down SDL
void close();

//The window we'll be rendering to
SDL_Window* gWindow = NULL;

//The window renderer
SDL_Renderer* gRenderer = NULL;

//Scene textures
LTexture gArrowTexture;

//Game Controller 1 handler
//SDL_Joystick* gGameController = NULL;
SDL_GameController* gGameController = NULL;
SDL_Haptic* gControllerHaptic = NULL;

//check joystickname     ########################################################################################
LTexture gJoystickName;
TTF_Font *gFont;

void loadJoystickName( std::string sJoystickName );
//-----------------------------------------------------------------------------


// MUSIC STUFF ########################################################################################
//The music that will be played
Mix_Music *gMusic = NULL;

//The sound effects that will be used
Mix_Chunk *gScratch = NULL;
Mix_Chunk *gHigh = NULL;
Mix_Chunk *gMedium = NULL;
Mix_Chunk *gLow = NULL;
//-----------------------------%%%%%%%%%%%%%%%%%&&&&&&&&&&&&&&&&&&&&&&&&&

LTexture::LTexture()
{
	//Initialize
	mTexture = NULL;
	mWidth = 0;
	mHeight = 0;
}

LTexture::~LTexture()
{
	//Deallocate
	free();
}



bool LTexture::loadFromFile( std::string path )
{
	//Get rid of preexisting texture
	free();

	//The final texture
	SDL_Texture* newTexture = NULL;

	//Load image at specified path
	SDL_Surface* loadedSurface = IMG_Load( path.c_str() );
	if( loadedSurface == NULL )
	{
		printf( "Unable to load image %s! SDL_image Error: %s\n", path.c_str(), IMG_GetError() );
	}
	else
	{
		//Color key image
		SDL_SetColorKey( loadedSurface, SDL_TRUE, SDL_MapRGB( loadedSurface->format, 0, 0xFF, 0xFF ) );

		//Create texture from surface pixels
        newTexture = SDL_CreateTextureFromSurface( gRenderer, loadedSurface );
		if( newTexture == NULL )
		{
			printf( "Unable to create texture from %s! SDL Error: %s\n", path.c_str(), SDL_GetError() );
		}
		else
		{
			//Get image dimensions
			mWidth = loadedSurface->w;
			mHeight = loadedSurface->h;
		}

		//Get rid of old loaded surface
		SDL_FreeSurface( loadedSurface );
	}

	//Return success
	mTexture = newTexture;
	return mTexture != NULL;
}

#ifdef _SDL_TTF_H
bool LTexture::loadFromRenderedText( std::string textureText, SDL_Color textColor )
{
	//Get rid of preexisting texture
	free();

	//Render text surface
	SDL_Surface* textSurface = TTF_RenderText_Solid( gFont, textureText.c_str(), textColor );
	if( textSurface != NULL )
	{
		//Create texture from surface pixels
        mTexture = SDL_CreateTextureFromSurface( gRenderer, textSurface );
		if( mTexture == NULL )
		{
			printf( "Unable to create texture from rendered text! SDL Error: %s\n", SDL_GetError() );
		}
		else
		{
			//Get image dimensions
			mWidth = textSurface->w;
			mHeight = textSurface->h;
		}

		//Get rid of old surface
		SDL_FreeSurface( textSurface );
	}
	else
	{
		printf( "Unable to render text surface! SDL_ttf Error: %s\n", TTF_GetError() );
	}


	//Return success
	return mTexture != NULL;
}
#endif

void LTexture::free()
{
	//Free texture if it exists
	if( mTexture != NULL )
	{
		SDL_DestroyTexture( mTexture );
		mTexture = NULL;
		mWidth = 0;
		mHeight = 0;
	}
}

void LTexture::setColor( Uint8 red, Uint8 green, Uint8 blue )
{
	//Modulate texture rgb
	SDL_SetTextureColorMod( mTexture, red, green, blue );
}

void LTexture::setBlendMode( SDL_BlendMode blending )
{
	//Set blending function
	SDL_SetTextureBlendMode( mTexture, blending );
}

void LTexture::setAlpha( Uint8 alpha )
{
	//Modulate texture alpha
	SDL_SetTextureAlphaMod( mTexture, alpha );
}

void LTexture::render( int x, int y, SDL_Rect* clip, double angle, SDL_Point* center, SDL_RendererFlip flip )
{
	//Set rendering space and render to screen
	SDL_Rect renderQuad = { x, y, mWidth, mHeight };

	//Set clip rendering dimensions
	if( clip != NULL )
	{
		renderQuad.w = clip->w;
		renderQuad.h = clip->h;
	}

	//Render to screen
	SDL_RenderCopyEx( gRenderer, mTexture, clip, &renderQuad, angle, center, flip );
}

int LTexture::getWidth()
{
	return mWidth;
}

int LTexture::getHeight()
{
	return mHeight;
}




bool init()
{
	//Initialization flag
	bool success = true;

	//Initialize SDL
	if( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER | SDL_INIT_HAPTIC | SDL_INIT_AUDIO ) < 0 )
	{
		printf( "SDL could not initialize! SDL Error: %s\n", SDL_GetError() );
		success = false;
	}
	else
	{
		//Set texture filtering to linear
		if( !SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "1" ) )
		{
			printf( "Warning: Linear texture filtering not enabled!" );
		}

		//Check for joysticks
		if( SDL_NumJoysticks() < 1 )
		{
			printf( "Warning: No joysticks connected!\n" );
		}
		else
		{
			//Load joystick
			//gGameController = SDL_JoystickOpen( 0 );
			gGameController = SDL_GameControllerOpen( 0 );
			if( gGameController == NULL )
			{
				printf( "Warning: Unable to open game controller! SDL Error: %s\n", SDL_GetError() );
			}

		}

		//Init. haptic stuff
		//Get haptic device for the controller
		gControllerHaptic = SDL_HapticOpen(0);
		if(gControllerHaptic == NULL)
        {
            printf("failed to get haptic device with the joystick command... \n");
        }

        //Init. rumble stuff
        if( SDL_HapticRumbleInit( gControllerHaptic ) < 0 )
        {
            printf("failed to call SDL_HapticRumbleInit: %s \n", SDL_GetError());
        }



		//Create window
		gWindow = SDL_CreateWindow( "SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN );
		if( gWindow == NULL )
		{
			printf( "Window could not be created! SDL Error: %s\n", SDL_GetError() );
			success = false;
		}
		else
		{
			//Create vsynced renderer for window
			gRenderer = SDL_CreateRenderer( gWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC );
			if( gRenderer == NULL )
			{
				printf( "Renderer could not be created! SDL Error: %s\n", SDL_GetError() );
				success = false;
			}
			else
			{
				//Initialize renderer color
				SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0xFF, 0xFF );

				//Initialize PNG loading
				int imgFlags = IMG_INIT_PNG;
				if( !( IMG_Init( imgFlags ) & imgFlags ) )
				{
					printf( "SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError() );
					success = false;
				}

				//Initialize SDL_mixer
                if( Mix_OpenAudio( 44100, MIX_DEFAULT_FORMAT, 2, 2048 ) < 0 )
                {
                    printf( "SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError() );
                    success = false;
                }
			}
		}
	}

	return success;
}

bool loadSounds()
{
    //Load music
    gMusic = Mix_LoadMUS( "21_sound_effects_and_music/beat.wav" );
    if( gMusic == NULL )
    {
        printf( "Failed to load beat music! SDL_mixer Error: %s\n", Mix_GetError() );
        return false;
    }

    //Load sound effects
    gScratch = Mix_LoadWAV( "21_sound_effects_and_music/scratch.wav" );
    if( gScratch == NULL )
    {
        printf( "Failed to load scratch sound effect! SDL_mixer Error: %s\n", Mix_GetError() );
        return false;
    }

    gHigh = Mix_LoadWAV( "21_sound_effects_and_music/high.wav" );
    if( gHigh == NULL )
    {
        printf( "Failed to load high sound effect! SDL_mixer Error: %s\n", Mix_GetError() );
        return false;
    }

    gMedium = Mix_LoadWAV( "21_sound_effects_and_music/medium.wav" );
    if( gMedium == NULL )
    {
        printf( "Failed to load medium sound effect! SDL_mixer Error: %s\n", Mix_GetError() );
        return false;
    }

    gLow = Mix_LoadWAV( "21_sound_effects_and_music/low.wav" );
    if( gLow == NULL )
    {
        printf( "Failed to load low sound effect! SDL_mixer Error: %s\n", Mix_GetError() );
        return false;
    }
    return true;
}

bool loadMedia()
{
	//Loading success flag
	bool success = true;

	//Load arrow texture
	if( !gArrowTexture.loadFromFile( "19_gamepads_and_joysticks/arrow.png" ) )
	{
		printf( "Failed to load arrow texture!\n" );
		success = false;
	}

	return success;
}

void close()
{
	//Free loaded images
	gArrowTexture.free();

	//Close game controller
	//SDL_JoystickClose( gGameController );
	SDL_GameControllerClose(gGameController);
    SDL_HapticClose( gControllerHaptic );
	gGameController = NULL;
    gControllerHaptic = NULL;
	//Destroy window
	SDL_DestroyRenderer( gRenderer );
	SDL_DestroyWindow( gWindow );
	gWindow = NULL;
	gRenderer = NULL;


    Mix_FreeMusic( gMusic );
    gMusic = NULL;

    Mix_FreeChunk( gScratch );
    Mix_FreeChunk( gHigh );
    Mix_FreeChunk( gMedium );
    Mix_FreeChunk( gLow );
    gScratch = NULL;
    gHigh = NULL;
    gMedium = NULL;
    gLow = NULL;

	//Quit SDL subsystems
	IMG_Quit();
	SDL_Quit();
}



int main( int argc, char* args[] )
{
	//Start up SDL and create window
	if( !init() )
	{
		printf( "Failed to initialize!\n" );
	}
	else
	{
	    printf("first init done\n");
	    if( !TTF_Init() == -1)
        {
            printf("failed to init TTF!\n");
            exit(1);
        }
        else
        {
            printf("TTF initialized\n");
            std::string sJoystickName = SDL_GameControllerName(gGameController);
            loadJoystickName(sJoystickName);
        }
		//Load media
		if( !loadMedia() )
		{
			printf( "Failed to load media!\n" );
		}
		else
		{
		    if( !loadSounds() )
            {
                printf( "Failed to load sounds!\n" );
                exit(1);
            }
			//Main loop flag
			bool quit = false;

			//Event handler
			SDL_Event e;

			//Normalized direction
			int xDir = 0;
			int yDir = 0;

			//While application is running
			while( !quit )
			{
				//Handle events on queue
				while( SDL_PollEvent( &e ) != 0 )
				{
					//User requests quit
					if( e.type == SDL_QUIT )
					{
						quit = true;
						printf("gamecontrollername: %s\n", SDL_GameControllerName(gGameController));
					}
                    else if( e.type == SDL_CONTROLLERBUTTONDOWN )
                    {
                        //rumble on for 500 millisec, 75 % power
                        if( SDL_HapticRumblePlay( gControllerHaptic, 0.75, 500 ) != 0)
                        {
                            printf("can't rumble for some reason %s\n", SDL_GetError());
                        }
                        printf("button: %d\n", e.button.button );
                        if( e.jbutton.button == SDL_CONTROLLER_BUTTON_A ) printf( "Button A pressed!\n" );
                        if( e.jbutton.button == SDL_CONTROLLER_BUTTON_B ) printf( "Button B pressed!\n" );
                        if( e.jbutton.button == SDL_CONTROLLER_BUTTON_X ) printf( "Button X pressed!\n" );
                        if( e.jbutton.button == SDL_CONTROLLER_BUTTON_Y ) printf( "Button Y pressed!\n" );
                        switch(e.jbutton.button)
                        {
                        case SDL_CONTROLLER_BUTTON_DPAD_UP:
                            Mix_PlayChannel( -1, gHigh, 0 );
                            break;
                        case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
                            Mix_PlayChannel( -1, gMedium, 0 );
                            break;
                        case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
                            Mix_PlayChannel( -1, gLow, 0 );
                            break;
                        case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
                            Mix_PlayChannel( -1, gScratch, 0 );
                            break;

                        case SDL_CONTROLLER_BUTTON_START:
                            if(Mix_PlayingMusic() == 0)
                            {
                                Mix_PlayMusic(gMusic, -1);
                            }
                            else
                            {
                                if(Mix_PausedMusic() == 1)
                                {
                                    Mix_ResumeMusic();
                                }
                                else
                                {
                                    Mix_PauseMusic();
                                }
                            }
                            break;
                        }

                    }
					else if (e.type == SDL_CONTROLLERAXISMOTION)
                    {
                        if( e.jaxis.which == 0 )
						{
							//X axis motion
							if( e.jaxis.axis == 0 )
							{
								//Left of dead zone
								if( e.jaxis.value < -JOYSTICK_DEAD_ZONE )
								{
									xDir = -1;
								}
								//Right of dead zone
								else if( e.jaxis.value > JOYSTICK_DEAD_ZONE )
								{
									xDir =  1;
								}
								else
								{
									xDir = 0;
								}
							}
							//Y axis motion
							else if( e.jaxis.axis == 1 )
							{
								//Below of dead zone
								if( e.jaxis.value < -JOYSTICK_DEAD_ZONE )
								{
									yDir = -1;
								}
								//Above of dead zone
								else if( e.jaxis.value > JOYSTICK_DEAD_ZONE )
								{
									yDir =  1;
								}
								else
								{
									yDir = 0;
								}
							}
                            else if(e.jaxis.axis == 2)
                            {
                                if( e.jaxis.value < -JOYSTICK_DEAD_ZONE ) printf("LEFT STICK LEFT! \n");
                                if( e.jaxis.value > JOYSTICK_DEAD_ZONE ) printf("LEFT STICK RIGHT! \n");
                            }
							else if(e.jaxis.axis == 3)
                            {
                                if( e.jaxis.value < -JOYSTICK_DEAD_ZONE ) printf("RIGHT Stick Up!\n");
                                if( e.jaxis.value > JOYSTICK_DEAD_ZONE ) printf("RIGHT Stick Down!\n");
                            }
                            else if(e.jaxis.axis == 4)
                            {
                                if( e.jaxis.value > JOYSTICK_DEAD_ZONE ) printf("LEFT TRIGGER !\n");
                            }

                            else if(e.jaxis.axis == 5)
                            {
                                if( e.jaxis.value > JOYSTICK_DEAD_ZONE ) printf("RIGHT TRIGGER  ! \n");
                            }
						}
                    }



				}

				//Clear screen
				SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0xFF, 0xFF );
				SDL_RenderClear( gRenderer );

				//Calculate angle
				double joystickAngle = atan2( (double)yDir, (double)xDir ) * ( 180.0 / M_PI );

				//Correct angle
				if( xDir == 0 && yDir == 0 )
				{
					joystickAngle = 0;
				}


                gJoystickName.render( ( SCREEN_WIDTH - gJoystickName.getWidth() ) / 2, 20 );
				//Render joystick 8 way angle
				gArrowTexture.render( ( SCREEN_WIDTH - gArrowTexture.getWidth() ) / 2, ( SCREEN_HEIGHT - gArrowTexture.getHeight() ) / 2, NULL, joystickAngle );

				//Update screen
				SDL_RenderPresent( gRenderer );
			}
		}
	}

	//Free resources and close SDL
	close();

	return 0;
}


void loadJoystickName( std::string sJoystickName )
{
	gFont = TTF_OpenFont( "16_true_type_fonts/lazy.ttf", 28 );
	if( gFont == NULL )
	{
		printf( "Failed to load lazy font! SDL_ttf Error: %s\n", TTF_GetError() );
	}
	else
	{
		//Render text
		SDL_Color textColor = { 0, 0, 0 };
		if( !gJoystickName.loadFromRenderedText( sJoystickName, textColor ) )
		{
			printf( "Failed to render text texture!\n" );
		}
	}
}
