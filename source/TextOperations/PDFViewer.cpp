/****************************************************************************
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
 * for WiiXplorer 2010
 ***************************************************************************/
#include <gccore.h>
#include <malloc.h>
#include "Controls/MainWindow.h"
#include "Controls/Taskbar.h"
#include "PDFViewer.hpp"
#include "menu.h"
#include "sys.h"

PDFViewer::PDFViewer(const char * filepath, const char * password)
    : ImageViewer(filepath)
{
	OutputImage = NULL;
	currentPage = 1;
	drawzoom = Settings.PDFLoadZoom;
	drawrotate = 0;
    drawcache = (fz_glyphcache*) nil;
    drawpage = (pdf_page*) nil;

	fz_cpudetect();
	fz_accelerate();
    closexref();
    Setup();

    OpenFile(filepath, password);
    LoadPage(currentPage);
}

PDFViewer::~PDFViewer()
{
    CloseFile();
}

void PDFViewer::OpenFile(const char * filepath, const char * password)
{
    CloseFile();
    drawcache = fz_newglyphcache();
    openxref((char *) filepath, (char *) password, 0);
    if(xref)
        PageCount = pdf_getpagecount(xref);
}

void PDFViewer::CloseFile()
{
    FreePage();

	if (xref && xref->store)
	{
		pdf_dropstore(xref->store);
		xref->store = (pdf_store_s*) nil;
	}
	if (drawcache)
	{
		fz_freeglyphcache(drawcache);
		drawcache = (fz_glyphcache*) nil;
	}

	closexref();
}

int PDFViewer::MainUpdate()
{
    if(shutdown)
        Sys_Shutdown();
    else if(reset)
        Sys_Reboot();

    if(Taskbar::Instance()->GetMenu() != MENU_NONE)
        return -1;

    return ImageViewer::MainUpdate();
}

void PDFViewer::SetImageSize(float scale)
{
    if(!image)
        return;

    if(scale < 0.05f)
        scale = 0.05f;
    else if(scale > 2.5f)
        scale = 2.5f;

    image->SetScale(scale);
}

int PDFViewer::PreparePage(int pagenum)
{
	fz_error error;
	fz_obj * pageobj = pdf_getpageobject(xref, pagenum);
	error = pdf_loadpage(&drawpage, xref, pageobj);
	if (error)
    {
        ShowError(tr("Can't load page."));
        return -1;
    }

    return 0;
}

bool PDFViewer::LoadPage(int pagenum)
{
    if(!xref)
        return false;

    FreePage();

    int ret = PreparePage(pagenum);
    if(ret < 0)
    {
        CloseFile();
        return false;
    }

    ret = PageToRGBA8(pagenum);
    if(ret <= 0)
    {
        CloseFile();
        return false;
    }

	MainWindow::Instance()->HaltGui();
	RemoveAll();
	image = new GuiImage(OutputImage, imagewidth, imageheight);
	image->SetAlignment(ALIGN_CENTRE, ALIGN_TOP);

	Append(backGround);
	Append(image);
	Append(backButton);
	Append(zoominButton);
	Append(zoomoutButton);
	Append(rotateRButton);
	Append(rotateLButton);
    Append(nextButton);
    Append(prevButton);
    Append(slideshowButton);
	Append(moveButton);
	MainWindow::Instance()->ResumeGui();

	return true;
}

bool PDFViewer::NextPage()
{
    ++currentPage;

    if(currentPage > PageCount)
        currentPage = 1;

    return LoadPage(currentPage);
}

bool PDFViewer::PreviousPage()
{
    --currentPage;
    if(currentPage < 1)
        currentPage = PageCount;

    return LoadPage(currentPage);
}

void PDFViewer::FreePage()
{
	MainWindow::Instance()->HaltGui();
	Remove(image);
	if(image)
        delete image;
    image = NULL;

    if(OutputImage)
        free(OutputImage);
	OutputImage = NULL;
	MainWindow::Instance()->ResumeGui();

    if(drawpage != nil)
        pdf_droppage(drawpage);
    drawpage = (pdf_page *) nil;
    flushxref();
    if (xref && xref->store)
    {
        pdf_agestoreditems(xref->store);
        pdf_evictageditems(xref->store);
    }
}

int PDFViewer::PageToRGBA8(int pagenum)
{
    fz_error error;
	fz_matrix ctm;
	fz_bbox bbox;
	fz_pixmap *pix;
	int x, y, w, h;

	ctm = fz_identity();
	ctm = fz_concat(ctm, fz_translate(0, -drawpage->mediabox.y1));
	ctm = fz_concat(ctm, fz_scale(drawzoom, -drawzoom));
	ctm = fz_concat(ctm, fz_rotate(drawrotate + drawpage->rotate));

	bbox = fz_roundrect(fz_transformrect(ctm, drawpage->mediabox));
	w = bbox.x1 - bbox.x0;
	h = bbox.y1 - bbox.y0;

    if(w % 4)
        w += 4 - w % 4;

    if(h % 4)
        h += 4 - h % 4;

	pix = fz_newpixmap(pdf_devicergb, bbox.x0, bbox.y0, w, h);
	fz_clearpixmap(pix, 0xFF);
	memset(pix->samples, 0xff, pix->h * pix->w * pix->n);

    int len =  ((pix->w+3)>>2)*((pix->h+3)>>2)*32*2;
    if(len%32)
        len += (32-len%32);

    OutputImage = (u8 *) memalign(32, len);
    if(!OutputImage)
    {
        fz_droppixmap(pix);
        FreePage();
        ShowError(tr("Not enough memory."));
        return -1;
    }

    u8 * dst = OutputImage;

    fz_device *dev = fz_newdrawdevice(drawcache, pix);
    error = pdf_runcontentstream(dev, ctm, xref, drawpage->resources, drawpage->contents);
    fz_freedevice(dev);
    if (error)
    {
        fz_droppixmap(pix);
        FreePage();
        return -1;
    }

    imagewidth = pix->w;
    imageheight = pix->h;

    for (y = 0; y < pix->h; y++)
    {
        unsigned char *src = pix->samples + y * pix->w * 4;

        for (x = 0; x < pix->w; x++)
        {
            u32 offset = ((((y >> 2) * (pix->w >> 2) + (x >> 2)) << 5) + ((y & 3) << 2) + (x & 3)) << 1;

            dst[offset] = src[x * 4 + 0];
            dst[offset+1] = src[x * 4 + 1];
            dst[offset+32] = src[x * 4 + 2];
            dst[offset+33] = src[x * 4 + 3];
        }
    }
    DCFlushRange(dst, len);

	fz_droppixmap(pix);

    if(drawpage != nil)
        pdf_droppage(drawpage);
    drawpage = (pdf_page *) nil;
    flushxref();
    if (xref && xref->store)
    {
        pdf_agestoreditems(xref->store);
        pdf_evictageditems(xref->store);
    }

	return len;
}