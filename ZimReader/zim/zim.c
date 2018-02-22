#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
//#include <lzma.h>

#include "zim.h"

#define ZIM_BYTEBLOCK_SIZE 512

// Other Constants
#define ZIM_MAGICNUM 72173914
#define ZIM_VERSION 5

// Header Constants
#define ZIM_MAGICNUM_OFFSET 0
#define ZIM_MAGICNUM_LENGTH 4
#define ZIM_VERSION_OFFSET 4
#define ZIM_VERSION_LENGTH 4
#define ZIM_UUID_OFFSET 8
#define ZIM_UUID_LENGTH 16
#define ZIM_ARTICLECOUNT_OFFSET 24
#define ZIM_ARTICLECOUNT_LENGTH 4
#define ZIM_CLUSTERCOUNT_OFFSET 28
#define ZIM_CLUSTERCOUNT_LENGTH 4
#define ZIM_URLPTRPOS_OFFSET 32
#define ZIM_URLPTRPOS_LENGTH 8
#define ZIM_TITLEPTRPOS_OFFSET 40
#define ZIM_TITLEPTRPOS_LENGTH 8
#define ZIM_CLUSTERPTRPOS_OFFSET 48
#define ZIM_CLUSTERPTRPOS_LENGTH 8
#define ZIM_MIMELISTPOS_OFFSET 56
#define ZIM_MIMELISTPOS_LENGTH 8
#define ZIM_MAINPAGE_OFFSET 64
#define ZIM_MAINPAGE_LENGTH 4
#define ZIM_LAYOUTPAGE_OFFSET 68
#define ZIM_LAYOUTPAGE_LENGTH 4
#define ZIM_CHECKSUMPOS_OFFSET 72
#define ZIM_CHECKSUMPOS_LENGTH 8
#define ZIM_GEOINDEXPOS_OFFSET 80
#define ZIM_GEOINDEXPOS_LENGTH 8

#define ZIM_HEADERWOUTGEO_LENGTH ZIM_GEOINDEXPOS_OFFSET
#define ZIM_HEADER_LENGTH (ZIM_GEOINDEXPOS_OFFSET + ZIM_GEOINDEXPOS_LENGTH)

// Directory Constants
#define ZIM_DIR_MIMETYPE_OFFSET 0
#define ZIM_DIR_MIMETYPE_LENGTH 2
#define ZIM_DIR_PARAM_OFFSET 2
#define ZIM_DIR_PARAM_LENGTH 1
#define ZIM_DIR_NAMESPACE_OFFSET 3
#define ZIM_DIR_NAMESPACE_LENGTH 1
#define ZIM_DIR_REVISION_OFFSET 4
#define ZIM_DIR_REVISION_LENGTH 4

#define ZIM_DIRENTRY_LENGTH (ZIM_DIR_REVISION_OFFSET + ZIM_DIR_REVISION_LENGTH)

// Article
#define ZIM_ARTICLE_MIMETYPE_OFFSET 0
#define ZIM_ARTICLE_MIMETYPE_LENGTH 2
#define ZIM_ARTICLE_PARAM_OFFSET 2
#define ZIM_ARTICLE_PARAM_LENGTH 1
#define ZIM_ARTICLE_NAMESPACE_OFFSET 3
#define ZIM_ARTICLE_NAMESPACE_LENGTH 1
#define ZIM_ARTICLE_REVISION_OFFSET 4
#define ZIM_ARTICLE_REVISION_LENGTH 4
#define ZIM_ARTICLE_CLUSTERNUM_OFFSET 8
#define ZIM_ARTICLE_CLUSTERNUM_LENGTH 4
#define ZIM_ARTICLE_BLOBNUM_OFFSET 12
#define ZIM_ARTICLE BLOBNUM_LENGTH 4
#define ZIM_ARTICLE_URL_OFFSET 16

#define ZIM_ARTICLE_EXTRA_LENGTH 8

// Redirect
#define ZIM_REDIRECT_MIMETYPE_OFFSET 0
#define ZIM_REDIRECT_MIMETYPE_LENGTH 2
#define ZIM_REDIRECT_PARAM_OFFSET 2
#define ZIM_REDIRECT_PARAM_LENGTH 1
#define ZIM_REDIRECT_NAMESPACE_OFFSET 3
#define ZIM_REDIRECT_NAMESPACE_LENGTH 1
#define ZIM_REDIRECT_REVISION_OFFSET 4
#define ZIM_REDIRECT_REVISION_OFFSET 4
#define ZIM_REDIRECT_INDEX_OFFSET 8
#define ZIM_REDIRECT_INDEX_LENGTH 4
#define ZIM_REDIRECT_URL_OFFSET 12

