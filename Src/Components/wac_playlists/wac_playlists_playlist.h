#pragma once

#include "wac_playlists_entry.h"

#include <bfc/multipatch.h>

namespace wa
{
    namespace components
    {
        namespace playlists
        {
            enum
            {
                patch_playlist,
                patch_playlistloadercallback
            };

            class playlist : public MultiPatch<patch_playlist, ifc_playlist>, public MultiPatch<patch_playlistloadercallback, ifc_playlistloadercallback>
            {

            };
        }
    }
}