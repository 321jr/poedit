﻿/*
 *  This file is part of Poedit (http://www.poedit.net)
 *
 *  Copyright (C) 2013 Vaclav Slavik
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a
 *  copy of this software and associated documentation files (the "Software"),
 *  to deal in the Software without restriction, including without limitation
 *  the rights to use, copy, modify, merge, publish, distribute, sublicense,
 *  and/or sell copies of the Software, and to permit persons to whom the
 *  Software is furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 *  DEALINGS IN THE SOFTWARE.
 *
 */

#include "welcomescreen.h"

#include "edapp.h"
#include "edframe.h"

#include <wx/dcbuffer.h>
#include <wx/statbmp.h>
#include <wx/stattext.h>
#include <wx/sizer.h>
#include <wx/artprov.h>
#include <wx/font.h>
#include <wx/button.h>
#include <wx/settings.h>
#include <wx/hyperlink.h>
#include <wx/xrc/xmlres.h>

#ifdef __WXOSX__
#include <wx/cocoa/string.h>
#else
#include <wx/commandlinkbutton.h>
#endif

namespace
{

#ifdef __WXOSX__

class ActionButton : public wxButton
{
public:
    ActionButton(wxWindow *parent, wxWindowID winid, const wxString& label, const wxString& note)
        : wxButton(parent, winid, "", wxDefaultPosition, wxSize(350, 50))
    {
        NSButton *btn = (NSButton*)GetHandle();

        NSString *str = wxNSStringWithWxString(label + "\n" + note);
        NSMutableAttributedString *s = [[NSMutableAttributedString alloc] initWithString:str];
        [s addAttribute:NSFontAttributeName
                  value:[NSFont boldSystemFontOfSize:0]
                  range:NSMakeRange(0, label.length())];
        [s addAttribute:NSFontAttributeName
                  value:[NSFont systemFontOfSize:[NSFont systemFontSize]-1.5]
                  range:NSMakeRange(label.length()+1, note.length())];
        [btn setAttributedTitle:s];
        [btn setShowsBorderOnlyWhileMouseInside:YES];
        [btn setBezelStyle:NSSmallSquareBezelStyle];
        [btn setButtonType:NSMomentaryPushInButton];

        SetBackgroundColour(wxColour("#e8fcdb"));
    }
};

#else

typedef wxCommandLinkButton ActionButton;

#endif

} // anonymous namespace


WelcomeScreenBase::WelcomeScreenBase(wxWindow *parent)
    : wxPanel(parent, wxID_ANY),
      m_clrHeader("#444444"),
      m_clrNorm("#444444"),
      m_clrSub("#aaaaaa")
{
#ifdef  __WXMSW__
    SetBackgroundColour(wxColour("#fffcf5"));
#else
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    Bind(wxEVT_PAINT, &WelcomeScreenPanel::OnPaint, this);
#endif

#if defined(__WXMAC__)
    #define HEADER_FACE "Helvetica Neue"
    m_fntHeader = wxFont(wxFontInfo(30).FaceName(HEADER_FACE).Light());
    m_fntNorm = wxFont(wxFontInfo(13).FaceName(HEADER_FACE).Light());
    m_fntSub = wxFont(wxFontInfo(11).FaceName(HEADER_FACE).Light());
#elif defined(__WXMSW__)
    #define HEADER_FACE "Segoe UI"
    m_fntHeader = wxFont(wxFontInfo(24).FaceName("Segoe UI Light"));
    m_fntNorm = wxFont(wxFontInfo(10).FaceName(HEADER_FACE));
    m_fntSub = wxFont(wxFontInfo(9).FaceName(HEADER_FACE));
#else
    #define HEADER_FACE "sans serif"
    m_fntHeader = wxFont(wxFontInfo(30).FaceName(HEADER_FACE).Light());
    m_fntNorm = wxFont(wxFontInfo(12).FaceName(HEADER_FACE).Light());
    m_fntSub = wxFont(wxFontInfo(11).FaceName(HEADER_FACE).Light());
#endif

    // Translate all button events to wxEVT_MENU and send them to the frame.
    Bind(wxEVT_BUTTON, [=](wxCommandEvent& e){
        wxCommandEvent event(wxEVT_MENU, e.GetId());
        event.SetEventObject(this);
        GetParent()->GetEventHandler()->AddPendingEvent(event);
    });
}


void WelcomeScreenBase::OnPaint(wxPaintEvent&)
{
    wxAutoBufferedPaintDC dc(this);
    wxRect rect(dc.GetSize());

    dc.GradientFillLinear(rect, wxColour("#ffffff"), wxColour("#fffceb"), wxBOTTOM);
}




