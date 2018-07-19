/*
	Copyright (c) July 2018 Leandro Silva <@gmail.com>

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in
	all copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
	THE SOFTWARE.

	Description: This is a source for OpenGL Tracer v1 using count and indeces from
	glDrawElements() to locate any 3D object by color it red, great for your game projects.

	Compile: Was used on MVS C++ 6.0 since don't need asm code. To use the 2 Method you
	may need a diff compiler like Borland Builder C++ or Dev-C++ with asm fuction support.
	In some compilers may need to add opengl32 library into your project.

	Usage: Use an .EXE injector to aply this .DLL into your aplication or game, 
	then use keys as your needs.

	Tested: Using...
	Windows XP SP2, OpenGL 2.1.8918; Ok
	Windows 10, OpenGL 4.1; Ok

	Tags: OpenGL, Tracer, Intercept, Hook, Chams.
	
 
   Please be kind and support my work for further release, any donation is apreciated
   at https://www.paypal.me/LeandroAdonis/5 for 5€ or 5$ amount. Any amount you wish, 
   I will be very gratefull and real happy to see my work worth something to you. Thank you.
*/

/*
		Key : Function
	Numpad 0: Activate eye throw behind the walls;
	Numpad 1: Activate filtering objects for count;
	Numpad 2: Incrise count filtering;
	Numpad 3: Decrise count filtering;
	Numpad 4: Activate filtering objects for indices;
	Numpad 5: Incrise indices filtering;
	Numpad 6: Decrise indices filtering;
	Numpad 7: Activate/Desactivate coloring red Object Models;
	Numpad 8: Log into a file "log.txt" menu vars;
*/

#pragma comment(lib,"OpenGL32.lib")
#pragma comment(lib,"GLu32.lib")

#include <windows.h>
#include <string>
#include <tlhelp32.h>
#include <iostream>
#include <fstream>
#include <gl\gl.h>
#include <gl\glu.h>

#define RED 1
#define GREEN 2
#define BLUE 3
#define YELLOW 4
#define WHITE 5
#define BLACK 6

typedef void ( APIENTRY *glBegin_t )( GLenum );
typedef void ( APIENTRY *glEnd_t )( void );
typedef void ( APIENTRY *glClear_t )( GLbitfield );
typedef void ( APIENTRY *glVertex3fv_t )( const GLfloat *v );
typedef void ( APIENTRY *glVertex3f_t )( GLfloat x,  GLfloat y,  GLfloat z );
typedef void ( APIENTRY *glVertexPointer_t)(GLint size, GLenum type, GLsizei stride, const GLvoid* pointer);
typedef void ( APIENTRY *glDrawElements_t )( GLenum mode, GLsizei count, GLenum type,const GLvoid *indices);
typedef void ( APIENTRY *glEnable_t )( GLenum );

glBegin_t pglBegin = NULL;
glEnd_t pglEnd = NULL;
glClear_t pglClear = NULL;
glVertex3fv_t pglVertex3fv = NULL;
glVertex3f_t pglVertex3f = NULL;
glVertexPointer_t pglVertexPointer = NULL;
glDrawElements_t pglDrawElements = NULL;
glEnable_t pglEnable = NULL;

bool wallhack = false;
bool count_status = false;
bool indices_status = false;
bool color_it = false;

int cnt = 0;
char txt_cnt[24] = "count: off";
int ind = 0;
char txt_ind[24] = "indices: off";
char txt_clr[24] = "colored: no";

int arr_list[128];

char* pcCurTex = new char[40];
DWORD dwEbxReg = 0;

using namespace std;

//maybe usefull later versions
void draw_box(GLfloat x, GLfloat y, GLfloat z, GLfloat width, GLfloat height) 
{ 
	glBegin(GL_QUADS); 
	glVertex3f(x,y,z); 
	glVertex3f(x+width,y,z); 
	glVertex3f(x+width,y+height,z); 
	glVertex3f(x,y+height,z); 
	glEnd(); 
}


