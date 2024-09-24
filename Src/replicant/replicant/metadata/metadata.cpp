#include "api__wasabi2_metadata.h"
#include "metadata.h"
#include "ArtworkManager.h"
#include "nswasabi/singleton.h"
#include "MetadataManager.h"

api_service *WASABI2_API_SVC=0;
MetadataManager metadata_manager;
static ArtworkManager artwork_manager;

static SingletonServiceFactory<MetadataManager, api_metadata> metadata_factory;
static SingletonServiceFactory<ArtworkManager, api_artwork> artwork_factory;

int Replicant_Metadata_Initialize(api_service *service_manager)
{
	WASABI2_API_SVC = service_manager;
#ifndef REPLICANT_NO_ARTWORKMANAGER
	int ret = artwork_manager.Initialize();
	if (ret != NErr_Success)
		return ret;
#endif

	metadata_factory.Register(WASABI2_API_SVC, &metadata_manager);
#ifndef REPLICANT_NO_ARTWORKMANAGER
	artwork_factory.Register(WASABI2_API_SVC, &artwork_manager);
#endif
	return NErr_Success;
}

void Replicant_Metadata_Shutdown()
{
#ifndef REPLICANT_NO_ARTWORKMANAGER
	artwork_manager.Shutdown();
#endif
	metadata_factory.Deregister(WASABI2_API_SVC);
#ifndef REPLICANT_NO_ARTWORKMANAGER
	artwork_factory.Deregister(WASABI2_API_SVC);
#endif
}
