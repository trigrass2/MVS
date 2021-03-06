// DynThreshed1.3.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"



#include "DynThreshed1.3.h"
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


	if(parm=="BlackMaskSize")
		m_Parameters.BlackMaskSize= atoi(value.c_str());
	if(parm=="BlackPointDynThresh")
		m_Parameters.BlackPointDynThresh= ((float)(atof(value.c_str())));
	if(parm=="BlackPointSize")
		m_Parameters.BlackPointSize= atoi(value.c_str());
	if(parm=="SeriousBlackPointDynThresh")
		m_Parameters.SeriousBlackPointDynThresh= ((float)(atof(value.c_str())));
	if(parm=="SeriousBlackPointSize")
		m_Parameters.SeriousBlackPointSize= atoi(value.c_str());


	if(parm=="WhiteMaskSize")
		m_Parameters.WhiteMaskSize= atoi(value.c_str());
	if(parm=="WhitePointDynThresh")
		m_Parameters.WhitePointDynThresh= ((float)(atof(value.c_str())));
	if(parm=="WhitePointSize")
		m_Parameters.WhitePointSize=atoi(value.c_str());
	if(parm=="SeriousWhitePointDynThresh")
		m_Parameters.SeriousWhitePointDynThresh= ((float)(atof(value.c_str())));
	if(parm=="SeriousWhitePointSize")
		m_Parameters.SeriousWhitePointSize=atoi(value.c_str());

}


void CDoProcess::DoProcess(CMData* pData,SubTestResult *testItem)
{	

	try
	{

		Hobject image = pData->m_Image;
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



  // Local iconic variables 
  Hobject  region_tai,BlackImageMean,WhiteImageMean;
  Hobject  BlackPointDynThresh, BLACK_POINT, SeriousBlackPointDynThresh;
  Hobject  SeriousBlackConnected, SERIOUS_BlackPoint;
  Hobject  WhitePointDynThresh, WHITE_POINT, SeriousWhitePointDynThresh;
  Hobject  SeriousWhiteConnected, SERIOUS_WhitePoint;
  Hlong  Number;
  Hobject  Dilation,ExpandedImage;

  	reduce_domain(image, ROI, &region_tai);
	expand_domain_gray(region_tai, &ExpandedImage, 2);
	reduce_domain(ExpandedImage, ROI, &region_tai);
  //*===================================================================
  //*找黑色划痕与污点
  mean_image(region_tai, &BlackImageMean, m_Parameters.BlackMaskSize, m_Parameters.BlackMaskSize);

  //*找大范围黑缺陷，适用于缺陷浅以及数量比较多的情况，参数越细，误报越高
  dyn_threshold(region_tai, BlackImageMean, &BlackPointDynThresh,m_Parameters.BlackPointDynThresh, "dark");
  select_shape(BlackPointDynThresh, &BLACK_POINT, "area", "and", m_Parameters.BlackPointSize, 99999);
  count_obj(BLACK_POINT, &Number);
  if (Number)
  {
	  //pData->m_ErrorRegist = BLACK_POINT;
	  pData->m_isFail =true;
	  testItem->m_bFailSubTest = true;
	  //testItem->m_ErrorMsg = "BlackPoint error";
	  //return;
	  if (pData->m_isDebug)
	  {
		  dilation_circle(BLACK_POINT,&Dilation,2.5);
		  set_color(pData->m_ShowHWindow,"red");
		  disp_obj(Dilation,pData->m_ShowHWindow);
	  }
  }
  //*找单一块极黑缺陷，适用于伤的比较深，污点比较黑的情况
  dyn_threshold(region_tai, BlackImageMean, &SeriousBlackPointDynThresh,m_Parameters.SeriousBlackPointDynThresh, "dark");
  connection(SeriousBlackPointDynThresh, &SeriousBlackConnected);
  select_shape(SeriousBlackConnected, &SERIOUS_BlackPoint, "area", "and",m_Parameters.SeriousBlackPointSize, 99999);
  count_obj(SERIOUS_BlackPoint, &Number);
  if (Number)
  {
	  pData->m_isFail =true;
	  testItem->m_bFailSubTest = true;
	  if (pData->m_isDebug)
	  {
		  dilation_circle(SERIOUS_BlackPoint,&Dilation,2.5);
		  set_color(pData->m_ShowHWindow,"red");
		  disp_obj(Dilation,pData->m_ShowHWindow);
	  }
  }
  //*===================================================================
  //*找白色划痕与污点
  mean_image(region_tai, &WhiteImageMean, m_Parameters.WhiteMaskSize, m_Parameters.WhiteMaskSize);

  //*找大范围白缺陷
  dyn_threshold(region_tai, WhiteImageMean, &WhitePointDynThresh, m_Parameters.WhitePointDynThresh, "light");
  select_shape(WhitePointDynThresh, &WHITE_POINT, "area", "and", m_Parameters.WhitePointSize, 99999);
  count_obj(WHITE_POINT, &Number);
  if (Number)
  {
	  //pData->m_ErrorRegist = WHITE_POINT;
	  pData->m_isFail =true;
	  testItem->m_bFailSubTest = true;
	  //testItem->m_ErrorMsg = "WhitePoint error";
	  if (pData->m_isDebug)
	  {
		  dilation_circle(WHITE_POINT,&Dilation,2.5);
		  set_color(pData->m_ShowHWindow,"yellow");
		  disp_obj(Dilation,pData->m_ShowHWindow);
	  }
  }
  //*找单一块极白污点

  dyn_threshold(region_tai, WhiteImageMean, &SeriousWhitePointDynThresh, m_Parameters.SeriousWhitePointDynThresh, "light");
  connection(SeriousWhitePointDynThresh, &SeriousWhiteConnected);
  select_shape(SeriousWhiteConnected, &SERIOUS_WhitePoint, "area", "and",m_Parameters.SeriousWhitePointSize, 99999);
  count_obj(SERIOUS_WhitePoint, &Number);
  if (Number)
  {
	  //pData->m_ErrorRegist = SERIOUS_WhitePoint;
	  pData->m_isFail =true;
	  testItem->m_bFailSubTest = true;
	  //testItem->m_ErrorMsg = "SeriousWhitePoint error";
	  if (pData->m_isDebug)
	  {
		  dilation_circle(SERIOUS_WhitePoint,&Dilation,2.5);
		  set_color(pData->m_ShowHWindow,"yellow");
		  disp_obj(Dilation,pData->m_ShowHWindow);
	  }
  }

  Hobject RegionUnionBlack,RegionUnionWhite,RegionUnionWhole;

  union2(BLACK_POINT, SERIOUS_BlackPoint, &RegionUnionBlack);
  union2(WHITE_POINT, SERIOUS_WhitePoint, &RegionUnionWhite);
  union2(RegionUnionBlack, RegionUnionWhite, &RegionUnionWhole);
  //dilation_circle(RegionUnionWhole, &RegionDilation, 2.5);
  union2(RegionUnionWhole, pData->m_ErrorRegist, &RegionUnionWhole);
  pData->m_ErrorRegist=RegionUnionWhole;

}
	catch (HException &except) 
	
	{
		if (pData->m_isDebug)
		{
			set_color(pData->m_ShowHWindow,"green");
			set_tposition( pData->m_ShowHWindow,25, 145);
			#ifdef _ENGLISH
						write_string(pData->m_ShowHWindow,"DynThreshed1.3 parameter values error,please re-adjust");
			#else
						write_string(pData->m_ShowHWindow,"DynThreshed1.3程序参数值出错,请重新调节参数");
			#endif

		}
		pData->m_isFail = true;
		testItem->m_bFailSubTest = true;
	}

}