void draw_text(char *str, float x, float y, int color)
{
	HDC hdc = wglGetCurrentDC();
	SelectObject (hdc, GetStockObject (SYSTEM_FONT)); 
	wglUseFontBitmaps(hdc, 0, 255, 3000);

	glRasterPos2f(x, y);
	switch(color) {
	case RED:
		glColor3f(0xff,0x00,0x00);
		break;
	case GREEN:
		glColor3f(0x00,0xff,0x00);
		break;
	case BLUE:
		glColor3f(0x00,0x00,0xff);
		break;
	case YELLOW:
		glColor3f(0xff,0xff,0x00);
		break;
	case WHITE:
		glColor3f(0xff,0xff,0xff);
		break;
	case BLACK:
		glColor3f(0x00,0x00,0x00);
		break;
	};

	glPushAttrib(GL_LIST_BIT);
	glListBase(3000);
	glCallLists (strlen(str), GL_UNSIGNED_BYTE, str);
	glPopAttrib();
	glEnd();
}

void write_file()
{
	char out_textfile[64];
	ofstream outfile;
	outfile.open ("log.txt", fstream::out | fstream::app);
	sprintf(out_textfile,"glDrawElements() GLsizei count: %d, GLvoid indices: 0x%hhx\n", cnt, arr_list[ind]);
	outfile << out_textfile;
	outfile.close();
  
}

void clear_addresslist() {
	int i;
	for (i=0; i < 128; i++) {
		arr_list[i]=0;
	}
}

void APIENTRY Hooked_glBegin( GLenum mode )
{

	if (GetAsyncKeyState(VK_NUMPAD0)&1) wallhack = !wallhack;

	if (wallhack) {
		if(mode == GL_TRIANGLES || mode == GL_TRIANGLE_STRIP || mode == GL_TRIANGLE_FAN)
			glDepthRange(1, 0.5);
		else
			glDepthRange(0.5, 0);
	} else {
		glDepthRange(0, 1);
	}


	(*pglBegin)(mode);
}

void APIENTRY Hooked_glEnd( void )
{
	(*pglEnd)();
}

void APIENTRY Hooked_glVertex3fv( GLfloat *v )
{	
	(*pglVertex3fv)( v );
}


void APIENTRY Hooked_glVertex3f ( GLfloat x,  GLfloat y,  GLfloat z )
{
	(*pglVertex3f)( x, y, z );
}

void APIENTRY Hooked_glClear( GLbitfield mask )
{
	(*pglClear)( mask );
}

void APIENTRY Hooked_glVertexPointer(GLint size, GLenum type, GLsizei stride, const GLvoid* pointer)
{
	/*
	// Method 2: only for register, function may not be compatible for some IDE C++ compilers.
	asm("movl %%ebx, %0;" : "=r"(dwEbxReg));

	//if not work alternative asm in below:
	//asm("movl %%esi, %0;" : "=r"(dwEbxReg));
	*/

	(*pglVertexPointer)( size, type, stride, pointer );
}

void APIENTRY Hooked_glDrawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid *indices)
{

	// Method 1: for count and indices
	if(count_status == true) {
		if(count == cnt) {
			if(mode == GL_TRIANGLES) {
				//get all indices available for this count
				if(indices_status == true) {
				    int i;
					//128 is arr_list size
					for (i=0; i < 128; i++) {
						//if not add to the list
						if (arr_list[i] == 0) {
							arr_list[i] =(int)indices;
							break;
						}
						//check if already exist
						if (arr_list[i] == (int)indices)
							break;
					}
					//filter if indices is activated
					if((int)indices == arr_list[ind]) {
						if(color_it) { 
							glDisable(GL_TEXTURE_2D);
							glDisableClientState(GL_COLOR_ARRAY);
							glEnable(GL_COLOR_MATERIAL);
						}

						glDepthFunc(GL_LEQUAL);
						if(color_it) glColor3f(0xff,0x00,0x00);
						pglDrawElements(mode, count, type, indices);

						glDepthFunc(GL_GREATER);
						if(color_it) glColor3f(0xff,0x00,0x00);
						pglDrawElements(mode, count, type, indices);
						
						glDepthFunc(GL_LEQUAL);
					}

				} else {
					//filter only for count values
					if(color_it) {
						glDisable(GL_TEXTURE_2D);
						glDisableClientState(GL_COLOR_ARRAY);
						glEnable(GL_COLOR_MATERIAL);
					}

					glDepthFunc(GL_LEQUAL);
					if(color_it) glColor3f(0xff,0x00,0x00);
					pglDrawElements(mode, count, type, indices);

					glDepthFunc(GL_GREATER);
					if(color_it) glColor3f(0xff,0x00,0x00);
					pglDrawElements(mode, count, type, indices);
					
					glDepthFunc(GL_LEQUAL);
				}

				//printf("%d\n",indices);

			}
		}

	}

	/* 
	// Method 2: for register
	if ((char*)dwEbxReg != NULL) {
		memset(pcCurTex, 0, 40);
		memcpy(pcCurTex, (char*)dwEbxReg, 39);
		pcCurTex[39] = '\0';

		if (strstr(pcCurTex, "yourstring") != NULL) {
				// TODO: shadding hook here
		}
	}
	*/


	(*pglDrawElements)(mode, count, type, indices);
}