#define ZIM_REDIRECT_EXTRA_LENGTH 4

// Other
#define ZIM_LINKTARGET_ID 65534
#define ZIM_DELETED_ID 65533

#define ZIM_OTHER_MIMETYPE_OFFSET 0
#define ZIM_OTHER_MIMETYPE_LENGTH 2
#define ZIM_OTHER_PARAM_OFFSET 2
#define ZIM_OTHER_PARAM_LENGTH 1
#define ZIM_OTHER_NAMESPACE_OFFSET 3
#define ZIM_OTHER_NAMESPACE_LENGTH 1
#define ZIM_OTHER_REVISION_OFFSET 4
#define ZIM_OTHER_REVISION_LENGTH 4
#define ZIM_OTHER_URL_OFFSET 16  // I'm having a difficult time believing this.

// Namespace Constants
#define ZIM_NAMESPACE_OTHER '-'
#define ZIM_NAMESPACE_ARTICLES 'A'
#define ZIM_NAMESPACE_ARTICLEMETADATA 'B'
#define ZIM_NAMESPACE_IMAGESFILES 'I'
#define ZIM_NAMESPACE_IMAGESTEXT 'J'
#define ZIM_NAMESPACE_ZIMMETADATA 'M'
#define ZIM_NAMESPACE_CATEGORIESTEXT 'U'
#define ZIM_NAMESAPCE_CATEGORIESARTICLELIST 'W'
#define ZIM_NAMESPACE_CATEGORIESPERARTICLEARTICLELIST 'W'
#define ZIM_NAMESPACE_FULLTEXTINDEX 'X'

// Cluster Constants
#define ZIM_CLUSTER_COMPRESSIONTYPE_OFFSET 0
#define ZIM_CLUSTER_COMPRESSIONTYPE_LENGTH 1

#define ZIM_CLUSTER_COMPRESSIONTYPE_DEFAULT 0  // Specifies no compression.
#define ZIM_CLUSTER_COMPRESSIONTYPE_NONE 1  // Inherited from Zeno.
#define ZIM_CLUSTER_COMPRESSIONTYPE_LZMA2 4

// Macros
#define CLEAR_STRUCT(memory, type) memset((void *)memory, 0, sizeof(type))

void createStringList(StringList *list, size_t capacity)
{
    // I should include some sort of error handling in here, but I'll do it later.
    list->list = (char **)malloc(sizeof(char *) * capacity);
    if (list->capacity == 0) list->capacity = 1;
    list->capacity = capacity;
    list->size = 0;
}

void destroyStringList(StringList *list)
{
    for (int stringIndex = 0; stringIndex < list->size; stringIndex++)
    {
        free(list->list[stringIndex]);
        list->list[stringIndex] = NULL;
    }
    free(list->list);
    list->list = NULL;
}

void pushStringToList(StringList *list, char *element)
{
    if (list->size >= list->capacity)
    {
        list->capacity *= 2;
        list->list = (char **)realloc(list->list, sizeof(char *) * list->capacity);
    }
    // I might have to use another string library, depending on future performance.
    // Making my own is also an option.
    list->list[list->size] = (char *)malloc(sizeof(char) * (strlen(element) + 1));
    strcpy(list->list[list->size], element);
    list->size++;
}

void popStringFromList(StringList *list)
{
    list->size--;
    free(list->list[list->size]);
    list->list[list->size] = NULL;
    if (list->size < list->capacity / 4)
    {
        list->capacity /= 4;
        list->list = (char **)realloc(list->list, sizeof(char *) * list->capacity);
    }
}

void printByteArray(uint8_t *byteArray, size_t length)
{
    for (int byteIndex = 0; byteIndex < length; byteIndex++)
    {
        printf("%02x ", byteArray[byteIndex]);
    }
    putc('\n', stdout);
}

// For the sake of completion.
void copyBytesToUInt8(uint8_t *byteArray, size_t uintPos, uint8_t *uint)
{
    *uint |= (uint8_t)byteArray[uintPos];
}

void copyBytesToUInt16(uint8_t *byteArray, size_t uintPos, uint16_t *uint)
{
    *uint |= (uint16_t)byteArray[uintPos];
    *uint |= (uint16_t)byteArray[uintPos + 1] << 8;
}

