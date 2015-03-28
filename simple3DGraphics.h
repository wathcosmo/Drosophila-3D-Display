#ifndef _SIMPLE_3D_GRAPHICS_H__
#define _SIMPLE_3D_GRAPHICS_H__

#define _USE_MATH_DEFINES
#include <gl\glut.h>
#include <cmath>

float uintCubePoint[8][3] = {{-0.5, -0.5, -0.5}, {-0.5, 0.5, -0.5}, {0.5, -0.5, -0.5}, {0.5, 0.5, -0.5},
							{-0.5, -0.5, 0.5}, {-0.5, 0.5, 0.5}, {0.5, -0.5, 0.5}, {0.5, 0.5, 0.5}};
int uintCubeByQuads[6][4] = {{2, 0, 1, 3}, {5, 7, 3, 1}, {0, 4, 5, 1}, {6, 2, 3, 7}, {0, 2, 6, 4}, {4, 6, 7, 5}};	//B U L R D F
float pointColorRGB[8][3] = {{0.2, 0.2, 0.2}, {1, 0, 0}, {0, 1, 0}, {1, 1, 0}, {0, 0, 1}, {1, 0, 1}, {0, 1, 1}, {1, 1, 1}};

//c = a 叉乘 b
void crossProduct(float *a, float *b, float *c)
{
	c[0] = a[1]*b[2] - a[2]*b[1];
	c[1] = a[2]*b[0] - a[0]*b[2];
	c[2] = a[0]*b[1] - a[1]*b[0];
}

//求与v垂直，长度为length的任意方向的向量p
void findPerpendicular(float *a, float length, float *b)
{
	float c[3] = {1, 0, 0};
	if (a[1] == 0 && a[2] == 0)
	{
		if (a[0] == 0)
			return;
		c[1] = 1;
	}
	b[0] = -a[2]*c[1];
	b[1] = a[2]*c[0];
	b[2] = a[0]*c[1] - a[1]*c[0];
	float l = sqrt(b[0]*b[0] + b[1]*b[1] + b[2]*b[2]);
	b[0] = b[0] / l *length;
	b[1] = b[1] / l *length;
	b[2] = b[2] / l *length;
	return;
}

//画一个中心在center，边长为length的正方体，每个点颜色不同，不旋转
void drawColorCube(float length, float *center, bool useColor = true)
{
	int i, j, k, index;
	float point[3];
	float textureQuadPoint[4][2] = {{0, 0}, {1, 0}, {1, 1}, {0, 1}};
	glBegin(GL_QUADS);	//开始
	for (i=0; i<6; i++)
		for (j=0; j<4; j++)
		{
			index = uintCubeByQuads[i][j];
			for (k=0; k<3; k++)
				point[k] = center[k] + uintCubePoint[index][k]*length;
			if (useColor)
				glColor3f(pointColorRGB[index][0], pointColorRGB[index][1], pointColorRGB[index][2]);
			else
				glTexCoord2f(textureQuadPoint[j][0], textureQuadPoint[j][1]);
			glVertex3f(point[0], point[1], point[2]);
		}
	glEnd();	//结束
	return;
}

//画一个中心在原点，边长为length的正方体，每个点颜色不同，不旋转
void drawColorCube(float length)
{
	float center[3] = {0, 0, 0};
	drawColorCube(length, center);
}

//画一个中心在center，底面到顶面长度为length，半径为radius的圆柱，不旋转
//圆柱的圆被分为slice个部分
//使用当前着色或纹理
void drawCylinder(int slice, float length, float radius, float *center)
{
	int i;
	float theta, height;
	float fi;
	//求每个点的坐标
	float (*up)[3] = new float [slice][3];
	float (*down)[3] = new float [slice][3];
	height = length/2;
	for (i=0; i<slice; i++)
	{
		theta = M_PI*2/(float)slice*(float)i;
		up[i][0] = radius*cos(theta) + center[0];
		down[i][0] = up[i][0];
		up[i][1] = radius*sin(theta) + center[1];
		down[i][1] = up[i][1];
		up[i][2] = height + center[2];
		down[i][2] = -height + center[2];
	}
	//绘制
	//顶面
	glBegin(GL_TRIANGLE_FAN);
	glTexCoord2f(0.5, 0.5);
	glVertex3f(center[0], center[1], center[2] + height);
	for (i=0; i<slice; i++)
	{
		theta = M_PI*2/(float)slice*(float)i;
		glTexCoord2f(0.5 + 0.5*cos(theta), 0.5 + 0.5*sin(theta));
		glVertex3f(up[i][0], up[i][1], up[i][2]);
	}
	glTexCoord2f(1, 0.5);
	glVertex3f(up[0][0], up[0][1], up[0][2]);
	glEnd();
	//底面
	glBegin(GL_TRIANGLE_FAN);
	glTexCoord2f(0.5, 0.5);
	glVertex3f(center[0], center[1], center[2] - height);
	for (i=slice-1; i>=0; i--)
	{
		theta = M_PI*2/(float)slice*(float)(i+1);
		glTexCoord2f(0.5 + 0.5*cos(theta), 0.5 + 0.5*sin(theta));
		glVertex3f(down[i][0], down[i][1], down[i][2]);
	}
	glTexCoord2f(1, 0.5);
	glVertex3f(down[slice-1][0], down[slice-1][1], down[slice-1][2]);
	glEnd();
	//侧面
	glBegin(GL_QUAD_STRIP);
	for (i=0; i<slice; i++)
	{
		fi = (float)(i) / (float)slice;
		glTexCoord2f(0, fi);
		glVertex3f(down[i][0], down[i][1], down[i][2]);
		glTexCoord2f(1, fi);
		glVertex3f(up[i][0], up[i][1], up[i][2]);
	}
	glTexCoord2f(0, 1);
	glVertex3f(down[0][0], down[0][1], down[0][2]);
	glTexCoord2f(1, 1);
	glVertex3f(up[0][0], up[0][1], up[0][2]);
	glEnd();

	delete[] up;
	delete[] down;
	return;
}