void APIENTRY Hooked_glEnable (GLenum cap)
{

	// show indices log value in screen
	
	if(!indices_status) draw_text(txt_ind, 5, 30, WHITE);
	if(indices_status) draw_text(txt_ind, 5, 30, WHITE);

	// show count log value in screen 
	
	if(!count_status) draw_text(txt_cnt, 5, 15, WHITE);
	if(count_status) draw_text(txt_cnt, 5, 15, WHITE);

	// show if objects is getting colored or not
	
	if(!color_it) draw_text(txt_clr, 5, 45, WHITE);
	if(color_it) draw_text(txt_clr, 5, 45, WHITE);
	

	if ((GetAsyncKeyState(VK_NUMPAD1)&1)) {
			count_status = !count_status;
			if(count_status==false)
				sprintf(txt_cnt, "count: off", cnt);
			else
				sprintf(txt_cnt, "count: %d", cnt);
			
			if(!count_status)
				draw_text(txt_cnt, 5, 15, WHITE);
			else
				draw_text(txt_cnt, 5, 15, WHITE);
	}
	// incrise count value
	if ((GetAsyncKeyState(VK_NUMPAD2)&1)) {
		    //clear indexes for diff counts
			if(indices_status==true) clear_addresslist();
			if(cnt<65535)	
				cnt++;
			else
				cnt=65535;

			count_status = true;
			sprintf(txt_cnt, "count: %d", cnt);
			draw_text(txt_cnt, 5, 15, WHITE);

	}
	// decrise count value
	if ((GetAsyncKeyState(VK_NUMPAD3)&1)) {
			//clear indexes for diff counts
			if(indices_status==true) clear_addresslist();
			if(cnt>0)	
				cnt--;
			else
				cnt=0;

			count_status = true;
			sprintf(txt_cnt, "count: %d", cnt);
			draw_text(txt_cnt, 5, 15, WHITE);
	}

	// #####

	if ((GetAsyncKeyState(VK_NUMPAD4)&1)) {
			//ativate indices
			indices_status = !indices_status;

			//count as to be activated to be possible get indices
			if (indices_status) count_status = true;
			sprintf(txt_cnt, "count: %d", cnt);

			draw_text(txt_cnt, 5, 15, RED);

			//clear indices for diff counts
			clear_addresslist();
			if(indices_status==false)
				sprintf(txt_ind, "indices: off", ind);
			else
				sprintf(txt_ind, "indices: %d, 0x%hhx", ind, arr_list[ind]);

			if(!indices_status)
				draw_text(txt_ind, 5, 30, WHITE);
			else
				draw_text(txt_ind, 5, 30, WHITE);
	}
	// incrise indices value
	if ((GetAsyncKeyState(VK_NUMPAD5)&1)) {
			if(ind<65535)	
				ind++;
			else
				ind=65535;

			indices_status = true;
			sprintf(txt_ind, "indices: %d, 0x%hhx", ind, arr_list[ind]);
			draw_text(txt_ind, 5, 30, WHITE);

	}
	// decrise indices value
	if ((GetAsyncKeyState(VK_NUMPAD6)&1)) {
			if(ind>0)	
				ind--;
			else
				ind=0;

			indices_status = true;
			sprintf(txt_ind, "indices: %d, 0x%hhx", ind, arr_list[ind]);
			draw_text(txt_ind, 5, 30, WHITE);

	}

	// #####

	// use color or just zorder out
	if ((GetAsyncKeyState(VK_NUMPAD7)&1)) {
			color_it = !color_it;
			if(color_it) {
				sprintf(txt_clr, "colored: yes");
				draw_text(txt_clr, 5, 45, WHITE);
			}
			if(!color_it) {
				sprintf(txt_clr, "colored: no");
				draw_text(txt_clr, 5, 45, WHITE);
			}
	}

	// #####

	// save to a file
	if ((GetAsyncKeyState(VK_NUMPAD8)&1)) {
		write_file();
	}
	

	(*pglEnable)(cap);
}

