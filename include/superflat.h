#ifndef H_SUPERFLAT
#define H_SUPERFLAT

#include <stdint.h>
#include "worldgen.h"

// Toggle features in superflat generation
#ifndef SF_ENABLE_TREES
#define SF_ENABLE_TREES 1
#endif

// Superflat API
uint8_t sf_getHeightAt(int x, int z);
uint8_t sf_getTerrainAt(int x, int y, int z, ChunkAnchor anchor);
uint8_t sf_getBlockAt(int x, int y, int z);
uint8_t sf_buildChunkSection(int cx, int cy, int cz);

#endif
