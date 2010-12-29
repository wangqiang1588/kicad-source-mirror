/************************************************/
/*			GERBVIEW		 main file			*/
/************************************************/

#include "fctsys.h"
#include "common.h"
#include "appl_wxstruct.h"
#include "class_drawpanel.h"
#include "confirm.h"
#include "gestfich.h"

#include "gerbview.h"
#include "gerbview_id.h"
#include "wxGerberFrame.h"
#include "pcbplot.h"
#include "bitmaps.h"
#include "zones.h"
#include "class_board_design_settings.h"
#include "colors_selection.h"
#include "hotkeys.h"

#include "build_version.h"

#include <wx/file.h>
#include <wx/snglinst.h>

extern bool Read_Config();

wxString g_PhotoFilenameExt;
wxString g_DrillFilenameExt;
wxString g_PenFilenameExt;

// Colors for layers and items
COLORS_DESIGN_SETTINGS g_ColorsSettings;

int      g_Default_GERBER_Format;
int      g_DisplayPolygonsModeSketch;

const wxString GerbviewProjectFileExt( wxT( "cnf" ) );
const wxString GerbviewProjectFileWildcard( _( "GerbView project files (.cnf)|*.cnf" ) );

// Config keywords
const wxString GerbviewDrawModeOption( wxT( "DrawModeOption" ) );
const wxString GerbviewShowPageSizeOption( wxT( "ShowPageSizeOpt" ) );
const wxString GerbviewShowDCodes( wxT( "ShowDCodesOpt" ) );

GERBER_IMAGE*  g_GERBER_List[32];

// List of page sizes
Ki_PageDescr* g_GerberPageSizeList[] =
{
    &g_Sheet_GERBER,    // Full size page selection, and do not show page limits
    &g_Sheet_GERBER,    // Full size page selection, and show page limits
    &g_Sheet_A4,   &g_Sheet_A3, &g_Sheet_A2,
    &g_Sheet_A,    &g_Sheet_B,  &g_Sheet_C,
    NULL                // End of list
 };


IMPLEMENT_APP( WinEDA_App )

/* MacOSX: Needed for file association
 * http://wiki.wxwidgets.org/WxMac-specific_topics
 */
void WinEDA_App::MacOpenFile(const wxString &fileName)
{
    wxFileName           filename = fileName;
    WinEDA_GerberFrame * frame = ((WinEDA_GerberFrame*)GetTopWindow());

    if( !filename.FileExists() )
        return;

    frame->LoadGerberFiles( fileName );
}


bool WinEDA_App::OnInit()
{
    wxFileName          fn;
    WinEDA_GerberFrame* frame = NULL;

#ifdef __WXMAC__
    wxApp::s_macAboutMenuItemId = ID_KICAD_ABOUT;
    wxApp::s_macPreferencesMenuItemId = ID_GERBVIEW_OPTIONS_SETUP;
#endif /* __WXMAC__ */

    InitEDA_Appl( wxT( "GerbView" ), APP_TYPE_GERBVIEW );

    if( m_Checker && m_Checker->IsAnotherRunning() )
    {
        if( !IsOK( NULL, _( "GerbView is already running. Continue?" ) ) )
            return false;
    }
    ScreenPcb = new PCB_SCREEN();
    ScreenPcb->m_CurrentSheetDesc = &g_Sheet_GERBER;
    ActiveScreen = ScreenPcb;

    // read current setup and reopen last directory if no filename to open in
    // command line
    bool reopenLastUsedDirectory = argc == 1;
    GetSettings( reopenLastUsedDirectory );

    extern PARAM_CFG_BASE* ParamCfgList[];
    wxGetApp().ReadCurrentSetupValues( ParamCfgList );

    g_DrawBgColor = BLACK;

   /* Must be called before creating the main frame in order to
    * display the real hotkeys in menus or tool tips */
    ReadHotkeyConfig( wxT("GerberFrame"), s_Gerbview_Hokeys_Descr );

    frame = new  WinEDA_GerberFrame( NULL, wxT( "GerbView" ),
                                     wxPoint( 0, 0 ),
                                     wxSize( 600, 400 ) );

    /* Gerbview mainframe title */
    frame->SetTitle( GetTitle() + wxT( " " ) + GetBuildVersion() );

    // Initialize some display options
    DisplayOpt.DisplayPadIsol = false;      // Pad clearance has no meaning
                                            // here
    DisplayOpt.ShowTrackClearanceMode = 0;  // tracks and vias clearance has no
                                            // meaning here

    SetTopWindow( frame );                  // Set GerbView mainframe on top
    frame->Show( TRUE );                    // Show GerbView mainframe
    frame->Zoom_Automatique( TRUE );        // Zoomfit drawing in frame

    Read_Config();

    if( argc <= 1 )
        return true;

    fn = argv[1];

    if( fn.IsOk() )
    {
        if( fn.DirExists() )
            wxSetWorkingDirectory( fn.GetPath() );

        // Load all files specified on the command line.
        for( int ii = 1; ii < argc && ii <= LAYER_COUNT; ++ii )
        {
            fn = wxFileName( argv[ii] );

            if( fn.FileExists() )
            {
                ( (PCB_SCREEN*) frame->GetScreen() )->m_Active_Layer = ii - 1;
                frame->LoadGerberFiles( fn.GetFullPath() );
            }
        }
    }

    return true;
}
