/****************************************************************************
 * Copyright (C) 2009
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
 * TextEditor.cpp
 * for WiiXplorer 2009
 ***************************************************************************/

#include <unistd.h>
#include "TextOperations/TextEditor.h"
#include "FileOperations/fileops.h"
#include "Prompts/PromptWindows.h"
#include "Controls/MainWindow.h"
#include "Memory/Resources.h"
#include "menu.h"

#define FONTSIZE    18

/**
 * Constructor for the TextEditor class.
 */
TextEditor::TextEditor(const wchar_t *intext, int LinesToDraw, const char *path)
{
	triggerupdate = true;
	ExitEditor = false;
	LineEditing = false;
	FileEdited = false;
	linestodraw = LinesToDraw;
	filesize = (u32) FileSize(path);

	filepath = new char[strlen(path)+1];
	snprintf(filepath, strlen(path)+1, "%s", path);

	char * filename = strrchr(filepath, '/')+1;

	trigA = new GuiTrigger;
	trigA->SetSimpleTrigger(-1, WiiControls.ClickButton | ClassicControls.ClickButton << 16, GCControls.ClickButton);
	trigHeldA = new GuiTrigger;
	trigHeldA->SetHeldTrigger(-1, WiiControls.ClickButton | ClassicControls.ClickButton << 16, GCControls.ClickButton);
	trigB = new GuiTrigger;
	trigB->SetButtonOnlyTrigger(-1, WiiControls.BackButton | ClassicControls.BackButton << 16, GCControls.BackButton);
	trigPlus = new GuiTrigger;
	trigPlus->SetButtonOnlyTrigger(-1, WiiControls.EditTextLine | ClassicControls.EditTextLine << 16, GCControls.EditTextLine);

	btnSoundOver = Resources::GetSound(button_over_wav, button_over_wav_size);
	btnSoundClick = Resources::GetSound(button_click_wav, button_click_wav_size);

	bgTexteditorData = Resources::GetImageData(textreader_box_png, textreader_box_png_size);
	bgTexteditorImg = new GuiImage(bgTexteditorData);

	closeImgData = Resources::GetImageData(close_png, close_png_size);
	closeImgOverData = Resources::GetImageData(close_over_png, close_over_png_size);
    closeImg = new GuiImage(closeImgData);
    closeImgOver = new GuiImage(closeImgOverData);
	maximizeImgData = Resources::GetImageData(maximize_dis_png, maximize_dis_png_size);
    maximizeImg = new GuiImage(maximizeImgData);
	minimizeImgData = Resources::GetImageData(minimize_dis_png, minimize_dis_png_size);
    minimizeImg = new GuiImage(minimizeImgData);

	scrollbar = new Scrollbar(230, LISTMODE);
	scrollbar->SetParent(this);
	scrollbar->SetAlignment(ALIGN_RIGHT, ALIGN_TOP);
	scrollbar->SetPosition(-25, 60);
	scrollbar->SetScrollSpeed(Settings.ScrollSpeed);

    closeBtn = new GuiButton(closeImg->GetWidth(), closeImg->GetHeight());
    closeBtn->SetImage(closeImg);
    closeBtn->SetImageOver(closeImgOver);
    closeBtn->SetAlignment(ALIGN_RIGHT, ALIGN_TOP);
    closeBtn->SetPosition(-30, 30);
    closeBtn->SetSoundOver(btnSoundOver);
    closeBtn->SetSoundClick(btnSoundClick);
    closeBtn->SetTrigger(trigA);
    closeBtn->SetTrigger(trigB);
    closeBtn->SetEffectGrow();
    closeBtn->Clicked.connect(this, &TextEditor::OnButtonClick);

    maximizeBtn = new GuiButton(maximizeImg->GetWidth(), maximizeImg->GetHeight());
    maximizeBtn->SetImage(maximizeImg);
    maximizeBtn->SetAlignment(ALIGN_RIGHT, ALIGN_TOP);
    maximizeBtn->SetPosition(-60, 30);
    maximizeBtn->SetSoundClick(btnSoundClick);

    minimizeBtn = new GuiButton(minimizeImg->GetWidth(), minimizeImg->GetHeight());
    minimizeBtn->SetImage(minimizeImg);
    minimizeBtn->SetAlignment(ALIGN_RIGHT, ALIGN_TOP);
    minimizeBtn->SetPosition(-90, 30);
    minimizeBtn->SetSoundClick(btnSoundClick);

    filenameTxt = new GuiText(filename, 22, (GXColor){0, 0, 0, 255});
    filenameTxt->SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
    filenameTxt->SetPosition(-30,30);
    filenameTxt->SetMaxWidth(340, DOTTED);

    MainFileTxt = new Text(intext, FONTSIZE, (GXColor){0, 0, 0, 255});
    MainFileTxt->SetAlignment(ALIGN_LEFT, ALIGN_TOP);
    MainFileTxt->SetPosition(0, 0);
    MainFileTxt->SetLinesToDraw(linestodraw);
    MainFileTxt->SetMaxWidth(340);

    TextPointerBtn = new TextPointer(MainFileTxt, linestodraw);
    TextPointerBtn->SetPosition(43, 75);
    TextPointerBtn->SetHoldable(true);
    TextPointerBtn->SetTrigger(trigHeldA);
    TextPointerBtn->Held.connect(this, &TextEditor::OnPointerHeld);

    PlusBtn = new GuiButton(0, 0);
    PlusBtn->SetTrigger(trigPlus);
    PlusBtn->SetSoundClick(btnSoundClick);
    PlusBtn->Clicked.connect(this, &TextEditor::OnButtonClick);

	width = bgTexteditorImg->GetWidth();
	height = bgTexteditorImg->GetHeight();

    this->Append(PlusBtn);
    this->Append(bgTexteditorImg);
    this->Append(filenameTxt);
    this->Append(TextPointerBtn);
    this->Append(scrollbar);
    this->Append(closeBtn);
    this->Append(maximizeBtn);
    this->Append(minimizeBtn);

    SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_IN, 50);
    SetPosition(0,0);
    SetAlignment(ALIGN_CENTRE, ALIGN_MIDDLE);
}