void copyBytesToUInt32(uint8_t *byteArray, size_t uintPos, uint32_t *uint)
{
    *uint |= (uint32_t)byteArray[uintPos];
    *uint |= (uint32_t)byteArray[uintPos + 1] << 8;
    *uint |= (uint32_t)byteArray[uintPos + 2] << 16;
    *uint |= (uint32_t)byteArray[uintPos + 3] << 24;
}

void copyBytesToUInt64(uint8_t *byteArray, size_t uintPos, uint64_t *uint)
{
    *uint |= (uint64_t)byteArray[uintPos];
    *uint |= (uint64_t)byteArray[uintPos + 1] << 8;
    *uint |= (uint64_t)byteArray[uintPos + 2] << 16;
    *uint |= (uint64_t)byteArray[uintPos + 3] << 24;
    *uint |= (uint64_t)byteArray[uintPos + 4] << 32;
    *uint |= (uint64_t)byteArray[uintPos + 5] << 40;
    *uint |= (uint64_t)byteArray[uintPos + 6] << 48;
    *uint |= (uint64_t)byteArray[uintPos + 7] << 56;
}

void printZimHeader(ZIMHeader *header)
{
    printf("Magic Number: %d\n", header->magicNumber);
    printf("Version: %d\n", header->version);
    printf("UUID Lower: %llu\n", header->uuidLower);
    printf("UUID Upper: %llu\n", header->uuidUpper);
    printf("Article Count: %d\n", header->articleCount);
    printf("Cluster Count: %d\n", header->clusterCount);
    printf("URL Pointer Position: %llu\n", header->urlPtrPos);
    printf("Title Pointer Position: %llu\n", header->titlePtrPos);
    printf("Cluster Pointer Position: %llu\n", header->clusterPtrPos);
    printf("Mime List Position: %llu\n", header->mimeListPos);
    printf("Main Page: %d\n", header->mainPage);
    printf("Layout Page: %d\n", header->layoutPage);
    printf("Checksum Position: %llu\n", header->checksumPos);
    printf("Geo Index Position: %llu\n", header->geoIndexPos);
}

void fillZimHeader(FILE *fptr, ZIMHeader *header)
{
    fseek(fptr, 0L, SEEK_SET);
    uint8_t zimFileHeader[ZIM_HEADER_LENGTH];
    if (fread(zimFileHeader, 1, ZIM_HEADER_LENGTH, fptr) != ZIM_HEADER_LENGTH)
    {
        // Something went wrong reading the header.
        header->errorFlags |= ZIM_ERROR_UNSPECIFIED;
        return;
    }

    CLEAR_STRUCT(header, ZIMHeader);
    copyBytesToUInt32(zimFileHeader, ZIM_MAGICNUM_OFFSET, &header->magicNumber);
    if (header->magicNumber != ZIM_MAGICNUM)
    {
        // File is not a Zim file.
        header->errorFlags |= ZIM_ERROR_WRONGFORMAT;
        return;
    }

    copyBytesToUInt32(zimFileHeader, ZIM_VERSION_OFFSET, &header->version);
    if (header->version != ZIM_VERSION)
    {
        // File uses an unsupported version of Zim.
        header->errorFlags |= ZIM_ERROR_WRONGVERSION;
        return;
    }

    copyBytesToUInt64(zimFileHeader, ZIM_UUID_OFFSET, &header->uuidLower);
    copyBytesToUInt64(zimFileHeader, ZIM_UUID_OFFSET + 8, &header->uuidUpper);
    copyBytesToUInt32(zimFileHeader, ZIM_ARTICLECOUNT_OFFSET, &header->articleCount);
    copyBytesToUInt32(zimFileHeader, ZIM_CLUSTERCOUNT_OFFSET, &header->clusterCount);
    copyBytesToUInt64(zimFileHeader, ZIM_URLPTRPOS_OFFSET, &header->urlPtrPos);
    copyBytesToUInt64(zimFileHeader, ZIM_TITLEPTRPOS_OFFSET, &header->titlePtrPos);
    copyBytesToUInt64(zimFileHeader, ZIM_CLUSTERPTRPOS_OFFSET, &header->clusterPtrPos);
    copyBytesToUInt64(zimFileHeader, ZIM_MIMELISTPOS_OFFSET, &header->mimeListPos);
    copyBytesToUInt32(zimFileHeader, ZIM_MAINPAGE_OFFSET, &header->mainPage);
    copyBytesToUInt32(zimFileHeader, ZIM_LAYOUTPAGE_OFFSET, &header->layoutPage);
    copyBytesToUInt64(zimFileHeader, ZIM_CHECKSUMPOS_OFFSET, &header->checksumPos);

    // Present if mimeListPos is at least 88
    if (header->mimeListPos >= ZIM_HEADER_LENGTH)
        copyBytesToUInt64(zimFileHeader, ZIM_GEOINDEXPOS_OFFSET, &header->geoIndexPos);
    else
        header->geoIndexPos = -1;
}