void *DetourFunc( BYTE *src, const BYTE *dst, const int len )
{
	BYTE *jmp = (BYTE*)malloc( len + 5 );
	DWORD dwback;
	VirtualProtect( src, len, PAGE_READWRITE, &dwback );
	memcpy( jmp, src, len );
	jmp += len;
	jmp[0] = 0xE9;
	*(DWORD*)( jmp + 1 ) = (DWORD)( src + len - jmp ) - 5;
	src[0] = 0xE9;
	*(DWORD*)( src + 1 ) = (DWORD)( dst - src ) - 5;
	VirtualProtect( src, len, dwback, &dwback );
	return ( jmp - len );
}

void HookGL()
{
	HMODULE hOpenGL = GetModuleHandle("opengl32.dll");
	pglVertex3fv =   (glVertex3fv_t) DetourFunc((LPBYTE)GetProcAddress(hOpenGL, "glVertex3fv" ), (LPBYTE) &Hooked_glVertex3fv, 6);
	pglVertex3f  =   (glVertex3f_t) DetourFunc((LPBYTE)GetProcAddress(hOpenGL, "glVertex3f" ), (LPBYTE) &Hooked_glVertex3f, 6);
	pglBegin     =   (glBegin_t) DetourFunc((LPBYTE)GetProcAddress(hOpenGL, "glBegin"),(LPBYTE) &Hooked_glBegin,6);
	pglEnd       =   (glEnd_t) DetourFunc((LPBYTE)GetProcAddress(hOpenGL, "glEnd" ), (LPBYTE) &Hooked_glEnd, 6);
	pglClear     =   (glClear_t) DetourFunc((LPBYTE)GetProcAddress(hOpenGL, "glClear" ), (LPBYTE) &Hooked_glClear, 7);
	pglEnable    =   (glEnable_t) DetourFunc((LPBYTE)GetProcAddress(hOpenGL, "glEnable"), (LPBYTE) &Hooked_glEnable, 6);
	pglVertexPointer = (glVertexPointer_t) DetourFunc((LPBYTE)GetProcAddress(hOpenGL, "glVertexPointer"), (LPBYTE) &Hooked_glVertexPointer, 6);
	pglDrawElements = (glDrawElements_t) DetourFunc((LPBYTE)GetProcAddress(hOpenGL, "glDrawElements"), (LPBYTE) &Hooked_glDrawElements, 6);

}	

DWORD WINAPI dwMainThread(LPVOID)
{	
	HookGL();

	return TRUE;
}

BOOL WINAPI DllMain(HINSTANCE hInstDLL,DWORD dwReason,LPVOID lpReserved)
{
    switch (dwReason)
    {
        case DLL_PROCESS_ATTACH:
            CreateThread(0, 0, dwMainThread, 0, 0, 0);
            MessageBoxA(0, "Sucessfully injected!", "OpenGL", MB_OK | MB_ICONINFORMATION);
            break;

        case DLL_PROCESS_DETACH:
            break;
    }
	return TRUE;
}
