#include <cstdlib>
#include <ctime>
#include "Render.h"
#include <sstream>
#include <iostream>
#include <windows.h>
#include <GL\GL.h>
#include <GL\GLU.h>
#include "MyOGL.h"
#include "Camera.h"
#include "Light.h"
#include "Primitives.h"
#include "GUItextRectangle.h"
#include <cmath>

#define PI 3.14159265

bool textureMode = true;
bool lightMode = true;
bool textureReplace = true;
bool orbit = true;
bool stop = false;



//класс для настройки камеры
class CustomCamera : public Camera
{
public:
	//дистанция камеры
	double camDist;
	//углы поворота камеры
	double fi1, fi2;


	//значния масеры по умолчанию
	CustomCamera()
	{
		camDist = 15;
		fi1 = 1;
		fi2 = 1;
	}


	//считает позицию камеры, исходя из углов поворота, вызывается движком
	void SetUpCamera()
	{
		//отвечает за поворот камеры мышкой
		lookPoint.setCoords(0, 0, 0);

		pos.setCoords(camDist * cos(fi2) * cos(fi1),
			camDist * cos(fi2) * sin(fi1),
			camDist * sin(fi2));

		if (cos(fi2) <= 0)
			normal.setCoords(0, 0, -1);
		else
			normal.setCoords(0, 0, 1);

		LookAt();
	}

	void CustomCamera::LookAt()
	{
		//функция настройки камеры
		gluLookAt(pos.X(), pos.Y(), pos.Z(), lookPoint.X(), lookPoint.Y(), lookPoint.Z(), normal.X(), normal.Y(), normal.Z());
	}



}  camera;   //создаем объект камеры


//Класс для настройки света
class CustomLight : public Light
{
public:
	CustomLight()
	{
		//начальная позиция света
		pos = Vector3(0, 0, 0);
	}


	//рисует сферу и линии под источником света, вызывается движком
	void  DrawLightGhismo()
	{
		glDisable(GL_LIGHTING);


		glColor3d(0.9, 0.8, 0);
		/*Sphere s;
		s.pos = pos;
		s.scale = s.scale*0.08;
		s.Show();*/

		//if (OpenGL::isKeyPressed('G'))
		//{
		//	glColor3d(0, 0, 0);
		//	//линия от источника света до окружности
		//	glBegin(GL_LINES);
		//	glVertex3d(pos.X(), pos.Y(), pos.Z());
		//	glVertex3d(pos.X(), pos.Y(), 0);
		//	glEnd();

		//	//рисуем окруность
		//	Circle c;
		//	c.pos.setCoords(pos.X(), pos.Y(), 0);
		//	c.scale = c.scale*1.5;
		//	c.Show();
		//}

	}

	void SetUpLight()
	{
		GLfloat amb[] = { 0.2, 0.2, 0.2, 0 };
		GLfloat dif[] = { 1.0, 1.0, 1.0, 1.0 };
		GLfloat spec[] = { 1.0, 1.0, 1.0, 1.0 };
		GLfloat position[] = { pos.X(), pos.Y(), pos.Z(), 1. };

		// параметры источника света
		glLightfv(GL_LIGHT0, GL_POSITION, position);
		// характеристики излучаемого света
		// фоновое освещение (рассеянный свет)
		glLightfv(GL_LIGHT0, GL_AMBIENT, amb);
		// диффузная составляющая света
		glLightfv(GL_LIGHT0, GL_DIFFUSE, dif);
		// зеркально отражаемая составляющая света
		glLightfv(GL_LIGHT0, GL_SPECULAR, spec);

		glEnable(GL_LIGHT0);
	}


} light;  //создаем источник света



//старые координаты мыши
int mouseX = 0, mouseY = 0;

void mouseEvent(OpenGL* ogl, int mX, int mY)
{
	int dx = mouseX - mX;
	int dy = mouseY - mY;
	mouseX = mX;
	mouseY = mY;

	//меняем углы камеры при нажатой левой кнопке мыши
	if (OpenGL::isKeyPressed(VK_RBUTTON))
	{
		camera.fi1 += 0.01 * dx;
		camera.fi2 += -0.01 * dy;
	}

	


	//двигаем свет по плоскости, в точку где мышь
	/*if (OpenGL::isKeyPressed('G') && !OpenGL::isKeyPressed(VK_LBUTTON))
	{
		LPPOINT POINT = new tagPOINT();
		GetCursorPos(POINT);
		ScreenToClient(ogl->getHwnd(), POINT);
		POINT->y = ogl->getHeight() - POINT->y;

		Ray r = camera.getLookRay(POINT->x, POINT->y);

		double z = light.pos.Z();

		double k = 0, x = 0, y = 0;
		if (r.direction.Z() == 0)
			k = 0;
		else
			k = (z - r.origin.Z()) / r.direction.Z();

		x = k*r.direction.X() + r.origin.X();
		y = k*r.direction.Y() + r.origin.Y();

		light.pos = Vector3(x, y, z);
	}*/

	/*if (OpenGL::isKeyPressed('G') && OpenGL::isKeyPressed(VK_LBUTTON))
	{
		light.pos = light.pos + Vector3(0, 0, 0.02*dy);
	}*/


}