int headerError(ZIMHeader *header)
{
    return header->errorFlags != ZIM_ERROR_NONE;
}

void ZIM_PrintErrorCode(uint32_t error)
{
    printf("ZIM Errors:\n");
    if (error & ZIM_ERROR_UNSPECIFIED) printf(" - An unspecified error occurred.");
    if (error & ZIM_ERROR_WRONGFORMAT) printf(" - The file loaded does not use the ZIM file format\n");
    if (error & ZIM_ERROR_WRONGVERSION) printf(" - The file loaded uses an unsupported version of ZIM\n");
}

int containsGeoIndex(ZIMHeader *header)
{
    return header->mimeListPos >= ZIM_HEADER_LENGTH;
}

void fillMimeList(FILE *fptr, StringList *list, size_t offset)
{
    fseek(fptr, offset, SEEK_SET);
    // This needs to be changed to allow for strings greater than 512 bytes,
    // or at the very least to save space.
    char zimMime[ZIM_BYTEBLOCK_SIZE];
    fgets(zimMime, ZIM_BYTEBLOCK_SIZE, fptr);

    size_t zimMimePos = 0;
    size_t mimeSize = strlen(zimMime);
    char *mime = (char *)malloc(sizeof(char) * (mimeSize + 1));
    strcpy(mime, zimMime);
    pushStringToList(list, mime);
    free(mime);
    zimMimePos = mimeSize + 1;

    while (zimMime[zimMimePos] != '\0')
    {
        mimeSize = strlen(zimMime + zimMimePos);
        mime = (char *)malloc(sizeof(char) * (mimeSize + 1));
        strcpy(mime, zimMime + zimMimePos);
        pushStringToList(list, mime);
        free(mime);
        zimMimePos += mimeSize + 1;
    }
}

void fillPointerList(FILE *fptr, uint64_t *ptrList, uint32_t count, uint64_t position)
{
    // More error handling!!!
    fseek(fptr, position, SEEK_SET);
    fread(ptrList, sizeof(uint64_t), count, fptr);
}

void ZIM_InitHandle(FILE *fptr, ZIMHandle *handle, uint32_t *errorFlags)
{
    handle->header = (ZIMHeader *)malloc(sizeof(ZIMHeader));
    fillZimHeader(fptr, handle->header);
    if (headerError(handle->header))
    {
        ZIM_PrintErrorCode(handle->header->errorFlags);
        free(handle->header);
        *errorFlags |= handle->header->errorFlags;
        return;
    }

    printf("Header read successfully\n");

    handle->mimeList = (StringList *)malloc(sizeof(StringList));
    createStringList(handle->mimeList, 8);
    size_t fileOffset = 0;
    if (containsGeoIndex(handle->header)) fileOffset = ZIM_HEADER_LENGTH;
    else fileOffset = ZIM_HEADERWOUTGEO_LENGTH;
    fillMimeList(fptr, handle->mimeList, fileOffset);
    printf("Mime List read successfully\n");

    handle->urlPtrs = (uint64_t *)malloc(sizeof(uint64_t) * handle->header->articleCount);
    handle->titlePtrs = (uint64_t *)malloc(sizeof(uint64_t) * handle->header->articleCount);
    handle->clusterPtrs = (uint64_t *)malloc(sizeof(uint64_t) * handle->header->clusterCount);

    fillPointerList(fptr, handle->urlPtrs, handle->header->articleCount, handle->header->urlPtrPos);
    fillPointerList(fptr, handle->titlePtrs, handle->header->articleCount, handle->header->titlePtrPos);
    fillPointerList(fptr, handle->clusterPtrs, handle->header->clusterCount, handle->header->clusterPtrPos);
    printf("Pointer Lists read successfully\n");
}

void ZIM_DeinitHandle(ZIMHandle *handle)
{
    if (handle->header) free(handle->header);
    if (handle->urlPtrs) free(handle->urlPtrs);
    if (handle->titlePtrs) free(handle->titlePtrs);
    if (handle->clusterPtrs) free(handle->clusterPtrs);
    if (handle->mimeList)
    {
        if (handle->mimeList->list) destroyStringList(handle->mimeList);
        free(handle->mimeList);
    }

    handle->header = NULL;
    handle->urlPtrs = NULL;
    handle->titlePtrs = NULL;
    handle->clusterPtrs = NULL;
    handle->mimeList = NULL;
}

