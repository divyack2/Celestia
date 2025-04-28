// loadsso.cpp
//
// Copyright (C) 2001-2023, the Celestia Development Team
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

#include "loadsso.h"

#include <fstream>
#include <memory>

#include <celengine/universe.h>
#include <celestia/configfile.h>
#include <celestia/progressnotifier.h>
#include <celestia/catalogloader.h>
#include <celutil/gettext.h>

namespace celestia
{

using SolarSystemLoader = CatalogLoader<Universe>;

template<> bool
CatalogLoader<Universe>::load(std::istream &in, const fs::path &dir)
{
    return LoadSolarSystemObjects(in, *m_objDB, dir);
}

void
loadSSO(const CelestiaConfig &config, ProgressNotifier *progressNotifier, Universe *universe)
{
   auto solarSystem = std::make_unique<SolarSystemCatalog>();
   universe->setSolarSystemCatalog(std::move(solarSystem));


   const char *typeDesc = C_("catalog", "solar system");


   SolarSystemLoader loader(universe,
                            typeDesc,
                            ContentType::CelestiaCatalog,
                            progressNotifier,
                            config.paths.skipExtras);


   // First read the solar system files listed individually in the config file.
   fs::path empty;


   // Parallelize the per-file processing:
   int n = static_cast<int>(config.paths.solarSystemFiles.size());


   #pragma omp parallel for
   for (int i = 0; i < n; ++i) {
       const fs::path &file = config.paths.solarSystemFiles[i];
       loader.process(file, empty);
   }


   // Next, read all the solar system files in the extras directories
   loader.loadExtras(config.paths.extrasDirs);
}


} // namespace celestia
