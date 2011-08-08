#include <unistd.h>

#include "HomeMenu.h"
#include "Controls/Application.h"
#include "Prompts/PromptWindows.h"
#include "SoundOperations/MusicPlayer.h"
#include "Memory/Resources.h"
#include "main.h"
#include "sys.h"


HomeMenu::HomeMenu()
	: GuiWindow(0, 0)
{
	choice = -1;

	this->SetPosition(0, 0);
	this->SetSize(screenwidth, screenheight);

	trigA = new SimpleGuiTrigger(-1, WiiControls.ClickButton | ClassicControls.ClickButton << 16, GCControls.ClickButton);
	trigHome = new GuiTrigger();
	trigHome->SetButtonOnlyTrigger(-1, WiiControls.HomeButton | ClassicControls.HomeButton << 16, GCControls.HomeButton);

	ButtonClickSnd = Resources::GetSound("button_click.wav");
	ButtonOverSnd = Resources::GetSound("button_over.wav");

	TopBtnImgData = Resources::GetImageData("homemenu_top.png");
	TopBtnOverImgData = Resources::GetImageData("homemenu_top_over.png");
	BottomBtnImgData = Resources::GetImageData("homemenu_bottom.png");
	BottomBtnOverImgData = Resources::GetImageData("homemenu_bottom_over.png");
	CloseBtnImgData = Resources::GetImageData("homemenu_close.png");
	StandardBtnImgData = Resources::GetImageData("homemenu_button.png");
	WiimoteBtnImgData = Resources::GetImageData("wiimote.png");

	BatteryImgData = Resources::GetImageData("battery.png");
	BatteryBarImgData = Resources::GetImageData("battery_bar.png");

	TopBtnImg = new GuiImage(TopBtnImgData);
	TopBtnOverImg = new GuiImage(TopBtnOverImgData);
	BottomBtnImg = new GuiImage(BottomBtnImgData);
	BottomBtnOverImg = new GuiImage(BottomBtnOverImgData);
	CloseBtnImg = new GuiImage(CloseBtnImgData);
	ExitBtnImg = new GuiImage(StandardBtnImgData);
	ShutdownBtnImg = new GuiImage(StandardBtnImgData);
	WiimoteBtnImg = new GuiImage(WiimoteBtnImgData);

	TitleText = new GuiText(tr("Home Menu"), 40, (GXColor) {255, 255, 255, 255});
	TitleText->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	TitleText->SetPosition(30, 40);
	TitleText->SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_IN, 50);

	TopBtn = new GuiButton(TopBtnImg->GetWidth(), TopBtnImg->GetHeight());
	TopBtn->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
	TopBtn->SetImage(TopBtnImg);
	TopBtn->SetImageOver(TopBtnOverImg);
	TopBtn->SetLabel(TitleText);
	TopBtn->SetSoundClick(ButtonClickSnd);
	TopBtn->SetSoundOver(ButtonOverSnd);
	TopBtn->SetTrigger(trigA);
	TopBtn->SetTrigger(trigHome);
	TopBtn->SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_IN, 50);

	BottomBtn = new GuiButton(BottomBtnImg->GetWidth(), BottomBtnImg->GetHeight());
	BottomBtn->SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	BottomBtn->SetImage(BottomBtnImg);
	BottomBtn->SetImageOver(BottomBtnOverImg);
	BottomBtn->SetSoundClick(ButtonClickSnd);
	BottomBtn->SetSoundOver(ButtonOverSnd);
	BottomBtn->SetTrigger(trigA);
	BottomBtn->SetEffect(EFFECT_SLIDE_BOTTOM | EFFECT_SLIDE_IN, 50);
	BottomBtn->SetSelectable(true);

	CloseBtnText = new GuiText(tr("Close"), 28, (GXColor) {0, 0, 0, 255});

	CloseBtn = new GuiButton(CloseBtnImg->GetWidth(), CloseBtnImg->GetHeight());
	CloseBtn->SetImage(CloseBtnImg);
	CloseBtn->SetAlignment(ALIGN_RIGHT, ALIGN_TOP);
	CloseBtn->SetPosition(-20, 30);
	CloseBtn->SetLabel(CloseBtnText);
	CloseBtn->SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_IN, 50);

	ExitBtnText = new GuiText(tr("Menu"), 28, (GXColor) {0, 0, 0, 255});

	ExitBtn = new GuiButton(ExitBtnImg->GetWidth(), ExitBtnImg->GetHeight());
	ExitBtn->SetImage(ExitBtnImg);
	ExitBtn->SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
	ExitBtn->SetPosition(-140, 0);
	ExitBtn->SetLabel(ExitBtnText);
	ExitBtn->SetSoundClick(ButtonClickSnd);
	ExitBtn->SetSoundOver(ButtonOverSnd);
	ExitBtn->SetTrigger(trigA);
	ExitBtn->SetEffectGrow();
	ExitBtn->SetEffect(EFFECT_FADE, 50);

	ShutdownBtnText = new GuiText(tr("Shutdown"), 28, (GXColor) {0, 0, 0, 255});

	ShutdownBtn = new GuiButton(ShutdownBtnImg->GetWidth(), ShutdownBtnImg->GetHeight());
	ShutdownBtn->SetImage(ShutdownBtnImg);
	ShutdownBtn->SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
	ShutdownBtn->SetPosition(140, 0);
	ShutdownBtn->SetLabel(ShutdownBtnText);
	ShutdownBtn->SetSoundClick(ButtonClickSnd);
	ShutdownBtn->SetSoundOver(ButtonOverSnd);
	ShutdownBtn->SetTrigger(trigA);
	ShutdownBtn->SetEffectGrow();
	ShutdownBtn->SetEffect(EFFECT_FADE, 50);

	WiimoteBtn = new GuiButton(WiimoteBtnImg->GetWidth(), WiimoteBtnImg->GetHeight());
	WiimoteBtn->SetImage(WiimoteBtnImg);
	WiimoteBtn->SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
	WiimoteBtn->SetPosition(45, 232);
	WiimoteBtn->SetTrigger(trigA);
	WiimoteBtn->SetEffect(EFFECT_SLIDE_BOTTOM | EFFECT_SLIDE_IN, 50);

	Append(BottomBtn);
	Append(TopBtn);
	Append(CloseBtn);
	Append(ExitBtn);
	Append(ShutdownBtn);
	Append(WiimoteBtn);

	for (int i = 0; i < 4; i++)
	{
		char player[] = "P0";
		player[1] = i+'1';

		PlayerText[i] = new GuiText(player, 28, (GXColor) {255, 255, 255, 255});
		PlayerText[i]->SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
		PlayerText[i]->SetPosition(178 + i*108, -76);
		PlayerText[i]->SetEffect(EFFECT_SLIDE_BOTTOM | EFFECT_SLIDE_IN, 50);
		Append(PlayerText[i]);

		BatteryBarImg[i] = new GuiImage(BatteryBarImgData);
		BatteryBarImg[i]->SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);

		BatteryImg[i] = new GuiImage(BatteryImgData);
		BatteryImg[i]->SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
		BatteryImg[i]->SetPosition(2, -4);

		BatteryBtn[i] = new GuiButton(0,0);
		BatteryBtn[i]->SetAlignment(ALIGN_LEFT, ALIGN_BOTTOM);
		BatteryBtn[i]->SetPosition(214 + i*108, -80);
		BatteryBtn[i]->SetImage(BatteryBarImg[i]);
		BatteryBtn[i]->SetIcon(BatteryImg[i]);
		BatteryBtn[i]->SetEffect(EFFECT_SLIDE_BOTTOM | EFFECT_SLIDE_IN, 50);

		Append(BatteryBtn[i]);
	}

	if(!MusicPlayer::Instance()->IsStopped())
		MusicPlayer::Instance()->Pause();
	Application::Instance()->SetState(STATE_DISABLED);
	//Application::Instance()->SetDim(true);
}