void fillZimDirectoryEntryUrlAndTitle(FILE *fptr, size_t offset, ZIMDirectoryEntry *dirEntry)
{
    uint8_t byteArray[ZIM_BYTEBLOCK_SIZE];
    fseek(fptr, offset, SEEK_SET);
    fread(byteArray, sizeof(uint8_t), ZIM_BYTEBLOCK_SIZE, fptr);

    size_t byteIndex = 0;
    size_t urlSize = 0;
    while (byteIndex + urlSize < ZIM_BYTEBLOCK_SIZE && byteArray[byteIndex + urlSize] != 0) urlSize++;
    dirEntry->url = (char *)malloc(sizeof(char) * (urlSize + 1));
    for (size_t index = 0; byteIndex + index < ZIM_BYTEBLOCK_SIZE && index < urlSize; index++)
        dirEntry->url[index] = byteArray[byteIndex + index];
    dirEntry->url[urlSize] = 0;
    byteIndex += urlSize + 1;

    size_t titleSize = 0;  // Zero terminated.
    while (byteIndex + titleSize < ZIM_BYTEBLOCK_SIZE && byteArray[byteIndex + titleSize] != 0) titleSize++;
    dirEntry->title = (char *)malloc(sizeof(char) * (titleSize + 1));
    for (size_t index = 0; byteIndex + index < ZIM_BYTEBLOCK_SIZE && index < titleSize; index++)
        dirEntry->title[index] = byteArray[byteIndex + index];
    dirEntry->title[titleSize] = 0;
}

void fillZimDirectoryArticleEntry(FILE *fptr, size_t offset, ZIMDirectoryArticleEntry *dirEntry)
{
    CLEAR_STRUCT(dirEntry, ZIMDirectoryArticleEntry);
    uint8_t byteArray[ZIM_ARTICLE_EXTRA_LENGTH];
    fseek(fptr, offset + ZIM_ARTICLE_CLUSTERNUM_OFFSET, SEEK_SET);
    fread(byteArray, sizeof(uint8_t), ZIM_ARTICLE_EXTRA_LENGTH, fptr);
    copyBytesToUInt32(byteArray, 0, &dirEntry->clusterNumber);
    copyBytesToUInt32(byteArray, ZIM_ARTICLE_CLUSTERNUM_LENGTH, &dirEntry->blobNumber);
}

void fillZimDirectoryRedirectEntry(FILE *fptr, size_t offset, ZIMDirectoryRedirectEntry *dirEntry)
{
    CLEAR_STRUCT(dirEntry, ZIMDirectoryRedirectEntry);
    uint8_t byteArray[ZIM_REDIRECT_EXTRA_LENGTH];
    fseek(fptr, offset + ZIM_REDIRECT_INDEX_OFFSET, SEEK_SET);
    fread(byteArray, sizeof(uint8_t), ZIM_REDIRECT_EXTRA_LENGTH, fptr);
    copyBytesToUInt8(byteArray, 0, &dirEntry->redirectIndex);
}

void fillZimDirectoryOtherEntry(FILE *fptr, size_t offset, ZIMDirectoryOtherEntry *dirEntry)
{
    // Nothing happens for now.
}

