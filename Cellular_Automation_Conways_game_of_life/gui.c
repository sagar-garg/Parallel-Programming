
// Most of this code was originally written for obst (https://github.com/suyjuris/obst).
// I would recommend not reading it, it is all pretty straightforward.

#ifdef DISABLE_GUI

void gui_create_window(int argc, char** argv) {}
void gui_draw(int height, int width, int* grid) {}

#else

#include <assert.h>
#include <time.h>
#include <locale.h>
#include <sys/stat.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/extensions/Xrandr.h>
#include <GL/glx.h>

#include <mpi.h>

typedef void (*glBindBuffer_t) (GLenum target, GLuint buffer);
typedef void (*glGenBuffers_t) (GLsizei n, GLuint *buffers);
typedef void (*glBufferData_t) (GLenum target, GLsizeiptr size, const void *data, GLenum usage);
typedef void (*glAttachShader_t) (GLuint program, GLuint shader);
typedef void (*glBindAttribLocation_t) (GLuint program, GLuint index, const GLchar *name);
typedef void (*glCompileShader_t) (GLuint shader);
typedef GLuint (*glCreateProgram_t) (void);
typedef GLuint (*glCreateShader_t) (GLenum type);
typedef void (*glEnableVertexAttribArray_t) (GLuint index);
typedef void (*glGetProgramiv_t) (GLuint program, GLenum pname, GLint *params);
typedef void (*glGetProgramInfoLog_t) (GLuint program, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
typedef void (*glGetShaderiv_t) (GLuint shader, GLenum pname, GLint *params);
typedef void (*glGetShaderInfoLog_t) (GLuint shader, GLsizei bufSize, GLsizei *length, GLchar *infoLog);
typedef GLint (*glGetUniformLocation_t) (GLuint program, const GLchar *name);
typedef void (*glLinkProgram_t) (GLuint program);
typedef void (*glShaderSource_t) (GLuint shader, GLsizei count, const GLchar *const*string, const GLint *length);
typedef void (*glUseProgram_t) (GLuint program);
typedef void (*glUniform1f_t) (GLint location, GLfloat v0);
typedef void (*glUniform2f_t) (GLint location, GLfloat v0, GLfloat v1);
typedef void (*glUniform1i_t) (GLint location, GLint v0);
typedef void (*glVertexAttribPointer_t) (GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer);
typedef void (*glGenerateMipmap_t) (GLenum target);
typedef void (*glBindVertexArray_t) (GLuint array);
typedef void (*glGenVertexArrays_t) (GLsizei n, GLuint *arrays);
glBindBuffer_t glBindBuffer;
glGenBuffers_t glGenBuffers;
glBufferData_t glBufferData;
glAttachShader_t glAttachShader;
glBindAttribLocation_t glBindAttribLocation;
glCompileShader_t glCompileShader;
glCreateProgram_t glCreateProgram;
glCreateShader_t glCreateShader;
glEnableVertexAttribArray_t glEnableVertexAttribArray;
glGetProgramiv_t glGetProgramiv;
glGetProgramInfoLog_t glGetProgramInfoLog;
glGetShaderiv_t glGetShaderiv;
glGetShaderInfoLog_t glGetShaderInfoLog;
glGetUniformLocation_t glGetUniformLocation;
glLinkProgram_t glLinkProgram;
glShaderSource_t glShaderSource;
glUseProgram_t glUseProgram;
glUniform1f_t glUniform1f;
glUniform2f_t glUniform2f;
glUniform1i_t glUniform1i;
glVertexAttribPointer_t glVertexAttribPointer;
glGenerateMipmap_t glGenerateMipmap;
glBindVertexArray_t glBindVertexArray;
glGenVertexArrays_t glGenVertexArrays;

void _platform_init_gl_pointers() {
    glBindBuffer = (glBindBuffer_t)glXGetProcAddress((uint8_t*)"glBindBuffer"); assert(glBindBuffer);
    glGenBuffers = (glGenBuffers_t)glXGetProcAddress((uint8_t*)"glGenBuffers"); assert(glGenBuffers);
    glBufferData = (glBufferData_t)glXGetProcAddress((uint8_t*)"glBufferData"); assert(glBufferData);
    glAttachShader = (glAttachShader_t)glXGetProcAddress((uint8_t*)"glAttachShader"); assert(glAttachShader);
    glBindAttribLocation = (glBindAttribLocation_t)glXGetProcAddress((uint8_t*)"glBindAttribLocation"); assert(glBindAttribLocation);
    glCompileShader = (glCompileShader_t)glXGetProcAddress((uint8_t*)"glCompileShader"); assert(glCompileShader);
    glCreateProgram = (glCreateProgram_t)glXGetProcAddress((uint8_t*)"glCreateProgram"); assert(glCreateProgram);
    glCreateShader = (glCreateShader_t)glXGetProcAddress((uint8_t*)"glCreateShader"); assert(glCreateShader);
    glEnableVertexAttribArray = (glEnableVertexAttribArray_t)glXGetProcAddress((uint8_t*)"glEnableVertexAttribArray"); assert(glEnableVertexAttribArray);
    glGetProgramiv = (glGetProgramiv_t)glXGetProcAddress((uint8_t*)"glGetProgramiv"); assert(glGetProgramiv);
    glGetProgramInfoLog = (glGetProgramInfoLog_t)glXGetProcAddress((uint8_t*)"glGetProgramInfoLog"); assert(glGetProgramInfoLog);
    glGetShaderiv = (glGetShaderiv_t)glXGetProcAddress((uint8_t*)"glGetShaderiv"); assert(glGetShaderiv);
    glGetShaderInfoLog = (glGetShaderInfoLog_t)glXGetProcAddress((uint8_t*)"glGetShaderInfoLog"); assert(glGetShaderInfoLog);
    glGetUniformLocation = (glGetUniformLocation_t)glXGetProcAddress((uint8_t*)"glGetUniformLocation"); assert(glGetUniformLocation);
    glLinkProgram = (glLinkProgram_t)glXGetProcAddress((uint8_t*)"glLinkProgram"); assert(glLinkProgram);
    glShaderSource = (glShaderSource_t)glXGetProcAddress((uint8_t*)"glShaderSource"); assert(glShaderSource);
    glUseProgram = (glUseProgram_t)glXGetProcAddress((uint8_t*)"glUseProgram"); assert(glUseProgram);
    glUniform1f = (glUniform1f_t)glXGetProcAddress((uint8_t*)"glUniform1f"); assert(glUniform1f);
    glUniform2f = (glUniform2f_t)glXGetProcAddress((uint8_t*)"glUniform2f"); assert(glUniform2f);
    glUniform1i = (glUniform1i_t)glXGetProcAddress((uint8_t*)"glUniform1i"); assert(glUniform1i);
    glVertexAttribPointer = (glVertexAttribPointer_t)glXGetProcAddress((uint8_t*)"glVertexAttribPointer"); assert(glVertexAttribPointer);
    glGenerateMipmap = (glGenerateMipmap_t)glXGetProcAddress((uint8_t*)"glGenerateMipmap"); assert(glGenerateMipmap);
    glBindVertexArray = (glBindVertexArray_t)glXGetProcAddress((uint8_t*)"glBindVertexArray"); assert(glBindVertexArray);
    glGenVertexArrays = (glGenVertexArrays_t)glXGetProcAddress((uint8_t*)"glGenVertexArrays"); assert(glGenVertexArrays);
}

typedef GLXContext (*glXCreateContextAttribsARB_t) (
    Display *dpy, GLXFBConfig config, GLXContext share_context, Bool direct, const int *attrib_list
);

// Indices for all the vertex attributes
enum Platform_state_Attributes {
    Platform_state_UIRECT_POS = 0, Platform_state_UIRECT_ATTR_COUNT,
};

// Names for all uniforms
enum Platform_state_Uniforms {
    Platform_state_UIRECT_ORIGIN, Platform_state_UIRECT_SCALE, Platform_state_UIRECT_SAMPLER,
    Platform_state_UNIFORMS_COUNT
};

typedef struct Platform_state {
    Display* display;
    Window window;
    GLXWindow window_glx;
    Atom net_wm_state, net_wm_state_fullscreen, type_atom;
    Atom wm_protocols, wm_delete_window;
    int xrandr_event;

    double redraw_next, redraw_last;

    int64_t screen_w, screen_h, rate;
    float ox, oy, sx, sy, sc;
    
    // Ids of the shader programs
    GLuint program_uirect;
    // Ids of the uniforms
    GLint uniforms[Platform_state_UNIFORMS_COUNT];

    // Buffers for the vertex attribute arrays
    float buf_uirect_pos_data[8];
    int buf_uirect_pos_size;

    // Names of the buffers for the vertex attributes of each shader
    GLuint buffers_uirect[Platform_state_UIRECT_ATTR_COUNT];

    GLuint uirect_tex;
} Platform_state;

Platform_state global_platform = {
    .rate = -1,
    .redraw_next = -1,
    .buf_uirect_pos_data = {0.f, 1.f, 0.f, 0.f, 1.f, 1.f, 1.f, 0.f},
    .buf_uirect_pos_size = 8
};

void platform_redraw(double t) {
    global_platform.redraw_next = fminf(global_platform.redraw_next, t);
}

double platform_now() {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC_RAW, &t);
    return (double)t.tv_sec + (double)t.tv_nsec * 1e-9;
}

