; media library API
SetOutPath $INSTDIR\gen_ml
File ${GenPlugins}\gen_ml\ml.h
File ${GenPlugins}\gen_ml\ml_ipc_0313.h
File ${GenPlugins}\gen_ml\childwnd.h

; local media API
SetOutPath $INSTDIR\ml_local
File ${LibPlugins}\ml_local\api_mldb.h
File ${LibPlugins}\ml_local\queries.txt

; Replay Gain API (this should probably be moved out of ml_rg eventually)
SetOutPath $INSTDIR\ml_rg
File ${LibPlugins}\ml_rg\obj_replaygain.h

; Podcast API
SetOutPath $INSTDIR\ml_wire
File ${LibPlugins}\ml_wire\api_podcasts.h
File ${LibPlugins}\ml_wire\ifc_podcast.h
File ${LibPlugins}\ml_wire\ifc_article.h