void fillZimDirectoryEntry(FILE *fptr, size_t offset, ZIMDirectoryEntry *dirEntry)
{
    CLEAR_STRUCT(dirEntry, ZIMDirectoryEntry);
    uint8_t byteArray[ZIM_DIRENTRY_LENGTH];
    fseek(fptr, offset, SEEK_SET);
    fread(byteArray, sizeof(uint8_t), ZIM_DIRENTRY_LENGTH, fptr);
    copyBytesToUInt16(byteArray, ZIM_DIR_MIMETYPE_OFFSET, &dirEntry->mimeType);
    copyBytesToUInt8(byteArray, ZIM_DIR_PARAM_OFFSET, &dirEntry->parameterLength);
    copyBytesToUInt8(byteArray, ZIM_DIR_NAMESPACE_OFFSET, &dirEntry->namespace);
    copyBytesToUInt32(byteArray, ZIM_DIR_REVISION_OFFSET, &dirEntry->revision);

    if (dirEntry->mimeType == 0xFFFF) dirEntry->type = ZIMDirectoryEntryType_Redirect;
    else dirEntry->type = ZIMDirectoryEntryType_Article;

    switch (dirEntry->type)
    {
        case ZIMDirectoryEntryType_Article:
        {
            dirEntry->typePtr = (ZIMDirectoryArticleEntry *)malloc(sizeof(ZIMDirectoryArticleEntry));
            fillZimDirectoryArticleEntry(fptr, offset, (ZIMDirectoryArticleEntry *)dirEntry->typePtr);
            fillZimDirectoryEntryUrlAndTitle(fptr, offset + ZIM_ARTICLE_URL_OFFSET, dirEntry);
        } break;
        case ZIMDirectoryEntryType_Redirect:
        {
            dirEntry->typePtr = (ZIMDirectoryRedirectEntry *)malloc(sizeof(ZIMDirectoryRedirectEntry));
            fillZimDirectoryRedirectEntry(fptr, offset, (ZIMDirectoryRedirectEntry *)dirEntry->typePtr);
            fillZimDirectoryEntryUrlAndTitle(fptr, offset + ZIM_REDIRECT_URL_OFFSET, dirEntry);
        } break;
        case ZIMDirectoryEntryType_Other:
        {
            // dirEntry->typePtr = (ZIMDirectoryOtherEntry *)malloc(sizeof(ZIMDirectoryOtherEntry));
            fillZimDirectoryOtherEntry(fptr, offset, (ZIMDirectoryOtherEntry *)dirEntry->typePtr);
            fillZimDirectoryEntryUrlAndTitle(fptr, offset + ZIM_OTHER_URL_OFFSET, dirEntry);
        } break;
        default: break;
    }
}

ZIMDirectoryEntry *ZIM_GetDirectoryEntryForIndex(FILE *fptr, ZIMHandle *handle, uint32_t index)
{
    // This assumes the handle is non-null.
    // I just realized how unsafe most of this code is.  I'll deal with it this summer (fun project!)
    if (index >= handle->header->articleCount) return NULL;

    ZIMDirectoryEntry *entry = (ZIMDirectoryEntry *)malloc(sizeof(ZIMDirectoryEntry));
    size_t offset = handle->urlPtrs[index];
    fillZimDirectoryEntry(fptr, offset, entry);

    return entry;
}

void ZIM_FreeDirectoryEntry(ZIMDirectoryEntry *entry)
{
    // Might need to free memory for each entry type separately, but so far that's not necessary.
    free(entry->typePtr);
    free(entry->url);
    free(entry->title);
    free(entry);
}

void printUtf8(uint32_t *utf8String)
{
    // Provisional.
    for (size_t index = 0; utf8String[index] != 0; index++) fputc((char)utf8String[index], stdout);
}

void ZIM_PrintDirectoryEntry(ZIMDirectoryEntry *entry)
{
    printf("Mime Type: %d\n", entry->mimeType);
    printf("Parameter Length: %d\n", entry->parameterLength);
    printf("Namespace: %c\n", entry->namespace);
    printf("Revision: %d\n", entry->revision);

    switch (entry->type)
    {
        case ZIMDirectoryEntryType_Article:
        {
            printf("Cluster Number: %d\n", ((ZIMDirectoryArticleEntry *)entry->typePtr)->clusterNumber);
            printf("Blob Number: %d\n", ((ZIMDirectoryArticleEntry *)entry->typePtr)->blobNumber);
        } break;
        case ZIMDirectoryEntryType_Redirect:
        {
            printf("Redirect Index: %d\n", ((ZIMDirectoryRedirectEntry *)entry->typePtr)->redirectIndex);
        } break;
        case ZIMDirectoryEntryType_Other:
        {
        } break;
        default:
        {
        } break;
    }

    printf("URL: %s\n", entry->url);

    printf("Title: %s\n", entry->title);
}

int shouldCompareChar(char c)
{
    // I might change this in the future.
    return 1;
}

int ZIM_MatchesTitle(ZIMDirectoryEntry *entry, const char *title)
{
    size_t entryIndex = 0;
    size_t titleIndex = 0;
    char curEntryChar = tolower(entry->title[entryIndex]);
    char curTitleChar = tolower(title[titleIndex]);
    while (!shouldCompareChar(curEntryChar)) curEntryChar = tolower(entry->title[++entryIndex]);
    while (!shouldCompareChar(curTitleChar)) curTitleChar = tolower(title[++titleIndex]);

    while (curEntryChar == curTitleChar || curTitleChar == '\0')
    {
        if (curTitleChar == '\0')
        {
            return 1;
        }

        curEntryChar = tolower(entry->title[++entryIndex]);
        curTitleChar = tolower(title[++titleIndex]);
        while (!shouldCompareChar(curEntryChar)) curEntryChar = tolower(entry->title[++entryIndex]);
        while (!shouldCompareChar(curTitleChar)) curTitleChar = tolower(title[++titleIndex]);
    }

    return 0;
}

