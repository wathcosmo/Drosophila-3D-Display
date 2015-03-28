#ifndef _BMP_LOADER_H__
#define _BMP_LOADER_H__

#include<stdio.h>
#include<fstream>
#include<Windows.h>

#define BMPTYPE_BGR 0
#define BMPTYPE_RGB 1
#define BMPTYPE_RGBA 2

//��ȡbmp�ļ���
class BMPLoader
{
public:
	unsigned char *pImg;	//����ָ��
	int imgHeight;
	int imgWidth;
	int lineByte;	//ÿ���ֽ���
	int type;	//��������

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

	//��ȡbmp�ļ��������Ƿ��ȡ�ɹ�
	bool loadImage(const char *filename)
	{
		errno_t err;
		FILE *fp;
		if ((err = fopen_s( &fp, filename, "rb")) != 0)
			return false;
		BITMAPFILEHEADER bmpFileHeader;
		BITMAPINFOHEADER bmpInfoHeader;
		
		//��ȡ�ļ�ͷ
		fread(&bmpFileHeader, sizeof(BITMAPFILEHEADER), 1, fp);
		if (bmpFileHeader.bfType != 0x4D42)
		{
			fclose(fp);
			return false;
		}

		//��ȡλͼ��Ϣ
		fread(&bmpInfoHeader, sizeof(BITMAPINFOHEADER), 1, fp);
		imgHeight = bmpInfoHeader.biHeight;
		imgWidth = bmpInfoHeader.biWidth;
		lineByte = (imgWidth * bmpInfoHeader.biBitCount/8+3)/4*4;
		bmpInfoHeader.biSizeImage = imgHeight * lineByte;
		type = BMPTYPE_BGR;

		//�����ڴ�
		pImg = new unsigned char[bmpInfoHeader.biSizeImage];
		if (!pImg)
		{
			fclose(fp);
			return false;
		}

		//��ȡ����
		fseek(fp, bmpFileHeader.bfOffBits, SEEK_SET);
		fread(pImg, 1, bmpInfoHeader.biSizeImage, fp);
		fclose(fp);
		return true;
	}

	//��BGR��ʽ������ת��ΪRGB
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

	//��BGR��ʽ������ת��ΪRGBA
	//���ȱ�ת��Ϊ͸����
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
