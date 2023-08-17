#include "video.h"
#include <SDL3/SDL.h>
#include <glad/glad.h>


typedef struct SDL_Context_s {
  
  SDL_GLContext* gl_context;
  SDL_Window*   window;
  const char*   window_name;
  int           width;
  int           height;
  Uint32        window_flags;

} SDL_Context;


char* file_to_cstring(char* path) {
  FILE* file;
  long length;
  char* buf;

  file = fopen(path, "rb");
  if (!file) {
    return NULL;
  }

  fseek(file, 0, SEEK_END);
  length = ftell(file);
  buf = (char*)malloc((length + 1) * sizeof(char));
  fseek(file, 0, SEEK_SET);
  fread(buf, length, 1, file);
  fclose(file);
  buf[length] = '\0';
  return buf;
}


int main(int argc, const char* argv[]) {

  if (argc < 2) return 0;


  // init SDL
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    fprintf(stderr, "SDL could not initialize.\n%s\n", SDL_GetError()); 
    return -1;
  }

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 32);
  

  SDL_Context sdl_context;
  sdl_context.width  = 1200;
  sdl_context.height = sdl_context.width * 9 / 16;
  sdl_context.window_name = "Editor";
  sdl_context.window_flags = SDL_WINDOW_OPENGL;
  printf("Attempting window creation of %d x %d\n", sdl_context.width, sdl_context.height);
  sdl_context.window = SDL_CreateWindow(
    sdl_context.window_name, 
    sdl_context.width,  sdl_context.height, 
    sdl_context.window_flags);

  if (!sdl_context.window) {
    fprintf(stderr, "SDL couldn't create the window.\n%s\n", SDL_GetError()); 
    return -1;
  }

  sdl_context.gl_context = SDL_GL_CreateContext(sdl_context.window);

  // i need VAO, VBO, IBO
  // quad rendering
  const GLfloat quad_triangles[] = {
    // BL, BR, TR // tri 1
    // TR, TL, BL // tri 2
    // coord, uv
    
    // TRI 1
    -0.5, -0.5, 0.0,  0, 1, // BL 
    0.5, -0.5, 0.0,   1, 1, // BR 
    0.5, 0.5, 0.0,    1, 0, // TR

    // TRI 2
    0.5, 0.5, 0.0,    1, 0, // TR 
    -0.5, 0.5, 0.0,   0, 0, // TL 
    -0.5, -0.5, 0.0,  0, 1, // BL
  };
  
  
  if (!gladLoadGLLoader((void*)SDL_GL_GetProcAddress)) {
    return -1;
  }

  GLuint vao;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);
  
  GLuint vbo;
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  // 6 vertices * 5 floats per vertex
  glBufferData(GL_ARRAY_BUFFER, 6 * 5 * sizeof(GLfloat), quad_triangles, GL_STATIC_DRAW);


  // coordinates are attribute 0
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 
    5 * sizeof(GLfloat), (void*)0);

  // uv coordinates
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 
    5 * sizeof(GLfloat), (void*)(3*sizeof(GLfloat)));


  // prepare shaders
  GLuint vert_shader = glCreateShader(GL_VERTEX_SHADER);
  char* vert_path = "../src/pass_through.vert";
  char* vert_src_str = file_to_cstring(vert_path);
  const GLchar* vert_src = (const GLchar*)vert_src_str;
  if (!vert_src) {
    fprintf(stderr, "Failure to open file with path %s\n", vert_path);
    return -1;
  }
  glShaderSource(vert_shader, 1, &vert_src, 0);
  glCompileShader(vert_shader);
  GLint is_compiled = GL_FALSE;  
  glGetShaderiv(vert_shader, GL_COMPILE_STATUS, &is_compiled);
  if (is_compiled == GL_FALSE) {
    // TODO: error handling
    fprintf(stderr, "Shader didn't compile. %s\n", vert_path);
    
    int log_length = 0;
    glGetShaderiv(vert_shader, GL_INFO_LOG_LENGTH, &log_length);
    char* info = (char*)malloc(log_length);
    glGetShaderInfoLog(vert_shader, log_length, &log_length, info);
    fprintf(stderr, "%s\n", info);
    free(info);
    glDeleteShader(vert_shader);
    return -1;
  }

  GLuint frag_shader = glCreateShader(GL_FRAGMENT_SHADER);
  char* frag_path = "../src/pass_through.frag";
  char* frag_src_str = file_to_cstring(frag_path);
  const GLchar* frag_src = (const GLchar*)frag_src_str;
  if (!frag_src) {
    fprintf(stderr, "Failure to open file with path %s\n", frag_path);
    return -1;
  }
  glShaderSource(frag_shader, 1, &frag_src, 0);
  glCompileShader(frag_shader);
  glGetShaderiv(frag_shader, GL_COMPILE_STATUS, &is_compiled);
  if (is_compiled == GL_FALSE) {
    // TODO: error handling
    fprintf(stderr, "Shader didn't compile. %s\n", frag_path);
    int log_length = 0;
    glGetShaderiv(frag_shader, GL_INFO_LOG_LENGTH, &log_length);
    char* info = (char*)malloc(log_length);
    glGetShaderInfoLog(frag_shader, log_length, &log_length, info);
    fprintf(stderr, "%s\n", info);
    free(info);
    glDeleteShader(vert_shader);
    glDeleteShader(frag_shader);
    return -1;
  }

  GLuint program = glCreateProgram();
  glAttachShader(program, vert_shader);
  glAttachShader(program, frag_shader);
  glBindAttribLocation(program, 0, "in_pos");
  glBindAttribLocation(program, 1, "in_uv");
  
  glLinkProgram(program);
  GLint is_linked = 0;
  glGetProgramiv(program, GL_LINK_STATUS, (int*)&is_linked);
  if (is_linked == GL_FALSE) {
    fprintf(stderr, "Shader didn't compile. %s\n", frag_path);
    glDeleteProgram(program);
    glDeleteShader(vert_shader);
    glDeleteShader(frag_shader);
    return -1;
  }
  glDetachShader(program, vert_shader);
  glDetachShader(program, frag_shader);
  free(vert_src_str);
  free(frag_src_str);

  glUseProgram(program);
  
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);
  glDisable(GL_CULL_FACE);


  // get the texture ready
  FrameContext frame_ctx = init_frame_context(argv[1]);

  GLuint frame_texture;
  glGenTextures(1, &frame_texture);
  
  glBindTexture(GL_TEXTURE_2D, frame_texture);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);  
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 
    frame_ctx.width, frame_ctx.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 
    frame_ctx.frame_data);
  
  
  bool running = true;
  while (running) {
    // event handling
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
        case SDL_EVENT_QUIT:
          running = false;
          break;
        case SDL_EVENT_KEY_DOWN:
          switch (event.key.keysym.sym) {
            case SDLK_ESCAPE:
              running = false;
              break;
            default:
              break;
          }
          break;
        default:
          break;
      }
    }
    
  
    int res = -1;
    // keep trying until we receive a frame
    do {
      res = get_one_video_frame(&frame_ctx);
    
      #if LOGGING
        if (res == AVERROR(EAGAIN)) {  fprintf(stderr, "EAGAIN (frame not in decodable state, probably try again)"); }
        else if (res == AVERROR_EOF) { fprintf(stderr, "End of frame.\n"); }
        else {                         fprintf(stderr, "Other error.\n"); }
      #endif
    
    } while (res < 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 
      frame_ctx.width, frame_ctx.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 
      frame_ctx.frame_data);
  
    
    glViewport(0, 0, sdl_context.width, sdl_context.height);
    glClearColor(1., 1., 1., 1.);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    
    
    SDL_GL_SwapWindow(sdl_context.window);
  
  }
  

  cleanup_frame_context(frame_ctx);
  return 0;
}