HomeMenu::~HomeMenu()
{
	SetEffect(EFFECT_FADE, -50);
	while(this->GetEffect() > 0) usleep(100);

	if(parentElement)
		((GuiWindow *) parentElement)->Remove(this);

	RemoveAll();

	delete WiimoteBtn;
	delete ShutdownBtn;
	delete ExitBtn;
	delete CloseBtn;
	delete TopBtn;
	delete BottomBtn;

	delete TopBtnImg;
	delete TopBtnOverImg;
	delete BottomBtnImg;
	delete BottomBtnOverImg;
	delete CloseBtnImg;
	delete ExitBtnImg;
	delete ShutdownBtnImg;
	delete WiimoteBtnImg;

	delete TitleText;
	delete ShutdownBtnText;
	delete ExitBtnText;
	delete CloseBtnText;

	for (int i = 0; i < 4; i++)
	{
		delete PlayerText[i];
		delete BatteryBarImg[i];
		delete BatteryImg[i];
		delete BatteryBtn[i];
	}

	delete trigA;
	delete trigHome;

	Resources::Remove(TopBtnImgData);
	Resources::Remove(TopBtnOverImgData);
	Resources::Remove(BottomBtnImgData);
	Resources::Remove(BottomBtnOverImgData);
	Resources::Remove(CloseBtnImgData);
	Resources::Remove(StandardBtnImgData);
	Resources::Remove(WiimoteBtnImgData);
	Resources::Remove(BatteryImgData);
	Resources::Remove(BatteryBarImgData);

	Resources::Remove(ButtonClickSnd);
	Resources::Remove(ButtonOverSnd);

	Application::Instance()->SetState(STATE_DEFAULT);
	//Application::Instance()->SetDim(false);
}

