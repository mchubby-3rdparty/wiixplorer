/***************************************************************************
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
 * ProcessChoice.cpp
 *
 * for WiiXplorer 2009
 ***************************************************************************/

#include "Prompts/PromptWindows.h"
#include "Prompts/ProgressWindow.h"
#include "FileStartUp/FileStartUp.h"
#include "Language/gettext.h"
#include "filebrowser.h"
#include "fileops.h"

/*** Extern functions ***/
extern CLIPBOARD Clipboard;

void ProcessChoice(int choice)
{
    if(choice == -1)
        return;

    else if(strcmp(browserList[browser.selIndex].filename,"..") != 0)
    {
        if(choice == CUT) {
            if(browserList[browser.selIndex].isdir)
                choice = WindowPrompt(browserList[browser.selIndex].filename, tr("Cut directory?"), tr("Yes"), tr("Cancel"));
            else
                choice = WindowPrompt(browserList[browser.selIndex].filename, tr("Cut file?"), tr("Yes"), tr("Cancel"));
            if(choice == 1) {
                sprintf(Clipboard.filepath, "%s%s", browser.rootdir, browser.dir);
                sprintf(Clipboard.filename, "%s", browserList[browser.selIndex].filename);
                if(browserList[browser.selIndex].isdir)
                    Clipboard.isdir = true;
                else
                    Clipboard.isdir = false;
                Clipboard.cutted = true;
            }
        }

        else if(choice == COPY) {
            if(browserList[browser.selIndex].isdir)
                choice = WindowPrompt(browserList[browser.selIndex].filename, tr("Copy directory?"), tr("Yes"), tr("Cancel"));
            else
                choice = WindowPrompt(browserList[browser.selIndex].filename, tr("Copy file?"), tr("Yes"), tr("Cancel"));
            if(choice == 1) {
                sprintf(Clipboard.filepath, "%s%s", browser.rootdir, browser.dir);
                sprintf(Clipboard.filename, "%s", browserList[browser.selIndex].filename);
                if(browserList[browser.selIndex].isdir)
                    Clipboard.isdir = true;
                else
                    Clipboard.isdir = false;
                Clipboard.cutted = false;
            }
        }

        else if(choice == RENAME) {
            char srcpath[MAXPATHLEN];
            char destdir[MAXPATHLEN];
            snprintf(srcpath, sizeof(srcpath), "%s/%s/%s", browser.rootdir, browser.dir, browserList[browser.selIndex].filename);
            char entered[151];
            snprintf(entered, sizeof(entered), "%s", browserList[browser.selIndex].filename);
            int result = OnScreenKeyboard(entered, 150);
            if(result == 1) {
                snprintf(destdir, sizeof(destdir), "%s%s/%s", browser.rootdir, browser.dir, entered);
                int ret = rename(srcpath, destdir);
                if(ret != 0)
                    WindowPrompt(tr("Failed renaming file"), tr("Name already exists."), tr("OK"));
                ParseDirectory();
            }
        }

        else if(choice == DELETE) {
            if(browserList[browser.selIndex].isdir) {
                char currentpath[MAXPATHLEN];
                snprintf(currentpath, sizeof(currentpath), "%s%s/%s/", browser.rootdir, browser.dir, browserList[browser.selIndex].filename);
                choice = WindowPrompt(browserList[browser.selIndex].filename, tr("Delete directory and its content?"), tr("Yes"), tr("Cancel"));
                if(choice == 1) {
                    StartProgress(tr("Deleting files:"), THROBBER);
                    int res = RemoveDirectory(currentpath);
                    StopProgress();
                    if(res == -10)
                        WindowPrompt(tr("Deleting files:"), tr("Action cancelled."), tr("OK"));
                    else if(res < 0)
                        WindowPrompt(tr("Error"), tr("Directory couldn't be deleted."), tr("OK"));
                    else
                        WindowPrompt(tr("Directory successfully deleted."), 0, tr("OK"));
                    ParseDirectory();
                }
            } else {
                char currentpath[MAXPATHLEN];
                snprintf(currentpath, sizeof(currentpath), "%s%s/%s", browser.rootdir, browser.dir, browserList[browser.selIndex].filename);
                choice = WindowPrompt(browserList[browser.selIndex].filename, tr("Delete this file?"), tr("Yes"), tr("Cancel"));
                if(choice == 1) {
                    if(RemoveFile(currentpath) == false) {
                        WindowPrompt(tr("Error"), tr("File couldn't be deleted."), tr("OK"));
                    }
                    ParseDirectory();
                }
            }
        }

        else if(choice == PROPERTIES) {
            char currentitem[MAXPATHLEN];
            snprintf(currentitem, sizeof(currentitem), "%s%s/", browser.rootdir, browser.dir);
            Properties(browserList[browser.selIndex].filename, currentitem, browserList[browser.selIndex].isdir, (float) browserList[browser.selIndex].length);
        }

    } else if(choice >= 0 && choice != PASTE && choice != NEWFOLDER)
        WindowPrompt(tr("You cant use this operation on:"), tr("Directory .."), tr("OK"));

    if(choice == PASTE) {
        choice = WindowPrompt(Clipboard.filename, tr("Paste into current directory?"), tr("Yes"), tr("Cancel"));
        if(choice == 1) {
            char srcpath[MAXPATHLEN];
            char destdir[MAXPATHLEN];
            if(Clipboard.isdir == true) {
                snprintf(srcpath, sizeof(srcpath), "%s/%s/", Clipboard.filepath, Clipboard.filename);
                snprintf(destdir, sizeof(destdir), "%s%s/%s/", browser.rootdir, browser.dir, Clipboard.filename);
                int res = 0;
                if(Clipboard.cutted == false) {
                    StartProgress(tr("Copying files:"));
                    res = CopyDirectory(srcpath, destdir);
                    StopProgress();
                } else {
                    if(strcmp(srcpath, destdir) != 0) {
                        if(CompareDevices(srcpath, destdir))
                            StartProgress(tr("Moving files:"), THROBBER);
                        else
                            StartProgress(tr("Moving files:"));
                        res = MoveDirectory(srcpath, destdir);
                        StopProgress();
                    } else {
                        WindowPrompt(tr("Error:"), tr("You can not cut into the directory itself.") , tr("OK"));
                        res =  -1;
                    }
                }
                if(res == -10)
                    WindowPrompt(tr("Transfering files:"), tr("Action cancelled."), tr("OK"));
                else if(res < 0)
                    WindowPrompt(tr("An error occured."), tr("Failed copying files."), tr("OK"));
                else {
                    if(Clipboard.cutted == false)
                        WindowPrompt(tr("Directory successfully copied."), 0, tr("OK"));
                    else {
                        WindowPrompt(tr("Directory successfully moved."), 0, tr("OK"));
                    }
                }
            } else {
                snprintf(srcpath, sizeof(srcpath), "%s/%s", Clipboard.filepath, Clipboard.filename);
                int ret = CheckFile(srcpath);
                if(ret == false)
                    WindowPrompt(tr("File does not exist anymore!"), 0, tr("OK"));
                else {
                    snprintf(destdir, sizeof(destdir), "%s%s/%s", browser.rootdir, browser.dir, Clipboard.filename);
                    if(strcmp(srcpath, destdir) != 0) {
                        StartProgress(tr("Copying file:"));
                        int res = 0;
                        if(Clipboard.cutted == false)
                            res = CopyFile(srcpath, destdir);
                        else
                            res = MoveFile(srcpath, destdir);
                        StopProgress();
                        if(res < 0)
                            WindowPrompt(tr("ERROR"), tr("Failed copying file."), tr("OK"));
                        else {
                            if(Clipboard.cutted == false)
                                WindowPrompt(tr("File successfully copied."), 0, tr("OK"));
                            else {
                                WindowPrompt(tr("File successfully moved."), 0, tr("OK"));
                            }
                        }
                    } else {
                        WindowPrompt(tr("Error:"), tr("You cannot read/write from/to the same file."), tr("OK"));
                    }
                }
            }
            ParseDirectory();
        }
    }

    else if(choice == NEWFOLDER) {
        char entered[151];
        snprintf(entered, sizeof(entered), tr("New Folder"));
        int result = OnScreenKeyboard(entered, 150);
        if(result == 1) {
            char currentpath[MAXPATHLEN];
            snprintf(currentpath, sizeof(currentpath), "%s%s/%s/", browser.rootdir, browser.dir, entered);
            bool ret = CreateSubfolder(currentpath);
            if(ret == false)
                WindowPrompt(tr("Error:"), tr("Unable to create folder."), tr("OK"));
            ParseDirectory();
        }
    }
}