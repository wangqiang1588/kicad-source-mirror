/*
 * This program source code file is part of KiCad, a free EDA CAD application.
 *
 * Copyright (C) 2012 NBEE Embedded Systems, Miguel Angel Ajo <miguelangel@nbee.es>
 * Copyright (C) 1992-2017 KiCad Developers, see AUTHORS.txt for contributors.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, you may find one here:
 * http://www.gnu.org/licenses/old-licenses/gpl-2.0.html
 * or you may search the http://www.gnu.org website for the version 2 license,
 * or you may write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

/**
 * @file pcbnew_scripting_helpers.cpp
 * @brief Scripting helper functions for pcbnew functionality
 */

#include <Python.h>

#include <pcbnew_scripting_helpers.h>
#include <pcbnew.h>
#include <pcbnew_id.h>
#include <build_version.h>
#include <class_board.h>
#include <class_drawpanel.h>
#include <kicad_string.h>
#include <io_mgr.h>
#include <macros.h>
#include <stdlib.h>
#include <pcb_draw_panel_gal.h>
#include <action_plugin.h>

static PCB_EDIT_FRAME* s_PcbEditFrame = NULL;

BOARD* GetBoard()
{
    if( s_PcbEditFrame )
        return s_PcbEditFrame->GetBoard();
    else
        return NULL;
}


void ScriptingSetPcbEditFrame( PCB_EDIT_FRAME* aPcbEditFrame )
{
    s_PcbEditFrame = aPcbEditFrame;
}


BOARD* LoadBoard( wxString& aFileName )
{
    if( aFileName.EndsWith( wxT( ".kicad_pcb" ) ) )
        return LoadBoard( aFileName, IO_MGR::KICAD_SEXP );

    else if( aFileName.EndsWith( wxT( ".brd" ) ) )
        return LoadBoard( aFileName, IO_MGR::LEGACY );

    // as fall back for any other kind use the legacy format
    return LoadBoard( aFileName, IO_MGR::LEGACY );
}


BOARD* LoadBoard( wxString& aFileName, IO_MGR::PCB_FILE_T aFormat )
{
    BOARD* brd = IO_MGR::Load( aFormat, aFileName );

    if( brd )
        brd->BuildConnectivity();

    return brd;
}


bool SaveBoard( wxString& aFileName, BOARD* aBoard, IO_MGR::PCB_FILE_T aFormat )
{
    aBoard->BuildConnectivity();
    aBoard->SynchronizeNetsAndNetClasses();
    aBoard->GetDesignSettings().SetCurrentNetClass( NETCLASS::Default );

    IO_MGR::Save( aFormat, aFileName, aBoard, NULL );

    return true;
}


bool SaveBoard( wxString& aFileName, BOARD* aBoard )
{
    return SaveBoard( aFileName, aBoard, IO_MGR::KICAD_SEXP );
}


void Refresh()
{
    if( s_PcbEditFrame )
    {
        auto board = s_PcbEditFrame->GetBoard();
        board->BuildConnectivity();

        if( s_PcbEditFrame->IsGalCanvasActive() )
        {
            auto gal_canvas = static_cast<PCB_DRAW_PANEL_GAL*>( s_PcbEditFrame->GetGalCanvas() );

            // Reinit everything: this is the easy way to do that
            s_PcbEditFrame->UseGalCanvas( true );
            gal_canvas->Refresh();
        }
        else
            // first argument is erase background, second is a wxRect that
            // defines a reftresh area (all canvas if null)
            s_PcbEditFrame->GetCanvas()->Refresh( true, NULL );
    }
}


void WindowZoom( int xl, int yl, int width, int height )
{
    EDA_RECT Rect( wxPoint( xl, yl ), wxSize( width, height ) );

    if( s_PcbEditFrame )
        s_PcbEditFrame->Window_Zoom( Rect );
}


void UpdateUserInterface()
{
    if( s_PcbEditFrame )
        s_PcbEditFrame->UpdateUserInterface();
}


bool IsActionRunning()
{
    return ACTION_PLUGINS::IsActionRunning();
}
