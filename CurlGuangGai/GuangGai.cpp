// GuangGai.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"



#include "GuangGai.h"
#include "..\Inc\MData.h"



#ifndef   DLLEXPT
#define DLLEXPT __declspec (dllexport )
#endif

void MyHalconExceptionHandler(const Halcon::HException& except)
{
	throw except;	
}

CDoProcess::CDoProcess()
{
	ShowObject=false;
	//m_ProcessName ="MianXinTestAuto";
	HException::InstallHHandler(&MyHalconExceptionHandler);

}


void CDoProcess::SetParmeter(string parm,string value)
{
	if(parm=="SHOWOBJECT")
		if (value=="YES")
		{
			ShowObject = true;
		}
		else
		{
			ShowObject = false;
		}

	if(parm=="Regional_Out")
		m_Parameters.Regional_Out = atof(value.c_str());
	if(parm=="ROIWidth")
		m_Parameters.ROIWidth = atoi(value.c_str());
	if(parm=="GuangGaiThreshed")
		m_Parameters.GuangGaiThreshed= atoi(value.c_str());
}


void CDoProcess::DoProcess(CMData* pData,SubTestResult *testItem)
{	

	try
	{
		Hobject image=pData->m_Image;

		Hobject  ROI,Circle1,Circle2;

		if((pData->r_real+m_Parameters.Regional_Out-m_Parameters.ROIWidth)>0)
		{
			gen_circle(&Circle1, (pData->m_center_y), (pData->m_center_x), (pData->r_real+m_Parameters.Regional_Out));
			gen_circle(&Circle2, (pData->m_center_y), (pData->m_center_x), pData->r_real+m_Parameters.Regional_Out-m_Parameters.ROIWidth);
			difference(Circle1, Circle2, &ROI);

			if (ShowObject&&pData->m_isDebug)
			{
				set_color(pData->m_ShowHWindow,"blue");	
				disp_obj(Circle1,pData->m_ShowHWindow);
				set_color(pData->m_ShowHWindow,"green");	
				disp_obj(Circle2,pData->m_ShowHWindow);
			}

		}
		else
		{
			gen_circle(&ROI, (pData->m_center_y), (pData->m_center_x), (pData->r_real+m_Parameters.Regional_Out));

			if (ShowObject&&pData->m_isDebug)
			{
				set_color(pData->m_ShowHWindow,"blue");	
				disp_obj(ROI,pData->m_ShowHWindow);
			}
		}


   Hobject region_tai;

   HTuple  Mean, Deviation;

  //*===================================================================
  reduce_domain(image, ROI, &region_tai);
  intensity(ROI, region_tai, &Mean, &Deviation);


  if (0 != (Mean>m_Parameters.GuangGaiThreshed))
  {

	  pData->m_isFail =true;//图片有问题
	  testItem->m_bFailSubTest = true;//检测项目报错
	  if (pData->m_isDebug)
	  {
		  set_color(pData->m_ShowHWindow,"red");	
		  disp_obj(ROI,pData->m_ShowHWindow);
		  set_tposition( pData->m_ShowHWindow,120, 0);
		#ifdef _ENGLISH
				  write_string(pData->m_ShowHWindow,"Out of compound");
		#else
				  write_string(pData->m_ShowHWindow,"光盖");
		#endif
	  }

	  Hobject RegionUnionWhole;
	  union2(ROI,pData->m_ErrorRegist,&RegionUnionWhole);
	  pData->m_ErrorRegist=RegionUnionWhole;
  }

}
	catch (HException &except) 
	
	{
		if (pData->m_isDebug)
		{
			set_color(pData->m_ShowHWindow,"green");
			set_tposition( pData->m_ShowHWindow,25, 145);
			#ifdef _ENGLISH
						write_string(pData->m_ShowHWindow,"GuangGai parameter values error,please re-adjust");
			#else
						write_string(pData->m_ShowHWindow,"GuangGai程序参数值出错,请重新调节参数");
			#endif
		}
		pData->m_isFail = true;
		testItem->m_bFailSubTest = true;
	}

}