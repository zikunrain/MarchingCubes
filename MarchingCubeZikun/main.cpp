//
//  main.cpp
//  MarchingCube
//
//  Created by Zikun Deng on 2019/6/28.
//  Copyright © 2019 Zikun Deng. All rights reserved.
//

#define _CRT_SECURE_NO_WARNINGS

#include "main.h"
#include "iostream"
#include "stdio.h"
#include "string"
#include "math.h"
#include "stdlib.h"
#include "GL/glut.h"

using namespace std;

#define width 256
#define height 256
#define layer 253

int data3D[width][height][layer];
int nIntersectingCube = 0;
int nNewlyTriangle = 0;
float isoThreshold = 100;
float XRotate = 180;
float YRotate = 0;
float ZRotate = -90;
float XOffset = -130;
float YOffset = -130;
float ZOffset = 0;
float Zoom = 110.0;

void render();
void MarchingCube(float X, float Y, float Z);
void MarchingCubes();
void mouseHandlers(int button, int state, int x, int y);
void keyboardHandlers(unsigned char key, int x, int y);
void SetNormal(Point &Normal, int x, int y, int z);
void SetColor(Point &Color, Point &Normal);
void resize(int windowWeight, int windowHeight);

int main(int argc, char * argv[]) {
	cout << "Program start!" << endl;

	FILE *fptr;
	if ((fptr = fopen("sunxia_256.raw", "rb")) == NULL) {
		cout << "Fail to open the file..." << endl;
		return 0;
	}

	// process the raw file
	int c;
	for (int i = 0; i < layer; i++)
		for (int j = 0; j < height; j++)
			for (int k = 0; k < width; k++) {
				if ((c = fgetc(fptr)) == EOF) {
					fprintf(stderr, "Unexpected end of file\n");
					exit(-1);
				}
				data3D[i][j][k] = c;
			}

	fclose(fptr);
	cout << "The file has been successfully loaded" << endl;

	int windowWeight = 800;
	int windowHeight = 800;

	glutInit(&argc, argv);
	glutInitWindowPosition(0, 0);
	glutInitWindowSize(windowWeight, windowHeight);
	glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
	glutCreateWindow("MarchingCubes");

	glutDisplayFunc(render);
	glutMouseFunc(mouseHandlers);
	glutKeyboardFunc(keyboardHandlers);
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClearDepth(1.0);

	glEnable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);
	//glDisable(GL_LIGHTING);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	GLfloat light_ambient[] = { 0.25, 0.25, 0.25, 1.00 };
	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
	glEnable(GL_LIGHT0);


	resize(windowWeight, windowHeight);
	glutMainLoop();

	return 0;
}

void keyboardHandlers(unsigned char key, int x, int y) {
	switch (key) {
		case 'd':
			YRotate += 10;
			glutPostRedisplay();
			break;
		case 'a':
			YRotate -= 10;
			glutPostRedisplay();
			break;
		case 'w':
			XRotate += 10;
			glutPostRedisplay();
			break;
		case 's':
			XRotate -= 10;
			glutPostRedisplay();
			break;
		default:
			break;
	}
}

void mouseHandlers(int button, int state, int x, int y) {
	switch (button) {
		case GLUT_LEFT_BUTTON:
			if (state == GLUT_DOWN) {
				isoThreshold += 10;
				cout << "The threshold increased to " << isoThreshold << endl;
				glutPostRedisplay();
			}
			break;
		case GLUT_RIGHT_BUTTON:
			if (state == GLUT_DOWN) {
				isoThreshold -= 10;
				cout << "The threshold decreased to " << isoThreshold << endl;
				glutPostRedisplay();
			}
			break;
		case GLUT_WHEEL_UP:
			isoThreshold += 10;
			cout << "The threshold increased to " << isoThreshold << endl;
			glutPostRedisplay();
			break;
		case GLUT_WHEEL_DOWN:
			isoThreshold -= 10;
			cout << "The threshold decreased to " << isoThreshold << endl;
			glutPostRedisplay();
			break;
		default:
			break;
	}

}
void resize(int windowWeight, int windowHeight) {
	glViewport(0, 0, windowWeight, windowHeight);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	GLfloat fAspect = (GLfloat)(windowHeight > windowWeight ? windowHeight / windowWeight : windowWeight / windowHeight);

	glOrtho(-2 * Zoom, 2 * Zoom, -2 * fAspect*Zoom,
		2 * fAspect*Zoom, -10 * 2 * Zoom, 10 * 2 * Zoom);
	glMatrixMode(GL_MODELVIEW);
}