void mouseWheelEvent(OpenGL* ogl, int delta)
{

	if (delta < 0 && camera.camDist <= 1)
		return;
	if (delta > 0 && camera.camDist >= 100)
		return;

	camera.camDist += 0.01 * delta;

}

void keyDownEvent(OpenGL* ogl, int key)
{
	if (key == 'T')
		orbit = !orbit;
	if (key == 'S') 
		stop = !stop;
}

void keyUpEvent(OpenGL* ogl, int key)
{

}


GLuint texId_Sun;
GLuint texId_Mercury;
GLuint texId_Venus;
GLuint texId_Earth;
GLuint texId_Mars;
GLuint texId_Jupiter;
GLuint texId_Saturn;
GLuint texId_Uranus;
GLuint texId_Neptune;
GLuint texId_Moon;
GLuint texId_Asteroid;



//выполняется перед первым рендером
void initRender(OpenGL* ogl)
{
	//настройка текстур

	//4 байта на хранение пикселя
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

	//настройка режима наложения текстур
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	//включаем текстуры
	glEnable(GL_TEXTURE_2D);


	//массив трехбайтных элементов  (R G B)
	{RGBTRIPLE* texarray;

	//массив символов, (высота*ширина*4      4, потомучто   выше, мы указали использовать по 4 байта на пиксель текстуры - R G B A)
	char* texCharArray;
	int texW, texH;
	OpenGL::LoadBMP("Texture_Earth.bmp", &texW, &texH, &texarray);
	OpenGL::RGBtoChar(texarray, texW, texH, &texCharArray);



	//генерируем ИД для текстуры
	glGenTextures(1, &texId_Earth);
	//биндим айдишник, все что будет происходить с текстурой, будте происходить по этому ИД
	glBindTexture(GL_TEXTURE_2D, texId_Earth);

	//загружаем текстуру в видеопямять, в оперативке нам больше  она не нужна
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW, texH, 0, GL_RGBA, GL_UNSIGNED_BYTE, texCharArray);

	//отчистка памяти
	free(texCharArray);
	free(texarray);

	//наводим шмон
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	}
	//массив трехбайтных элементов  (R G B)
	{RGBTRIPLE* texarray2;

	//массив символов, (высота*ширина*4      4, потомучто   выше, мы указали использовать по 4 байта на пиксель текстуры - R G B A)
	char* texCharArray2;
	int texW2, texH2;
	OpenGL::LoadBMP("Texture_Mercury.bmp", &texW2, &texH2, &texarray2);
	OpenGL::RGBtoChar(texarray2, texW2, texH2, &texCharArray2);



	//генерируем ИД для текстуры
	glGenTextures(1, &texId_Mercury);
	//биндим айдишник, все что будет происходить с текстурой, будте происходить по этому ИД
	glBindTexture(GL_TEXTURE_2D, texId_Mercury);

	//загружаем текстуру в видеопямять, в оперативке нам больше  она не нужна
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW2, texH2, 0, GL_RGBA, GL_UNSIGNED_BYTE, texCharArray2);

	//отчистка памяти
	free(texCharArray2);
	free(texarray2);

	//наводим шмон
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	}

	//массив трехбайтных элементов  (R G B)
	{RGBTRIPLE* texarray3;

	//массив символов, (высота*ширина*4      4, потомучто   выше, мы указали использовать по 4 байта на пиксель текстуры - R G B A)
	char* texCharArray3;
	int texW3, texH3;
	OpenGL::LoadBMP("Texture_Venus.bmp", &texW3, &texH3, &texarray3);
	OpenGL::RGBtoChar(texarray3, texW3, texH3, &texCharArray3);



	//генерируем ИД для текстуры
	glGenTextures(1, &texId_Venus);
	//биндим айдишник, все что будет происходить с текстурой, будте происходить по этому ИД
	glBindTexture(GL_TEXTURE_2D, texId_Venus);

	//загружаем текстуру в видеопямять, в оперативке нам больше  она не нужна
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW3, texH3, 0, GL_RGBA, GL_UNSIGNED_BYTE, texCharArray3);

	//отчистка памяти
	free(texCharArray3);
	free(texarray3);

	//наводим шмон
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	}

	{RGBTRIPLE* texarray4;

	//массив символов, (высота*ширина*4      4, потомучто   выше, мы указали использовать по 4 байта на пиксель текстуры - R G B A)
	char* texCharArray4;
	int texW4, texH4;
	OpenGL::LoadBMP("Texture_Sun.bmp", &texW4, &texH4, &texarray4);
	OpenGL::RGBtoChar(texarray4, texW4, texH4, &texCharArray4);



	//генерируем ИД для текстуры
	glGenTextures(1, &texId_Sun);
	//биндим айдишник, все что будет происходить с текстурой, будте происходить по этому ИД
	glBindTexture(GL_TEXTURE_2D, texId_Sun);

	//загружаем текстуру в видеопямять, в оперативке нам больше  она не нужна
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW4, texH4, 0, GL_RGBA, GL_UNSIGNED_BYTE, texCharArray4);

	//отчистка памяти
	free(texCharArray4);
	free(texarray4);

	//наводим шмон
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	}

	{RGBTRIPLE* texarray5;

	//массив символов, (высота*ширина*4      4, потомучто   выше, мы указали использовать по 4 байта на пиксель текстуры - R G B A)
	char* texCharArray5;
	int texW5, texH5;
	OpenGL::LoadBMP("Texture_Mars.bmp", &texW5, &texH5, &texarray5);
	OpenGL::RGBtoChar(texarray5, texW5, texH5, &texCharArray5);



	//генерируем ИД для текстуры
	glGenTextures(1, &texId_Mars);
	//биндим айдишник, все что будет происходить с текстурой, будте происходить по этому ИД
	glBindTexture(GL_TEXTURE_2D, texId_Mars);

	//загружаем текстуру в видеопямять, в оперативке нам больше  она не нужна
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW5, texH5, 0, GL_RGBA, GL_UNSIGNED_BYTE, texCharArray5);

	//отчистка памяти
	free(texCharArray5);
	free(texarray5);

	//наводим шмон
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	}

	{RGBTRIPLE* texarray6;

	//массив символов, (высота*ширина*4      4, потомучто   выше, мы указали использовать по 4 байта на пиксель текстуры - R G B A)
	char* texCharArray6;
	int texW6, texH6;
	OpenGL::LoadBMP("Texture_Jupiter.bmp", &texW6, &texH6, &texarray6);
	OpenGL::RGBtoChar(texarray6, texW6, texH6, &texCharArray6);



	//генерируем ИД для текстуры
	glGenTextures(1, &texId_Jupiter);
	//биндим айдишник, все что будет происходить с текстурой, будте происходить по этому ИД
	glBindTexture(GL_TEXTURE_2D, texId_Jupiter);

	//загружаем текстуру в видеопямять, в оперативке нам больше  она не нужна
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW6, texH6, 0, GL_RGBA, GL_UNSIGNED_BYTE, texCharArray6);

	//отчистка памяти
	free(texCharArray6);
	free(texarray6);

	//наводим шмон
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	}

	{RGBTRIPLE* texarray7;

	//массив символов, (высота*ширина*4      4, потомучто   выше, мы указали использовать по 4 байта на пиксель текстуры - R G B A)
	char* texCharArray7;
	int texW7, texH7;
	OpenGL::LoadBMP("Texture_Saturn.bmp", &texW7, &texH7, &texarray7);
	OpenGL::RGBtoChar(texarray7, texW7, texH7, &texCharArray7);



	//генерируем ИД для текстуры
	glGenTextures(1, &texId_Saturn);
	//биндим айдишник, все что будет происходить с текстурой, будте происходить по этому ИД
	glBindTexture(GL_TEXTURE_2D, texId_Saturn);

	//загружаем текстуру в видеопямять, в оперативке нам больше  она не нужна
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW7, texH7, 0, GL_RGBA, GL_UNSIGNED_BYTE, texCharArray7);

	//отчистка памяти
	free(texCharArray7);
	free(texarray7);

	//наводим шмон
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	}

	{RGBTRIPLE* texarray8;

	//массив символов, (высота*ширина*4      4, потомучто   выше, мы указали использовать по 4 байта на пиксель текстуры - R G B A)
	char* texCharArray8;
	int texW8, texH8;
	OpenGL::LoadBMP("Texture_Uranus.bmp", &texW8, &texH8, &texarray8);
	OpenGL::RGBtoChar(texarray8, texW8, texH8, &texCharArray8);



	//генерируем ИД для текстуры
	glGenTextures(1, &texId_Uranus);
	//биндим айдишник, все что будет происходить с текстурой, будте происходить по этому ИД
	glBindTexture(GL_TEXTURE_2D, texId_Uranus);

	//загружаем текстуру в видеопямять, в оперативке нам больше  она не нужна
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW8, texH8, 0, GL_RGBA, GL_UNSIGNED_BYTE, texCharArray8);

	//отчистка памяти
	free(texCharArray8);
	free(texarray8);

	//наводим шмон
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	}

	{RGBTRIPLE* texarray9;

	//массив символов, (высота*ширина*4      4, потомучто   выше, мы указали использовать по 4 байта на пиксель текстуры - R G B A)
	char* texCharArray9;
	int texW9, texH9;
	OpenGL::LoadBMP("Texture_Neptune.bmp", &texW9, &texH9, &texarray9);
	OpenGL::RGBtoChar(texarray9, texW9, texH9, &texCharArray9);



	//генерируем ИД для текстуры
	glGenTextures(1, &texId_Neptune);
	//биндим айдишник, все что будет происходить с текстурой, будте происходить по этому ИД
	glBindTexture(GL_TEXTURE_2D, texId_Neptune);

	//загружаем текстуру в видеопямять, в оперативке нам больше  она не нужна
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW9, texH9, 0, GL_RGBA, GL_UNSIGNED_BYTE, texCharArray9);

	//отчистка памяти
	free(texCharArray9);
	free(texarray9);

	//наводим шмон
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	}

	{RGBTRIPLE* texarray10;

	//массив символов, (высота*ширина*4      4, потомучто   выше, мы указали использовать по 4 байта на пиксель текстуры - R G B A)
	char* texCharArray10;
	int texW10, texH10;
	OpenGL::LoadBMP("Texture_Moon.bmp", &texW10, &texH10, &texarray10);
	OpenGL::RGBtoChar(texarray10, texW10, texH10, &texCharArray10);



	//генерируем ИД для текстуры
	glGenTextures(1, &texId_Moon);
	//биндим айдишник, все что будет происходить с текстурой, будте происходить по этому ИД
	glBindTexture(GL_TEXTURE_2D, texId_Moon);

	//загружаем текстуру в видеопямять, в оперативке нам больше  она не нужна
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW10, texH10, 0, GL_RGBA, GL_UNSIGNED_BYTE, texCharArray10);

	//отчистка памяти
	free(texCharArray10);
	free(texarray10);

	//наводим шмон
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	}

	{RGBTRIPLE* texarray11;

	//массив символов, (высота*ширина*4      4, потомучто   выше, мы указали использовать по 4 байта на пиксель текстуры - R G B A)
	char* texCharArray11;
	int texW11, texH11;
	OpenGL::LoadBMP("Texture_Asteroid.bmp", &texW11, &texH11, &texarray11);
	OpenGL::RGBtoChar(texarray11, texW11, texH11, &texCharArray11);



	//генерируем ИД для текстуры
	glGenTextures(1, &texId_Asteroid);
	//биндим айдишник, все что будет происходить с текстурой, будте происходить по этому ИД
	glBindTexture(GL_TEXTURE_2D, texId_Asteroid);

	//загружаем текстуру в видеопямять, в оперативке нам больше  она не нужна
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texW11, texH11, 0, GL_RGBA, GL_UNSIGNED_BYTE, texCharArray11);

	//отчистка памяти
	free(texCharArray11);
	free(texarray11);

	//наводим шмон
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	}

	//камеру и свет привязываем к "движку"
	ogl->mainCamera = &camera;
	ogl->mainLight = &light;

	// нормализация нормалей : их длины будет равна 1
	glEnable(GL_NORMALIZE);

	// устранение ступенчатости для линий
	glEnable(GL_LINE_SMOOTH);


	//   задать параметры освещения
	//  параметр GL_LIGHT_MODEL_TWO_SIDE - 
	//                0 -  лицевые и изнаночные рисуются одинаково(по умолчанию), 
	//                1 - лицевые и изнаночные обрабатываются разными режимами       
	//                соответственно лицевым и изнаночным свойствам материалов.    
	//  параметр GL_LIGHT_MODEL_AMBIENT - задать фоновое освещение, 
	//                не зависящее от сточников
	// по умолчанию (0.2, 0.2, 0.2, 1.0)

	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, 0);

	camera.fi1 = -1.3;
	camera.fi2 = 0.8;
}



