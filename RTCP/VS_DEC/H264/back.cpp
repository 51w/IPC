#include <iostream>
#include <vector>

#include "hi_config.h"
#include "hi_h264api.h"

typedef struct _NaluUnit
{
	unsigned int size;
	int   type;
	char *data;
}NaluUnit;
int parse_h264(std::vector<NaluUnit> &input);

using namespace std;
#define BYTE_LEN 300*1024

int main()
{
	vector<NaluUnit> input;
	parse_h264(input);
	cout << input.size() << endl;

	H264_LIBINFO_S    lib_info;
	if (0 == Hi264DecGetInfo(&lib_info))
	{
		printf("Version: %s\nCopyright: %s\n", lib_info.sVersion, lib_info.sCopyRight);
		printf("FunctionSet: 0x%x\n", lib_info.uFunctionSet);
	}

	FILE *yuv = NULL;
	yuv = fopen("out.yuv", "wb");
	if (NULL == yuv)
	{
		fprintf(stderr, "Unable to open the file to save yuv.\n");
	}
	
	H264_DEC_ATTR_S dec_attrbute;
	dec_attrbute.uBufNum = 4;     // reference frames number: 16
	dec_attrbute.uPicHeightInMB = 256;     // D1(720x576)
	dec_attrbute.uPicWidthInMB = 256;
	dec_attrbute.uStreamInType = 0x00;   // bitstream begin with "00 00 01" or "00 00 00 01"
	
	dec_attrbute.uWorkMode = 0x01;
	dec_attrbute.pUserData = NULL;

	//create a decoder
	HI_HDL handle = NULL;
	handle = Hi264DecCreate(&dec_attrbute);
	if (NULL == handle)
	{
		goto exitmain;
	}

	HI_U8  buf[BYTE_LEN];
	H264_DEC_FRAME_S  dec_frame;
	buf[0] = 0x00;
	buf[1] = 0x00;
	buf[2] = 0x00;
	buf[3] = 0x01;

	int npos = 0;
	HI_S32 end = 0;
	while (!end)
	{
		//fprintf(stderr, "1111111111  %d   %d\n", npos, input[npos].size);
		memcpy(buf + 4, input[npos].data, input[npos].size);
		HI_U32  len = input[npos].size + 4;

		npos++;
		if (npos == input.size()) break;
		//	npos = 0;

		HI_S32 result = 0;
		HI_U32  flags = (len>0) ? 0 : 1;
		result = Hi264DecFrame(handle, buf, len, 0, &dec_frame, flags);

		while (HI_H264DEC_NEED_MORE_BITS != result)
		{
			if (HI_H264DEC_NO_PICTURE == result)   //flush over and all the remain picture are output
			{
				end = 1;
				break;
			}

			if (HI_H264DEC_OK == result)   //get a picture
			{
				const HI_U8 *pY = dec_frame.pY;
				const HI_U8 *pU = dec_frame.pU;
				const HI_U8 *pV = dec_frame.pV;
				HI_U32 width = dec_frame.uWidth;
				HI_U32 height = dec_frame.uHeight - dec_frame.uCroppingBottomOffset;
				HI_U32 yStride = dec_frame.uYStride;
				HI_U32 uvStride = dec_frame.uUVStride;

				fwrite(pY, 1, height* yStride, yuv);
				fwrite(pU, 1, height* uvStride / 2, yuv);
				fwrite(pV, 1, height* uvStride / 2, yuv);
			}

			result = Hi264DecFrame(handle, NULL, 0, 0, &dec_frame, flags);

		}
	}

	Hi264DecDestroy(handle);

exitmain:
	fclose(yuv);
	return 0;
}


int parse_h264(std::vector<NaluUnit> &input)
{
	FILE *fp = fopen("22.h264", "rb");
	fseek(fp, 0L, SEEK_END);
	int length = ftell(fp);
	fseek(fp, 0L, SEEK_SET);
	char *fbuff = (char *)malloc(length);
	fread(fbuff, 1, length, fp);


	int Spos = 0;
	int Epos = 0;
	while (Spos < length)
	{
		if (fbuff[Spos++] == 0x00 && fbuff[Spos++] == 0x00)
		{
			if (fbuff[Spos++] == 0x01)
				goto gotnal_head;
			else
			{
				Spos--;
				if (fbuff[Spos++] == 0x00 && fbuff[Spos++] == 0x01)
					goto gotnal_head;
				else
					continue;
			}
		}
		else
		{
			continue;
		}

	gotnal_head:
		Epos = Spos;
		//int size = 0;
		NaluUnit NALdata;
		while (Epos < length)
		{
			if (fbuff[Epos++] == 0x00 && fbuff[Epos++] == 0x00)
			{
				if (fbuff[Epos++] == 0x01)
				{
					NALdata.size = (Epos - 3) - Spos;
					break;
				}
				else
				{
					Epos--;
					if (fbuff[Epos++] == 0x00 && fbuff[Epos++] == 0x01)
					{
						NALdata.size = (Epos - 4) - Spos;
						break;
					}
				}
			}
		}
		if (Epos >= length)
		{
			NALdata.size = Epos - Spos;
			NALdata.type = fbuff[Spos] & 0x1f;
			NALdata.data = (char*)malloc(NALdata.size);
			memcpy(NALdata.data, fbuff + Spos, NALdata.size);
			input.push_back(NALdata);

			break;
		}

		NALdata.type = fbuff[Spos] & 0x1f;
		NALdata.data = (char*)malloc(NALdata.size);
		memcpy(NALdata.data, fbuff + Spos, NALdata.size);
		if (NALdata.type != 6)	input.push_back(NALdata);

		Spos = Epos - 4;
	}

	free(fbuff);
	fclose(fp);
	return 0;
}