void render() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glPushMatrix();
	glTranslatef(XOffset, YOffset, ZOffset);
	glRotatef(XRotate, 1.0, 0.0, 0.0);
	glRotatef(YRotate, 0.0, 1.0, 0.0);
	glRotatef(ZRotate, 0.0, 0.0, 1.0);
	glPushAttrib(GL_LIGHTING_BIT);
	glDisable(GL_LIGHTING);
	glPopAttrib();
	glBegin(GL_TRIANGLES);
	MarchingCubes();
	glEnd();
	glPopMatrix();
	glutSwapBuffers();
}

void MarchingCube(float X, float Y, float Z) {
	Point color;
	float CubeVertexValue[8];
	for (int i = 0; i < 8; i++) {
		CubeVertexValue[i] = (float)data3D[(int)(X + CubeVertex[i][0])]
			[(int)(Y + CubeVertex[i][1])]
		[(int)(Z + CubeVertex[i][2])];
	}

	int intersectEdge = 0;
	int intersectEdgeIndex = 0; // 标记八个顶点的状态 从而能够索引到12条边的状况
	Point intersectVertex[12];
	Point NormalVector[12];

	for (int i = 0; i < 8; i++) {
		if (CubeVertexValue[i] <= isoThreshold)
			intersectEdgeIndex |= 1 << i; // 1 << i 把1左移动i位; C |= 2  <-> C = C|2;
	}

	intersectEdge = CubeEdgeFlags[intersectEdgeIndex];
	if (intersectEdge == 0) return;

	nIntersectingCube++;

	for (int i = 0; i < 12; i++) { // 遍历12条边
		if (intersectEdge & (1 << i)) { // 如果第i条边有交点
			// 用的是一次、线性的
			float difference = CubeVertexValue[CubeEdge[i][1]] - CubeVertexValue[CubeEdge[i][0]]; // 第i条边两个端点的差
			float offset = (isoThreshold - CubeVertexValue[CubeEdge[i][0]]) / difference; 

			// 第i条边上的交点的x,y,z坐标
			intersectVertex[i].x = X + CubeVertex[CubeEdge[i][0]][0] + CubeEdgeDirection[i][0] * offset;
			intersectVertex[i].y = Y + CubeVertex[CubeEdge[i][0]][1] + CubeEdgeDirection[i][1] * offset;
			intersectVertex[i].z = Z + CubeVertex[CubeEdge[i][0]][2] + CubeEdgeDirection[i][2] * offset;

			// 获取顶点法向量
			SetNormal(NormalVector[i], (int)intersectVertex[i].x, (int)intersectVertex[i].y, (int)intersectVertex[i].z);
		}
	}

	for (int i = 0; i < 5; i++) { // 最多只有5个面
		if (TriTable[intersectEdgeIndex][3 * i] < 0) // 每三个构成一个面
			break;
		nNewlyTriangle++;
		for (int j = 0; j < 3; j++) {
			int vertex = TriTable[intersectEdgeIndex][3 * i + j];
			SetColor(color, NormalVector[vertex]);
			glColor3f(color.x, color.y, color.z); // Some bugs were obeserved here. It did not work.
			glNormal3f(NormalVector[vertex].x, NormalVector[vertex].y, NormalVector[vertex].z);
			glVertex3f(intersectVertex[vertex].x, intersectVertex[vertex].y, intersectVertex[vertex].z);
		}
	}
}

void MarchingCubes() {
	nNewlyTriangle = 0;
	nIntersectingCube = 0;
	int i, j, k;
	for (i = 0; i < width - 1; i++)
		for (j = 0; j < height - 1; j++)
			for (k = 0; k < layer - 1; k++)
				MarchingCube(i, j, k);

	cout << nIntersectingCube << " cubes are intersected." << endl;
	cout << nNewlyTriangle << " triangles are newly generated." << endl;
}


void SetNormal(Point &Normal, int x, int y, int z) {
	if (x > 1 && x < width - 1 && y>1 && y < height - 1 && z>1 && z < layer - 1) {
		// 中心差分法 /2可以省略
		Normal.x = (float)(data3D[x - 1][y][z] - data3D[x + 1][y][z]);
		Normal.y = (float)(data3D[x][y - 1][z] - data3D[x][y + 1][z]);
		Normal.z = (float)(data3D[x][y][z - 1] - data3D[x][y][z + 1]);

		// normalized
		float length = sqrtf((Normal.x * Normal.x) + (Normal.y * Normal.y) + (Normal.z * Normal.z));
		Normal.x = Normal.x * 1.0 / length;
		Normal.y = Normal.y * 1.0 / length;
		Normal.z = Normal.z * 1.0 / length;
	}
}

void SetColor(Point &color, Point &normal) {
	GLfloat fx = normal.x;
	GLfloat fy = normal.y;
	GLfloat fz = normal.z;
    color.x = fx;
    color.y = fy;
    color.z = fz;
}