void drawSphere(float radius, int stacks, int slices, float startX, float startY, float startZ, bool Sun) {

	for (int i = 0; i <= stacks; ++i) {
		float theta1 = i * M_PI / stacks - M_PI / 2.0f;     // Current stack angle (shifted by -pi/2)
		float theta2 = (i + 1) * M_PI / stacks - M_PI / 2.0f; // Next stack angle (shifted by -pi/2)

		glBegin(GL_TRIANGLE_STRIP);
		for (int j = 0; j <= slices; ++j) {
			float phi = j * 2 * M_PI / slices;            // Current slice angle

			// Vertices for the current stack
			float x1 = cos(phi) * cos(theta1) * radius + startX;
			float y1 = sin(phi) * cos(theta1) * radius + startY;
			float z1 = sin(theta1) * radius + startZ;
			float u1 = (float)j / slices;                  // Texture coordinate u
			float v1 = 1.0f - (float)i / stacks;           // Texture coordinate v (flipped)
			glTexCoord2f(u1, v1);
			if (Sun)
				glNormal3f(-(x1 - startX), -(y1 - startY), -(z1 - startZ));
			else
				glNormal3f((x1 - startX), (y1 - startY), (z1 - startZ)); // Adjust normals
			glVertex3f(x1, y1, z1);

			// Vertices for the next stack
			float x2 = cos(phi) * cos(theta2) * radius + startX;
			float y2 = sin(phi) * cos(theta2) * radius + startY;
			float z2 = sin(theta2) * radius + startZ;
			float u2 = (float)j / slices;                  // Texture coordinate u
			float v2 = 1.0f - (float)(i + 1) / stacks;     // Texture coordinate v (flipped)
			glTexCoord2f(u2, v2);
			if (Sun)
				glNormal3f(-(x2 - startX), -(y2 - startY), -(z2 - startZ));
			else
				glNormal3f((x2 - startX), (y2 - startY), (z2 - startZ)); // Adjust normals
			glVertex3f(x2, y2, z2);
		}
		glEnd();
	}
}


