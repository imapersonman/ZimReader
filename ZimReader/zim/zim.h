#ifndef __ZIM_H__
#define __ZIM_H__

// Errors
#define ZIM_ERROR_NONE 0
#define ZIM_ERROR_UNSPECIFIED 1
#define ZIM_ERROR_WRONGFORMAT 2
#define ZIM_ERROR_WRONGVERSION 4

#include <stdint.h>

typedef struct _ZIMHeader
{
    uint32_t magicNumber;
    uint32_t version;
    uint64_t uuidLower;
    uint64_t uuidUpper;
    uint32_t articleCount;
    uint32_t clusterCount;
    uint64_t urlPtrPos;
    uint64_t titlePtrPos;
    uint64_t clusterPtrPos;
    uint64_t mimeListPos;
    uint32_t mainPage;
    uint32_t layoutPage;
    uint64_t checksumPos;
    uint64_t geoIndexPos;
    uint32_t errorFlags;
} ZIMHeader;

typedef struct _StringList
{
    char **list;
    size_t size;
    size_t capacity;
} StringList;

typedef struct _ZIMHandle
{
    ZIMHeader *header;
    StringList *mimeList;
    uint64_t *urlPtrs;
    uint64_t *titlePtrs;
    uint64_t *clusterPtrs;
} ZIMHandle;

typedef enum _ZIMDirectoryEntryType
{
    ZIMDirectoryEntryType_Article,
    ZIMDirectoryEntryType_Redirect,
    ZIMDirectoryEntryType_Other
} ZIMDirectoryEntryType;

typedef struct _ZIMDirectoryEntry
{
    ZIMDirectoryEntryType type;
    void *typePtr;
    uint16_t mimeType;
    uint8_t parameterLength;
    uint8_t namespace;
    uint32_t revision;
    char *url;
    char *title;
} ZIMDirectoryEntry;

typedef struct _ZIMDirectoryArticleEntry
{
    uint32_t clusterNumber;
    uint32_t blobNumber;
} ZIMDirectoryArticleEntry;

typedef struct _ZIMDirectoryRedirectEntry
{
    uint8_t redirectIndex;
} ZIMDirectoryRedirectEntry;

typedef struct _ZIMDirectoryOtherEntry
{
    // Empty for now (according to spec).
} ZIMDirectoryOtherEntry;

typedef struct _ZIMCluster
{
    uint8_t compressionType;
    uint64_t inSize;
    FILE *stream;
} ZIMCluster;

void ZIM_InitHandle(FILE *fptr, ZIMHandle *handle, uint32_t *errorFlags);
void ZIM_DeinitHandle(ZIMHandle *handle);
void ZIM_PrintErrorCode(uint32_t error);
void ZIM_PrintDirectoryEntry(ZIMDirectoryEntry *entry);
void ZIM_FreeDirectoryEntry(ZIMDirectoryEntry *entry);
void ZIM_DecompressClusterAtIndex(FILE *fptr, ZIMHandle *handle, ZIMCluster *cluster, uint32_t index);
ZIMDirectoryEntry *ZIM_GetDirectoryEntryForIndex(FILE *fptr, ZIMHandle *handle, uint32_t index);
uint32_t ZIM_DirectoryEntryIndexByTitle(FILE *fptr, ZIMHandle *handle, const char *entryTitle);
uint32_t ZIM_DirectoryEntryIndexByTitle_SLOW(FILE *fptr, ZIMHandle *handle, const char *entryTitle);
int ZIM_MatchesTitle(ZIMDirectoryEntry *entry, const char *title);

#endif /* __ZIM_H__ */