WelcomeScreenPanel::WelcomeScreenPanel(wxWindow *parent)
    : WelcomeScreenBase(parent)
{
    auto sizer = new wxBoxSizer(wxVERTICAL);
    auto uberSizer = new wxBoxSizer(wxHORIZONTAL);
    uberSizer->AddStretchSpacer();
    uberSizer->Add(sizer, wxSizerFlags().Center().Border(wxALL, 50));
    uberSizer->AddStretchSpacer();
    SetSizer(uberSizer);

    auto hdr = new wxStaticBitmap(this, wxID_ANY, wxArtProvider::GetBitmap("PoeditWelcome"), wxDefaultPosition, wxDefaultSize, wxTRANSPARENT_WINDOW);
    sizer->Add(hdr, wxSizerFlags().Center());

    auto header = new wxStaticText(this, wxID_ANY, _("Welcome to Poedit"), wxDefaultPosition, wxDefaultSize, wxTRANSPARENT_WINDOW);
    header->SetFont(m_fntHeader);
    header->SetForegroundColour(m_clrHeader);
    sizer->Add(header, wxSizerFlags().Center().Border(wxTOP, 10));

    auto version = new wxStaticText(this, wxID_ANY, wxString::Format(_("Version %s"), wxGetApp().GetAppVersion()), wxDefaultPosition, wxDefaultSize, wxTRANSPARENT_WINDOW);
    version->SetFont(m_fntSub);
    version->SetForegroundColour(m_clrSub);
    sizer->Add(version, wxSizerFlags().Center());

    sizer->AddSpacer(20);

    sizer->Add(new ActionButton(
                       this, wxID_OPEN,
                       _("Edit a translation"),
                       _("Open an existing PO file and edit the translation.")),
               wxSizerFlags().Border().Expand());

    sizer->Add(new ActionButton(
                       this, XRCID("menu_new_from_pot"),
                       _("Create new translation"),
                       _("Take an existing PO file or POT template and create a new translation from it.")),
               wxSizerFlags().Border().Expand());

    sizer->AddSpacer(50);
}




EmptyPOScreenPanel::EmptyPOScreenPanel(PoeditFrame *parent)
    : WelcomeScreenBase(parent)
{
    auto sizer = new wxBoxSizer(wxVERTICAL);
    auto uberSizer = new wxBoxSizer(wxHORIZONTAL);
    uberSizer->AddStretchSpacer();
    uberSizer->Add(sizer, wxSizerFlags().Center().Border(wxALL, 100));
    uberSizer->AddStretchSpacer();
    SetSizer(uberSizer);

    auto header = new wxStaticText(this, wxID_ANY, _("There are no translations. That’s unusual."));
    header->SetFont(m_fntHeader);
    header->SetForegroundColour(m_clrHeader);
    sizer->Add(header, wxSizerFlags().Center().Border());

    auto explain = new wxStaticText(this, wxID_ANY, _("Translatable entries aren't added manually in the Gettext system, but are automatically extracted\nfrom source code. This way, they stay up to date and accurate.\nTranslators typically use PO template files (POTs) prepared for them by the developer."));
    explain->SetFont(m_fntNorm);
    explain->SetForegroundColour(m_clrNorm);
    sizer->Add(explain, wxSizerFlags());

    auto learnMore = new wxHyperlinkCtrl(this, wxID_ANY, _("(Learn more about GNU gettext)"), "http://www.gnu.org/software/gettext/manual/");
    learnMore->SetFont(m_fntNorm);
    sizer->Add(learnMore, wxSizerFlags().Border(wxTOP|wxBOTTOM).Align(wxALIGN_RIGHT));

    auto explain2 = new wxStaticText(this, wxID_ANY, _("The simplest way to fill this catalog is to update it from a POT:"));
    explain2->SetFont(m_fntNorm);
    explain2->SetForegroundColour(m_clrNorm);
    sizer->Add(explain2, wxSizerFlags().DoubleBorder(wxTOP));

    sizer->Add(new ActionButton(
                       this, XRCID("menu_update_from_pot"),
                       _("Update from POT"),
                       _("Take translatable strings from an existing POT template.")),
               wxSizerFlags().Border().Expand());

    auto explain3 = new wxStaticText(this, wxID_ANY, _("You can also extract translatable strings directly from the source code:"));
    explain3->SetFont(m_fntNorm);
    explain3->SetForegroundColour(m_clrNorm);
    sizer->Add(explain3, wxSizerFlags().DoubleBorder(wxTOP));

    auto btnSources = new ActionButton(
                       this, wxID_ANY,
                       _("Extract from sources"),
                       _("Configure source code extraction in Properties."));
    sizer->Add(btnSources, wxSizerFlags().Border().Expand());

    btnSources->Bind(wxEVT_BUTTON, [=](wxCommandEvent&){
        parent->EditCatalogPropertiesAndUpdateFromSources();
    });
}