float randomFloat() {
	return ((float)rand() / RAND_MAX) * 2 - 1;
}


void drawStarsOnSphere(float stars[][3], int numStars, float colors[][3]) {
	

	glPointSize(2.0f); // Set the size of the points

	glBegin(GL_POINTS);
	for (int i = 0; i < numStars; ++i) {
		// Get the coordinates of the star
		float x = stars[i][0];
		float y = stars[i][1];
		float z = stars[i][2];

	


		// Draw the star
		glColor3f(colors[i][0], colors[i][1], colors[i][2]);
		glVertex3f(x, y, z);
	}
	glEnd();
}



void generateStars(float stars[][3], int numStars, float sphereRadius) 
{
	for (int i = 0; i < numStars; ++i) {
		// Generate random spherical coordinates
		float theta = ((float)rand() / RAND_MAX) * 2 * 3.14159f;
		float phi = acos(1 - 2 * ((float)rand() / RAND_MAX));

		// Convert spherical coordinates to Cartesian coordinates
		stars[i][0] = sphereRadius * sin(phi) * cos(theta);
		stars[i][1] = sphereRadius * sin(phi) * sin(theta);
		stars[i][2] = sphereRadius * cos(phi);
	}
}
void generateRandomColors(float colors[][3], int numColors)
{
	for (int i = 0; i < numColors; ++i) {
		colors[i][0] = randomFloat(); // Red
		colors[i][1] = randomFloat(); // Green
		colors[i][2] = randomFloat(); // Blue
	}
}

