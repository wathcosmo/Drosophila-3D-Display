#ifndef _COORDINATE_TRANS_H__
#define _COORDINATE_TRANS_H__
#include <gl\glut.h>

class CoordinateTrans
{
public:
	GLint viewport[4];
	GLdouble modelview[16];
	GLdouble projection[16];

	CoordinateTrans()
	{
	}

	~CoordinateTrans()
	{
	}
	
	//刷新变换矩阵
	void refreshMatrix()
	{
		glGetIntegerv(GL_VIEWPORT, viewport);
		glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
		glGetDoublev(GL_PROJECTION_MATRIX, projection);
	}

	//鼠标XY转化为三维坐标
	void mouseXYtoPosXYZ(GLint mx, GLint my, GLdouble *px, GLdouble *py, GLdouble *pz)
	{
		refreshMatrix();
		GLdouble newY = viewport[3] - (double)my;
		GLfloat mz;
		glReadPixels(mx, (int)newY, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &mz);
		gluUnProject((double)mx, newY, (double)mz, modelview, projection, viewport, px, py, pz);
	}

	//三维坐标转化为视镜坐标
	void posXYZtoWinXYZ(GLdouble px, GLdouble py, GLdouble pz, GLdouble *wx, GLdouble *wy, GLdouble *wz)
	{
		refreshMatrix();
		gluProject(px, py, pz, modelview, projection, viewport, wx, wy, wz);
	}

	//三维坐标转化为鼠标坐标
	void posXYZtoMouseXY(GLdouble px, GLdouble py, GLdouble pz, GLint *mx, GLint *my)
	{
		GLdouble wx, wy, wz;
		posXYZtoWinXYZ(px, py, pz, &wx, &wy, &wz);
		*mx = (int)wx;
		*my = (int)(viewport[3] - wy);
	}
};

#endif