/***************************************************************************
 * Copyright (C) 2010
 * by Dimok
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any
 * damages arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any
 * purpose, including commercial applications, and to alter it and
 * redistribute it freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you
 * must not claim that you wrote the original software. If you use
 * this software in a product, an acknowledgment in the product
 * documentation would be appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and
 * must not be misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source
 * distribution.
 *
 * PromptWindows.cpp
 *
 * All promptwindows
 * for WiiXplorer 2010
 ***************************************************************************/
#include "GUI/gui_keyboard.h"
#include "network/networkops.h"
#include "Prompts/PromptWindows.h"
#include "Prompts/KeyboardWindow.h"
#include "Controls/Taskbar.h"
#include "FileOperations/fileops.h"
#include "FileOperations/filebrowser.h"
#include "menu.h"
#include "main.h"
#include "Controls/Application.h"
#include "Tools/StringTools.h"
#include "TextOperations/wstring.hpp"
#include "sys.h"
#include "svnrev.h"

int OnScreenKeyboard(char * var, u16 maxlen)
{
	wchar_t *wtext = new wchar_t[maxlen+2];
	wtext[0] = 0;

	char2wchar_t(var, wtext);

	int save = OnScreenKeyboard(wtext, maxlen);

	if(save)
	{
		wString Converter(wtext);
		snprintf(var, maxlen, Converter.toUTF8().c_str());
	}

	delete [] wtext;

	return save;
}

int OnScreenKeyboard(wchar_t * var, u16 maxlen)
{
	int save = 0;
	KeyboardWindow *keyboard = new KeyboardWindow(var, maxlen);
	keyboard->DimBackground(true);

	GuiElement *onlyUpdateElement = Application::Instance()->GetUpdateOnly();
	Application::Instance()->Append(keyboard);
	Application::Instance()->UpdateOnly(keyboard);

	while((save = keyboard->GetChoice()) < 0)
	{
		Application::Instance()->updateEvents();
	}

	Application::Instance()->UpdateOnly(onlyUpdateElement);

	delete keyboard;
	keyboard = NULL;

	return save;
}

/****************************************************************************
* NetworkInitPrompt
***************************************************************************/
bool NetworkInitPrompt()
{
	if(IsNetworkInit())
		return true;

	PromptWindow * Prompt = new PromptWindow(tr("Network initialising..."), tr("Please wait..."), tr("Cancel"));
	Application::Instance()->Append(Prompt);

	if(!Settings.AutoConnect)
	{
		InitNetworkThread();
		ResumeNetworkThread();
	}

	while(Prompt->GetChoice() == -1 && !IsNetworkInit())
	{
		usleep(100);
	}

	delete Prompt;
	Prompt = NULL;

	if(!IsNetworkInit())
	{
		ShowError(tr("No network connection."));
		return false;
	}

	return true;
}

/****************************************************************************
* WindowPrompt
*
* Shortcut function
***************************************************************************/
int WindowPrompt(const char *title, const char *msg,
				 const char *btn1Label, const char *btn2Label,
				 const char *btn3Label, const char *btn4Label)
{
	int choice = -1;

	PromptWindow * Prompt = new PromptWindow(title, msg, btn1Label, btn2Label, btn3Label, btn4Label);
	Prompt->DimBackground(true);
	GuiElement *onlyUpdateElement = Application::Instance()->GetUpdateOnly();
	Application::Instance()->Append(Prompt);
	Application::Instance()->UpdateOnly(Prompt);

	while((choice = Prompt->GetChoice()) < 0)
	{
		Application::Instance()->updateEvents();
	}

	Application::Instance()->UpdateOnly(onlyUpdateElement);
	delete Prompt;
	Prompt = NULL;

	return choice;
}

/****************************************************************************
* Wait SMB Connection
***************************************************************************/
int WaitSMBConnect(void)
{
	static bool firsttimestart = true;

	if(!firsttimestart)
		return 1;

	firsttimestart = false;
	int choice = -1;

	if(Settings.LastUsedPath.compare(0, 3, "smb") != 0 && Settings.LastUsedPath.compare(0, 3, "ftp") != 0)
		return 1;

	PromptWindow * Prompt = new PromptWindow(tr("Please wait:"), tr("Network initialising..."), tr("Cancel"));

	if(!Settings.AutoConnect)
	{
		InitNetworkThread();
		ResumeNetworkThread();
	}

	time_t timer1 = 0;

	Application::Instance()->Append(Prompt);

	while(choice == -1)
	{
		usleep(100);

		choice = Prompt->GetChoice();

		if(IsNetworkInit())
		{
			Prompt->SetMessage(tr("SMB is connecting..."));

			if(timer1 == 0)
				timer1 = time(0);

			if(time(0) - timer1 > 5)
				choice = -2;

			for(int i = 0; i < 4; i++)
			{
				if(IsSMB_Mounted(i))
				{
					choice = 2;
					break;
				}
			}
		}
	}

	delete Prompt;
	Prompt = NULL;

	return choice;
}
