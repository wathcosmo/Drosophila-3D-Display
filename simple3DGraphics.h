#ifndef _SIMPLE_3D_GRAPHICS_H__
#define _SIMPLE_3D_GRAPHICS_H__

#define _USE_MATH_DEFINES
#include <gl\glut.h>
#include <cmath>

float uintCubePoint[8][3] = {{-0.5, -0.5, -0.5}, {-0.5, 0.5, -0.5}, {0.5, -0.5, -0.5}, {0.5, 0.5, -0.5},
							{-0.5, -0.5, 0.5}, {-0.5, 0.5, 0.5}, {0.5, -0.5, 0.5}, {0.5, 0.5, 0.5}};
int uintCubeByQuads[6][4] = {{2, 0, 1, 3}, {5, 7, 3, 1}, {0, 4, 5, 1}, {6, 2, 3, 7}, {0, 2, 6, 4}, {4, 6, 7, 5}};	//B U L R D F
float pointColorRGB[8][3] = {{0.2, 0.2, 0.2}, {1, 0, 0}, {0, 1, 0}, {1, 1, 0}, {0, 0, 1}, {1, 0, 1}, {0, 1, 1}, {1, 1, 1}};

//c = a ��� b
void crossProduct(float *a, float *b, float *c)
{
	c[0] = a[1]*b[2] - a[2]*b[1];
	c[1] = a[2]*b[0] - a[0]*b[2];
	c[2] = a[0]*b[1] - a[1]*b[0];
}

//����v��ֱ������Ϊlength�����ⷽ�������p
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

//��һ��������center���߳�Ϊlength�������壬ÿ������ɫ��ͬ������ת
void drawColorCube(float length, float *center, bool useColor = true)
{
	int i, j, k, index;
	float point[3];
	float textureQuadPoint[4][2] = {{0, 0}, {1, 0}, {1, 1}, {0, 1}};
	glBegin(GL_QUADS);	//��ʼ
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
	glEnd();	//����
	return;
}

//��һ��������ԭ�㣬�߳�Ϊlength�������壬ÿ������ɫ��ͬ������ת
void drawColorCube(float length)
{
	float center[3] = {0, 0, 0};
	drawColorCube(length, center);
}

//��һ��������center�����浽���泤��Ϊlength���뾶Ϊradius��Բ��������ת
//Բ����Բ����Ϊslice������
//ʹ�õ�ǰ��ɫ������
void drawCylinder(int slice, float length, float radius, float *center)
{
	int i;
	float theta, height;
	float fi;
	//��ÿ���������
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
	//����
	//����
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
	//����
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
	//����
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

//��һ��������ԭ�㣬���浽���泤��Ϊlength���뾶Ϊradius��Բ��������ת
//Բ����Բ����Ϊslice������
//ʹ�õ�ǰ��ɫ������
void drawCylinder(int slice, float length, float radius)
{
	float center[3] = {0, 0, 0};
	drawCylinder(slice, length, radius, center);
}

//��һ������������center�����浽���㳤��Ϊheight������뾶Ϊradius��Բ׶������ת
//Բ׶��Բ����Ϊslice������
//ʹ�õ�ǰ��ɫ������
void drawCone(int slice, float height, float radius, float *center)
{
	int i;
	float theta;
	//��ÿ���������
	float (*down)[3] = new float [slice][3];
	for (i=0; i<slice; i++)
	{
		theta = M_PI*2/(float)slice*(float)i;
		down[i][0] = radius*cos(theta) + center[0];
		down[i][1] = radius*sin(theta) + center[1];
		down[i][2] = center[2];
	}
	//����
	//����
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
	//����
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

//��һ������������ԭ�㣬���浽���㳤��Ϊheight������뾶Ϊradius��Բ׶������ת
//Բ׶��Բ����Ϊslice������
//ʹ�õ�ǰ��ɫ������
void drawCone(int slice, float height, float radius)
{
	float center[3] = {0, 0, 0};
	drawCone(slice, height, radius, center);
}

//��һ��������center���뾶Ϊradius����ϸΪthickRadius���뾶����Բ��������ת
//��Բ���ֳ�rSlice�����֣�СԲ���ֳ�tSlice������
//ʹ�õ�ǰ��ɫ������
void drawRing(int rSlice, int tSlice, float radius, float thickRadius, float *center)
{
	int i, j;
	float theta, alpha;
	float fi, fi1, fj;
	//��ÿһƬԲ�������꣬������
	float ringCenter[3] = {radius + center[0], center[1], center[2]};
	float (*point)[2][3] = new float[tSlice][2][3];	//ÿһƬԲ�������ߣ�һ��Բ����ǰ������һ��Բ���ĺ��
	float sr;
	int front = 0, next = 1;
	//�������һ��Ƭǰ�ߵ�����
	for (j=0; j<tSlice; j++)
	{
		alpha = M_PI*2/(float)tSlice*(float)j;
		point[j][front][0] = ringCenter[0] + thickRadius * cos(alpha);
		point[j][front][1] = ringCenter[1];
		point[j][front][2] = ringCenter[2] + thickRadius * sin(alpha);
	}
	for (i=0; i<rSlice; i++)
	{
		//��ǰƬ���������
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
		//���Ƹò���
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
		//����
		next = 1-next;
		front = 1-front;
	}

	delete[] point;
}

//��һ��������ԭ�㣬�뾶Ϊradius����ϸΪthickRadius���뾶����Բ��������ת
//��Բ���ֳ�rSlice�����֣�СԲ���ֳ�tSlice������
//ʹ�õ�ǰ��ɫ������
void drawRing(int rSlice, int tSlice, float radius, float thickRadius)
{
	float center[3] = {0, 0, 0};
	drawRing(rSlice, tSlice, radius, thickRadius, center);
}

//��һ��������center���뾶Ϊradius����
//���ȱ��ֳ�loSlice�����֣�γ�ȱ��ֳ�laSlice������
//ʹ�õ�ǰ��ɫ������
void drawSphere(int loSlice, int laSlice, float radius, float *center, int texture = -1)
{
	int i, j;
	float theta, alpha;
	float fi, fi1, fj;
	//��ÿһƬͬ������������꣬������
	float (*point)[2][3] = new float[laSlice+1][2][3];	//ÿһƬ�����ߣ�ÿһƬ��ǰ������һ��Ƭ�ĺ��
	float sr;
	int front = 0, next = 1;
	//�������һ��Ƭǰ�ߵ�����
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
		//��ǰƬ���������
		theta = M_PI*2 * fi1;
		for (j=0; j<=laSlice; j++)
		{
			alpha = M_PI/(float)laSlice*(float)j;
			sr = radius * sin(alpha);
			point[j][next][0] = sr * cos(theta) + center[0];
			point[j][next][1] = sr * sin(theta) + center[1];
			point[j][next][2] = radius * cos(alpha) + center[2];
		}
		//���Ƹò���
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
		//����
		next = 1-next;
		front = 1-front;
	}

	delete[] point;
}

//��һ��������center���뾶Ϊradius����
//���ȱ��ֳ�loSlice�����֣�γ�ȱ��ֳ�laSlice������
//ʹ�õ�ǰ��ɫ������
void drawSphere(int loSlice, int laSlice, float radius)
{
	float center[3] = {0, 0, 0};
	drawSphere(loSlice, laSlice, radius, center);
}

#endif