//画一个中心在原点，底面到顶面长度为length，半径为radius的圆柱，不旋转
//圆柱的圆被分为slice个部分
//使用当前着色或纹理
void drawCylinder(int slice, float length, float radius)
{
	float center[3] = {0, 0, 0};
	drawCylinder(slice, length, radius, center);
}

//画一个底面中心在center，底面到顶点长度为height，底面半径为radius的圆锥，不旋转
//圆锥的圆被分为slice个部分
//使用当前着色或纹理
void drawCone(int slice, float height, float radius, float *center)
{
	int i;
	float theta;
	//求每个点的坐标
	float (*down)[3] = new float [slice][3];
	for (i=0; i<slice; i++)
	{
		theta = M_PI*2/(float)slice*(float)i;
		down[i][0] = radius*cos(theta) + center[0];
		down[i][1] = radius*sin(theta) + center[1];
		down[i][2] = center[2];
	}
	//绘制
	//底面
	glBegin(GL_TRIANGLE_FAN);
	glTexCoord2f(0.5, 0.5);
	glVertex3f(center[0], center[1], center[2]);
	for (i=slice-1; i>=0; i--)
	{
		theta = M_PI*2/(float)slice*(float)(i+1);
		glTexCoord2f(0.5 + 0.5*cos(theta), 0.5 + 0.5*sin(theta));
		glVertex3f(down[i][0], down[i][1], down[i][2]);
	}
	glTexCoord2f(1, 0.5);
	glVertex3f(down[slice-1][0], down[slice-1][1], down[slice-1][2]);
	glEnd();
	//侧面
	glBegin(GL_TRIANGLE_FAN);
	glTexCoord2f(0.5, 0.5);
	glVertex3f(center[0], center[1], center[2] + height);
	for (i=slice-1; i>=0; i--)
	{
		theta = M_PI*2/(float)slice*(float)(i+1);
		glTexCoord2f(0.5 + 0.5*cos(theta), 0.5 + 0.5*sin(theta));
		glVertex3f(down[i][0], down[i][1], down[i][2]);
	}
	glTexCoord2f(1, 0.5);
	glVertex3f(down[slice-1][0], down[slice-1][1], down[slice-1][2]);
	glEnd();

	delete[] down;
	return;
}

//画一个底面中心在原点，底面到顶点长度为height，底面半径为radius的圆锥，不旋转
//圆锥的圆被分为slice个部分
//使用当前着色或纹理
void drawCone(int slice, float height, float radius)
{
	float center[3] = {0, 0, 0};
	drawCone(slice, height, radius, center);
}