/**
 * Destructor for the TextEditor class.
 */
TextEditor::~TextEditor()
{
    MainWindow::Instance()->ResumeGui();
    SetEffect(EFFECT_SLIDE_TOP | EFFECT_SLIDE_OUT, 50);
    while(this->GetEffect() > 0) usleep(50);

    MainWindow::Instance()->HaltGui();
    if(parentElement)
        ((GuiWindow *) parentElement)->Remove(this);

    this->RemoveAll();

    delete scrollbar;

    /** Buttons **/
	delete maximizeBtn;
	delete minimizeBtn;
	delete closeBtn;
	delete TextPointerBtn;

    /** Images **/
	delete bgTexteditorImg;
	delete closeImg;
	delete closeImgOver;
	delete maximizeImg;
	delete minimizeImg;

    /** ImageDatas **/
	Resources::Remove(bgTexteditorData);
	Resources::Remove(closeImgData);
	Resources::Remove(closeImgOverData);
	Resources::Remove(maximizeImgData);
	Resources::Remove(minimizeImgData);

    /** Sounds **/
	Resources::Remove(btnSoundOver);
	Resources::Remove(btnSoundClick);

    /** Triggers **/
	delete trigHeldA;
	delete trigA;
	delete trigB;
	delete PlusBtn;

    /** Texts **/
    delete filenameTxt;
    delete MainFileTxt;
    delete [] filepath;

    MainWindow::Instance()->ResumeGui();
}

void TextEditor::SetText(const wchar_t *intext)
{
    LOCK(this);
    if(TextPointerBtn)
    {
        delete TextPointerBtn;
        TextPointerBtn = NULL;
    }

    MainFileTxt->SetText(intext);

    TextPointerBtn = new TextPointer(MainFileTxt, 0);
    TextPointerBtn->SetPosition(43, 75);
    TextPointerBtn->SetHoldable(true);
    TextPointerBtn->SetTrigger(trigHeldA);
    TextPointerBtn->Held.connect(this, &TextEditor::OnPointerHeld);
}

void TextEditor::SetTriggerUpdate(bool set)
{
    LOCK(this);
	triggerupdate = set;
}

void TextEditor::WriteTextFile(const char * path)
{
    FILE * f = fopen(path, "wb");
    if(!f)
    {
        ShowError(tr("Cannot write to the file."));
        return;
    }

    std::string FullText = MainFileTxt->GetUTF8String();

    fwrite(FullText.c_str(), 1, strlen(FullText.c_str())+1, f);

    fclose(f);
}