void _platform_handle_resize(int64_t width, int64_t height) {
    if (width  != -1) global_platform.screen_w = width;
    if (height != -1) global_platform.screen_h = height;

    glViewport(0.0, 0.0, global_platform.screen_w, global_platform.screen_h);

    global_platform.ox = -5.e-3f;
    global_platform.oy = -5.e-3f;
    global_platform.sc = fminf((float)global_platform.screen_w / 1010e-3f, (float)global_platform.screen_h / 1010e-3f);
    global_platform.oy -=  (float)global_platform.screen_h / global_platform.sc - 1010e-3f;
    global_platform.ox -= ((float)global_platform.screen_w / global_platform.sc - 1010e-3f) * 0.5f;
    global_platform.sx =  global_platform.sc / (float)global_platform.screen_w *  2.f;
    global_platform.sy = -global_platform.sc / (float)global_platform.screen_h *  2.f;
    global_platform.ox += (float)global_platform.screen_w / global_platform.sc / 2.f;
    global_platform.oy += (float)global_platform.screen_h / global_platform.sc / 2.f;
}

#define OBST_LOAD_SHADER(x, y) \
    GLuint x##_id = opengl_shader_load(y, x, sizeof(x), (char*)#x); \
    glAttachShader(program, x##_id)

#define OBST_PROGRAM_INIT(x) \
    program = glCreateProgram(); \
    context->program_##x = program; \
    assert(program); \
    OBST_LOAD_SHADER(shader_v_##x, GL_VERTEX_SHADER); \
    OBST_LOAD_SHADER(shader_f_##x, GL_FRAGMENT_SHADER)

#define OBST_PROGRAM_LINK(x) \
    opengl_program_link(program, (char*)#x)

#define OBST_ATTRIB(x, y) \
    glBindAttribLocation(program, Platform_state_##x, #y);
    
#define OBST_UNIFORM(x, y) \
    context->uniforms[Platform_state_##x] = glGetUniformLocation(program, #y); \
    assert(context->uniforms[Platform_state_##x] != -1)

#define OBST_GEN_BUFFERS(x, y) \
    glGenBuffers(Platform_state_##y##_ATTR_COUNT, context->buffers_##x)

#define OBST_DO_BUFFER(x, name, member, siz, type, norm)                      \
    glBindBuffer(GL_ARRAY_BUFFER, context->buffers_##x[Platform_state_##name]); \
    glBufferData(GL_ARRAY_BUFFER, context->member##_size * sizeof(context->member##_data[0]), context->member##_data, GL_STREAM_DRAW); \
    glVertexAttribPointer(Platform_state_##name, siz, type, norm, 0, 0); \
    glEnableVertexAttribArray(Platform_state_##name)

#define OBST_DO_UNIFORM(name, type, ...) \
    glUniform##type(context->uniforms[Platform_state_##name], __VA_ARGS__)

// Load shader from string, print an error and abort if there is an error
GLuint opengl_shader_load(GLenum type, char* source, int _source_size, char* name) {
   GLuint shader = glCreateShader(type);

   if (!shader) {
       fprintf(stderr, "Error: Could not create shader of type %d", type);
       abort();
   }

   GLint source_size = _source_size - 1;
   glShaderSource(shader, 1, (GLchar const**)&source, &source_size);
   glCompileShader(shader);
   
   GLint compiled;
   glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
   if (!compiled) {
      GLint info_size = 0;
      glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &info_size);

      char* info = (char*)malloc(info_size);
      glGetShaderInfoLog(shader, info_size, NULL, info);
      printf("Error while compiling shader %s:\n\n", name);
      puts(info);
      abort();
   }

   return shader;
}

// Link a shader program, print an error and abort if there is an error
void opengl_program_link(GLuint program, char* program_name) {
    glLinkProgram(program);
    GLint linked;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    if (!linked) {
        GLint info_size = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &info_size);
      
        char* info = (char*)malloc(info_size);
        glGetProgramInfoLog(program, info_size, NULL, info);
        printf("Error while linking program %s:\n\n", program_name);
        puts(info);
        abort();
    }
}

void _platform_init(Platform_state* context) {
    _platform_init_gl_pointers();
    
    // Webgl has no vertex array objects, OpenGL requires them. Yay. However, creating a single one
    // is fine, and we can do everything else Opengl style.
    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    char shader_v_uirect[] = 
        "attribute vec2 pos;\n"
        "varying vec2 v_pos;\n"
        "uniform vec2 origin;\n"
        "uniform vec2 scale;\n"
        "void main() {\n"
        "    gl_Position = vec4((pos - origin)*scale, vec2(0.5, 1));\n"
        "    v_pos = pos;\n"
        "}\n";

    char shader_f_uirect[] =
        "varying vec2 v_pos;\n"
        "uniform sampler2D sampler;\n"
        "void main() {\n"
        "    float c = texture2D(sampler, v_pos).r;\n"
        "    gl_FragColor = vec4(c, c, c, 1.0);\n"
        "}\n";

    GLuint program;
    OBST_PROGRAM_INIT(uirect);
    OBST_ATTRIB(UIRECT_POS, pos);
    OBST_PROGRAM_LINK(uirect);
    OBST_UNIFORM(UIRECT_ORIGIN, origin);
    OBST_UNIFORM(UIRECT_SCALE, scale);
    OBST_UNIFORM(UIRECT_SAMPLER, sampler);
    OBST_GEN_BUFFERS(uirect, UIRECT);

    glClearColor(1.f, 1.f, 1.f, 1.f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void _platform_draw(Platform_state* context, int height, int width, int* grid) {
    static char* tmp;
    if (!tmp) tmp = malloc(1000000);

    for (int y = 0; y < height && y < 1000; ++y) {
        for (int x = 0; x < width && x < 1000; ++x) {
            tmp[y*1000+x] = grid[y*width + x] ? 255 : 0;
        }
    }
    
    if (context->uirect_tex) {
        glDeleteTextures(1, &context->uirect_tex);
    }
    glGenTextures(1, &context->uirect_tex);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, context->uirect_tex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, 1000, 1000, 0, GL_RED, GL_UNSIGNED_BYTE, tmp);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    glClear(GL_COLOR_BUFFER_BIT);
    glClear(GL_DEPTH_BUFFER_BIT);
    
    glUseProgram(context->program_uirect);
    glActiveTexture(GL_TEXTURE1);
    
    OBST_DO_UNIFORM(UIRECT_ORIGIN,  2f, global_platform.ox, global_platform.oy);
    OBST_DO_UNIFORM(UIRECT_SCALE,   2f, global_platform.sx, global_platform.sy);
    OBST_DO_UNIFORM(UIRECT_SAMPLER, 1i, 1);

    OBST_DO_BUFFER(uirect, UIRECT_POS,    buf_uirect_pos,    2, GL_FLOAT, 0);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, context->buf_uirect_pos_size / 2);
    
    glXSwapBuffers(context->display, context->window_glx);

    platform_redraw(0);
}

void linux_set_wm_prop(Display* display, Window window, char const* property, char const* data) {
    Atom prop = XInternAtom(display, property, true);
    assert(prop != None);

    Atom type_string = XInternAtom(display, "STRING", true);
    assert(type_string != None);
    
    XChangeProperty(display, window, prop, type_string, 8, PropModeReplace, (uint8_t*)data, strlen(data));
}

void linux_set_wm_class(Display* display, Window window, int argc, char** argv) {
    char* instance = NULL;
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "-name") == 0 && i+1 < argc) {
            instance = argv[i+1];
        }
    }
    if (!instance) {
        instance = getenv("RESOURCE_NAME");
    }
    if (!instance) {
        int last_sep = 0;
        for (int i = 0; argv[0][i]; ++i) {
            if (argv[0][i] == '/') last_sep = i;
        }
        if (argv[0][last_sep] && argv[0][last_sep+1]) {
            instance = &argv[0][last_sep + 1];
        }
    }
    if (!instance) {
        instance = (char*)"nbody";
    }

    char* cls = (char*)"nbody";
    int buf_size = strlen(instance) + strlen(cls) + 2;
    char* buf = (char*)alloca(buf_size);
    assert( snprintf(buf, buf_size, "%s %s", instance, cls) < buf_size );
    
    linux_set_wm_prop(display, window, "WM_CLASS", buf);
}

void linux_fullscreen(Platform_state* platform) {
    // This tries to go to fullscreen via NET_WM_STATE, which might not work for some window managers

    if (platform->net_wm_state != None && platform->net_wm_state_fullscreen != None) {
        XEvent ev;
        memset(&ev, 0, sizeof(ev));
        ev.type = ClientMessage;
        ev.xclient.window = platform->window;
        ev.xclient.format = 32;
        ev.xclient.message_type = platform->net_wm_state;
        ev.xclient.data.l[0] = 2; // _NET_WM_STATE_TOGGLE
        ev.xclient.data.l[1] = platform->net_wm_state_fullscreen;
        ev.xclient.data.l[2] = 0;
        ev.xclient.data.l[3] = 1;
        ev.xclient.data.l[4] = 0;
        XSendEvent(platform->display, DefaultRootWindow(platform->display), false,
            SubstructureNotifyMask | SubstructureRedirectMask, &ev);
    }
}

void gui_create_window(int argc, char** argv) {
    // Do the OpenGL and X dance. I would recommend everyone to not read this code, if at all
    // possible, to preserve sanity. This should have been a single function call. If you must,
    // refer to the GLX 1.4 specification, the GLX_ARB_create_context extension, and the Xlib
    // specification to understand what all of this does.

    if (setlocale(LC_ALL, "") == NULL) {
        fprintf(stderr, "Warning: Could not set default locale.\n");
    }
    
    // Create the display, connect to the X server
    Display* display = XOpenDisplay(NULL);
    if (!display) {
        fprintf(stderr, "Error: could not open display (is the DISPLAY environment variable set correctly?)\n");
        exit(101);
    }
    global_platform.display = display;

    int screen = DefaultScreen(display);
    
    // Check for GLX 1.4 and extensions
    int glx_version[] = {1, 4};
    if (!glXQueryVersion(display, &glx_version[0], &glx_version[1])) {
        fprintf(stderr, "Error: glX version %d.%d not present\n", glx_version[0], glx_version[1]);
        exit(102);
    }

    char* gl_ext_present = (char*)glXQueryExtensionsString(display, screen);
    char* gl_ext_want[] = {(char*)"GLX_ARB_create_context_profile"};
    for (uint64_t i = 0; i < (uint64_t)(sizeof(gl_ext_want) / sizeof(gl_ext_want[0])); ++i) {
        if (!strstr(gl_ext_present, gl_ext_want[i])) {
            fprintf(stderr, "Error: OpenGL extension %s not present\n", gl_ext_want[i]);
            exit(105);
        }
    }

    int config_size;
    int config_attribs[] = {
        GLX_DOUBLEBUFFER, true,
        GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
        GLX_DEPTH_SIZE, 8,
        None
    };
    GLXFBConfig* config = glXChooseFBConfig(display, screen, config_attribs, &config_size);
    if (!config || config_size == 0) {
        fprintf(stderr, "Error: did not find a GLXFBConfig with the required attributes\n");
        exit(103);
    }

    glXCreateContextAttribsARB_t glXCreateContextAttribsARB
        = (glXCreateContextAttribsARB_t)glXGetProcAddress((uint8_t*)"glXCreateContextAttribsARB");
    assert(glXCreateContextAttribsARB); // We already checked for the presence of the extension

    // Create an OpenGL 3.2 context
    int context_attribs[] = {
        GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
        GLX_CONTEXT_MINOR_VERSION_ARB, 2,
        GLX_CONTEXT_FLAGS_ARB, GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
        GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
        None
    };
    GLXContext context = glXCreateContextAttribsARB(display, *config, 0, true, context_attribs);

    // Create the window
    XVisualInfo* visual = glXGetVisualFromFBConfig(display, *config);
    assert(visual);

    XSetWindowAttributes window_attrs = {0};
    window_attrs.colormap = XCreateColormap(display, DefaultRootWindow(display), visual->visual, AllocNone); // Apparently you need the colormap, else XCreateWindow gives a BadMatch error. No worries, this fact features prominently in the documentation and it was no bother at all.
    window_attrs.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask | ButtonPressMask
        | ButtonReleaseMask | StructureNotifyMask | PointerMotionMask | FocusChangeMask;

    Window window = XCreateWindow(display, DefaultRootWindow(display), 0, 0, 1080, 1080, 0,
        visual->depth, InputOutput, visual->visual, CWColormap | CWEventMask, &window_attrs); // We pass a type of InputOutput explicitly, as visual->c_class is an illegal value for some reason. A good reason, I hope.
    global_platform.window = window;

    // Set up xrandr
    int xrandr_event = 1 << 30, xrandr_error = 1 << 30;
    if (XRRQueryExtension(display, &xrandr_event, &xrandr_error)) {
        XRRSelectInput(display, window, RRScreenChangeNotifyMask);

        XRRScreenConfiguration* config_screen = XRRGetScreenInfo(display, window);
        global_platform.rate = XRRConfigCurrentRate(config_screen);
        XRRFreeScreenConfigInfo(config_screen);
    } else {
        fprintf(stderr, "Warning: Xrandr extension not present on X server, assuming refresh rate of 60 Hz.\n");
        global_platform.rate = 60;
    }
    global_platform.xrandr_event = xrandr_event;

    // Initialise window properties
    linux_set_wm_prop(display, window, "WM_NAME", "life simulation");
    linux_set_wm_prop(display, window, "WM_ICON_NAME", "life");
    linux_set_wm_class(display, window, argc, argv);

    // Set WM_PROTOCOLS
    Atom wm_protocols = XInternAtom(display, "WM_PROTOCOLS", true);
    Atom wm_delete_window = XInternAtom(display, "WM_DELETE_WINDOW", true);
    Atom type_atom = XInternAtom(display, "ATOM", true);
    assert(wm_protocols != None && wm_delete_window != None && type_atom != None);
    XChangeProperty(display, window, wm_protocols, type_atom, 32, PropModeReplace, (uint8_t*)&wm_delete_window, 1);
    global_platform.wm_protocols = wm_protocols;
    global_platform.wm_delete_window = wm_delete_window;

    // And some atoms needed for fullscreen mode
    global_platform.net_wm_state            = XInternAtom(display, "_NET_WM_STATE", true);
    global_platform.net_wm_state_fullscreen = XInternAtom(display, "_NET_WM_STATE_FULLSCREEN", true);
    global_platform.type_atom               = type_atom;
    
    // Map the context to the window
    GLXWindow window_glx = glXCreateWindow(display, *config, window, NULL);
    global_platform.window_glx = window_glx;
    glXMakeContextCurrent(display, window_glx, window_glx, context); // This returns a bool, but I cannot find what it means in the spec, so just ignore it. The greatness continues.

    // Do application-specific initialisation
    _platform_init(&global_platform);

    // Show the window
    XMapWindow(display, window);
}

void gui_draw(int height, int width, int* grid) {
    int x_fd = ConnectionNumber(global_platform.display);

    while (true) {
        bool redraw = false;
        if (XPending(global_platform.display) <= 0) {
            // No events left to process, now we redraw or wait
            
            double wait = global_platform.redraw_next - platform_now();
            if (wait == INFINITY) {
                // Wait for next event
            } else if (wait * (double)global_platform.rate > 0.5f) {
                fd_set fds;
                FD_ZERO(&fds);
                FD_SET(x_fd, &fds);
                
                struct timeval t;
                t.tv_sec = (long)wait;
                t.tv_usec = (long)((wait - (double)t.tv_sec)*1e6);
                
                int num = select(x_fd+1, &fds, NULL, NULL, &t);
                if (num == -1) {
                    fprintf(stderr, "Error: while executing select()\n");
                    perror("Error");
                    exit(105);
                } else if (num == 0) {
                    // Got timeout, redraw
                    redraw = true;
                } else {
                    // Check whether there is really an event there, or this is just one of select's
                    // classic wakeup pranks.
                    if (XPending(global_platform.display) <= 0) continue;
                    
                    // An event arrived, process.
                }
            } else {
                // Next frame is imminent, draw
                redraw = true;
            }
        }
        
        if (redraw) {
            global_platform.redraw_last = global_platform.redraw_next;
            global_platform.redraw_next = INFINITY;
            _platform_draw(&global_platform, height, width, grid);
            break;
        }
        
        XEvent event;
        XNextEvent(global_platform.display, &event);

        switch (event.type) {
        case Expose:
            platform_redraw(0);
            break;
        case KeyPress: {
            KeySym keysym;

            // You would think that we could do some dynamic resizing here if the buffer is too
            // small. However, the API does not seem to support it.
            char buffer_[64];
            XLookupString(&event.xkey, buffer_, 64, &keysym, NULL);
            if (keysym == XK_F11) {
                // Toggle fullscreen
                linux_fullscreen(&global_platform);
            }
        } break;
            
        case MappingNotify:
            if (event.xmapping.request == MappingModifier || event.xmapping.request == MappingKeyboard) {
                XRefreshKeyboardMapping(&event.xmapping);
            }
            break;
        case ClientMessage:
            if (event.xclient.message_type == global_platform.wm_protocols && event.xclient.data.l[0] - global_platform.wm_delete_window == 0) {
                MPI_Finalize();
                exit(0);
            }
            break;
        case ConfigureNotify:
            _platform_handle_resize(event.xconfigure.width, event.xconfigure.height);
            break;
        default:
            break;
        }

        if (event.type == global_platform.xrandr_event + RRScreenChangeNotify) {
            assert(XRRUpdateConfiguration(&event));
            XRRScreenConfiguration* config = XRRGetScreenInfo(global_platform.display, global_platform.window);
            global_platform.rate = XRRConfigCurrentRate(config);
            XRRFreeScreenConfigInfo(config);
        }
    }
}

#endif

