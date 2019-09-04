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
    *handles,
    *programs,
    *time_locations,
    *resolution_locations,
    *fader0_locations,
    *fader1_locations,
    *fader2_locations,
    *fader3_locations,
    *fader4_locations,
    *fader5_locations,
    *fader6_locations,
    *fader7_locations,
    *fader8_locations, 
    index = 0,
    dt_interval = 0;

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

int btns = 0;
void select_button(int index)
{
    for(int i=0; i<nfiles; ++i)
    {
        DWORD out_msg = 0x9 << 4 | i << 8 | 72 << 16;
        midiOutShortMsg(hMidiOut, out_msg);
    }
    
    if(index < 40)
    {
        override_index = index;
    }
    else dt_interval = (index - 0x52) % 5;
    
    DWORD out_msg = 0x9 << 4 | index << 8 | 13 << 16;
    midiOutShortMsg(hMidiOut, out_msg);
    
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

#define NOTE_OFF 0x8
#define NOTE_ON 0x9
#define CONTROL_CHANGE 0xB
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
//             if(button == TIME_DIAL)
//             {
//                 waveOutReset(hWaveOut);
//                 time_dial = (double)b2/(double)0x7F;
//                 
//                 int delta = (.9*time_dial+.09*time_fine_dial+.01*time_very_fine_dial) * t_end * (double)sample_rate;
//                 header.lpData = min(max(smusic1, smusic1+delta), smusic1+music1_size);
//                 header.dwBufferLength = 4 * (music1_size-delta);
//                 waveOutPrepareHeader(hWaveOut, &header, sizeof(WAVEHDR));
//                 waveOutWrite(hWaveOut, &header, sizeof(WAVEHDR));
//                 waveOutPause(hWaveOut);
//                 paused = 1;
//             }
//             else if(button == TIME_FINE_DIAL)
//             {
//                 waveOutReset(hWaveOut);
//                 time_fine_dial = (double)b2/(double)0x7F;
//                 
//                 int delta = (.9*time_dial+.09*time_fine_dial+.01*time_very_fine_dial) * t_end * (double)sample_rate;
//                 header.lpData = min(max(smusic1, smusic1+delta), smusic1+music1_size);
//                 header.dwBufferLength = 4 * (music1_size-delta);
//                 waveOutPrepareHeader(hWaveOut, &header, sizeof(WAVEHDR));
//                 waveOutWrite(hWaveOut, &header, sizeof(WAVEHDR));
//                 waveOutPause(hWaveOut);
//                 paused = 1;
//             }
//             else if(button == TIME_VERYFINE_DIAL)
//             {
//                 waveOutReset(hWaveOut);
//                 time_very_fine_dial = (double)b2/(double)0x7F;
//                 
//                 int delta = (.9*time_dial+.09*time_fine_dial+.01*time_very_fine_dial) * t_end * (double)sample_rate;
//                 header.lpData = min(max(smusic1, smusic1+delta), smusic1+music1_size);
//                 header.dwBufferLength = 4 * (music1_size-delta);
//                 waveOutPrepareHeader(hWaveOut, &header, sizeof(WAVEHDR));
//                 waveOutWrite(hWaveOut, &header, sizeof(WAVEHDR));
//                 waveOutPause(hWaveOut);
//                 paused = 1;
//             }
//             else
//             {
                if(button == 0x07)
                {
                    fader_values[channel] = (double)b2/(double)0x7F;
//                     glUniform1f(
                }
//             }
        }
        printf("wMsg=MIM_DATA, dwParam1=%08x, byte=%02x %02x h_%01x l_%01x %02x, dwParam2=%08x\n", dwParam1, b1, b2, b3hi, b3lo, b4, dwParam2);
    }
    
    index = override_index % nfiles;
    printf("fader0: %le\n", fader_values[0]);
    
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
    
    ShowCursor(FALSE);
    
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
    handles = (int*)malloc(nfiles*sizeof(int));
    programs = (int*)malloc(nfiles*sizeof(int));
    time_locations = (int*)malloc(nfiles*sizeof(int));
    resolution_locations = (int*)malloc(nfiles*sizeof(int));
    fader0_locations = (int*)malloc(nfiles*sizeof(int));
    fader1_locations = (int*)malloc(nfiles*sizeof(int));
    fader2_locations = (int*)malloc(nfiles*sizeof(int));
    fader3_locations = (int*)malloc(nfiles*sizeof(int));
    fader4_locations = (int*)malloc(nfiles*sizeof(int));
    fader5_locations = (int*)malloc(nfiles*sizeof(int));
    fader6_locations = (int*)malloc(nfiles*sizeof(int));
    fader7_locations = (int*)malloc(nfiles*sizeof(int));
    fader8_locations = (int*)malloc(nfiles*sizeof(int));

    for(int i=0; i<nfiles; ++i)
    {
        printf("Loading Shader %d\n", i);
        
        FILE *f = fopen(filenames[i], "rt");
        if(f == 0)printf("Failed to open file: %s\n", filenames[i]);
        fseek(f, 0, SEEK_END);
        int filesize = ftell(f);
        fseek(f, 0, SEEK_SET);
        char *source = (char*)malloc(filesize+2);
        fread(source, 1, filesize, f);
        fclose(f);
        
        handles[i] = glCreateShader(GL_FRAGMENT_SHADER);
        programs[i] = glCreateProgram();
        glShaderSource(handles[i], 1, (GLchar **)&source, &filesize);
        glCompileShader(handles[i]);
        debug(handles[i]);
        glAttachShader(programs[i], handles[i]);
        glLinkProgram(programs[i]);
        debugp(programs[i]);
        
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
    }
    
    return 0;
}