void HomeMenu::FadeOut()
{
	TitleText->SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_OUT, 50);
	TopBtn->SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_OUT, 50);
	CloseBtn->SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_OUT, 50);
	ExitBtn->SetEffect(EFFECT_FADE, -50);
	ShutdownBtn->SetEffect(EFFECT_FADE, -50);
	BottomBtn->SetEffect(EFFECT_SLIDE_BOTTOM | EFFECT_SLIDE_OUT, 50);
	WiimoteBtn->SetEffect(EFFECT_SLIDE_BOTTOM | EFFECT_SLIDE_OUT, 50);

	for (int i = 0; i < 4; i++)
	{
		PlayerText[i]->SetEffect(EFFECT_SLIDE_BOTTOM | EFFECT_SLIDE_OUT, 50);
		BatteryBtn[i]->SetEffect(EFFECT_SLIDE_BOTTOM | EFFECT_SLIDE_OUT, 50);
	}
}

int HomeMenu::GetChoice()
{
	for (int i = 0; i < 4; i++)
	{
		if (WPAD_Probe(i, NULL) == WPAD_ERR_NONE)
		{
			int level = (WPAD_BatteryLevel(i) / 100.0) * 4;
			if (level > 4) level = 4;

			BatteryImg[i]->SetTileHorizontal(level);
			BatteryBtn[i]->SetAlpha(255);
			PlayerText[i]->SetAlpha(255);
		}
		else
		{
			BatteryImg[i]->SetTileHorizontal(0);
			BatteryBtn[i]->SetAlpha(130);
			PlayerText[i]->SetAlpha(100);
		}
	}

	if (TopBtn->GetState() == STATE_CLICKED)
	{
		TopBtn->ResetState();
		FadeOut();
		choice = 0; // return to WiiXplorer
	}
	else if (BottomBtn->GetState() == STATE_CLICKED)
	{
		BottomBtn->ResetState();
		FadeOut();
		choice = 0; // return to WiiXplorer
	}
	else if (ExitBtn->GetState() == STATE_CLICKED)
	{
		ExitBtn->ResetState();

		this->SetState(STATE_DISABLED);
		int ret = WindowPrompt(tr("Exit WiiXplorer?"), 0, tr("To Loader"), tr("To Menu"), tr("Cancel"));
		if (ret == 1)
		{
			Sys_BackToLoader();
		}
		else if(ret == 2)
		{
			Sys_LoadMenu();
		}
		this->SetState(STATE_DEFAULT);
	}
	else if (ShutdownBtn->GetState() == STATE_CLICKED)
	{
		ShutdownBtn->ResetState();
		this->SetState(STATE_DISABLED);
		int ret = WindowPrompt(tr("Shutdown the Wii?"), 0, tr("Default"), tr("To Standby"), tr("To Idle"), tr("Cancel"));
		if (ret == 1)
		{
			Sys_Shutdown();
		}
		else if (ret == 2)
		{
			Sys_ShutdownToStandby();
		}
		else if(ret == 3)
		{
			Sys_ShutdownToIdle();
		}
		this->SetState(STATE_DEFAULT);
	}
	else if (BottomBtn->GetState() == STATE_SELECTED)
	{
		WiimoteBtn->SetPosition(WiimoteBtn->GetLeft(), 210);
	}
	else if (BottomBtn->GetState() != STATE_SELECTED)
	{
		WiimoteBtn->SetPosition(WiimoteBtn->GetLeft(), 232);
	}

	return choice;
}