void drawRing(GLfloat innerRadius, GLfloat outerRadius, GLint segments, GLfloat offsetX, GLfloat offsetY, GLfloat offsetZ) 
{
	
	glBegin(GL_QUAD_STRIP);
	for (int i = 0; i <= segments; ++i) {
		GLfloat theta = 2.0f * M_PI * float(i) / float(segments);
		GLfloat x = cos(theta);
		GLfloat y = sin(theta);

		glNormal3f(0.0f, 1.0f, 0.0f);
		glColor3f(1.0f, 1.0f, 1.0f);
		glVertex3f(innerRadius * x + offsetX, innerRadius * y + offsetY, offsetZ);
		glVertex3f(outerRadius * x + offsetX, outerRadius * y + offsetY, offsetZ);
	}
	glEnd();
}

void drawOrbit(float radius, int segments) {
	glColor3f(0.3f, 0.3f, 0.3f);
	glBegin(GL_LINE_LOOP);
	for (int i = 0; i < segments; ++i) {
		float theta = 2.0f * M_PI * float(i) / float(segments); // get the current angle
		float x = radius * cosf(theta); // calculate the x component
		float y = radius * sinf(theta); // calculate the y component
		glVertex2f(x, y); // output vertex
	}
	glEnd();
}

struct Asteroid {
	float x, y, z;
	float size;
};


const int numAsteroids = 250; // Number of asteroids in the belt
Asteroid asteroids[numAsteroids];
bool asteroidsGenerated = false;



void generateAsteroidBelt(float radius, float beltWidth) {
	for (int i = 0; i < numAsteroids; ++i) {
		float angle = static_cast<float>(rand()) / RAND_MAX * 2.0f * M_PI;
		float distance = radius + (static_cast<float>(rand()) / RAND_MAX * beltWidth);
		float size = 2.0f + static_cast<float>(rand()) / RAND_MAX * 2.0f; // Size between 2 and 4

		asteroids[i].x = distance * cos(angle);
		asteroids[i].y = distance * sin(angle);
		asteroids[i].z = (static_cast<float>(rand()) / RAND_MAX - 0.5f) * 2.0f * beltWidth;
		asteroids[i].size = size;

		// Debug output to verify asteroid creation
		printf("Asteroid %d: Position (%f, %f, %f), Size %f\n", i, asteroids[i].x, asteroids[i].y, asteroids[i].z, asteroids[i].size);
	}
	asteroidsGenerated = true;
}

void drawAsteroidBelt(int k) {
	if (!asteroidsGenerated) {
		std::cerr << "Error: Asteroid coordinates not generated!" << std::endl;
		return;
	}

	for (int i = 0; i < numAsteroids; ++i) {
		glPushMatrix();
		glTranslatef(asteroids[i].x/k, asteroids[i].y/k, asteroids[i].z/k);
		glColor3f(0.4f, 0.4f, 0.4f);
		glBindTexture(GL_TEXTURE_2D, texId_Asteroid);
		drawSphere(asteroids[i].size/k, 10, 10, 0, 0, 0, false); // Draw each asteroid as a sphere
		glPopMatrix();
	}
}

void initialize(float beltRadius,float beltWidth) {
	//float beltRadius = 100.0f; // Customizable radius
	//float beltWidth = 10.0f;   // Customizable belt width
	generateAsteroidBelt(beltRadius, beltWidth);
}






bool generated = false;
const int numStars = 9500;
const int numColors = 9500;
float stars[numStars][3];
float colors[numColors][3];
int k = 10; //Размерность
double h = 1; //Скорость

float rotationAngle = 0.0f;