void TextEditor::ResetState()
{
    LOCK(this);
	state = STATE_DEFAULT;
	stateChan = -1;

	maximizeBtn->ResetState();
	minimizeBtn->ResetState();
	closeBtn->ResetState();
}

int TextEditor::GetState()
{
    if(LineEditing)
    {
        SetTriggerUpdate(false);
        if(EditLine() > 0)
        {
            FileEdited = true;
        }
        SetTriggerUpdate(true);

        MainWindow::Instance()->SetState(STATE_DISABLED);
        MainWindow::Instance()->SetDim(true);
        this->SetDim(false);
        this->SetState(STATE_DEFAULT);
        LineEditing = false;
    }

    if(state == STATE_CLOSED && FileEdited)
    {
        int choice = WindowPrompt(tr("File was edited."), tr("Do you want to save changes?"), tr("Yes"), tr("No"), tr("Cancel"));
        if(choice == 1)
        {
            WriteTextFile(filepath);
            state = STATE_CLOSED;
        }
        else if(choice == 2)
        {
            //to revert the state reset
            state = STATE_CLOSED;
        }
        else
        {
            MainWindow::Instance()->SetState(STATE_DISABLED);
            MainWindow::Instance()->SetDim(true);
            this->SetDim(false);
            this->SetState(STATE_DEFAULT);
        }
    }

    return GuiWindow::GetState();
}

int TextEditor::EditLine()
{
    u32 currentline = TextPointerBtn->GetCurrentLine();

    if(currentline < 0 || currentline >= (u32) linestodraw)
        return -1;

    u32 LetterNumInLine = TextPointerBtn->GetCurrentLetter();

    wString * wText = MainFileTxt->GetwString();
    if(!wText)
        return -1;

    const wchar_t * lineText = MainFileTxt->GetTextLine(currentline);
    if(!lineText)
        return -1;

    wchar_t temptxt[150];
    memset(temptxt, 0, sizeof(temptxt));

    int LineOffset = MainFileTxt->GetLineOffset(currentline+MainFileTxt->GetCurrPos());

    wcsncpy(temptxt, lineText, LetterNumInLine);
    temptxt[LetterNumInLine] = 0;

    int result = OnScreenKeyboard(temptxt, 150);
    if(result == 1)
    {
        wText->replace(LineOffset, LetterNumInLine, temptxt);
        MainFileTxt->Refresh();
        return 1;
    }

    return -1;
}

void TextEditor::OnButtonClick(GuiElement *sender, int pointer, POINT p)
{
    sender->ResetState();

    if(sender == closeBtn)
        SetState(STATE_CLOSED);

    else if(sender == PlusBtn)
    {
        LineEditing = true;
    }
}

void TextEditor::OnPointerHeld(GuiElement *sender, int pointer, POINT p)
{
    if(!userInput[pointer].wpad->ir.valid)
        return;

    TextPointerBtn->PositionChanged(pointer, p.x, p.y);
}

void TextEditor::Update(GuiTrigger * t)
{
	if(state == STATE_DISABLED || !t || !triggerupdate)
		return;

	scrollbar->Update(t);
	maximizeBtn->Update(t);
	closeBtn->Update(t);
	minimizeBtn->Update(t);
	TextPointerBtn->Update(t);
	PlusBtn->Update(t);

    if(scrollbar->ListChanged())
    {
        MainFileTxt->SetTextLine(scrollbar->GetSelectedItem()+scrollbar->GetSelectedIndex());
    }

	if(t->Right())
	{
	    for(int i = 0; i < linestodraw; i++)
            MainFileTxt->NextLine();
	}
	else if(t->Left())
	{
	    for(int i = 0; i < linestodraw; i++)
            MainFileTxt->PreviousLine();
	}
	else if(t->Down())
	{
        MainFileTxt->NextLine();
	}
	else if(t->Up())
	{
        MainFileTxt->PreviousLine();
	}

	int EntrieCount = (MainFileTxt->GetTotalLinesCount()-linestodraw);

    scrollbar->SetEntrieCount(EntrieCount > 0 ? EntrieCount+1 : 0);
    scrollbar->SetPageSize(linestodraw);
    scrollbar->SetRowSize(0);
    scrollbar->SetSelectedItem(0);
    scrollbar->SetSelectedIndex(MainFileTxt->GetCurrPos());
}