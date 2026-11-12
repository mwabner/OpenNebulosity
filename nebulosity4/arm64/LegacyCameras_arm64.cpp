/*

BSD 3-Clause License

Copyright (c) 2021, Craig Stark
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
// ARM64 preview: legacy SDKs in this repository contain Intel code only.
// Keep class metadata and image reconstruction; fail every hardware operation.
#include "../precomp.h"
#include "../Nebulosity.h"
#include "../camera.h"
#include "../debayer.h"
#include "../image_math.h"
#include "../file_tools.h"
#if defined(__APPLE__) && (defined(__arm64__) || defined(__aarch64__))

Cam_ZWOASIClass::Cam_ZWOASIClass() {
	ConnectedModel = CAMERA_ZWOASI;
	Name="ZWOASI";
	ColorMode = COLOR_BW;
	ArrayType = COLOR_BW;
	AmpOff = true;
	DoubleRead = false;
	HighSpeed = false;
	BinMode = BIN1x1;
	FullBitDepth = true;
	Cap_Colormode = COLOR_BW;
	Cap_DoubleRead = false;
	Cap_HighSpeed = false;
	Cap_BinMode = BIN1x1 | BIN2x2;
	Cap_AmpOff = false;
	Cap_LowBit = false;
    Cap_GainCtrl = true;
    Cap_OffsetCtrl = true;
	Cap_ExtraOption = false;
	ExtraOption=false;
	ExtraOptionName = "";
	Cap_FineFocus = true;
	Cap_BalanceLines = false;
	Cap_AutoOffset = false;
	Cap_TECControl = true;
	TECState = true;
	DebayerXOffset = 1;
	DebayerYOffset = 1;
	Has_ColorMix = false;
	Has_PixelBalance = false;
    USB3 = true;
    MinExposure = 1;
	CamId = -1;
}

void Cam_ZWOASIClass::Disconnect() {

}

bool Cam_ZWOASIClass::Connect() {

    wxMessageBox("This camera SDK is not available in the Apple Silicon preview.", "Camera unavailable", wxOK | wxICON_INFORMATION);
    return true;

}

int Cam_ZWOASIClass::Capture () {
 return 1;
}

void Cam_ZWOASIClass::CaptureFineFocus(int click_x, int click_y) {

}

bool Cam_ZWOASIClass::Reconstruct(int mode) {
	bool retval = false;
	if (!CurrImage.IsOK())
		return true;
	if (CurrImage.ArrayType == COLOR_BW) mode = BW_SQUARE;
	if (Has_PixelBalance && (mode != BW_SQUARE))
		BalancePixels(CurrImage,ConnectedModel);
	if (mode != BW_SQUARE) retval = DebayerImage(ArrayType, DebayerXOffset, DebayerYOffset);
	else retval = false;
	if (!retval) {
		if (Has_ColorMix && (mode != BW_SQUARE)) ColorRebalance(CurrImage,ConnectedModel);
		SquarePixels(CurrImage.Header.XPixelSize,CurrImage.Header.YPixelSize);
	}
	return retval;
}

void Cam_ZWOASIClass::SetTEC(bool state, int temp) {

}

float Cam_ZWOASIClass::GetTECTemp() {
 return -273.1f;
}

float Cam_ZWOASIClass::GetTECPower() {
 return 0.0f;
}

void Cam_ZWOASIClass::UpdateGainCtrl(wxSpinCtrl *GainCtrl, wxStaticText *GainText) {

}

Cam_ArtemisClass::Cam_ArtemisClass() {
	ConnectedModel = CAMERA_ARTEMIS285;
	Name="Atik 16HR";
	Size[0]=1392;
	Size[1]=1040;
	Npixels = Size[0] * Size[1];
	PixelSize[0]=6.45;
	PixelSize[1]=6.45;
	ColorMode = COLOR_BW;
	ArrayType = COLOR_BW;
	AmpOff = true;
	DoubleRead = false;
	HighSpeed = false;
	BinMode = BIN1x1;
	FullBitDepth = true;
	Cap_Colormode = COLOR_BW;
	Cap_DoubleRead = false;
	Cap_HighSpeed = false;
	Cap_BinMode = BIN1x1 | BIN2x2;
	Cap_AmpOff = true;
	Cap_LowBit = false;
	Cap_BalanceLines = false;
	Cap_ExtraOption = false;
	ExtraOption=false;
	ExtraOptionName = "";
	Cap_FineFocus = true;
	Cap_AutoOffset = false;
	Has_ColorMix = false;
	Has_PixelBalance = false;
	HSModel = false;
	Cap_Shutter = false;
	CFWPositions=0;
	CFWPosition=0;
	Cam_Handle = NULL;
}

bool Cam_ArtemisClass::Connect() {

    wxMessageBox("This camera SDK is not available in the Apple Silicon preview.", "Camera unavailable", wxOK | wxICON_INFORMATION);
    return true;

}

void Cam_ArtemisClass::Disconnect() {

}

int Cam_ArtemisClass::Capture () {
 return 1;
}

void Cam_ArtemisClass::CaptureFineFocus(int click_x, int click_y) {

}

bool Cam_ArtemisClass::Reconstruct(int mode) {
	bool retval=false;
	if (!CurrImage.IsOK())
		return true;
	if (Has_PixelBalance && (mode != BW_SQUARE))
		BalancePixels(CurrImage,ConnectedModel);
	int xoff = DebayerXOffset;
	int yoff = DebayerYOffset;
	if (CurrImage.Header.CameraName.IsSameAs(_T("Art285/ATK16HR")))
		xoff = yoff = 0;
	else if (CurrImage.Header.CameraName.Find(_T("Art285/ATK16HR:")) != wxNOT_FOUND) {
		xoff = 0;
		yoff = 1;
	}
	else if (CurrImage.Header.CameraName.Find(_T("314")) != wxNOT_FOUND) {
		xoff = 0;
		yoff = 1;
	}
	else if (CurrImage.Header.CameraName.Find(_T("428")) != wxNOT_FOUND) {
		xoff = 1;
		yoff = 1;
	}
	else if (CurrImage.Header.CameraName.Find(_T("460")) != wxNOT_FOUND) {
		xoff = 0;
		yoff = 1;
	}
	if (mode != BW_SQUARE) {
		if ((ConnectedModel == CAMERA_ARTEMIS429) || (ConnectedModel == CAMERA_ARTEMIS429C))
			retval = VNG_Interpolate(COLOR_CMYG,xoff,yoff,1);
		else
			retval = DebayerImage(COLOR_RGB,xoff,yoff);
	}
	else retval = false;
	if (!retval) {
		if (Has_ColorMix && (mode != BW_SQUARE)) ColorRebalance(CurrImage,ConnectedModel);
		SquarePixels(PixelSize[0],PixelSize[1]);
	}
	return retval;
}

float Cam_ArtemisClass::GetTECTemp() {
 return -273.1f;
}

float Cam_ArtemisClass::GetTECPower() {
 return 0.0f;
}

void Cam_ArtemisClass::SetTEC(bool state, int temp) {

}

void Cam_ArtemisClass::SetFilter(int position) {

}

int Cam_ArtemisClass::GetCFWState() {
 return 2;
}

Cam_CanonDSLRClass::Cam_CanonDSLRClass() {
	ConnectedModel = CAMERA_CANONDSLR;
	Name="Canon DSLR";
	ColorMode = COLOR_BW;
	ArrayType = COLOR_RGB;
	BinMode = BIN1x1;
	Cap_BinMode = BIN1x1 | BIN2x2;
	Cap_FineFocus = true;
	Cap_GainCtrl = true;
	LiveViewUse = LV_NONE;
	LiveViewRunning = false;
	RawSize = wxSize(0,0);
	Has_ColorMix = false;
	RR = 1.2;
	GG = 0.8;
	BB = 1.0;
	Has_PixelBalance = true;
	WBSet=0;
	WB00[0] = 1.0;
	WB01[0] = 0.589;
	WB02[0] = 1.0;
	WB03[0] = 0.589;
	WB10[0] = 0.589;
	WB11[0] = 0.809;
	WB12[0] = 0.589;
	WB13[0] = 0.809;
	WB00[1] = 1.0;
	WB01[1] = 0.648;
	WB02[1] = 1.0;
	WB03[1] = 0.648;
	WB10[1] = 0.648;
	WB11[1] = 0.868;
	WB12[1] = 0.648;
	WB13[1] = 0.868;
	WB00[2] = 0.811;
	WB01[2] = 0.711;
	WB02[2] = 0.811;
	WB03[2] = 0.711;
	WB10[2] = 0.711;
	WB11[2] = 1.0;
	WB12[2] = 0.711;
	WB13[2] = 1.0;
	WB00[3] = 1.0;
	WB01[3] = 0.606;
	WB02[3] = 1.0;
	WB03[3] = 0.606;
	WB10[3] = 0.606;
	WB11[3] = 0.719;
	WB12[3] = 0.606;
	WB13[3] = 0.719;
	WB00[4] = 1.31;
	WB01[4] = 0.89;
	WB02[4] = 1.31;
	WB03[4] = 0.89;
	WB10[4] = 0.89;
	WB11[4] = 1.10;
	WB12[4] = 0.89;
	WB13[4] = 1.10;
	WB00[5] = 1.15;
	WB01[5] = 0.82;
	WB02[5] = 1.15;
	WB03[5] = 0.82;
	WB10[5] = 0.82;
	WB11[5] = 1.0;
	WB12[5] = 0.82;
	WB13[5] = 1.0;
	DebayerXOffset=1;
	DebayerYOffset=1;
	CameraRef=NULL;
	SDKLoaded=false;
	IsDownloading=false;
	IsExposing=false;
	DownloadReady=false;
	AbortDownload = false;
	LEConnected = false;
	Progress=0;
	LEAdapter = DIGIC3_BULB;
	PortAddress = 0;
	MirrorLockupCFn = (EdsUInt32) 12;
	RAWMode = 0;
	BulbDialMode = false;
	MaxGain = 5;
	UsingInternalNR = false;
	SaveLocation = 2;
	MirrorDelay = 1500;
	ShutterBulbMode = false;
	ScaleTo16bit = true;
    LVFullSize.height = 0;
    LVFullSize.width = 0;
    LVStream = NULL;
    LVImage = NULL;
}

bool Cam_CanonDSLRClass::Reconstruct(int mode) {
	bool retval=false;
	if (!CurrImage.IsOK())
		return true;
	if (CurrImage.ColorMode) return false;
	if (WBSet < 0) {
		Has_PixelBalance = false;
		Has_ColorMix = true;
	}
	else {
		Has_PixelBalance = true;
		Has_ColorMix = false;
	}
	if (Has_PixelBalance && (mode != BW_SQUARE)) {
		Pix00=WB00[WBSet];
		Pix01=WB01[WBSet];
		Pix02=WB02[WBSet];
		Pix03=WB03[WBSet];
		Pix10=WB10[WBSet];
		Pix11=WB11[WBSet];
		Pix12=WB12[WBSet];
		Pix13=WB13[WBSet];
		BalancePixels(CurrImage,ConnectedModel);
	}
	if (mode != BW_SQUARE) retval = DebayerImage(ArrayType, DebayerXOffset, DebayerYOffset);
	else retval = false;
	if (!retval) {
		if (Has_ColorMix && (mode != BW_SQUARE)) {
			ColorRebalance(CurrImage,ConnectedModel);
		}
		SquarePixels(PixelSize[0],PixelSize[1]);
	}
	return retval;
}

bool Cam_CanonDSLRClass::Connect() {

    wxMessageBox("This camera SDK is not available in the Apple Silicon preview.", "Camera unavailable", wxOK | wxICON_INFORMATION);
    return true;

}

void Cam_CanonDSLRClass::Disconnect() {

}

int Cam_CanonDSLRClass::Capture() {
 return 1;
}

void Cam_CanonDSLRClass::CaptureFineFocus(int click_x, int click_y) {

}

void Cam_CanonDSLRClass::UpdateGainCtrl (wxSpinCtrl *GainCtrl, wxStaticText *GainText) {

}

bool Cam_CanonDSLRClass::StartLiveView(EdsUInt32 ZoomMode) {
 return true;
}

bool Cam_CanonDSLRClass::StopLiveView() {
 return true;
}

bool Cam_CanonDSLRClass::GetLiveViewImage() {
 return true;
}

bool Cam_CanonDSLRClass::OpenLEAdapter() {
 return true;
}

void Cam_CanonDSLRClass::CloseLEAdapter() {

}

void Cam_CanonDSLRClass::OpenLEShutter() {

}

void Cam_CanonDSLRClass::CloseLEShutter() {

}

Cam_MeadeDSIClass::Cam_MeadeDSIClass() {
	ConnectedModel = CAMERA_MEADEDSI;
	Name="Meade DSI";
	Size[0]=752;
	Size[1]=582;
	PixelSize[0]=6.5;
	PixelSize[1]=6.25;
	Npixels = Size[0] * Size[1];
	ColorMode = COLOR_RGB;
	ArrayType = COLOR_CMYG;
	AmpOff = true;
	DoubleRead = false;
	HighSpeed = false;
	BinMode = BIN1x1;
	FullBitDepth = true;
	Cap_Colormode = COLOR_RGB;
	Cap_DoubleRead = false;
	Cap_HighSpeed = true;
	Cap_BinMode = BIN1x1;
	Cap_AmpOff = true;
	Cap_LowBit = false;
	Cap_ExtraOption = false;
	Cap_FineFocus = true;
	Cap_BalanceLines = false;
	Cap_ExtraOption = true;
	ExtraOption=true;
	ExtraOptionName = "Gain boost";
	Cap_GainCtrl = Cap_OffsetCtrl = true;
	Cap_AutoOffset = false;
	DebayerXOffset = 1;
	DebayerYOffset = 1;
	RR = 0.92;
	RG = 0.53;
	RB = -0.31;
	GR = -0.31;
	GG = 0.92;
	GB = 0.53;
	BR = 0.53;
	BG = -0.31;
	BB = 0.92;
	Pix00 = 0.74;
	Pix01 = 0.877;
	Pix02 = 0.74;
	Pix03 = 1.55;
	Pix10 = 1.152;
	Pix11 = 1.55;
	Pix12 = 1.152;
	Pix13 = 0.877;
	Has_ColorMix = true;
	Has_PixelBalance = true;
}

Cam_MeadeDSIClass::~Cam_MeadeDSIClass() {

}

void Cam_MeadeDSIClass::Disconnect() {

}

bool Cam_MeadeDSIClass::Connect() {

    wxMessageBox("This camera SDK is not available in the Apple Silicon preview.", "Camera unavailable", wxOK | wxICON_INFORMATION);
    return true;

}

int Cam_MeadeDSIClass::Capture () {
 return 1;
}

void Cam_MeadeDSIClass::CaptureFineFocus(int click_x, int click_y) {

}

bool Cam_MeadeDSIClass::Reconstruct(int mode) {
	bool retval = false;
	if (!CurrImage.IsOK())
		return true;
	if (CurrImage.ArrayType == COLOR_BW) mode = BW_SQUARE;
	if (CurrImage.Header.CameraName.Find(_T("Meade DSI III")) != wxNOT_FOUND) {
		DebayerXOffset = 1;
		DebayerYOffset = 1;
		Has_PixelBalance = false;
		Has_ColorMix = false;
		ArrayType = CurrImage.ArrayType = COLOR_RGB;
	}
	else if (CurrImage.Header.CameraName.Find(_T("Meade DSI II")) != wxNOT_FOUND) {
		DebayerXOffset = 1;
		DebayerYOffset = 3;
		Has_PixelBalance = true;
		Has_ColorMix = true;
		ArrayType = CurrImage.ArrayType = COLOR_CMYG;
		Pix02 = 0.823;
		Pix03 = 0.871;
		Pix00 = 0.823;
		Pix01 = 1.348;
		Pix12 = 1.117;
		Pix13 = 1.348;
		Pix10 = 1.117;
		Pix11 = 0.871;
		RR = 0.92;
		RG = 0.53;
		RB = -0.31;
		GR = -0.31;
		GG = 0.92;
		GB = 0.53;
		BR = 0.53;
		BG = -0.31;
		BB = 0.92;
	}
	else {
		DebayerXOffset = 1;
		DebayerYOffset = 1;
		ArrayType = CurrImage.ArrayType = COLOR_CMYG;
		Pix00 = 0.845;
		Pix01 = 1.0;
		Pix02 = 0.845;
		Pix03 = 1.15;
		Pix10 = 1.05;
		Pix11 = 1.15;
		Pix12 = 1.05;
		Pix13 = 1.0;
		RR = 1.04;
		RG = 0.17;
		RB = -0.19;
		GR = -0.31;
		GG = 0.87;
		GB = 0.34;
		BR = 0.0;
		BG = -0.21;
		BB = 1.04;
		Has_PixelBalance = true;
		Has_ColorMix = true;
	}
	if (Has_PixelBalance && (mode != BW_SQUARE))
		BalancePixels(CurrImage,ConnectedModel);
	if (mode != BW_SQUARE) {
		if (CurrImage.ArrayType == COLOR_CMYG) retval = VNG_Interpolate(CurrImage.ArrayType,DebayerXOffset,DebayerYOffset,1);
		else retval = DebayerImage(ArrayType, DebayerXOffset, DebayerYOffset);
	}
	else retval = false;
	if (!retval) {
		if (Has_ColorMix && (mode != BW_SQUARE)) ColorRebalance(CurrImage,ConnectedModel);
		SquarePixels(CurrImage.Header.XPixelSize,CurrImage.Header.YPixelSize);
	}
	return retval;
}

void Cam_MeadeDSIClass::UpdateGainCtrl(wxSpinCtrl *GainCtrl, wxStaticText *GainText) {

}

float Cam_MeadeDSIClass::GetTECTemp() {
 return -273.1f;
}

Cam_SXVClass::Cam_SXVClass() {
	ConnectedModel = CAMERA_SXV;
	Name="Starlight Xpress";
	PixelSize[0]=0.0;
	PixelSize[1]=0.0;
	ColorMode = COLOR_BW;
	ArrayType = COLOR_BW;
	AmpOff = false;
	DoubleRead = false;
	HighSpeed = false;
	BinMode = BIN1x1;
	FullBitDepth = true;
	Cap_Colormode = COLOR_BW;
	Cap_DoubleRead = false;
	Cap_HighSpeed = true;
	Cap_BinMode = BIN1x1 | BIN2x2;
	Cap_AmpOff = false;
	Cap_LowBit = false;
	Cap_ExtraOption = false;
	Cap_FineFocus = true;
	Cap_BalanceLines = false;
	Cap_AutoOffset = false;
	Has_ColorMix = false;
	Has_PixelBalance = false;
	Interlaced = false;
	RawData = NULL;
	ReadTime = 0;
	ScaleConstant = 1.0;
	Regulated = false;
	HasShutter = false;
	CurrentTECState = 0;
	CurrentTECSetPoint = 0;
	ReverseInterleave=false;
}

void Cam_SXVClass::Disconnect() {

}

bool Cam_SXVClass::Connect() {

    wxMessageBox("This camera SDK is not available in the Apple Silicon preview.", "Camera unavailable", wxOK | wxICON_INFORMATION);
    return true;

}

void Cam_SXVClass::SetupColorParams(int ModelNum) {

}

int Cam_SXVClass::Capture () {
 return 1;
}

void Cam_SXVClass::CaptureFineFocus(int click_x, int click_y) {

}

bool Cam_SXVClass::Reconstruct(int mode) {
	bool retval = false;
	if (!CurrImage.IsOK())
		return true;
	wxString FullModel = CurrImage.Header.CameraName.AfterFirst(wxChar('-'));
	bool color = FullModel.EndsWith(_T("C"));
	if (color) FullModel = FullModel.Mid(0,FullModel.Length() - 1);
	FullModel = FullModel.Mid(1);
	if (FullModel.StartsWith(_T("X"))) FullModel = FullModel.Mid(1);
	long ModelNum;
	FullModel.ToLong(&ModelNum);
	if (((ModelNum == 8) || (ModelNum == 25)) && (!color)) {
		color = true;
		mode = HQ_DEBAYER;
		CurrImage.ArrayType = COLOR_RGB;
	}
	if (ModelNum == 26) {
		color = true;
		CurrImage.ArrayType = COLOR_RGB;
	}
	if (CurrImage.Header.XPixelSize == 0.0) {
		switch (ModelNum) {
			case 5:
				CurrImage.Header.XPixelSize = 9.8;
				CurrImage.Header.YPixelSize = 6.3;
				break;
			case 7:
				CurrImage.Header.XPixelSize = 8.6;
				CurrImage.Header.YPixelSize = 8.3;
				break;
			case 8:
				CurrImage.Header.XPixelSize = 3.1;
				CurrImage.Header.YPixelSize = 3.1;
				break;
			case 9:
            case 825:
				CurrImage.Header.XPixelSize = 6.4;
				CurrImage.Header.YPixelSize = 6.4;
				break;
			case 25:
				CurrImage.Header.XPixelSize = 7.8;
				CurrImage.Header.YPixelSize = 7.8;
				break;
			case 16:
				CurrImage.Header.XPixelSize = 7.4;
				CurrImage.Header.YPixelSize = 7.4;
				break;
			default:
				CurrImage.Header.XPixelSize = 1.0;
				CurrImage.Header.YPixelSize = 1.0;
				break;
		}
	}
	if (1)
		SetupColorParams(ModelNum);
	if ((CurrImage.ArrayType == COLOR_BW) || (!color)) mode = BW_SQUARE;
	if (Has_PixelBalance && (mode != BW_SQUARE))
		BalancePixels(CurrImage,ConnectedModel);
	if (mode != BW_SQUARE) retval = DebayerImage(ArrayType, DebayerXOffset, DebayerYOffset);
	else retval = false;
	if (!retval) {
		if (Has_ColorMix && (mode != BW_SQUARE)) ColorRebalance(CurrImage,ConnectedModel);
		SquarePixels(CurrImage.Header.XPixelSize,CurrImage.Header.YPixelSize);
	}
	return retval;
}

void Cam_SXVClass::UpdateGainCtrl(wxSpinCtrl *GainCtrl, wxStaticText *GainText) {

}

void Cam_SXVClass::SetTEC(bool state, int temp) {

}

float Cam_SXVClass::GetTECTemp() {
 return -273.1f;
}

void Cam_SXVClass::ReconM26C() {

}

void Cam_SXVClass::ReconM25C() {

}

void Cam_SXVClass::ReconCMOSGuider() {

}

wxString Cam_SXVClass::GetNameFromModel(unsigned short model) {
 return wxString();
}

Cam_SBIGClass::Cam_SBIGClass() {
	ConnectedModel = CAMERA_SBIG;
	Name="SBIG";
	PixelSize[0]=0.0;
	PixelSize[1]=0.0;
	ColorMode = COLOR_BW;
	ArrayType = COLOR_BW;
	AmpOff = false;
	DoubleRead = false;
	HighSpeed = false;
	BinMode = BIN1x1;
	FullBitDepth = true;
	Cap_Colormode = COLOR_BW;
	Cap_DoubleRead = false;
	Cap_HighSpeed = false;
	Cap_BinMode = BIN1x1 | BIN2x2 | BIN3x3;
	Cap_AmpOff = false;
	Cap_LowBit = false;
	Cap_ExtraOption = false;
	Cap_TECControl = true;
	TECState = true;
	Cap_FineFocus = true;
	Cap_BalanceLines = false;
	Cap_AutoOffset = false;
	Has_ColorMix = false;
	Has_PixelBalance = false;
	DebayerXOffset = 0;
	DebayerYOffset = 0;
	LineBuffer = NULL;
	GuideChipData = NULL;
	HasGuideChip = false;
	GuideChipConnected = false;
	GuideChipActive = false;
	TECState = true;
	ShutterState = false;
}

bool Cam_SBIGClass::LoadDriver() {
 return true;
}

void Cam_SBIGClass::Disconnect() {

}

bool Cam_SBIGClass::Connect() {

    wxMessageBox("This camera SDK is not available in the Apple Silicon preview.", "Camera unavailable", wxOK | wxICON_INFORMATION);
    return true;

}

int Cam_SBIGClass::Capture () {
 return 1;
}

void Cam_SBIGClass::CaptureFineFocus(int click_x, int click_y) {

}

bool Cam_SBIGClass::Reconstruct(int mode) {
	bool retval = false;
	if (!CurrImage.IsOK())
		return true;
	if ((CurrImage.Header.Creator.Find("MaxIm")  == wxNOT_FOUND ) &&
        (CurrImage.Header.Creator.Find("TheSkyX")) )
		if (CurrImage.ArrayType == COLOR_BW)
            mode = BW_SQUARE;
	if (Has_PixelBalance && (mode != BW_SQUARE))
		BalancePixels(CurrImage,ConnectedModel);
	if (mode != BW_SQUARE) {
		if (CurrImage.Header.Creator.Find("MaxIm")  != wxNOT_FOUND )
			retval = DebayerImage(CurrImage.ArrayType,0,0);
		else
			retval = DebayerImage(CurrImage.ArrayType, DebayerXOffset, DebayerYOffset);
	}
	else retval = false;
	if (!retval) {
		if (Has_ColorMix && (mode != BW_SQUARE)) ColorRebalance(CurrImage,ConnectedModel);
		SquarePixels(CurrImage.Header.XPixelSize,CurrImage.Header.YPixelSize);
	}
	return retval;
}

void Cam_SBIGClass::GetTemp(float& TECTemp, float& AmbientTemp, float& Power) {
 TECTemp = AmbientTemp = -273.1f; Power = 0.0f;
}

void Cam_SBIGClass::SetTEC(bool state, int temp) {

}

void Cam_SBIGClass::UpdateGainCtrl(wxSpinCtrl *GainCtrl, wxStaticText *GainText) {

}

float Cam_SBIGClass::GetTECTemp() {
 return -273.1f;
}

float Cam_SBIGClass::GetAmbientTemp() {
 return -273.1f;
}

float Cam_SBIGClass::GetTECPower() {
 return 0.0f;
}

void Cam_SBIGClass::GetTempStatus(float& TECTemp, float& AmbientTemp, float& Power) {
 TECTemp = AmbientTemp = -273.1f; Power = 0.0f;
}

int Cam_SBIGClass::GetCFWState() {
 return 2;
}

void Cam_SBIGClass::SetFilter (int position) {

}

bool Cam_SBIGClass::ConnectGuideChip(int& xsize, int& ysize) {
 return true;
}

void Cam_SBIGClass::DisconnectGuideChip() {

}

bool Cam_SBIGClass::GetGuideFrame (int duration, int& xsize, int& ysize) {
 return true;
}

Cam_Q8HRClass::Cam_Q8HRClass() {
	ConnectedModel = CAMERA_Q8HR;
	Name="CCD Labs Q8";
	Size[0]=3110;
	Size[1]=2030;
	PixelSize[0]=7.8;
	PixelSize[1]=7.8;
	Npixels = Size[0] * Size[1];
	ColorMode = COLOR_RGB;
	ArrayType = COLOR_RGB;
	AmpOff = true;
	DoubleRead = false;
	HighSpeed = false;
	BinMode = BIN1x1;
	FullBitDepth = true;
	Cap_Colormode = COLOR_RGB;
	Cap_DoubleRead = false;
	Cap_HighSpeed = true;
	Cap_BinMode = BIN1x1 | BIN2x2 | BIN4x4;
	Cap_AmpOff = true;
	Cap_LowBit = false;
	Cap_ExtraOption = false;
	Cap_FineFocus = true;
#if defined (__APPLE__)
#endif
	Cap_BalanceLines = false;
	Cap_AutoOffset = true;
	Cap_GainCtrl = Cap_OffsetCtrl = true;
	Has_ColorMix = false;
	Has_PixelBalance = false;
	DebayerXOffset = DebayerYOffset = 1;
}

void Cam_Q8HRClass::Disconnect() {

}

bool Cam_Q8HRClass::Connect() {

    wxMessageBox("This camera SDK is not available in the Apple Silicon preview.", "Camera unavailable", wxOK | wxICON_INFORMATION);
    return true;

}

int Cam_Q8HRClass::Capture () {
 return 1;
}

void Cam_Q8HRClass::CaptureFineFocus(int click_x, int click_y) {

}

bool Cam_Q8HRClass::Reconstruct(int mode) {
	bool retval = false;
	if (!CurrImage.IsOK())
		return true;
	if (CurrImage.ArrayType == COLOR_BW) mode = BW_SQUARE;
	if (Has_PixelBalance && (mode != BW_SQUARE))
		BalancePixels(CurrImage,ConnectedModel);
	if (mode != BW_SQUARE) retval = DebayerImage(ArrayType, DebayerXOffset, DebayerYOffset);
	else retval = false;
	if (!retval) {
		if (Has_ColorMix && (mode != BW_SQUARE)) ColorRebalance(CurrImage,ConnectedModel);
		SquarePixels(CurrImage.Header.XPixelSize,CurrImage.Header.YPixelSize);
	}
	return retval;
}

void Cam_Q8HRClass::UpdateGainCtrl(wxSpinCtrl *GainCtrl, wxStaticText *GainText) {

}

Cam_QHY2PROClass::Cam_QHY2PROClass() {
	ConnectedModel = CAMERA_QHY2PRO;
	Name="Q285M / QHY2Pro";
	Size[0]=1360;
	Size[1]=1024;
	PixelSize[0]=6.54;
	PixelSize[1]=6.54;
	Npixels = Size[0] * Size[1];
	ColorMode = COLOR_BW;
	ArrayType = COLOR_BW;
	AmpOff = true;
	DoubleRead = false;
	HighSpeed = false;
	BinMode = BIN1x1;
	FullBitDepth = true;
	Cap_Colormode = COLOR_BW;
	Cap_DoubleRead = false;
	Cap_HighSpeed = true;
	Cap_BinMode = BIN1x1 | BIN2x2;
	Cap_AmpOff = true;
	Cap_LowBit = false;
	Cap_GainCtrl = Cap_OffsetCtrl = true;
	Cap_ExtraOption = true;
	ExtraOption=true;
	ExtraOptionName = "Freeze TEC on d/l";
	Cap_FineFocus = true;
	Cap_BalanceLines = false;
	Cap_AutoOffset = false;
	Cap_TECControl = true;
	TECState = true;
	Has_ColorMix = false;
	Has_PixelBalance = false;
}

void Cam_QHY2PROClass::Disconnect() {

}

bool Cam_QHY2PROClass::Connect() {

    wxMessageBox("This camera SDK is not available in the Apple Silicon preview.", "Camera unavailable", wxOK | wxICON_INFORMATION);
    return true;

}

int Cam_QHY2PROClass::Capture () {
 return 1;
}

void Cam_QHY2PROClass::CaptureFineFocus(int click_x, int click_y) {

}

bool Cam_QHY2PROClass::Reconstruct(int mode) {
	bool retval = false;
	if (!CurrImage.IsOK())
		return true;
	if (CurrImage.ArrayType == COLOR_BW) mode = BW_SQUARE;
	if (Has_PixelBalance && (mode != BW_SQUARE))
		BalancePixels(CurrImage,ConnectedModel);
	if (mode != BW_SQUARE) retval = DebayerImage(ArrayType, DebayerXOffset, DebayerYOffset);
	else retval = false;
	if (!retval) {
		if (Has_ColorMix && (mode != BW_SQUARE)) ColorRebalance(CurrImage,ConnectedModel);
		SquarePixels(CurrImage.Header.XPixelSize,CurrImage.Header.YPixelSize);
	}
	return retval;
}

void Cam_QHY2PROClass::UpdateGainCtrl(wxSpinCtrl *GainCtrl, wxStaticText *GainText) {

}

void Cam_QHY2PROClass::SetTEC(bool state, int temp) {

}

float Cam_QHY2PROClass::GetTECTemp() {
 return -273.1f;
}

Cam_QHY268Class::Cam_QHY268Class() {
	ConnectedModel = CAMERA_QHY268;
	Name="QHY8";
	Model = 8;
	Size[0]=3110;
	Size[1]=2030;
	PixelSize[0]=7.8;
	PixelSize[1]=7.8;
	Npixels = Size[0] * Size[1];
	ColorMode = COLOR_RGB;
	ArrayType = COLOR_RGB;
	AmpOff = true;
	DoubleRead = false;
	HighSpeed = false;
	BinMode = BIN1x1;
	FullBitDepth = true;
	Cap_Colormode = COLOR_RGB;
	Cap_DoubleRead = false;
	Cap_HighSpeed = true;
	Cap_BinMode = BIN1x1 | BIN2x2;
	Cap_AmpOff = true;
	Cap_LowBit = false;
	Cap_ExtraOption = false;
	Cap_FineFocus = true;
	Cap_BalanceLines = false;
	Cap_AutoOffset = false;
	Cap_GainCtrl = Cap_OffsetCtrl = true;
	Has_ColorMix = false;
	Has_PixelBalance = false;
	DebayerXOffset = DebayerYOffset = 1;
}

void Cam_QHY268Class::Disconnect() {

}

bool Cam_QHY268Class::Connect() {

    wxMessageBox("This camera SDK is not available in the Apple Silicon preview.", "Camera unavailable", wxOK | wxICON_INFORMATION);
    return true;

}

int Cam_QHY268Class::Capture () {
 return 1;
}

void Cam_QHY268Class::CaptureFineFocus(int click_x, int click_y) {

}

bool Cam_QHY268Class::Reconstruct(int mode) {
	bool retval = false;
	if (!CurrImage.IsOK())
		return true;
	if (CurrImage.ArrayType == COLOR_BW) mode = BW_SQUARE;
	if (Has_PixelBalance && (mode != BW_SQUARE))
		BalancePixels(CurrImage,ConnectedModel);
	if (mode != BW_SQUARE) retval = DebayerImage(ArrayType, DebayerXOffset, DebayerYOffset);
	else retval = false;
	if (!retval) {
		if (Has_ColorMix && (mode != BW_SQUARE)) ColorRebalance(CurrImage,ConnectedModel);
		SquarePixels(CurrImage.Header.XPixelSize,CurrImage.Header.YPixelSize);
	}
	return retval;
}

void Cam_QHY268Class::UpdateGainCtrl(wxSpinCtrl *GainCtrl, wxStaticText *GainText) {

}

Cam_QHY9Class::Cam_QHY9Class() {
	ConnectedModel = CAMERA_QHY9;
	Name="QHY9";
	Size[0]=3584;
	Size[1]=2574;
	PixelSize[0]=5.4;
	PixelSize[1]=5.4;
	Npixels = Size[0] * Size[1];
	ColorMode = COLOR_BW;
	ArrayType = COLOR_BW;
	AmpOff = true;
	DoubleRead = false;
	HighSpeed = false;
	BinMode = BIN1x1;
	FullBitDepth = true;
	Cap_Colormode = COLOR_BW;
	Cap_DoubleRead = false;
	Cap_HighSpeed = true;
	Cap_BinMode = BIN1x1 | BIN2x2 | BIN3x3 | BIN4x4;
	Cap_AmpOff = true;
	Cap_LowBit = false;
	Cap_GainCtrl = Cap_OffsetCtrl = true;
	Cap_ExtraOption = true;
	ExtraOption=false;
	ExtraOptionName = "CLAMP mode";
	Cap_FineFocus = true;
	Cap_BalanceLines = false;
	Cap_AutoOffset = false;
	Cap_TECControl = true;
    Cap_Shutter = true;
	TECState = true;
	Has_ColorMix = false;
	Has_PixelBalance = false;
	ShutterState = false;
	TEC_Temp = -271.3;
	TEC_Power = 0.0;
	CFWPositions = 5;
	DebayerYOffset = 0;
	DebayerXOffset=1;
	Driver = 0;
}

void Cam_QHY9Class::Disconnect() {

}

bool Cam_QHY9Class::Connect() {

    wxMessageBox("This camera SDK is not available in the Apple Silicon preview.", "Camera unavailable", wxOK | wxICON_INFORMATION);
    return true;

}

void Cam_QHY9Class::ResetREG() {

}

void Cam_QHY9Class::SendREG(unsigned long P_Size,unsigned long &Total_P,unsigned int &PatchNumber) {

}

int Cam_QHY9Class::Capture () {
 return 1;
}

void Cam_QHY9Class::CaptureFineFocus(int click_x, int click_y) {

}

bool Cam_QHY9Class::Reconstruct(int mode) {
	bool retval = false;
	if (!CurrImage.IsOK())
		return true;
	if (Has_PixelBalance && (mode != BW_SQUARE))
		BalancePixels(CurrImage,ConnectedModel);
	if (mode != BW_SQUARE) retval = DebayerImage(ArrayType, DebayerXOffset, DebayerYOffset);
	else retval = false;
	if (!retval) {
		if (Has_ColorMix && (mode != BW_SQUARE)) ColorRebalance(CurrImage,ConnectedModel);
		SquarePixels(CurrImage.Header.XPixelSize,CurrImage.Header.YPixelSize);
	}
	return retval;
}

void Cam_QHY9Class::UpdateGainCtrl(wxSpinCtrl *GainCtrl, wxStaticText *GainText) {

}

void Cam_QHY9Class::SetTEC(bool state, int temp) {

}

float Cam_QHY9Class::GetTECTemp() {
 return -273.1f;
}

float Cam_QHY9Class::GetTECPower() {
 return 0.0f;
}

void Cam_QHY9Class::SetFilter(int position) {

}

Cam_QHY8ProClass::Cam_QHY8ProClass() {
	ConnectedModel = CAMERA_QHY8PRO;
	Name="QHY8Pro";
	Size[0]=3328;
	Size[1]=2030;
	PixelSize[0]=7.8;
	PixelSize[1]=7.8;
	Npixels = Size[0] * Size[1];
	ColorMode = COLOR_BW;
	ArrayType = COLOR_RGB;
	AmpOff = true;
	DoubleRead = false;
	HighSpeed = false;
	BinMode = BIN1x1;
	FullBitDepth = true;
	Cap_Colormode = COLOR_RGB;
	Cap_DoubleRead = false;
	Cap_HighSpeed = true;
	Cap_BinMode = BIN1x1 | BIN2x2 | BIN4x4;
	Cap_AmpOff = true;
	Cap_LowBit = false;
	Cap_GainCtrl = Cap_OffsetCtrl = true;
	Cap_ExtraOption = false;
	ExtraOption=false;
	ExtraOptionName = "";
	Cap_FineFocus = true;
	Cap_BalanceLines = false;
	Cap_AutoOffset = false;
	Cap_TECControl = true;
	TECState = true;
	Pro = true;
	DebayerXOffset = 1;
	DebayerYOffset = 1;
	Has_ColorMix = false;
	Has_PixelBalance = false;
	TEC_Temp = -271.3;
	TEC_Power = 0.0;
	Driver = 0;
}

void Cam_QHY8ProClass::Disconnect() {

}

bool Cam_QHY8ProClass::Connect() {

    wxMessageBox("This camera SDK is not available in the Apple Silicon preview.", "Camera unavailable", wxOK | wxICON_INFORMATION);
    return true;

}

void Cam_QHY8ProClass::ResetREG() {

}

void Cam_QHY8ProClass::SendREG(unsigned long P_Size,unsigned long &Total_P,unsigned int &PatchNumber) {

}

int Cam_QHY8ProClass::Capture () {
 return 1;
}

void Cam_QHY8ProClass::CaptureFineFocus(int click_x, int click_y) {

}

bool Cam_QHY8ProClass::Reconstruct(int mode) {
	bool retval = false;
	if (!CurrImage.IsOK())
		return true;
	if (CurrImage.ArrayType == COLOR_BW) mode = BW_SQUARE;
	if (Has_PixelBalance && (mode != BW_SQUARE))
		BalancePixels(CurrImage,ConnectedModel);
	if (mode != BW_SQUARE) retval = DebayerImage(ArrayType, DebayerXOffset, DebayerYOffset);
	else retval = false;
	if (!retval) {
		if (Has_ColorMix && (mode != BW_SQUARE)) ColorRebalance(CurrImage,ConnectedModel);
		SquarePixels(CurrImage.Header.XPixelSize,CurrImage.Header.YPixelSize);
	}
	return retval;
}

void Cam_QHY8ProClass::UpdateGainCtrl(wxSpinCtrl *GainCtrl, wxStaticText *GainText) {

}

void Cam_QHY8ProClass::SetTEC(bool state, int temp) {

}

float Cam_QHY8ProClass::GetTECTemp() {
 return -273.1f;
}

float Cam_QHY8ProClass::GetTECPower() {
 return 0.0f;
}

void Cam_QHY8ProClass::PeriodicUpdate() {

}

Cam_QHY10Class::Cam_QHY10Class() {
	ConnectedModel = CAMERA_QHY10;
	Name="QHY10";
	Size[0]=3328;
	Size[1]=2030;
	PixelSize[0]=6.05;
	PixelSize[1]=6.05;
	Npixels = Size[0] * Size[1];
	ColorMode = COLOR_BW;
	ArrayType = COLOR_RGB;
	AmpOff = true;
	DoubleRead = false;
	HighSpeed = false;
	BinMode = BIN1x1;
	FullBitDepth = true;
	Cap_Colormode = COLOR_RGB;
	Cap_DoubleRead = false;
	Cap_HighSpeed = true;
	Cap_BinMode = BIN1x1 | BIN2x2 | BIN4x4;
	Cap_AmpOff = true;
	Cap_LowBit = false;
	Cap_GainCtrl = Cap_OffsetCtrl = true;
	Cap_ExtraOption = false;
	ExtraOption=false;
	ExtraOptionName = "";
	Cap_FineFocus = true;
	Cap_BalanceLines = false;
	Cap_AutoOffset = false;
	Cap_TECControl = true;
	TECState = true;
	DebayerXOffset = 1;
	DebayerYOffset = 1;
	Has_ColorMix = false;
	Has_PixelBalance = false;
	TEC_Temp = -271.3;
	TEC_Power = 0.0;
}

void Cam_QHY10Class::Disconnect() {

}

bool Cam_QHY10Class::Connect() {

    wxMessageBox("This camera SDK is not available in the Apple Silicon preview.", "Camera unavailable", wxOK | wxICON_INFORMATION);
    return true;

}

void Cam_QHY10Class::ResetREG() {

}

void Cam_QHY10Class::SendREG(unsigned long P_Size,unsigned long &Total_P,unsigned int &PatchNumber) {

}

int Cam_QHY10Class::Capture () {
 return 1;
}

void Cam_QHY10Class::CaptureFineFocus(int click_x, int click_y) {

}

bool Cam_QHY10Class::Reconstruct(int mode) {
	bool retval = false;
	if (!CurrImage.IsOK())
		return true;
	if (CurrImage.ArrayType == COLOR_BW) mode = BW_SQUARE;
	if (Has_PixelBalance && (mode != BW_SQUARE))
		BalancePixels(CurrImage,ConnectedModel);
	if (mode != BW_SQUARE) retval = DebayerImage(ArrayType, DebayerXOffset, DebayerYOffset);
	else retval = false;
	if (!retval) {
		if (Has_ColorMix && (mode != BW_SQUARE)) ColorRebalance(CurrImage,ConnectedModel);
		SquarePixels(CurrImage.Header.XPixelSize,CurrImage.Header.YPixelSize);
	}
	return retval;
}

void Cam_QHY10Class::UpdateGainCtrl(wxSpinCtrl *GainCtrl, wxStaticText *GainText) {

}

void Cam_QHY10Class::SetTEC(bool state, int temp) {

}

float Cam_QHY10Class::GetTECTemp() {
 return -273.1f;
}

float Cam_QHY10Class::GetTECPower() {
 return 0.0f;
}

void Cam_QHY10Class::PeriodicUpdate() {

}

Cam_QHY12Class::Cam_QHY12Class() {
	ConnectedModel = CAMERA_QHY12;
	Name="QHY12";
	Size[0]=4610;
	Size[1]=3080;
	PixelSize[0]=5.12;
	PixelSize[1]=5.12;
	Npixels = Size[0] * Size[1];
	ColorMode = COLOR_BW;
	ArrayType = COLOR_RGB;
	AmpOff = true;
	DoubleRead = false;
	HighSpeed = false;
	BinMode = BIN1x1;
	FullBitDepth = true;
	Cap_Colormode = COLOR_RGB;
	Cap_DoubleRead = false;
	Cap_HighSpeed = true;
	Cap_BinMode = BIN1x1 | BIN2x2 | BIN4x4;
	Cap_AmpOff = true;
	Cap_LowBit = false;
	Cap_GainCtrl = Cap_OffsetCtrl = true;
	Cap_ExtraOption = false;
	ExtraOption=false;
	ExtraOptionName = "";
	Cap_FineFocus = true;
	Cap_BalanceLines = false;
	Cap_AutoOffset = false;
	Cap_TECControl = true;
	TECState = true;
	DebayerXOffset = 1;
	DebayerYOffset = 1;
	Has_ColorMix = false;
	Has_PixelBalance = false;
	TEC_Temp = -271.3;
	TEC_Power = 0.0;
}

void Cam_QHY12Class::Disconnect() {

}

bool Cam_QHY12Class::Connect() {

    wxMessageBox("This camera SDK is not available in the Apple Silicon preview.", "Camera unavailable", wxOK | wxICON_INFORMATION);
    return true;

}

void Cam_QHY12Class::ResetREG() {

}

void Cam_QHY12Class::SendREG(unsigned long P_Size,unsigned long &Total_P,unsigned int &PatchNumber) {

}

int Cam_QHY12Class::Capture () {
 return 1;
}

void Cam_QHY12Class::CaptureFineFocus(int click_x, int click_y) {

}

bool Cam_QHY12Class::Reconstruct(int mode) {
	bool retval = false;
	if (!CurrImage.IsOK())
		return true;
	if (CurrImage.ArrayType == COLOR_BW) mode = BW_SQUARE;
	if (Has_PixelBalance && (mode != BW_SQUARE))
		BalancePixels(CurrImage,ConnectedModel);
	if (mode != BW_SQUARE) retval = DebayerImage(ArrayType, DebayerXOffset, DebayerYOffset);
	else retval = false;
	if (!retval) {
		if (Has_ColorMix && (mode != BW_SQUARE)) ColorRebalance(CurrImage,ConnectedModel);
		SquarePixels(CurrImage.Header.XPixelSize,CurrImage.Header.YPixelSize);
	}
	return retval;
}

void Cam_QHY12Class::UpdateGainCtrl(wxSpinCtrl *GainCtrl, wxStaticText *GainText) {

}

void Cam_QHY12Class::SetTEC(bool state, int temp) {

}

float Cam_QHY12Class::GetTECTemp() {
 return -273.1f;
}

float Cam_QHY12Class::GetTECPower() {
 return 0.0f;
}

void Cam_QHY12Class::PeriodicUpdate() {

}

Cam_QHY8LClass::Cam_QHY8LClass() {
	ConnectedModel = CAMERA_QHY8L;
	Name="QHY8L";
	Size[0]=3032;
	Size[1]=2016;
	PixelSize[0]=7.8;
	PixelSize[1]=7.8;
	Npixels = Size[0] * Size[1];
	ColorMode = COLOR_BW;
	ArrayType = COLOR_RGB;
	AmpOff = true;
	DoubleRead = false;
	HighSpeed = false;
	BinMode = BIN1x1;
	FullBitDepth = true;
	Cap_Colormode = COLOR_RGB;
	Cap_DoubleRead = false;
	Cap_HighSpeed = true;
	Cap_BinMode = BIN1x1 | BIN2x2 | BIN4x4;
	Cap_AmpOff = true;
	Cap_LowBit = false;
	Cap_GainCtrl = Cap_OffsetCtrl = true;
	Cap_ExtraOption = false;
	ExtraOption=false;
	ExtraOptionName = "";
	Cap_FineFocus = true;
	Cap_BalanceLines = false;
	Cap_AutoOffset = false;
	Cap_TECControl = true;
	TECState = true;
	DebayerXOffset = 1;
	DebayerYOffset = 1;
	Has_ColorMix = false;
	Has_PixelBalance = false;
	TEC_Temp = -271.3;
	TEC_Power = 0.0;
}

void Cam_QHY8LClass::Disconnect() {

}

bool Cam_QHY8LClass::Connect() {

    wxMessageBox("This camera SDK is not available in the Apple Silicon preview.", "Camera unavailable", wxOK | wxICON_INFORMATION);
    return true;

}

void Cam_QHY8LClass::ResetREG() {

}

void Cam_QHY8LClass::SendREG(unsigned long P_Size,unsigned long &Total_P,unsigned int &PatchNumber) {

}

int Cam_QHY8LClass::Capture () {
 return 1;
}

void Cam_QHY8LClass::CaptureFineFocus(int click_x, int click_y) {

}

bool Cam_QHY8LClass::Reconstruct(int mode) {
	bool retval = false;
	if (!CurrImage.IsOK())
		return true;
	if (CurrImage.ArrayType == COLOR_BW) mode = BW_SQUARE;
	if (Has_PixelBalance && (mode != BW_SQUARE))
		BalancePixels(CurrImage,ConnectedModel);
	if (mode != BW_SQUARE) retval = DebayerImage(ArrayType, DebayerXOffset, DebayerYOffset);
	else retval = false;
	if (!retval) {
		if (Has_ColorMix && (mode != BW_SQUARE)) ColorRebalance(CurrImage,ConnectedModel);
		SquarePixels(CurrImage.Header.XPixelSize,CurrImage.Header.YPixelSize);
	}
	return retval;
}

void Cam_QHY8LClass::UpdateGainCtrl(wxSpinCtrl *GainCtrl, wxStaticText *GainText) {

}

void Cam_QHY8LClass::SetTEC(bool state, int temp) {

}

float Cam_QHY8LClass::GetTECTemp() {
 return -273.1f;
}

float Cam_QHY8LClass::GetTECPower() {
 return 0.0f;
}

void Cam_QHY8LClass::PeriodicUpdate() {

}

Cam_QHYUnivClass::Cam_QHYUnivClass() {
	ConnectedModel = CAMERA_QHYUNIV;
	Name="QHY Universal";
	ColorMode = COLOR_BW;
	ArrayType = COLOR_BW;
	AmpOff = true;
	DoubleRead = false;
	HighSpeed = false;
	BinMode = BIN1x1;
	FullBitDepth = true;
	Cap_Colormode = COLOR_BW;
	Cap_DoubleRead = false;
	Cap_HighSpeed = false;
	Cap_BinMode = BIN1x1 | BIN2x2;
	Cap_AmpOff = false;
	Cap_LowBit = false;
    Cap_GainCtrl = true;
    Cap_OffsetCtrl = true;
	Cap_ExtraOption = false;
	ExtraOption=false;
	ExtraOptionName = "";
	Cap_FineFocus = true;
	Cap_BalanceLines = false;
	Cap_AutoOffset = false;
	Cap_TECControl = true;
	TECState = true;
	DebayerXOffset = 1;
	DebayerYOffset = 1;
	Has_ColorMix = false;
	Has_PixelBalance = false;
	Cam_h = NULL;
	RawData = NULL;
}

void Cam_QHYUnivClass::Disconnect() {

}

bool Cam_QHYUnivClass::Connect() {

    wxMessageBox("This camera SDK is not available in the Apple Silicon preview.", "Camera unavailable", wxOK | wxICON_INFORMATION);
    return true;

}

int Cam_QHYUnivClass::Capture () {
 return 1;
}

void Cam_QHYUnivClass::CaptureFineFocus(int click_x, int click_y) {

}

bool Cam_QHYUnivClass::Reconstruct(int mode) {
	bool retval = false;
	if (!CurrImage.IsOK())
		return true;
	if (CurrImage.ArrayType == COLOR_BW) mode = BW_SQUARE;
	if (Has_PixelBalance && (mode != BW_SQUARE))
		BalancePixels(CurrImage,ConnectedModel);
	if (mode != BW_SQUARE) retval = DebayerImage(ArrayType, DebayerXOffset, DebayerYOffset);
	else retval = false;
	if (!retval) {
		if (Has_ColorMix && (mode != BW_SQUARE)) ColorRebalance(CurrImage,ConnectedModel);
		SquarePixels(CurrImage.Header.XPixelSize,CurrImage.Header.YPixelSize);
	}
	return retval;
}

void Cam_QHYUnivClass::SetTEC(bool state, int temp) {

}

float Cam_QHYUnivClass::GetTECTemp() {
 return -273.1f;
}

float Cam_QHYUnivClass::GetTECPower() {
 return 0.0f;
}

void Cam_QHYUnivClass::UpdateGainCtrl(wxSpinCtrl *GainCtrl, wxStaticText *GainText) {

}

void Cam_QHYUnivClass::PeriodicUpdate() {

}

Cam_QSI500Class::Cam_QSI500Class() {
	ConnectedModel = CAMERA_QSI500;
	Name="QSI 500/600";
	Size[0]=1360;
	Size[1]=1024;
	PixelSize[0]=6.54;
	PixelSize[1]=6.54;
	Npixels = Size[0] * Size[1];
	ColorMode = COLOR_BW;
	ArrayType = COLOR_BW;
	AmpOff = true;
	DoubleRead = false;
	HighSpeed = false;
	BinMode = BIN1x1;
	FullBitDepth = true;
	Cap_Colormode = COLOR_BW;
	Cap_DoubleRead = false;
	Cap_HighSpeed = false;
	Cap_BinMode = BIN1x1 | BIN2x2 | BIN3x3;
	Cap_AmpOff = false;
	Cap_LowBit = false;
	Cap_ExtraOption = false;
	Cap_FineFocus = true;
	Cap_BalanceLines = false;
	Cap_AutoOffset = false;
	Cap_TECControl = true;
	Cap_GainCtrl = true;
	TECState = true;
	Has_ColorMix = false;
	Has_PixelBalance = false;
	DebayerXOffset = 0;
	DebayerYOffset = 1;
	ShutterState = false;
}

void Cam_QSI500Class::Disconnect() {

}

bool Cam_QSI500Class::Connect() {

    wxMessageBox("This camera SDK is not available in the Apple Silicon preview.", "Camera unavailable", wxOK | wxICON_INFORMATION);
    return true;

}

int Cam_QSI500Class::Capture () {
 return 1;
}

void Cam_QSI500Class::CaptureFineFocus(int click_x, int click_y) {

}

int Cam_QSI500Class::GetCFWState() {
 return 2;
}

void Cam_QSI500Class::SetFilter(int position) {

}

void Cam_QSI500Class::ShowMfgSetupDialog() {

}

bool Cam_QSI500Class::Reconstruct(int mode) {
	bool retval = false;
	if (!CurrImage.IsOK())
		return true;
	if (CurrImage.ArrayType == COLOR_BW) mode = BW_SQUARE;
	if (Has_PixelBalance && (mode != BW_SQUARE))
		BalancePixels(CurrImage,ConnectedModel);
	if (mode != BW_SQUARE) retval = DebayerImage(ArrayType, DebayerXOffset, DebayerYOffset);
	else retval = false;
	if (!retval) {
		if (Has_ColorMix && (mode != BW_SQUARE)) ColorRebalance(CurrImage,ConnectedModel);
		SquarePixels(CurrImage.Header.XPixelSize,CurrImage.Header.YPixelSize);
	}
	return retval;
}

void Cam_QSI500Class::UpdateGainCtrl(wxSpinCtrl *GainCtrl, wxStaticText *GainText) {

}

void Cam_QSI500Class::SetTEC(bool state, int temp) {

}

float Cam_QSI500Class::GetTECTemp() {
 return -273.1f;
}

float Cam_QSI500Class::GetAmbientTemp() {
 return -273.1f;
}

float Cam_QSI500Class::GetTECPower() {
 return 0.0f;
}

Cam_FLIClass::Cam_FLIClass() {
	ConnectedModel = CAMERA_FLI;
	Name="FLI";
	Size[0]=1024;
	Size[1]=1024;
	PixelSize[0]=1.0;
	PixelSize[1]=1.0;
	Npixels = Size[0] * Size[1];
	ColorMode = COLOR_BW;
	ArrayType = COLOR_BW;
	AmpOff = true;
	DoubleRead = false;
	HighSpeed = false;
	BinMode = BIN1x1;
	FullBitDepth = true;
	Cap_Colormode = COLOR_BW;
	Cap_DoubleRead = false;
	Cap_HighSpeed = true;
	Cap_BinMode = BIN1x1 | BIN2x2 | BIN3x3 | BIN4x4;
	Cap_AmpOff = true;
	Cap_LowBit = false;
	Cap_FineFocus = true;
	Cap_BalanceLines = false;
	Cap_AutoOffset = false;
	Cap_TECControl = true;
	TECState = true;
	Has_ColorMix = false;
	Has_PixelBalance = false;
	ShutterState = false;
    fdev_fw = NULL;
	fdev = NULL;
	CFWPositions = CFWPosition = 0;
}

void Cam_FLIClass::Disconnect() {

}

bool Cam_FLIClass::Connect() {

    wxMessageBox("This camera SDK is not available in the Apple Silicon preview.", "Camera unavailable", wxOK | wxICON_INFORMATION);
    return true;

}

int Cam_FLIClass::Capture () {
 return 1;
}

void Cam_FLIClass::CaptureFineFocus(int click_x, int click_y) {

}

bool Cam_FLIClass::Reconstruct(int mode) {
	bool retval = false;
	if (!CurrImage.IsOK())
		return true;
	if (CurrImage.ArrayType == COLOR_BW) mode = BW_SQUARE;
	if (Has_PixelBalance && (mode != BW_SQUARE))
		BalancePixels(CurrImage,ConnectedModel);
	if (mode != BW_SQUARE) retval = VNG_Interpolate(ArrayType,DebayerXOffset,DebayerYOffset,1);
	else retval = false;
	if (!retval) {
		if (Has_ColorMix && (mode != BW_SQUARE)) ColorRebalance(CurrImage,ConnectedModel);
		SquarePixels(CurrImage.Header.XPixelSize,CurrImage.Header.YPixelSize);
	}
	return retval;
}

void Cam_FLIClass::UpdateGainCtrl(wxSpinCtrl *GainCtrl, wxStaticText *GainText) {

}

void Cam_FLIClass::SetTEC(bool state, int temp) {

}

float Cam_FLIClass::GetTECTemp() {
 return -273.1f;
}

float Cam_FLIClass::GetTECPower() {
 return 0.0f;
}

void Cam_FLIClass::SetFilter(int position) {

}


#endif