uint32_t ZIM_DirectoryEntryIndexByTitle_SLOW(FILE *fptr, ZIMHandle *handle, const char *entryTitle)
{
    uint32_t timesSearched = 0;
    while (timesSearched < handle->header->articleCount)
    {
        ZIMDirectoryEntry *foundEntry = ZIM_GetDirectoryEntryForIndex(fptr, handle, (uint32_t)timesSearched);
        size_t entryIndex = 0;
        size_t foundIndex = 0;
        char curEntryChar = tolower(entryTitle[entryIndex]);
        char curFoundChar = tolower(foundEntry->title[foundIndex]);
        while (!shouldCompareChar(curEntryChar)) curEntryChar = tolower(entryTitle[++entryIndex]);
        while (!shouldCompareChar(curFoundChar)) curFoundChar = tolower(foundEntry->title[++foundIndex]);

        while (curEntryChar == curFoundChar)
        {
            if (curEntryChar == '\0')
            {
                ZIM_FreeDirectoryEntry(foundEntry);
                return timesSearched;
            }

            curEntryChar = tolower(entryTitle[++entryIndex]);
            curFoundChar = tolower(foundEntry->title[++foundIndex]);
            while (!shouldCompareChar(curEntryChar)) curEntryChar = tolower(entryTitle[++entryIndex]);
            while (!shouldCompareChar(curFoundChar)) curFoundChar = tolower(foundEntry->title[++foundIndex]);
        }

        ZIM_FreeDirectoryEntry(foundEntry);
        timesSearched++;
    }

    return -1;
}

uint32_t ZIM_DirectoryEntryIndexByTitle(FILE *fptr, ZIMHandle *handle, const char *entryTitle)
{
    uint32_t articleCount = handle->header->articleCount;
    uint32_t searchIndex = 0;

    uint32_t l = 0;
    uint32_t r = articleCount - 1;

    // Doesn't work properly due to UTF-8 mishandling.
    // Char keeps wrapping.
    // TODO(koissi): UTF-8 comparison function.
    while (l <= r)
    {
        searchIndex = (l + r) / 2;

        ZIMDirectoryEntry *foundEntry = ZIM_GetDirectoryEntryForIndex(fptr, handle, searchIndex);
        size_t entryIndex = 0;
        size_t foundIndex = 0;
        char curEntryChar = tolower(entryTitle[entryIndex]);
        char curFoundChar = tolower(foundEntry->title[foundIndex]);
        while (!shouldCompareChar(curEntryChar)) curEntryChar = tolower(entryTitle[++entryIndex]);
        while (!shouldCompareChar(curFoundChar)) curFoundChar = tolower(foundEntry->title[++foundIndex]);

        while (curEntryChar == curFoundChar || curEntryChar == '\0')
        {
            if (curEntryChar == '\0')
            {
                ZIM_FreeDirectoryEntry(foundEntry);
                return searchIndex;
            }

            curEntryChar = tolower(entryTitle[++entryIndex]);
            curFoundChar = tolower(foundEntry->title[++foundIndex]);
            while (!shouldCompareChar(curEntryChar)) curEntryChar = tolower(entryTitle[++entryIndex]);
            while (!shouldCompareChar(curFoundChar)) curFoundChar = tolower(foundEntry->title[++foundIndex]);
        }

        if (curEntryChar < curFoundChar) r = searchIndex - 1;
        else if (curEntryChar > curFoundChar) l = searchIndex + 1;
    }

    return -1;
}

uint64_t getClusterLengthAtIndex(ZIMHandle *handle, uint32_t index)
{
    if (index >= handle->header->clusterCount)
    {
        if (containsGeoIndex(handle->header))
        {
            return handle->header->geoIndexPos - handle->clusterPtrs[index];
        }
        else
        {
            return UINT64_MAX;
        }
    }

    return handle->clusterPtrs[index + 1] - handle->clusterPtrs[index];
}