//画一个中心在center，半径为radius，粗细为thickRadius（半径）的圆环，不旋转
//大圆被分成rSlice个部分，小圆被分成tSlice个部分
//使用当前着色或纹理
void drawRing(int rSlice, int tSlice, float radius, float thickRadius, float *center)
{
	int i, j;
	float theta, alpha;
	float fi, fi1, fj;
	//求每一片圆环的坐标，并绘制
	float ringCenter[3] = {radius + center[0], center[1], center[2]};
	float (*point)[2][3] = new float[tSlice][2][3];	//每一片圆环的两边，一个圆环的前边是上一个圆环的后边
	float sr;
	int front = 0, next = 1;
	//单独求第一个片前边的坐标
	for (j=0; j<tSlice; j++)
	{
		alpha = M_PI*2/(float)tSlice*(float)j;
		point[j][front][0] = ringCenter[0] + thickRadius * cos(alpha);
		point[j][front][1] = ringCenter[1];
		point[j][front][2] = ringCenter[2] + thickRadius * sin(alpha);
	}
	for (i=0; i<rSlice; i++)
	{
		//求当前片后面的坐标
		fi = (float)(i) / (float)rSlice;
		fi1 = (float)(i+1) / (float)rSlice;
		theta = M_PI*2 * fi1;
		ringCenter[0] = center[0] + radius * cos(theta);
		ringCenter[1] = center[1] + radius * sin(theta);
		for (j=0; j<tSlice; j++)
		{
			alpha = M_PI*2/(float)tSlice*(float)j;
			sr = thickRadius * cos(alpha);
			point[j][next][0] = ringCenter[0] + sr * cos(theta);
			point[j][next][1] = ringCenter[1] + sr * sin(theta);
			point[j][next][2] = ringCenter[2] + thickRadius * sin(alpha);
		}
		//绘制该部分
		glBegin(GL_QUAD_STRIP);
		for (j=0; j<tSlice; j++)
		{
			fj = (float)(j) / (float)tSlice;
			glTexCoord2f(fi, fj);
			glVertex3f(point[j][front][0], point[j][front][1], point[j][front][2]);
			glTexCoord2f(fi1, fj);
			glVertex3f(point[j][next][0], point[j][next][1], point[j][next][2]);
		}
		glTexCoord2f(fi, 1);
		glVertex3f(point[0][front][0], point[0][front][1], point[0][front][2]);
		glTexCoord2f(fi1, 1);
		glVertex3f(point[0][next][0], point[0][next][1], point[0][next][2]);
		glEnd();
		//交换
		next = 1-next;
		front = 1-front;
	}

	delete[] point;
}

//画一个中心在原点，半径为radius，粗细为thickRadius（半径）的圆环，不旋转
//大圆被分成rSlice个部分，小圆被分成tSlice个部分
//使用当前着色或纹理
void drawRing(int rSlice, int tSlice, float radius, float thickRadius)
{
	float center[3] = {0, 0, 0};
	drawRing(rSlice, tSlice, radius, thickRadius, center);
}

//画一个中心在center，半径为radius的球
//经度被分成loSlice个部分，纬度被分成laSlice个部分
//使用当前着色或纹理
void drawSphere(int loSlice, int laSlice, float radius, float *center, int texture = -1)
{
	int i, j;
	float theta, alpha;
	float fi, fi1, fj;
	//求每一片同经度区域的坐标，并绘制
	float (*point)[2][3] = new float[laSlice+1][2][3];	//每一片的两边，每一片的前边是上一个片的后边
	float sr;
	int front = 0, next = 1;
	//单独求第一个片前边的坐标
	for (j=0; j<=laSlice; j++)
	{
		alpha = M_PI/(float)laSlice*(float)j;
		point[j][front][0] = radius * sin(alpha) + center[0];
		point[j][front][1] = center[1];
		point[j][front][2] = radius * cos(alpha) + center[2];
	}
	for (i=0; i<loSlice; i++)
	{
		fi = (float)(i) / (float)loSlice;
		fi1 = (float)(i+1) / (float)loSlice;
		//求当前片后面的坐标
		theta = M_PI*2 * fi1;
		for (j=0; j<=laSlice; j++)
		{
			alpha = M_PI/(float)laSlice*(float)j;
			sr = radius * sin(alpha);
			point[j][next][0] = sr * cos(theta) + center[0];
			point[j][next][1] = sr * sin(theta) + center[1];
			point[j][next][2] = radius * cos(alpha) + center[2];
		}
		//绘制该部分
		glBegin(GL_QUAD_STRIP);
		for (j=0; j<=laSlice; j++)
		{
			fj = (float)(j) / (float)laSlice;
			glTexCoord2f(fi, fj);
			glVertex3f(point[j][front][0], point[j][front][1], point[j][front][2]);
			glTexCoord2f(fi1, fj);
			glVertex3f(point[j][next][0], point[j][next][1], point[j][next][2]);
		}
		glTexCoord2f(fi, 1);
		glVertex3f(point[0][front][0], point[0][front][1], point[0][front][2]);
		glTexCoord2f(fi1, 1);
		glVertex3f(point[0][next][0], point[0][next][1], point[0][next][2]);
		glEnd();
		//交换
		next = 1-next;
		front = 1-front;
	}

	delete[] point;
}

//画一个中心在center，半径为radius的球
//经度被分成loSlice个部分，纬度被分成laSlice个部分
//使用当前着色或纹理
void drawSphere(int loSlice, int laSlice, float radius)
{
	float center[3] = {0, 0, 0};
	drawSphere(loSlice, laSlice, radius, center);
}

#endif