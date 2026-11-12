#include <wx/fileconf.h>
#include <wx/intl.h>
#include <wx/log.h>
#include <cstdio>
#include "../Language_compat.h"
int main(int argc, char** argv) {
    wxFileConfig config("test", "", wxString::FromUTF8(argv[1]), "", wxCONFIG_USE_LOCAL_FILE);
    config.SetPath("/Preferences");
    const long legacy[] = {59,88,93,55,79,109,153,179};
    const wxLanguage expected[] = {wxLANGUAGE_ENGLISH_US,wxLANGUAGE_GERMAN,wxLANGUAGE_GERMAN_SWISS,wxLANGUAGE_DUTCH,wxLANGUAGE_FRENCH,wxLANGUAGE_ITALIAN,wxLANGUAGE_PORTUGUESE,wxLANGUAGE_SPANISH};
    for (int i=0;i<8;++i) {
        config.DeleteEntry("LanguageCodeARM64"); config.Write("Language", legacy[i]);
        if (ReadARM64Language(config)!=expected[i]) return 1;
        WriteARM64Language(config,expected[i]);
        long original=0;config.Read("Language",&original);
        if(original!=legacy[i] || ReadARM64Language(config)!=expected[i])return 2;
    }
    std::puts("Legacy language migration and stable-code round-trip: PASS; Intel key unchanged");
    wxLocale::AddCatalogLookupPathPrefix(wxString::FromUTF8(argv[2]));
    {wxLocale locale;if(!locale.Init(wxLANGUAGE_ENGLISH_US))return 3;std::puts("English locale initialization: PASS");}
    {wxLocale locale;if(!locale.Init(wxLANGUAGE_GERMAN))return 4;if(!locale.AddCatalog("nebulosity"))return 5;std::puts("German locale and bundled translation: PASS");}
    return 0;
}
