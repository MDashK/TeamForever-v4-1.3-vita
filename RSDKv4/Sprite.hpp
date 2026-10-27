#ifndef SPRITE_H
#define SPRITE_H

int AddGraphicsFile(const char *filePath);
void RemoveGraphicsFile(const char *filePath, int sheetID);

int LoadBMPFile(const char *filePath, byte sheetID);
int LoadGIFFile(const char *filePath, byte sheetID);

// keep decoded sprite sheets between stage loads (vita only, loading is slow there)
#define RETRO_GFX_CACHE (RETRO_PLATFORM == RETRO_VITA && !RETRO_USE_ORIGINAL_CODE)
#if RETRO_GFX_CACHE
void ClearGfxCache();
#endif
int LoadPVRFile(const char *filePath, byte sheetID);

void ReadGifPictureData(int width, int height, bool interlaced, byte *gfxData, int offset);

#endif // !SPRITE_H