void Render(OpenGL* ogl)
{



	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);

	glEnable(GL_DEPTH_TEST);
	if (textureMode)
		glEnable(GL_TEXTURE_2D);

	if (lightMode)
		glEnable(GL_LIGHTING);

	//альфаналожение
	//glEnable(GL_BLEND);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	//настройка материала
	GLfloat amb[] = { 0.2, 0.2, 0.2, 1. };
	GLfloat dif[] = { 1.0, 1.0, 1.0, 1. };
	GLfloat spec[] = { 1.0, 1.0, 1.0, 1. };
	GLfloat sh = 0.1f * 256;

	//фоновая
	glMaterialfv(GL_FRONT, GL_AMBIENT, amb);
	//дифузная
	glMaterialfv(GL_FRONT, GL_DIFFUSE, dif);
	//зеркальная
	glMaterialfv(GL_FRONT, GL_SPECULAR, spec); \
		//размер блика
		glMaterialf(GL_FRONT, GL_SHININESS, sh);

	//чтоб было красиво, без квадратиков (сглаживание освещения)
	glShadeModel(GL_SMOOTH);
	//===================================
	//Прогать тут 



	
	if (h < 50) 
		if (OpenGL::isKeyPressed('F')) { h = h + 1; }
	if (h > 1)
		if (OpenGL::isKeyPressed('R')) { h = h - 1; }
	if (k > 10) 
		if (OpenGL::isKeyPressed('P')) { k = k - 1; }
	if (k < 39) 
		if (OpenGL::isKeyPressed('M')) { k = k + 1; }

	float SunSize = 30.0f / k;
	float MercurySize = 9.0f / k;
	float VenusSize = 9.0f / k;
	float EarthSize = 12.0f / k;
	float MarsSize = 9.0f / k;
	float JupiterSize = 20.1f / k;
	float SaturnSize = 20.8f / k;
	float UranusSize = 20.5f / k;
	float NeptuneSize = 20.5f / k;

	float MoonSize = 6.0f / k;



	float MercuryOrbitRadius = 60.0f / k;
	float VenusOrbitRadius = 140.0f / k;
	float EarthOrbitRadius = 220.0f / k;
	float MarsOrbitRadius = 300.0f / k;
	float beltRadius = 380.0f;
	float beltWidth = 10.0f;
	float JupiterOrbitRadius = 460.0f / k;
	float SaturnOrbitRadius = 540.0f / k;
	float UranusOrbitRadius = 620.0f / k;
	float NeptuneOrbitRadius = 700.0f / k;

	
	if (orbit)
	{
		glDisable(GL_LIGHTING);
		drawOrbit(MercuryOrbitRadius, 40);
		drawOrbit(VenusOrbitRadius, 40);
		drawOrbit(EarthOrbitRadius, 40);
		drawOrbit(MarsOrbitRadius, 40);
		drawOrbit(beltRadius/k, 40);
		drawOrbit(JupiterOrbitRadius, 40);
		drawOrbit(SaturnOrbitRadius, 40);
		drawOrbit(UranusOrbitRadius, 40);
		drawOrbit(NeptuneOrbitRadius, 40);
		glEnable(GL_LIGHTING);
	}


	float MercuryOrbitSpeed = 0.04147f * h;
	float VenusOrbitSpeed = 0.01622f * h;
	float EarthOrbitSpeed = 0.01f * h;
	float MarsOrbitSpeed = 0.005f * h;
	float JupiterOrbitSpeed = 0.001f * h;
	float SaturnOrbitSpeed = 0.0009f * h;
	float UranusOrbitSpeed = 0.0007f * h;
	float NeptuneOrbitSpeed = 0.0004f * h;

	float MoonOrbitSpeed = 0.01f * h;
	if (stop) 
	{
		
		MercuryOrbitSpeed = 0.0f * h;
		VenusOrbitSpeed = 0.0f * h;
		EarthOrbitSpeed = 0.0f * h;
		MarsOrbitSpeed = 0.0f * h;
		JupiterOrbitSpeed = 0.0f * h;
		SaturnOrbitSpeed = 0.0f * h;
		UranusOrbitSpeed = 0.0f * h;
		NeptuneOrbitSpeed = 0.0f * h;
	}
	
		

	static float Mercury_angle = 0.0f;
	static float Venus_angle = 0.0f;
	static float Earth_angle = 0.0f;
	static float Mars_angle = 0.0f;
	static float Jupiter_angle = 0.0f;
	static float Saturn_angle = 0.0f;
	static float Uranus_angle = 0.0f;
	static float Neptune_angle = 0.0f;

	static float Moon_angle = 0.0f;



	float Universal_z = 0.0;

	

	float Mercury_x = MercuryOrbitRadius * cos(Mercury_angle);
	float Mercury_y = MercuryOrbitRadius * sin(Mercury_angle);

	float Venus_x = VenusOrbitRadius * cos(Venus_angle);
	float Venus_y = VenusOrbitRadius * sin(Venus_angle);

	float Earth_x = EarthOrbitRadius * cos(Earth_angle);
	float Earth_y = EarthOrbitRadius * sin(Earth_angle);

	float Mars_x = MarsOrbitRadius * cos(Mars_angle);
	float Mars_y = MarsOrbitRadius * sin(Mars_angle);

	float Jupiter_x = JupiterOrbitRadius * cos(Jupiter_angle);
	float Jupiter_y = JupiterOrbitRadius * sin(Jupiter_angle);

	float Saturn_x = SaturnOrbitRadius * cos(Saturn_angle);
	float Saturn_y = SaturnOrbitRadius * sin(Saturn_angle);

	float Uranus_x = UranusOrbitRadius * cos(Uranus_angle);
	float Uranus_y = UranusOrbitRadius * sin(Uranus_angle);

	float Neptune_x = NeptuneOrbitRadius * cos(Neptune_angle);
	float Neptune_y = NeptuneOrbitRadius * sin(Neptune_angle);

	float Saturn_Ring_Inner_Radius = 1.5 / k;
	float Saturn_Ring_Outer_Radius = 7.5 / k;

	float Uranus_Ring_Inner_Radius = 1.5 / k;
	float Uranus_Ring_Outer_Radius = 7.5 / k;

	float Saturn_Ring_X = SaturnOrbitRadius * cos(Saturn_angle);
	float Saturn_Ring_Y = SaturnOrbitRadius * sin(Saturn_angle);

	float Uranus_Ring_X = UranusOrbitRadius * cos(Uranus_angle);
	float Uranus_Ring_Y = UranusOrbitRadius * sin(Uranus_angle);

	//Own axis spin angles

	static float SunSpinAngle = 0.0f;
	static float MercurySpinAngle = 0.0f;
	static float VenusSpinAngle = 0.0f;
	static float EarthSpinAngle = 0.0f;
	static float MarsSpinAngle = 0.0f;
	static float JupiterSpinAngle = 0.0f;
	static float SaturnSpinAngle = 0.0f;
	static float UranusSpinAngle = 0.0f;
	static float NeptuneSpinAngle = 0.0f;
	static float MoonSpinAngle = 0.0f;


	float SunOrbitSpeedOwnStep = 0.3f * h;
	float MercuryOrbitSpeedOwnStep = 3.0f * h;
	float VenusOrbitSpeedOwnStep = 3.0f * h;
	float EarthOrbitSpeedOwnStep = 3.0f * h;
	float MarsOrbitSpeedOwnStep = 3.0f * h;
	float JupiterOrbitSpeedOwnStep = 2.0f * h;
	float SaturnOrbitSpeedOwnStep = 2.0f * h;
	float UranusOrbitSpeedOwnStep = 2.0f * h;
	float NeptuneOrbitSpeedOwnStep = 2.0f * h;

	float MoonOrbitSpeedOwnStep = 6.0f * h;


	
	static float beltSpinAngle = 0.0f;
	float beltSpinAngleStep = 0.3f*h;

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glDisable(GL_LIGHTING);


	if (generated == false) 
	{
		generateStars(stars, numStars, 500.0f);
		generateRandomColors(colors, numColors);
	}
	drawStarsOnSphere(stars, numStars, colors);

	generated = true;

	glEnable(GL_LIGHTING);

	//Астеройдный пояс между Марсом и Юпитером

	if (asteroidsGenerated == false)
	{
		initialize(beltRadius, beltWidth);
	}
	asteroidsGenerated = true;

	glPushMatrix();
	glRotatef(beltSpinAngle, 0.0f, 0.0f, 1.0f);
	drawAsteroidBelt(k);
	glPopMatrix();

	//Солнце
	glBindTexture(GL_TEXTURE_2D, texId_Sun);
	glPushMatrix();
	glTranslatef(0.0f, 0.0f, 0.0f);
	glRotatef(SunSpinAngle, 0.0f, 0.0f, 1.0f);
	drawSphere(SunSize,20, 20, 0, 0, 0, true);
	glPopMatrix();

	//Меркурий
	glBindTexture(GL_TEXTURE_2D, texId_Mercury);
	glPushMatrix();
	glTranslatef(Mercury_x, Mercury_y, Universal_z);
	glRotatef(MercurySpinAngle, 0.0f, 0.0f, 1.0f);
	drawSphere(MercurySize, 20, 20, 0, 0, 0, false);
	glPopMatrix();	
	Mercury_angle += MercuryOrbitSpeed;

	//Венера
	glBindTexture(GL_TEXTURE_2D, texId_Venus);
	glPushMatrix();
	glTranslatef(Venus_x, Venus_y, Universal_z);
	glRotatef(VenusSpinAngle, 0.0f, 0.0f, 1.0f);
	drawSphere(VenusSize, 20, 20, 0, 0, 0, false);
	glPopMatrix();
	Venus_angle += VenusOrbitSpeed;

	//Земля
	glBindTexture(GL_TEXTURE_2D, texId_Earth);
	glPushMatrix();
	glTranslatef(Earth_x, Earth_y, Universal_z);
	glRotatef(EarthSpinAngle, 0.0f, 0.0f, 1.0f);
	drawSphere(EarthSize, 20, 20, 0, 0, 0, false);
	glPopMatrix();
	Earth_angle += EarthOrbitSpeed;

	//Луна
	float MoonDistanceCoefficient = 1.15f;
	glBindTexture(GL_TEXTURE_2D, texId_Moon);
	glPushMatrix();
	glTranslatef(Earth_x, Earth_y, Universal_z);
	glRotatef(MoonSpinAngle, 0.0f, 0.0f, 1.0f);
	drawSphere(MoonSize, 20, 20, Earth_x-Earth_x/MoonDistanceCoefficient, Earth_y-Earth_y/MoonDistanceCoefficient, Universal_z, false);
	glPopMatrix();
	
	//Марс
	glBindTexture(GL_TEXTURE_2D, texId_Mars);
	glPushMatrix();
	glTranslatef(Mars_x, Mars_y, Universal_z);
	glRotatef(EarthSpinAngle, 0.0f, 0.0f, 1.0f);
	drawSphere(MarsSize, 20, 20, 0, 0, 0, false);
	glPopMatrix();
	Mars_angle += MarsOrbitSpeed;

	//Юпитер
	glBindTexture(GL_TEXTURE_2D, texId_Jupiter);
	glPushMatrix();
	glTranslatef(Jupiter_x, Jupiter_y, Universal_z);
	glRotatef(JupiterSpinAngle, 0.0f, 0.0f, 1.0f);
	drawSphere(JupiterSize, 20, 20, 0, 0, 0, false);
	glPopMatrix();
	Jupiter_angle += JupiterOrbitSpeed;

	//Сатурн
	glBindTexture(GL_TEXTURE_2D, texId_Saturn);
	glPushMatrix();
	glTranslatef(Saturn_x, Saturn_y, Universal_z);
	glRotatef(SaturnSpinAngle, 0.0f, 0.0f, 1.0f);
	drawSphere(SaturnSize, 20, 20, 0, 0, 0, false);
	glPopMatrix();
	drawRing(SaturnSize + Saturn_Ring_Inner_Radius, SaturnSize + Saturn_Ring_Outer_Radius, 30, Saturn_Ring_X, Saturn_Ring_Y, 0);
	Saturn_angle += SaturnOrbitSpeed;

	//Уран
	glBindTexture(GL_TEXTURE_2D, texId_Uranus);
	glPushMatrix();
	glTranslatef(Uranus_x, Uranus_y, Universal_z);
	glRotatef(UranusSpinAngle, 0.0f, 0.0f, 1.0f);
	drawSphere(UranusSize, 20, 20, 0, 0, 0, false);
	glPopMatrix();
	drawRing(UranusSize + Uranus_Ring_Inner_Radius, UranusSize + Uranus_Ring_Outer_Radius, 30, Uranus_Ring_X, Uranus_Ring_Y, 0);
	Uranus_angle += UranusOrbitSpeed;

	//Нептун
	glBindTexture(GL_TEXTURE_2D, texId_Neptune);
	glPushMatrix();
	glTranslatef(Neptune_x, Neptune_y, Universal_z);
	glRotatef(NeptuneSpinAngle, 0.0f, 0.0f, 1.0f);
	drawSphere(NeptuneSize, 20, 20, 0, 0, 0, false);
	glPopMatrix();
	Neptune_angle += NeptuneOrbitSpeed;

	if (!stop)
	{

		beltSpinAngle = beltSpinAngle + beltSpinAngleStep;
		SunSpinAngle = SunSpinAngle + SunOrbitSpeedOwnStep;
		MercurySpinAngle = MercurySpinAngle + MercuryOrbitSpeedOwnStep;
		VenusSpinAngle = VenusSpinAngle + VenusOrbitSpeedOwnStep;
		EarthSpinAngle = EarthSpinAngle + EarthOrbitSpeedOwnStep;
		MoonSpinAngle = MoonSpinAngle + MoonOrbitSpeedOwnStep;
		MarsSpinAngle = MarsSpinAngle + MarsOrbitSpeedOwnStep;
		JupiterSpinAngle = JupiterSpinAngle + JupiterOrbitSpeedOwnStep;
		SaturnSpinAngle = SaturnSpinAngle + SaturnOrbitSpeedOwnStep;
		UranusSpinAngle = UranusSpinAngle + UranusOrbitSpeedOwnStep;
		NeptuneSpinAngle = NeptuneSpinAngle + NeptuneOrbitSpeedOwnStep;
	}

	//Сообщение вверху экрана


	glMatrixMode(GL_PROJECTION);	//Делаем активной матрицу проекций. 
	//(всек матричные операции, будут ее видоизменять.)
	glPushMatrix();   //сохраняем текущую матрицу проецирования (которая описывает перспективную проекцию) в стек 				    
	glLoadIdentity();	  //Загружаем единичную матрицу
	glOrtho(0, ogl->getWidth(), 0, ogl->getHeight(), 0, 1);	 //врубаем режим ортогональной проекции

	glMatrixMode(GL_MODELVIEW);		//переключаемся на модел-вью матрицу
	glPushMatrix();			  //сохраняем текущую матрицу в стек (положение камеры, фактически)
	glLoadIdentity();		  //сбрасываем ее в дефолт

	glDisable(GL_LIGHTING);



	GuiTextRectangle rec;		   //классик моего авторства для удобной работы с рендером текста.
	rec.setSize(300, 150);
	rec.setPosition(10, ogl->getHeight() - 150 - 10);



	std::stringstream ss;
	ss << "T - Вкл/Выкл линии орбит" << std::endl;
	ss << "S - Вкл/Выкл орбиту" << std::endl;
	ss << "F/R - Ускорить/Замедлить - " <<  "Скорость: X" << h << std::endl;
	ss << "P/M- Приблизить/Отдалить " << "Zoom: X" << 40 - k << std::endl;



	rec.setText(ss.str().c_str());
	rec.Draw();

	glMatrixMode(GL_PROJECTION);	  //восстанавливаем матрицы проекции и модел-вью обратьно из стека.
	glPopMatrix();


	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

}

