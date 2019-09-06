/* symmetrical-lamp - Shader Live Editor by Team210
 * Copyright (C) 2019  Alexander Kraus <nr4@z10.info>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "Windows.h"
#include "GL/GL.h"
#include "glext.h"

HMIDIOUT hMidiOut;
HWND hwnd;
HDC hdc;
int override_index = 0,
nfiles,
*handles = 0,
*programs = 0,
*time_locations = 0,
*resolution_locations = 0,
*fader0_locations = 0,
*fader1_locations = 0,
*fader2_locations = 0,
*fader3_locations = 0,
*fader4_locations = 0,
*fader5_locations = 0,
*fader6_locations = 0,
*fader7_locations = 0,
*fader8_locations = 0, 
index = 0,
dt_interval = 0,
dirty = 0;
char **shader_sources = 0;

double t_now = 0.,
fader_values[] = { 1.,0.,0.,0.,0.,0.,0.,0.,0.};

PFNGLCREATESHADERPROC glCreateShader;
PFNGLCREATEPROGRAMPROC glCreateProgram;
PFNGLSHADERSOURCEPROC glShaderSource;
PFNGLCOMPILESHADERPROC glCompileShader;
PFNGLATTACHSHADERPROC glAttachShader;
PFNGLLINKPROGRAMPROC glLinkProgram;
PFNGLUSEPROGRAMPROC glUseProgram;
PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;
PFNGLUNIFORM1FPROC glUniform1f;
PFNGLUNIFORM2FPROC glUniform2f;
PFNGLUNIFORM1IPROC glUniform1i;
PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers;
PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer;
PFNGLGETSHADERIVPROC glGetShaderiv;
PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
PFNGLGETPROGRAMIVPROC glGetProgramiv;
PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;
PFNGLDELETESHADERPROC glDeleteShader;
PFNGLDELETEPROGRAMPROC glDeleteProgram;
PFNGLDETACHSHADERPROC glDetachShader;

DWORD dwWaitStatus, watch_directory_thread_id; 
HANDLE dwChangeHandles[2],
    watch_directory_thread; 
TCHAR lpDrive[4];
TCHAR lpFile[_MAX_FNAME];
TCHAR lpExt[_MAX_EXT];

void debug(int shader_handle)
{
    printf("    Debugging shader with handle %d.\n", shader_handle);
    int compile_status = 0;
    glGetShaderiv(shader_handle, GL_COMPILE_STATUS, &compile_status);
    if(compile_status != GL_TRUE)
    {
        printf("    FAILED.\n");
        GLint len;
        glGetShaderiv(shader_handle, GL_INFO_LOG_LENGTH, &len);
        printf("    Log length: %d\n", len);
        GLchar *CompileLog = (GLchar*)malloc(len*sizeof(GLchar));
        glGetShaderInfoLog(shader_handle, len, NULL, CompileLog);
        printf("    Error messages:\n%s\n", CompileLog);
        free(CompileLog);
    }
    else
        printf("    Shader compilation successful.\n");
}

void debugp(int program)
{
    printf("    Debugging program with handle %d.\n", program);
    int compile_status = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &compile_status);
    if(compile_status != GL_TRUE)
    {
        printf("    FAILED.\n");
        GLint len;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &len);
        printf("    Log length: %d\n", len);
        GLchar *CompileLog = (GLchar*)malloc(len*sizeof(GLchar));
        glGetProgramInfoLog(program, len, NULL, CompileLog);
        printf("    Error messages:\n%s\n", CompileLog);
        free(CompileLog);
    }
    else
        printf("    Program linking successful.\n");
}

#define ACREA(id, type) \
    if(id == 0) { id = (type*)malloc(nfiles*sizeof(type));}\
    else {id = (type*)realloc(id, nfiles*sizeof(type));}

void ReloadShaders()
{
    // Remove old shaders
    for(int i=0; i<nfiles; ++i)
    {
        glDeleteShader(handles[i]);
        glDeleteProgram(programs[i]);
        free(shader_sources[i]);
    }
    
    // Browse shaders folder for shaders
    WIN32_FIND_DATA data;
    HANDLE hfile = FindFirstFile(".\\shaders\\*.frag", &data);
    char **filenames = (char **)malloc(sizeof(char*));
    filenames[0] = (char*)malloc(strlen(data.cFileName)+2+strlen(".\\shaders\\"));
    sprintf(filenames[0], ".\\shaders\\%s", data.cFileName);
    printf("Found %s\n", filenames[0]);
    nfiles = 1;
    while(FindNextFile(hfile, &data))
    {
        ++nfiles;
        filenames = (char**)realloc(filenames, nfiles*sizeof(char*));
        filenames[nfiles-1] = (char*)malloc(strlen(data.cFileName)+2+strlen(".\\shaders\\"));
        sprintf(filenames[nfiles-1], ".\\shaders\\%s", data.cFileName);
        printf("Found %s\n", filenames[nfiles-1]);
    } 
    FindClose(hfile);
    printf("Read %d files.\n", nfiles);
    
    // Load shaders
    ACREA(handles, int);
    ACREA(programs, int);
    ACREA(time_locations, int);
    ACREA(resolution_locations, int);
    ACREA(fader0_locations, int);
    ACREA(fader1_locations, int);
    ACREA(fader2_locations, int);
    ACREA(fader3_locations, int);
    ACREA(fader4_locations, int);
    ACREA(fader5_locations, int);
    ACREA(fader6_locations, int);
    ACREA(fader7_locations, int);
    ACREA(fader8_locations, int);
    ACREA(shader_sources, char*);
    
    for(int i=0; i<nfiles; ++i)
    {
        printf("Loading Shader %d from %s\n", i, filenames[i]);
        
        FILE *f = fopen(filenames[i], "rt");
        if(f == 0)printf("Failed to open file: %s\n", filenames[i]);
        fseek(f, 0, SEEK_END);
        int filesize = ftell(f);
        fseek(f, 0, SEEK_SET);
        shader_sources[i] = (char*)malloc(filesize+2);
        fread(shader_sources[i], 1, filesize, f);
        fclose(f);
        printf("%s\n\n==============\n", shader_sources[i]);
        
        handles[i] = glCreateShader(GL_FRAGMENT_SHADER);
        programs[i] = glCreateProgram();
        glShaderSource(handles[i], 1, (GLchar **)&shader_sources[i], &filesize);
        glCompileShader(handles[i]);
        debug(handles[i]);
        glAttachShader(programs[i], handles[i]);
        glLinkProgram(programs[i]);
        debugp(programs[i]);
        
        glDetachShader(programs[i], handles[i]);
        
        glUseProgram(programs[i]);
        time_locations[i] = glGetUniformLocation(programs[i], "iTime");
        resolution_locations[i] = glGetUniformLocation(programs[i], "iResolution");
        fader0_locations[i] = glGetUniformLocation(programs[i], "iFader0");
        fader1_locations[i] = glGetUniformLocation(programs[i], "iFader1");
        fader2_locations[i] = glGetUniformLocation(programs[i], "iFader2");
        fader3_locations[i] = glGetUniformLocation(programs[i], "iFader3");
        fader4_locations[i] = glGetUniformLocation(programs[i], "iFader4");
        fader5_locations[i] = glGetUniformLocation(programs[i], "iFader5");
        fader6_locations[i] = glGetUniformLocation(programs[i], "iFader6");
        fader7_locations[i] = glGetUniformLocation(programs[i], "iFader7");
        fader8_locations[i] = glGetUniformLocation(programs[i], "iFader8");
        
    }
    
    for(int i=0; i<nfiles; ++i) free(filenames[i]);
    free(filenames);
}

void watch_directory(const char *lpDir)
{
    _splitpath_s(lpDir, lpDrive, 4, NULL, 0, lpFile, _MAX_FNAME, lpExt, _MAX_EXT);
    
    lpDrive[2] = (TCHAR)'\\';
    lpDrive[3] = (TCHAR)'\0';
    
    dwChangeHandles[0] = FindFirstChangeNotification( 
    lpDir,                         // directory to watch 
    FALSE,                         // do not watch subtree 
    FILE_NOTIFY_CHANGE_FILE_NAME); // watch file name changes 
    
    if (dwChangeHandles[0] == INVALID_HANDLE_VALUE) 
    {
        printf("\n ERROR: FindFirstChangeNotification function failed.\n");
        ExitProcess(GetLastError()); 
    }
    
    dwChangeHandles[1] = FindFirstChangeNotification( 
    lpDir,                       // directory to watch 
    TRUE,                          // watch the subtree 
    FILE_NOTIFY_CHANGE_LAST_WRITE);  // watch last write changes
    
    if (dwChangeHandles[1] == INVALID_HANDLE_VALUE) 
    {
        printf("\n ERROR: FindFirstChangeNotification function failed.\n");
        ExitProcess(GetLastError()); 
    }

    if ((dwChangeHandles[0] == NULL) || (dwChangeHandles[1] == NULL))
    {
        printf("\n ERROR: Unexpected NULL from FindFirstChangeNotification.\n");
        ExitProcess(GetLastError()); 
    }
}

void __stdcall  directory_watch_thread()
{
    while (TRUE) 
    { 
        printf("\nWaiting for notification...\n");
        
        dwWaitStatus = WaitForMultipleObjects(2, dwChangeHandles, 
                                              FALSE, INFINITE); 
        
        switch (dwWaitStatus) 
        { 
            case WAIT_OBJECT_0: 
                printf("File created/renamed/deleted\n");
                
                dirty = 1;
                
                if ( FindNextChangeNotification(dwChangeHandles[0]) == FALSE )
                {
                    printf("\n ERROR: FindNextChangeNotification function failed.\n");
                    ExitProcess(GetLastError()); 
                }
                break; 
                
            case WAIT_OBJECT_0 + 1: 
                printf("File was touched.\n");
                
                dirty = 1;
                
                if (FindNextChangeNotification(dwChangeHandles[1]) == FALSE )
                {
                    printf("\n ERROR: FindNextChangeNotification function failed.\n");
                    ExitProcess(GetLastError()); 
                }
                break; 
                
            case WAIT_TIMEOUT:
                printf("\nNo changes in the timeout period.\n");
                break;
                
            default: 
                printf("\n ERROR: Unhandled dwWaitStatus.\n");
                ExitProcess(GetLastError());
                break;
        }
    }
}

int btns = 0;
void select_button(int _index)
{
    if(_index < 40)
    {
        for(int i=0; i<nfiles; ++i)
        {
            DWORD out_msg = 0x9 << 4 | i << 8 | 72 << 16;
            midiOutShortMsg(hMidiOut, out_msg);
        }
        for(int i=nfiles; i<40; ++i)
        {
           DWORD out_msg = 0x8 << 4 | i << 8 | 0 << 16;
            midiOutShortMsg(hMidiOut, out_msg);
        }
        
        override_index = _index;
        
        DWORD out_msg = 0x9 << 4 | _index << 8 | 13 << 16;
        midiOutShortMsg(hMidiOut, out_msg);
    }
    else 
    {
        dt_interval = (_index - 0x52) % 5;
        
        for(int i=dt_interval; i<5; ++i)
        {
            DWORD out_msg = 0x9 << 4 | (0x52 + i) << 8 | 67 << 16;
            midiOutShortMsg(hMidiOut, out_msg);
        }
        for(int i=0; i<=dt_interval; ++i)
        {
            DWORD out_msg = 0x9 << 4 | (0x52 + i) << 8 | 95 << 16;
            midiOutShortMsg(hMidiOut, out_msg);
        }
    }
}

#define NOTE_OFF 0x8
#define NOTE_ON 0x9
#define CONTROL_CHANGE 0xB
#define TIME_DIAL 0x2F
void CALLBACK MidiInProc_apc40mk2(HMIDIIN hMidiIn, UINT wMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2)
{
    if(wMsg == MIM_DATA)
    {
        BYTE b1 = (dwParam1 >> 24) & 0xFF,
        b2 = (dwParam1 >> 16) & 0xFF,
        b3 = (dwParam1 >> 8) & 0xFF,
        b4 = dwParam1 & 0xFF;
        BYTE b3lo = b3 & 0xF,
        b3hi = (b3 >> 4) & 0xF,
        b4lo = b4 & 0xF,
        b4hi = (b4 >> 4) & 0xF;
        
        BYTE channel = b4lo,
        button = b3;
        
        if(b4hi == NOTE_OFF)
        {
            select_button(button);
            
            // Logo 210
            if(button == 0x59)
            {
                char data[40] = 
                {
                    1,  1,  0,  1,  0,  1,  1,  0,
                    12, 12, 1,  1,  1,  12, 12, 1,
                    0,  0,  1,  1,  1,  0,  0,  1,
                    0,  0,  1,  1,  1,  0,  0,  1,
                    1,  1,  12, 1,  12, 1,  1,  12
                };
                
                for(int i=0; i<40; ++i)
                {
                    
                    DWORD out_msg;
                    if(data[i] == 0) 
                    {
                        out_msg = 0x8 << 4 | i << 8 | 0 << 16;
                    }
                    else
                    {
                        out_msg = 0x9 << 4 | i << 8 | 1+(data[i]+btns) %125 << 16;
                    }
                    midiOutShortMsg(hMidiOut, out_msg);
                }
                btns = 1+(btns+1)%125;
            }
            // Kewlers Logo
            else if(button == 0x57)
            {
                char data[40] = 
                {
                    3,3,3,3,1,1,1,1,
                    3,3,3,3,1,1,1,1,
                    3,3,3,0,0,1,1,1,
                    0,3,0,0,0,0,1,0,
                    0,7,0,0,0,0,9,0
                };
                
                for(int i=0; i<40; ++i)
                {
                    
                    DWORD out_msg;
                    if(data[i] == 0) 
                    {
                        out_msg = 0x8 << 4 | i << 8 | 0 << 16;
                    }
                    else
                    {
                        out_msg = 0x9 << 4 | i << 8 | 1+(data[i]+btns) %125 << 16;
                    }
                    midiOutShortMsg(hMidiOut, out_msg);
                }
                btns = 1+(btns+1)%125;
            }
            else if(button >= 0x52 && button <= 0x56)
            {
                printf("dt_interval: %d\n", dt_interval);
            }
        }
        else if(b4hi == CONTROL_CHANGE)// Channel select
        {
            if(button == TIME_DIAL)
            {
                if(b2 < 0x3A)
                {
                    if(dt_interval == 0)t_now += 1.e-3;
                    else if(dt_interval == 1)t_now += 1.e-2;
                    else if(dt_interval == 2)t_now += 1.e-1;
                    else if(dt_interval == 3)t_now += 1.;
                    else if(dt_interval == 3)t_now += 1.e1;
                }
                else
                {
                    if(dt_interval == 0)t_now -= 1.e-3;
                    else if(dt_interval == 1)t_now -= 1.e-2;
                    else if(dt_interval == 2)t_now -= 1.e-1;
                    else if(dt_interval == 3)t_now -= 1.;
                    else if(dt_interval == 3)t_now -= 1.e1;
                }
            }
            else
            {
                if(button == 0x07)
                {
                    fader_values[channel] = (double)b2/(double)0x7F;
                }
            }
        }
        printf("wMsg=MIM_DATA, dwParam1=%08x, byte=%02x %02x h_%01x l_%01x %02x, dwParam2=%08x\n", dwParam1, b1, b2, b3hi, b3lo, b4, dwParam2);
    }
    
    index = override_index % nfiles;
    
    UpdateWindow(hwnd);
}

int WINAPI demo(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    AllocConsole();
    freopen("CONIN$", "r", stdin);
    freopen("CONOUT$", "w", stdout);
    freopen("CONOUT$", "w", stderr);
    
    // Display demo window
    CHAR WindowClass[]  = "Team210 Demo Window";
    
    WNDCLASSEX wc = { 0 };
    wc.cbSize = sizeof(wc);
    wc.style = CS_OWNDC | CS_VREDRAW | CS_HREDRAW;
    wc.lpfnWndProc = &DefWindowProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = NULL;
    wc.lpszMenuName = NULL;
    wc.lpszClassName = WindowClass;
    wc.hIconSm = NULL;
    
    RegisterClassEx(&wc);
    
    hwnd = CreateWindowEx(0, WindowClass, ":: Team210 :: GO - MAKE A DEMO ::", WS_POPUP | WS_VISIBLE, 0, 0, 1920, 1080, NULL, NULL, hInstance, 0);
    
    DEVMODE dm = { 0 };
    dm.dmSize = sizeof(dm);
    dm.dmPelsWidth = 1920;
    dm.dmPelsHeight = 1080;
    dm.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT;
    
    ChangeDisplaySettings(&dm, CDS_FULLSCREEN);
    
    ShowWindow(hwnd, TRUE);
    UpdateWindow(hwnd);
    
    PIXELFORMATDESCRIPTOR pfd =
    {
        sizeof(PIXELFORMATDESCRIPTOR),
        1,
        PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,    //Flags
        PFD_TYPE_RGBA,        // The kind of framebuffer. RGBA or palette.
        32,                   // Colordepth of the framebuffer.
        0, 0, 0, 0, 0, 0,
        0,
        0,
        0,
        0, 0, 0, 0,
        24,                   // Number of bits for the depthbuffer
        8,                    // Number of bits for the stencilbuffer
        0,                    // Number of Aux buffers in the framebuffer.
        PFD_MAIN_PLANE,
        0,
        0, 0, 0
    };
    
    hdc = GetDC(hwnd);
    
    int  pf = ChoosePixelFormat(hdc, &pfd);
    SetPixelFormat(hdc, pf, &pfd);
    
    HGLRC glrc = wglCreateContext(hdc);
    wglMakeCurrent(hdc, glrc);
    
    glCreateShader = (PFNGLCREATESHADERPROC) wglGetProcAddress("glCreateShader");
    glCreateProgram = (PFNGLCREATEPROGRAMPROC) wglGetProcAddress("glCreateProgram");
    glShaderSource = (PFNGLSHADERSOURCEPROC) wglGetProcAddress("glShaderSource");
    glCompileShader = (PFNGLCOMPILESHADERPROC) wglGetProcAddress("glCompileShader");
    glAttachShader = (PFNGLATTACHSHADERPROC) wglGetProcAddress("glAttachShader");
    glLinkProgram = (PFNGLLINKPROGRAMPROC) wglGetProcAddress("glLinkProgram");
    glUseProgram = (PFNGLUSEPROGRAMPROC) wglGetProcAddress("glUseProgram");
    glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC) wglGetProcAddress("glGetUniformLocation");
    glUniform1f = (PFNGLUNIFORM1FPROC) wglGetProcAddress("glUniform1f");
    glUniform2f = (PFNGLUNIFORM2FPROC) wglGetProcAddress("glUniform2f");
    glUniform1i = (PFNGLUNIFORM1IPROC) wglGetProcAddress("glUniform1i");
    glGenFramebuffers = (PFNGLGENFRAMEBUFFERSPROC) wglGetProcAddress("glGenFramebuffers");
    glBindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC) wglGetProcAddress("glBindFramebuffer");
    glGetProgramiv = (PFNGLGETPROGRAMIVPROC) wglGetProcAddress("glGetProgramiv");
    glGetShaderiv = (PFNGLGETSHADERIVPROC) wglGetProcAddress("glGetShaderiv");
    glGetShaderInfoLog = (PFNGLGETSHADERINFOLOGPROC) wglGetProcAddress("glGetShaderInfoLog");
    glGetProgramInfoLog = (PFNGLGETPROGRAMINFOLOGPROC) wglGetProcAddress("glGetProgramInfoLog");
    glDeleteShader = (PFNGLDELETESHADERPROC) wglGetProcAddress("glDeleteShader");
    glDetachShader = (PFNGLDETACHSHADERPROC) wglGetProcAddress("glDetachShader");
    glDeleteProgram = (PFNGLDELETEPROGRAMPROC) wglGetProcAddress("glDeleteProgram");
    
    ShowCursor(FALSE);
    
    ReloadShaders();
    
    UINT nMidiDeviceNum;
    MIDIINCAPS caps;
    
    nMidiDeviceNum = midiInGetNumDevs();
    if(nMidiDeviceNum == 0) 
    {
        printf("No MIDI input devices connected.\n");
    }
    else
    {
        printf("Available MIDI input devices:\n");
        for (unsigned int i = 0; i < nMidiDeviceNum; ++i) 
        {
            midiInGetDevCaps(i, &caps, sizeof(MIDIINCAPS));
            printf("->%d: %s ", i, caps.szPname);
            
            if(!strcmp("APC40 mkII", caps.szPname))
            {
                HMIDIIN hMidiDevice;
                MMRESULT rv = midiInOpen(&hMidiDevice, i, (DWORD)(void*)MidiInProc_apc40mk2, 0, CALLBACK_FUNCTION);
                midiInStart(hMidiDevice);
                
                printf(" >> opened.\n");
            }
            else
            {
                printf("(Unsupported MIDI controller).\n");
            }
        }
    }
    
    MIDIOUTCAPS ocaps;
    nMidiDeviceNum = midiOutGetNumDevs();
    
    if(nMidiDeviceNum == 0) 
    {
        printf("No MIDI output devices connected.\n");
    }
    else
    {
        printf("Available MIDI output devices:\n");
        for (unsigned int i = 0; i < nMidiDeviceNum; ++i) 
        {
            midiOutGetDevCaps(i, &ocaps, sizeof(MIDIOUTCAPS));
            printf("->%d: %s ", i, ocaps.szPname);
            
            if(!strcmp("APC40 mkII", ocaps.szPname))
            {
                MMRESULT rv = midiOutOpen (&hMidiOut, i, 0, 0, CALLBACK_NULL);
            }
            else
            {
                printf("(Unsupported MIDI controller).\n");
            }
        }
    }
    
    select_button(0);
    
    watch_directory(".\\shaders");
    watch_directory_thread = CreateThread(NULL, // security attributes ( default if NULL )
                            0, // stack SIZE default if 0
                            directory_watch_thread, // Start Address
                            NULL, // input data
                            0, // creational flag ( start if  0 )
                            &watch_directory_thread_id); // thread ID
    
    while(1)
    {
        MSG msg = { 0 };
        while ( PeekMessageA( &msg, NULL, 0, 0, PM_REMOVE ) )
        {
            if ( msg.message == WM_QUIT || (msg.message = WM_KEYDOWN && msg.wParam == VK_ESCAPE ) )
            {
                ExitProcess(0);
                return 0;
            }
            
            TranslateMessage( &msg );
            DispatchMessageA( &msg );
        }
        
        glUseProgram(programs[index]);
        glUniform2f(resolution_locations[index], 1920, 1080);
        glUniform1f(time_locations[index], t_now);
        glUniform1f(fader0_locations[index], fader_values[0]);
        glUniform1f(fader1_locations[index], fader_values[1]);
        glUniform1f(fader2_locations[index], fader_values[2]);
        glUniform1f(fader3_locations[index], fader_values[3]);
        glUniform1f(fader4_locations[index], fader_values[4]);
        glUniform1f(fader5_locations[index], fader_values[5]);
        glUniform1f(fader6_locations[index], fader_values[6]);
        glUniform1f(fader7_locations[index], fader_values[7]);
        glUniform1f(fader8_locations[index], fader_values[8]);
        
        glViewport(0,0,1920,1080);
        
        glBegin(GL_QUADS);
        glVertex3f(-1,-1,0);
        glVertex3f(-1,1,0);
        glVertex3f(1,1,0);
        glVertex3f(1,-1,0);
        glEnd();
        
        SwapBuffers(hdc);
        
        if(dirty) 
        {
            ReloadShaders();
            dirty = 0;
        }
    }
    
    return 0;
}