void ZIM_DecompressClusterAtIndex(FILE *fptr, ZIMHandle *handle, ZIMCluster *cluster, uint32_t index)
{
}
/*
void decodeLzmaReturn(lzma_ret ret)
{
    printf("LZMA: ");
    switch (ret)
    {
        case LZMA_OK:
        {
            printf("All is well");
        } break;
        case LZMA_STREAM_END:
        {
            printf("End of stream");
        } break;
        case LZMA_MEM_ERROR:
        {
            printf("Memory allocation failed");
        } break;
        case LZMA_FORMAT_ERROR:
        {
            printf("The input is not in the .xz format");
        } break;
        case LZMA_OPTIONS_ERROR:
        {
            printf("Unsupported compression options");
        } break;
        case LZMA_DATA_ERROR:
        {
            printf("Compressed file is corrupt");
        } break;
        case LZMA_BUF_ERROR:
        {
            printf("Compressed file is truncated or otherwise corrupt");
        } break;
        default:
        {
            printf("An unknown error occurred");
        } break;
    }
    printf("\n");
}
 */

#if 0
int main(int argc, char **argv)
{
    FILE *zimFile = fopen("data/wikibooks_nopic.zim", "rb");
    if (zimFile) printf("Successfully opened file\n");
    else
    {
        printf("Unable to open file\n");
        return -1;
    }

    uint32_t error;
    ZIMHandle *handle = (ZIMHandle *)malloc(sizeof(ZIMHandle));
    ZIM_InitHandle(zimFile, handle, &error);

    if (error != ZIM_ERROR_NONE)
    {
        ZIM_PrintErrorCode(handle->header->errorFlags);
        ZIM_DeinitHandle(handle);
        fclose(zimFile);
        return -1;
    }
#if 1
    ZIMDirectoryEntry *entry = ZIM_GetDirectoryEntryForIndex(zimFile, handle, 600);
    ZIM_PrintDirectoryEntry(entry);
    ZIM_FreeDirectoryEntry(entry);
#else

    // Init decoder.
    lzma_stream stream = LZMA_STREAM_INIT;
    lzma_ret ret = lzma_stream_decoder(&stream, UINT64_MAX, LZMA_CONCATENATED);
    if (ret != LZMA_OK)
    {
        printf("LZMA Decoder failed to init\n");
        exit(-1);
    }

    ZIMCluster *cluster = (ZIMCluster *)malloc(sizeof(ZIMCluster));
    fseek(zimFile, handle->clusterPtrs[0], SEEK_SET);
    cluster->stream = tmpfile();
    cluster->inSize = getClusterLengthAtIndex(handle, 0);
    printf("Before pos: %ld\n", ftell(zimFile));
    fread(&cluster->compressionType, 1, 1, zimFile);
    printf("After pos: %ld\n", ftell(zimFile));
    printf("cluster->inSize: %llu\n", cluster->inSize);
    printf("cluster->compressionType: %d\n", cluster->compressionType);
    printf("cluster position: %llu\n", handle->clusterPtrs[0]);
    // Decompress Cluster.
    lzma_action action = LZMA_RUN;
    uint8_t inbuf[BUFSIZ];
    uint8_t outbuf[BUFSIZ];
    stream.next_in = NULL;
    stream.avail_in = 0;
    stream.next_out = outbuf;
    stream.avail_out = sizeof(outbuf);

    while (1)
    {
        if (stream.avail_in == 0 && !feof(zimFile)/* && ftell(zimFile) < cluster->inSize*/)
        {
            stream.next_in = inbuf;
            stream.avail_in = fread(inbuf, 1, sizeof(inbuf), zimFile);
            if (ferror(zimFile))
            {
                printf("Error reading in file\n");
                exit(-1);
            }
            if (feof(zimFile)/* || ftell(zimFile) >= cluster->inSize*/) action = LZMA_FINISH;
        }

        lzma_ret ret = lzma_code(&stream, action);
        if (stream.avail_out == 0 || ret == LZMA_STREAM_END)
        {
            size_t writeSize = sizeof(outbuf) - stream.avail_out;
            if (fwrite(outbuf, 1, writeSize, cluster->stream) != writeSize)
            {
                printf("Error writing out file\n");
                exit(-1);
            }
            stream.next_out = outbuf;
            stream.avail_out = sizeof(outbuf);
        }

        if (ret != LZMA_OK)
        {
            if (ret == LZMA_STREAM_END) break;
            decodeLzmaReturn(ret);
            printf("Something terrible happened decoding the cluster!!!\n");
            exit(-1);
        }
    }
#endif

    ZIM_DeinitHandle(handle);
    //free(cluster);
    fclose(zimFile);

    return 0;
}
#endif
