#include <stdint.h>
#include <stdlib.h>
#include "globals.h"
#include "worldgen.h"
#include "registries.h"
#include "tools.h"
#include "procedures.h"
#include "structures.h"

// Ensure div_floor is available even in translation units where inline
// definitions might not be visible to the compiler at link time.
int div_floor(int a, int b);

// Superflat parameters
#define SUPERFLAT_TOP_Y 80
#define SUPERFLAT_DIRT_LAYERS 4

uint8_t sf_getHeightAt(int x, int z) {
  (void)x; (void)z;
  // Height means the topmost solid block Y. For top at 80 return 80.
  return SUPERFLAT_TOP_Y;
}

uint8_t sf_getTerrainAt(int x, int y, int z, ChunkAnchor anchor) {
  (void)x; (void)z; (void)anchor;
  if (y > SUPERFLAT_TOP_Y) return B_air;
  if (y == SUPERFLAT_TOP_Y) return B_grass_block;
  if (y <= SUPERFLAT_TOP_Y - 1 && y >= SUPERFLAT_TOP_Y - SUPERFLAT_DIRT_LAYERS) return B_dirt;
  if (y >= 1 && y < SUPERFLAT_TOP_Y - SUPERFLAT_DIRT_LAYERS) return B_stone;
  if (y == 0) return B_bedrock;
  if (y < 0) return B_bedrock;
  return B_air;
}

uint8_t sf_getBlockAt(int x, int y, int z) {
  uint8_t block_change = getBlockChange(x, y, z);
  if (block_change != 0xFF) return block_change;
  return sf_getTerrainAt(x, y, z, (ChunkAnchor){0});
}

// Build simple chunk section without features (features will be placed as
// block changes by sf_placeFeatures)
uint8_t sf_buildChunkSection(int cx, int cy, int cz) {
  for (int j = 0; j < 4096; j += 8) {
    int y = j / 256 + cy;
    for (int offset = 7; offset >= 0; offset--) {
      uint8_t block;
      if (y > SUPERFLAT_TOP_Y) block = B_air;
      else if (y == SUPERFLAT_TOP_Y) block = B_grass_block;
      else if (y <= SUPERFLAT_TOP_Y - 1 && y >= SUPERFLAT_TOP_Y - SUPERFLAT_DIRT_LAYERS) block = B_dirt;
      else if (y >= 1 && y < SUPERFLAT_TOP_Y - SUPERFLAT_DIRT_LAYERS) block = B_stone;
      else if (y == 0) block = B_bedrock;
      else if (y < 0) block = B_bedrock;
      else block = B_air;
      chunk_section[j + 7 - offset] = block;
    }
  }

  // Deterministically place trees as block changes for this chunk section
  // Iterate anchors covering this chunk and place features if selected
#if SF_ENABLE_TREES
  for (int ax = cx - (cx % CHUNK_SIZE); ax < cx + 16 + CHUNK_SIZE; ax += CHUNK_SIZE) {
    task_yield();
    for (int az = cz - (cz % CHUNK_SIZE); az < cz + 16 + CHUNK_SIZE; az += CHUNK_SIZE) {
      task_yield();
      short anchor_x = ax / CHUNK_SIZE;
      short anchor_z = az / CHUNK_SIZE;
      uint32_t h = getChunkHash(anchor_x, anchor_z);
      uint8_t feature_position = h % (CHUNK_SIZE * CHUNK_SIZE);
      short fx = feature_position % CHUNK_SIZE + anchor_x * CHUNK_SIZE;
      short fz = feature_position / CHUNK_SIZE + anchor_z * CHUNK_SIZE;
      // Simple deterministic chance ~1/16
      uint32_t mix = h ^ ((uint32_t)fx << 16) ^ (uint32_t)fz;
      if ((mix & 15) == 0) {
        // Only place if feature is within this chunk section bounds
        if (fx >= cx && fx < cx + 16 && fz >= cz && fz < cz + 16) {
          // Place tree structure rooted at SUPERFLAT_TOP_Y
          placeTreeStructure(fx, SUPERFLAT_TOP_Y, fz);
          task_yield();
        }
      }
    }
  }
#endif

  // Return a biome id for this chunk section (use existing chunk_anchors[0]
  // contract elsewhere expects return value to be biome at origin)
  short origin_ax = div_floor(cx, CHUNK_SIZE);
  short origin_az = div_floor(cz, CHUNK_SIZE);
  return getChunkBiome(origin_ax, origin_az);
}
