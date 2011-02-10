#include <plist/plist.h>

#include "ipsw_t.h"

extern plist_t ipsw_load_build_manifest();
extern char *ipsw_component_get_path(plist_t man, const char *component);
extern plist_t build_manifest_get_build_identity(plist_t build_manifest, uint32_t identity);
extern plist_t build_manifest_get_build_target(plist_t build_manifest, uint32_t identity);
//extern ipsw_t load_ipsw(const char *ipsw);
