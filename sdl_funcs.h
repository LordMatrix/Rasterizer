//Global SDL vars
SDL_Surface  *g_SDLSrf;
SDL_Event g_event;
int g_end;
unsigned int* g_screen_pixels;

int g_keydown;


/*******************************/
/****** SDL FUNCTIONS  *********/
/*******************************/

//Inits window and such
static int init_SDL() {
  
  //int mouse_x = 0, mouse_y = 0;
  int req_w = 640;
  int req_h = 480;

  // Init SDL and screen
  if ( SDL_Init(SDL_INIT_AUDIO|SDL_INIT_VIDEO) < 0 ) 
  {
    fprintf(stderr, "Can't Initialise SDL: %s\n", SDL_GetError());
    exit(1);
  }
  if (0 == SDL_SetVideoMode( req_w, req_h, 32,  SDL_HWSURFACE | SDL_DOUBLEBUF))
  {
    printf("Couldn't set %dx%dx32 video mode: %s\n", req_w, req_h, SDL_GetError());
    return 0;
  }

  g_SDLSrf = SDL_GetVideoSurface();
  return 1;
}


//Process SDL input & other events
static void input_SDL() {
  // Check input events
    while ( SDL_PollEvent(&g_event) ) 
    {
      switch (g_event.type) 
      {
        case SDL_QUIT:
          g_end = 1;
          break;
        case SDL_KEYDOWN:
          g_keydown = 1;
          break;
        case SDL_KEYUP:
          g_keydown = 0;
          break;
      }
    }
}


//Controls double buffer switch
static void frame_SDL() {
  SDL_UnlockSurface( g_SDLSrf);
  SDL_Flip( g_SDLSrf);
}


//Clears screen and allows for screen buffer reading
static void clear_SDL() {
  // Lock screen to get access to the memory array
  SDL_LockSurface( g_SDLSrf);
  g_screen_pixels = (unsigned int *) g_SDLSrf->pixels;

  // Borrar pantalla
  memset ( g_screen_pixels, 0, g_SDLSrf->h * g_SDLSrf->pitch);
}
