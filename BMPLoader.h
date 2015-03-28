#ifndef _BMP_LOADER_H__
#define _BMP_LOADER_H__

#include<stdio.h>
#include<fstream>
#include<Windows.h>

#define BMPTYPE_BGR 0
#define BMPTYPE_RGB 1
#define BMPTYPE_RGBA 2

//读取bmp文件类
class BMPLoader
{
public:
	unsigned char *pImg;	//数据指针
	int imgHeight;
	int imgWidth;
	int lineByte;	//每行字节数
	int type;	//数据类型

	BMPLoader()
	{
		pImg = NULL;
		imgHeight = 0;
		imgWidth = 0;
		type = BMPTYPE_BGR;
	}

	~BMPLoader()
	{
		if (pImg != NULL)
			delete[] pImg;
	}

	//读取bmp文件，返回是否读取成功
	bool loadImage(const char *filename)
	{
		errno_t err;
		FILE *fp;
		if ((err = fopen_s( &fp, filename, "rb")) != 0)
			return false;
		BITMAPFILEHEADER bmpFileHeader;
		BITMAPINFOHEADER bmpInfoHeader;
		
		//读取文件头
		fread(&bmpFileHeader, sizeof(BITMAPFILEHEADER), 1, fp);
		if (bmpFileHeader.bfType != 0x4D42)
		{
			fclose(fp);
			return false;
		}

		//读取位图信息
		fread(&bmpInfoHeader, sizeof(BITMAPINFOHEADER), 1, fp);
		imgHeight = bmpInfoHeader.biHeight;
		imgWidth = bmpInfoHeader.biWidth;
		lineByte = (imgWidth * bmpInfoHeader.biBitCount/8+3)/4*4;
		bmpInfoHeader.biSizeImage = imgHeight * lineByte;
		type = BMPTYPE_BGR;

		//分配内存
		pImg = new unsigned char[bmpInfoHeader.biSizeImage];
		if (!pImg)
		{
			fclose(fp);
			return false;
		}

		//读取数据
		fseek(fp, bmpFileHeader.bfOffBits, SEEK_SET);
		fread(pImg, 1, bmpInfoHeader.biSizeImage, fp);
		fclose(fp);
		return true;
	}

	//将BGR格式的数据转换为RGB
	void BGRToRGB()
	{
		if (pImg == NULL || type != BMPTYPE_BGR)
			return;
		int i, j, index;
		unsigned char temp;
		for (i=0; i<imgHeight; i++)
			for (j=0; j<imgWidth; j++)
			{
				index = i*lineByte + j*3;
				temp = pImg[index];
				pImg[index] = pImg[index+2];
				pImg[index+2] = temp;
			}
		type = BMPTYPE_RGB;
	}

	//将BGR格式的数据转换为RGBA
	//亮度被转化为透明度
	void BGRToRGBA()
	{
		if (pImg == NULL || type != BMPTYPE_BGR)
			return;
		unsigned char *lastImg = pImg;
		pImg = new unsigned char[imgHeight * imgWidth * 4];
		int i, j, indexLast, indexImg;
		unsigned char Y, R, G, B;
		for (i=0; i<imgHeight; i++)
		{
			for (j=0; j<imgWidth; j++)
			{
				indexImg = (i*imgWidth + j)*4;
				indexLast = i*lineByte + j*3;
				R = lastImg[indexLast + 2];
				G = lastImg[indexLast + 1];
				B = lastImg[indexLast];
				Y = (unsigned char)(0.299*(float)R + 0.587*(float)G + 0.114*(float)B);
				pImg[indexImg] = R;
				pImg[indexImg + 1] = G;
				pImg[indexImg + 2] = B;
				pImg[indexImg + 3] = Y;
			}
		}
		type = BMPTYPE_RGBA;
		delete[] lastImg;
	}

	void freeImage()
	{
		if (pImg != NULL)
		{
			delete[] pImg;
			pImg = NULL;
		}
	}
};

#endif
