#ifndef _MY_CONTROLLER_H__
#define _MY_CONTROLLER_H__

#define _USE_MATH_DEFINES
#include <iostream>
#include <fstream>
#include <cmath>
using namespace std;

//用arcball模拟旋转
class ArcBall
{
protected:
	double winWidth;	//窗口宽度
	double winHeight;	//窗口高度
	double offsetX;	//(0, 0)在窗口中位置距窗口中心的偏移
	double offsetY;
	double lastX;	//上次旋转鼠标位置
	double lastY;
	double lastZ;

	//通过x和y计算球体上的z
	double getZ(double *pnx, double *pny)
	{
		double nz;
		double square = (*pnx)*(*pnx) + (*pny)*(*pny);
		if (square <= 1)	//在球体内
			nz = sqrt(1 - square);
		else	//在球体外
		{
			nz = 0;
			square = sqrt(square);
			*pnx /= square;
			*pny /= square;
		}
		return nz;
	}

	//旋转矩阵重新正交化
	void orthogonalization()
	{
		//正交化第一行
		float length = sqrt(rotateMatrix[0] * rotateMatrix[0] + rotateMatrix[1] * rotateMatrix[1] + rotateMatrix[2] * rotateMatrix[2]);
		rotateMatrix[0] /= length;
		rotateMatrix[1] /= length;
		rotateMatrix[2] /= length;
		//正交化第二行
		float dot = rotateMatrix[0] * rotateMatrix[4] + rotateMatrix[1] * rotateMatrix[5] + rotateMatrix[2] * rotateMatrix[6];
		rotateMatrix[4] -= dot*rotateMatrix[0];
		rotateMatrix[5] -= dot*rotateMatrix[1];
		rotateMatrix[6] -= dot*rotateMatrix[2];
		length = sqrt(rotateMatrix[4] * rotateMatrix[4] + rotateMatrix[5] * rotateMatrix[5] + rotateMatrix[6] * rotateMatrix[6]);
		rotateMatrix[4] /= length;
		rotateMatrix[5] /= length;
		rotateMatrix[6] /= length;
		//正交化第三行
		dot = rotateMatrix[0] * rotateMatrix[8] + rotateMatrix[1] * rotateMatrix[9] + rotateMatrix[2] * rotateMatrix[10];
		rotateMatrix[8] -= dot*rotateMatrix[0];
		rotateMatrix[9] -= dot*rotateMatrix[1];
		rotateMatrix[10] -= dot*rotateMatrix[2];
		dot = rotateMatrix[4] * rotateMatrix[8] + rotateMatrix[5] * rotateMatrix[9] + rotateMatrix[6] * rotateMatrix[10];
		rotateMatrix[8] -= dot*rotateMatrix[4];
		rotateMatrix[9] -= dot*rotateMatrix[5];
		rotateMatrix[10] -= dot*rotateMatrix[6];
		length = sqrt(rotateMatrix[8] * rotateMatrix[8] + rotateMatrix[9] * rotateMatrix[9] + rotateMatrix[10] * rotateMatrix[10]);
		rotateMatrix[8] /= length;
		rotateMatrix[9] /= length;
		rotateMatrix[10] /= length;
	}

public:
	double rotateMatrix[16];	//旋转矩阵
	double rotateRate;	//旋转速度
	bool active;	//是否激活

	ArcBall(bool a, int width = 640, int height = 480, int rate = 2.0, int cX = 0, int cY = 0)
	{
		winWidth = width;
		winHeight = height;
		rotateRate = rate;
		offsetX = cX;
		offsetY = cY;
		active = a;
		resetMatrix();
		return;
	}

	~ArcBall()
	{
	}

	//重置
	void reset()
	{
		offsetX = 0;
		offsetY = 0;
		resetMatrix();
	}

	//改变窗口大小
	void resetWindowSize(int width, int height)
	{
		winWidth = width;
		winHeight = height;
		return;
	}
	
	//中心点移动
	void offsetMoveTo(int cX, int cY)
	{
		offsetX = cX;
		offsetY = cY;
	}
	
	//重置矩阵
	void resetMatrix()
	{
		for (int i=0; i<16; i++)
			rotateMatrix[i] = 0;
		rotateMatrix[0] = 1;
		rotateMatrix[5] = 1;
		rotateMatrix[10] = 1;
		rotateMatrix[15] = 1;
		return;
	}

	//鼠标按下
	void mouseClickAt(int mx, int my)
	{
		//计算鼠标在球体上的位置
		lastX = ((double)mx - offsetX) / winWidth * 2 - 1;
		lastY = 1 - ((double)my - offsetY) / winHeight * 2;
		lastZ = getZ(&lastX, &lastY);
		return;
	}

