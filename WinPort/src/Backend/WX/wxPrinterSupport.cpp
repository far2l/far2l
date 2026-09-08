#include <wx/wx.h>
#include <wx/display.h>

#if defined(__APPLE__)
#define MAC_NATIVE_PRINTING // comment this define if you want wxWidgets-aware dialogs
#endif

#if defined(__APPLE__)
	#include <TargetConditionals.h>
	#include<AvailabilityMacros.h>
	#if TARGET_OS_MAC
		#if defined(__GNUC__) && !defined(__clang__)
//			#if MAC_OS_X_VERSION_MIN_REQUIRED  < 1070
				// macOS < 10.7 + GCC
				#undef MAC_NATIVE_PRINTING
//			#endif
		#endif
	#endif
#endif

#ifndef MAC_NATIVE_PRINTING
#include <wx/html/htmprint.h>
#include <wx/richtext/richtextprint.h>
#include <wx/event.h>
#endif

#include <string.h>

#include "wxPrinterSupport.h"
#include "CallInMain.h"

#ifdef MAC_NATIVE_PRINTING
#include "Mac/printing.h"
#endif

#include "wxMain.h"

class PreviewWatcher : public wxEvtHandler
{
public:
    PreviewWatcher(wxWindow* parent) : m_parent(parent) 
    {
        wxTheApp->Bind(wxEVT_CREATE, &PreviewWatcher::OnWindowCreate, this);
    }

    ~PreviewWatcher()
    {
        wxTheApp->Unbind(wxEVT_CREATE, &PreviewWatcher::OnWindowCreate, this);
    }

private:

	template<typename T> T* FindChildOfType(wxWindow* parent) {
	    for (wxWindow* w : parent->GetChildren()) {
	        if (auto p = wxDynamicCast(w, T))
    	        return p;

        	if (auto p = FindChildOfType<T>(w))
            	return p;
    	}
    	return nullptr;
	}

	void OnClose(wxCloseEvent& evt) 
	{ 
		evt.Skip(); 
		if (m_parent) {
			WinPortPanel* panel = FindChildOfType<WinPortPanel>(m_parent);
			if (panel) panel->SetFocus();
		}
        m_tracked -> Unbind(wxEVT_CLOSE_WINDOW, &PreviewWatcher::OnClose, this);
        delete this;
	}

    void OnWindowCreate(wxWindowCreateEvent& evt)
    {
        wxWindow* win = evt.GetWindow();
       	wxClassInfo* ci =  win->GetClassInfo();

        if (win->IsTopLevel() && ci && wxString(ci->GetClassName()).Contains("wxPreviewFrame"))
        {
            // Make it behave like a floating tool window
            /*
            long style = win->GetWindowStyle();
            style |= wxSTAY_ON_TOP;
            style |= wxFRAME_TOOL_WINDOW;
            win->SetWindowStyle(style);
            */
            m_tracked = win;

            win->Bind(wxEVT_SHOW, &PreviewWatcher::OnPreviewShown, this);
            win->Bind(wxEVT_CLOSE_WINDOW, &PreviewWatcher::OnClose, this);
        }

        evt.Skip();
    }

    void OnPreviewShown(wxShowEvent& evt)
    {
        wxWindow* win = static_cast<wxWindow*>(evt.GetEventObject());

        if (evt.IsShown())
        {
            win->Raise();          // Now safe
            win->SetFocus();       // Now safe
            win->CentreOnParent(); // Now safe

            // m_parent->Enable(true);
            win->Unbind(wxEVT_SHOW, &PreviewWatcher::OnPreviewShown, this);
        }

        evt.Skip();
    }

    wxWindow* m_parent;
    wxWindow* m_tracked;
};

void wxPrinterSupportBackend::ensurePrinterCreated () {
#ifndef MAC_NATIVE_PRINTING
	if (!html_printer) {
		wxWindow* top = wxTheApp->GetTopWindow();
		html_printer = new wxHtmlEasyPrinting("Printing", top);
		html_printer->SetStandardFonts(10 /*, "Arial", "Lucida Console" */);
		// new PreviewWatcher(top);
	}

    /*
	if (!text_printer) {
		wxWindow* top = wxTheApp->GetTopWindow();
		text_printer = new wxRichTextPrinting("Printing", top);
		// text_printer->SetStandardFonts(10);
		// new PreviewWatcher(top);
	}*/
#endif
}

wxPrinterSupportBackend::wxPrinterSupportBackend() : html_printer(nullptr), text_printer(nullptr) 
{
}

wxPrinterSupportBackend::~wxPrinterSupportBackend() {
#ifndef MAC_NATIVE_PRINTING
	if (html_printer) delete html_printer;
	html_printer = nullptr;
	// if (text_printer) delete text_printer;
	// text_printer = nullptr;
#endif
}

void wxPrinterSupportBackend::PrintText(const wchar_t* jobName, const wchar_t* text)
{
	if (!wxIsMainThread()) {
		auto fn = std::bind(&wxPrinterSupportBackend::PrintText, this, jobName, text);
		CallInMainNoRet(fn);
		return;
	}

	ensurePrinterCreated();

	wxString wxText(text); 

#ifndef MAC_NATIVE_PRINTING
	/*
	wxRichTextBuffer buffer; 
	wxArrayString lines = wxSplit(wxText, '\n'); 
	
	for (auto& line : lines) buffer.AddParagraph(line);

	text_printer->PrintBuffer(buffer);
    */
    PrintReducedHTML(jobName, text);
#else
	MacNativePrintText(wxText);
#endif
}

