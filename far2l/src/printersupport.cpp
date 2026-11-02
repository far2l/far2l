/*
printersupport.cpp

Print capabilities from wxWidgets (if any).
*/
/*
Copyright (c) 1996 Eugene Roshal
Copyright (c) 2000 Far Group
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. The name of the authors may not be used to endorse or promote products
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "headers.hpp"

#include "printersupport.hpp"
#include <Colorspace.h>

////////////////////////////
// Bridge to backend

bool PrinterSupport::IsReducedHTMLSupported()
{
	return WINPORT(PrintIsHTMLSupported)();
}

bool PrinterSupport::IsPrinterSetupDialogSupported()
{
	return WINPORT(PrintIsSettingsDialogSupported)();
}

bool PrinterSupport::IsPrintPreviewSupported()
{
	return WINPORT(PrintIsPreviewSupported)();
}

void PrinterSupport::PrintText(const std::wstring& jobName, const std::wstring& text) 
{
	WINPORT(PrintTextFragment)(jobName.c_str(), text.c_str());
}

void PrinterSupport::PrintReducedHTML(const std::wstring& jobName, const std::wstring& text)
{
	WINPORT(PrintHtmlFragment)(jobName.c_str(), text.c_str());
}

void PrinterSupport::PrintTextFile(const std::wstring& fileName)
{
	WINPORT(PrintTextFile)(fileName.c_str());
}

void PrinterSupport::PrintHtmlFile(const std::wstring& fileName)
{
	WINPORT(PrintHtmlFile)(fileName.c_str());
}

void PrinterSupport::ShowPreviewForText(const std::wstring& jobName, const std::wstring& text)
{
	WINPORT(PrintPreviewTextFragment)(jobName.c_str(), text.c_str());
}

void PrinterSupport::ShowPreviewForReducedHTML(const std::wstring& jobName, const std::wstring& text)
{
	WINPORT(PrintPreviewHtmlFragment)(jobName.c_str(), text.c_str());
}

void PrinterSupport::ShowPreviewForTextFile(const std::wstring& fileName)
{
	WINPORT(PrintPreviewTextFile)(fileName.c_str());
}

void PrinterSupport::ShowPreviewForHtmlFile(const std::wstring& fileName)
{
	WINPORT(PrintPreviewHtmlFile)(fileName.c_str());
}

void PrinterSupport::ShowPrinterSetupDialog()
{
	WINPORT(PrintSettingsDialog)();
}

FarTrueColor ColorspaceSupport::ConvertForPrintLAB(const FarTrueColor& fga, const FarTrueColor& bga) {
	iRGB fg = toIRGB(fga.R, fga.G, fga.B);
	iRGB bg = toIRGB(bga.R, bga.G, bga.B);

	iRGB r = ::ConvertForPrintLAB(fg, bg);

	FarTrueColor x;
	x.R = r.r;
	x.G = r.g;
	x.B = r.b;
    return x;
}

FILE* PrinterSupport::BeginPrint() 
{
	fprintf(stderr, "Printer caps: HTML=%c, preview=%c, setup dialog=%c\n",
		IsReducedHTMLSupported() ? 'Y' : 'N',
		IsPrintPreviewSupported() ? 'Y' : 'N',
		IsPrinterSetupDialogSupported() ?  'Y' : 'N');

	_fileName = InMyTemp(StrPrintf("print/%d", getpid()).c_str());

	FILE* fp = fopen(_fileName.c_str(), "a+");
	if (fp && IsReducedHTMLSupported()) {
		fputs(HTML_PRE_HEADER, fp);
	}

	return fp;
}

void PrinterSupport::EndPrint(FILE* fp) 
{
	if (IsReducedHTMLSupported()) {
		fputs(HTML_PRE_FOOTER, fp);
	}
	fclose(fp);
    
    std::wstring ws_fileName;
    StrMB2Wide(_fileName, ws_fileName);

	if (IsPrintPreviewSupported()) {
		if (IsReducedHTMLSupported()) 
			ShowPreviewForHtmlFile(ws_fileName);
		else 
			ShowPreviewForTextFile(ws_fileName);
	}
	else {
		if (IsReducedHTMLSupported()) 
			PrintHtmlFile(ws_fileName);
		else
			PrintTextFile(ws_fileName);
	}

	_fileName.clear();
}

void PrinterSupport::PrintRawFile(const wchar_t* fileName)
{
	if (IsPrintPreviewSupported()) {
		ShowPreviewForTextFile(fileName);
	} else {
		PrintTextFile(fileName);
	}
}

