#ifndef NEBULOSITY_ARM64_LANGUAGE_COMPAT_H
#define NEBULOSITY_ARM64_LANGUAGE_COMPAT_H

#include <wx/config.h>
#include <wx/intl.h>

// wxLanguage enum numbers changed between the Intel wxWidgets 3.1 build and 3.3.
// Read Intel's original selection without rewriting its preference key.
inline wxLanguage ReadARM64Language(wxConfigBase& config) {
    wxString code;
    if (config.Read("LanguageCodeARM64", &code)) {
        const wxLanguageInfo* info = wxLocale::FindLanguageInfo(code);
        if (info) return static_cast<wxLanguage>(info->Language);
    }
    long legacy = 1; // wxLANGUAGE_UNKNOWN in the bundled wxWidgets 3.1 headers.
    config.Read("Language", &legacy);
    switch (legacy) {
        case 0: case 57: case 58: case 59: case 60:
            return wxLANGUAGE_ENGLISH_US;
        case 55: return wxLANGUAGE_DUTCH;
        case 79: return wxLANGUAGE_FRENCH;
        case 88: return wxLANGUAGE_GERMAN;
        case 93: return wxLANGUAGE_GERMAN_SWISS;
        case 109: return wxLANGUAGE_ITALIAN;
        case 153: return wxLANGUAGE_PORTUGUESE;
        case 179: return wxLANGUAGE_SPANISH;
        case 1: return wxLANGUAGE_UNKNOWN;
        default: return wxLANGUAGE_ENGLISH_US;
    }
}

inline void WriteARM64Language(wxConfigBase& config, wxLanguage language) {
    const wxLanguageInfo* info = wxLocale::GetLanguageInfo(language);
    config.Write("LanguageCodeARM64", info ? info->CanonicalName : wxString("en_US"));
}
#endif