void wxPrinterSupportBackend::PrintReducedHTML(const wchar_t* jobName, const wchar_t* text)
{
	if (!wxIsMainThread()) {
		auto fn = std::bind(&wxPrinterSupportBackend::PrintReducedHTML, this, jobName, text);
		CallInMainNoRet(fn);
		return;
	}

	ensurePrinterCreated ();

#ifndef MAC_NATIVE_PRINTING
	html_printer->PrintText(text);
#else
	wxString wxText(text); 
	MacNativePrintHtml(wxText);
#endif
}

void wxPrinterSupportBackend::PrintTextFile(const wchar_t* fileName)
{
	if (!wxIsMainThread()) {
		auto fn = std::bind(&wxPrinterSupportBackend::PrintTextFile, this, fileName);
		CallInMainNoRet(fn);
		return;
	}

	ensurePrinterCreated();

#ifndef MAC_NATIVE_PRINTING
	// text_printer->PrintFile(fileName);
	PrintHtmlFile(fileName);
#else
	wxString wxText(fileName); 
	MacNativePrintTextFile(wxText);
#endif
}

void wxPrinterSupportBackend::PrintHtmlFile(const wchar_t* fileName)
{
	if (!wxIsMainThread()) {
		auto fn = std::bind(&wxPrinterSupportBackend::PrintHtmlFile, this, fileName);
		CallInMainNoRet(fn);
		return;
	}

	ensurePrinterCreated ();

#ifndef MAC_NATIVE_PRINTING
	// wxHtmlEasyPrinting html_printer(fileName);
	// html_printer.PrintFile(fileName);
	html_printer->PrintFile(fileName);
#else
	wxString wxText(fileName); 
	MacNativePrintHtmlFile(wxText);
#endif
}

void wxPrinterSupportBackend::ShowPreviewForText(const wchar_t* jobName, const wchar_t* text)
{
	if (!wxIsMainThread()) {
		auto fn = std::bind(&wxPrinterSupportBackend::ShowPreviewForText, this, jobName, text);
		CallInMainNoRet(fn);
		return;
	}

	ensurePrinterCreated();

#ifndef MAC_NATIVE_PRINTING
	/*
	wxRichTextBuffer buffer; 
	wxString wxText(text); 
	wxArrayString lines = wxSplit(wxText, '\n'); 
	for (auto& line : lines) buffer.AddParagraph(line);

	text_printer->PreviewBuffer(buffer);
    */
    ShowPreviewForReducedHTML(jobName, text);
#else
	wxString wxText(text); 
	MacNativePrintPreviewText(wxText);
#endif
}

void wxPrinterSupportBackend::ShowPreviewForReducedHTML(const wchar_t* jobName, const wchar_t* text)
{
	if (!wxIsMainThread()) {
		auto fn = std::bind(&wxPrinterSupportBackend::ShowPreviewForReducedHTML, this, jobName, text);
		CallInMainNoRet(fn);
		return;
	}

	ensurePrinterCreated ();

#ifndef MAC_NATIVE_PRINTING
	// wxHtmlEasyPrinting html_printer(jobName);
	// html_printer.PreviewText(text);
	html_printer->PreviewText(text);
#else
	wxString wxText(text); 
	MacNativePrintPreviewHtml(wxText);
#endif
}

void wxPrinterSupportBackend::ShowPreviewForTextFile(const wchar_t* fileName)
{
	if (!wxIsMainThread()) {
		auto fn = std::bind(&wxPrinterSupportBackend::ShowPreviewForTextFile, this, fileName);
		CallInMainNoRet(fn);
		return;
	}

	ensurePrinterCreated();

	fprintf(stderr, "wxPrinterSupportBackend::ShowPreviewForTextFile(`%ls`)\n", fileName);

#ifndef MAC_NATIVE_PRINTING
	//text_printer->PreviewFile(fileName);
	// text_printer->PrintFile(fileName, true);
	ShowPreviewForHtmlFile(fileName);
#else
	wxString wxText(fileName); 
	MacNativePrintPreviewTextFile(wxText);
#endif
}

void wxPrinterSupportBackend::ShowPreviewForHtmlFile(const wchar_t* fileName)
{
	if (!wxIsMainThread()) {
		auto fn = std::bind(&wxPrinterSupportBackend::ShowPreviewForHtmlFile, this, fileName);
		CallInMainNoRet(fn);
		return;
	}

	ensurePrinterCreated ();

#ifndef MAC_NATIVE_PRINTING
	// wxHtmlEasyPrinting html_printer(fileName);
	// html_printer.PreviewFile(fileName);
	html_printer->PreviewFile(fileName);
#else
	wxString wxText(fileName); 
	MacNativePrintPreviewHtmlFile(wxText);
#endif
}

void wxPrinterSupportBackend::ShowPrinterSetupDialog()
{
	if (!wxIsMainThread()) {
		auto fn = std::bind(&wxPrinterSupportBackend::ShowPrinterSetupDialog, this);
		CallInMainNoRet(fn);
		return;
	}

	ensurePrinterCreated ();

#ifndef MAC_NATIVE_PRINTING
	// wxHtmlEasyPrinting html_printer("Printer Setup");
	// html_printer.PageSetup();
	html_printer->PageSetup();
#else
	MacNativeShowPageSetupDialog();
#endif
}

bool wxPrinterSupportBackend::IsPrintPreviewSupported(){ return true; }
bool wxPrinterSupportBackend::IsReducedHTMLSupported(){ return true; }
bool wxPrinterSupportBackend::IsPrinterSetupDialogSupported(){ return true; }