	//鼠标移动
	void mouseMoveTo(int mx, int my)
	{
		//计算鼠标在球体上的位置
		double normX = ((double)mx - offsetX) / winWidth * 2 - 1;
		double normY = 1 - ((double)my - offsetY) / winHeight * 2;
		double normZ = getZ(&normX, &normY);

		//计算旋转
		double angle = rotateRate * acos(normX*lastX + normY*lastY + normZ*lastZ);
		double axisX = lastY*normZ - lastZ*normY;
		double axisY = lastZ*normX - lastX*normZ;
		double axisZ = lastX*normY - lastY*normX;
		double length = sqrt(axisX*axisX + axisY*axisY + axisZ*axisZ);
		if (length == 0)
			return;
		axisX /= length;
		axisY /= length;
		axisZ /= length;
		rotate(axisX*rotateMatrix[0] + axisY*rotateMatrix[1] + axisZ*rotateMatrix[2],
			axisX*rotateMatrix[4] + axisY*rotateMatrix[5] + axisZ*rotateMatrix[6],
			axisX*rotateMatrix[8] + axisY*rotateMatrix[9] + axisZ*rotateMatrix[10], angle);
		lastX = normX;
		lastY = normY;
		lastZ = normZ;
		return;
	}

	//绕轴(ux, uy, uz)旋转angle（弧度），angle为正表示右手旋转
	void rotate(double ux, double uy, double uz, double angle)
	{
		angle = -angle;
		int i, j, k, index;
		double *newRotate = new double [9];
		double *temp = new double [16];
		double cosA = cos(angle);
		double _cosA = 1-cosA;
		double sinA = sin(angle);
		newRotate[0] = cosA + ux*ux*_cosA;
		newRotate[1] = ux*uy*_cosA - uz*sinA;
		newRotate[2] = ux*uz*_cosA + uy*sinA;
		newRotate[3] = ux*uy*_cosA + uz*sinA;
		newRotate[4] = cosA + uy*uy*_cosA;
		newRotate[5] = uy*uz*_cosA - ux*sinA;
		newRotate[6] = ux*uz*_cosA - uy*sinA;
		newRotate[7] = uy*uz*_cosA + ux*sinA;
		newRotate[8] = cosA + uz*uz*_cosA;

		for (i=0; i<16; i++)
			temp[i] = rotateMatrix[i];
		for (i=0; i<3; i++)
			for (j=0; j<3; j++)
			{
				index = i*4+j;
				rotateMatrix[index] = 0;
				for (k=0; k<3; k++)
					rotateMatrix[index] += newRotate[i*3+k]*temp[k*4+j];
			}
		orthogonalization();
		delete[] newRotate;
		delete[] temp;
	}

	//输出矩阵
	void outputMatrix()
	{
		ofstream fout("1.txt");
		int i, j;
		for (i = 0; i < 4; i++)
		{
			for (j = 0; j < 4; j++)
				fout << rotateMatrix[i * 4 + j] << '\t';
			fout << endl;
		}
	}
};


class MoveController2D
{
protected:
	int lastX;	//鼠标上次位置
	int lastY;

public:
	double moveRateX;	//移动速率
	double moveRateY;
	double moveX2D;	//平面移动向量
	double moveY2D;
	bool active;	//是否激活

	MoveController2D(bool a, double rateX = 0.02, double rateY = 0.02)
	{
		moveRateX = rateX;
		moveRateY = rateY;
		active = a;
	}

	~MoveController2D()
	{
	}

	//重置
	void reset()
	{
		moveX2D = 0;
		moveY2D = 0;
	}

	//设置移动速率
	void setMoveRate(double rateX, double rateY)
	{
		moveRateX = rateX;
		moveRateY = rateY;
	}

	//鼠标单击
	void mouseClickAt(int mx, int my)
	{
		lastX = mx;
		lastY = my;
	}

	//鼠标移动
	void mouseMoveTo(int mx, int my)
	{
		moveX2D += (mx-lastX) * moveRateX;
		moveY2D += (lastY-my) * moveRateY;
		lastX = mx;
		lastY = my;
	}
};

class ScaleController
{
protected:
	double winWidth;	//窗口宽度
	double winHeight;	//窗口高度
	double offsetX;	//缩放中心在窗口中位置距窗口中心的偏移
	double offsetY;
	double lastLength;	//上次鼠标位置距中心点的距离

public:
	double scale;	//缩放率
	bool active;	//是否激活

	ScaleController(bool a, double rate = 0.015, int cX = 0, int cY = 0)
	{
		scale = 1;
		offsetX = cX;
		offsetY = cY;
		active = a;
	}

	~ScaleController()
	{
	}

	//重置
	void reset()
	{
		scale = 1;
		offsetX = 0;
		offsetY = 0;
	}

	//改变窗口大小
	void resetWindowSize(int width, int height)
	{
		winWidth = width;
		winHeight = height;
		return;
	}
	
	//中心点移动
	void offsetMoveTo(int cX, int cY)
	{
		offsetX = cX;
		offsetY = cY;
	}

	//鼠标单击
	void mouseClickAt(int mx, int my)
	{
		double dx = mx - offsetX - winWidth/2;
		double dy = my - offsetY - winHeight/2;
		lastLength = sqrt(dx*dx+dy*dy);
	}

	//鼠标移动
	void mouseMoveTo(int mx, int my)
	{
		double dx = mx - offsetX - winWidth/2;
		double dy = my - offsetY - winHeight/2;
		double length = sqrt(dx*dx+dy*dy);
		scale = scale / lastLength * length;
		lastLength = length;
	}
};

#endif