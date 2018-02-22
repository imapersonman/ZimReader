//
//  EntriesViewController.m
//  ZimReader
//
//  Created by Koissi Adjorlolo on 11/9/16.
//  Copyright © 2016 Koissi Adjorlolo. All rights reserved.
//

#import "EntriesViewController.h"
#import "ArticleWindowController.h"

static NSString *emptyKeys[] = { @"", @"", @"", @"", @"", @"", @"", @"", @"" };

@interface EntriesViewController () <NSSearchFieldDelegate, NSTableViewDelegate, NSTableViewDataSource>
@property (weak) IBOutlet NSTableView *tableView;
@property (weak) IBOutlet NSTableView *entryTableView;
@property (weak) IBOutlet NSSearchField *searchField;
@property (strong, nonatomic) NSMutableArray<NSString *> *searchSuggestions;
@property (strong, nonatomic) NSArray<NSString *> *entryValues;
@property (strong, nonatomic) ArticleWindowController *articleWindowController;
@property NSInteger suggestionOffset;
@property ZIMDirectoryEntry *entry;
@end

@implementation EntriesViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do view setup here.
    self.entryValues = [NSArray arrayWithObjects:emptyKeys count:9];
    [self.tableView setDelegate:self];
    [self.tableView setDataSource:self];
    [self.entryTableView setDelegate:self];
    [self.entryTableView setDataSource:self];
    [self.searchField setDelegate:self];
    [self.searchField setSendsSearchStringImmediately:YES];
    [self.searchField setSendsWholeSearchString:NO];
    [self.tableView setDoubleAction:@selector(doubleClickedCell:)];
}

- (void)doubleClickedCell:(id)sender {
    NSInteger row = self.tableView.clickedRow;
    NSString *articleTitle = self.searchSuggestions[row];
    NSLog(@"Article Title: %@", articleTitle);
    if (self.entry) {
        switch (self.entry->type) {
            case ZIMDirectoryEntryType_Article: {
                NSLog(@"Cluster Number: %d", ((ZIMDirectoryArticleEntry *)self.entry->typePtr)->clusterNumber);
                NSLog(@"Blob Number: %d", ((ZIMDirectoryArticleEntry *)self.entry->typePtr)->blobNumber);
            } break;
            case ZIMDirectoryEntryType_Redirect: {
                NSLog(@"Redirect Index: %d", ((ZIMDirectoryRedirectEntry *)self.entry->typePtr)->redirectIndex);
            } break;
            case ZIMDirectoryEntryType_Other: {
                NSLog(@"Other");
            } break;
            default: NSLog(@"NO"); break;
        }
    } else {
        NSLog(@"No Entry Currently Selected");
    }
    
    FILE *fptr = [self.handleDelegate getFilePointer];
    ZIMHandle *handle = [self.handleDelegate getZimHandle];
    ZIMCluster *cluster = (ZIMCluster *)malloc(sizeof(ZIMCluster));
    
    /*
    ZIM_DecompressClusterAtIndex(fptr, handle, cluster, 0);
    char *utf8Html = ZIM_GetBlobAtIndex(cluster, 0);
    */  
    self.articleWindowController = [[ArticleWindowController alloc] init];
    [self.articleWindowController showWindow:self];
    //[self.articleWindowController loadHTML:[NSString stringWithUTF8String:utf8Html]];
}

- (IBAction)sentSearchResults:(id)sender {
    if ([self.searchField.stringValue isEqualToString:@""]) return;
    NSLog(@"%@", self.searchField.stringValue);
    if (self.handleDelegate) {
        ZIMHandle *handle = [self.handleDelegate getZimHandle];
        FILE *fptr = [self.handleDelegate getFilePointer];
        if (handle && fptr) {
            const char *stringValue = [self.searchField.stringValue UTF8String];
            uint32_t entryIndex = ZIM_DirectoryEntryIndexByTitle(fptr, handle, stringValue);
            uint32_t count = 0;
            ZIMDirectoryEntry *entry = ZIM_GetDirectoryEntryForIndex(fptr, handle, entryIndex);
            self.suggestionOffset = entryIndex;
            self.searchSuggestions = [[NSMutableArray alloc] init];
            
            while (entry && count < 100 && ZIM_MatchesTitle(entry, stringValue)) {
                [self.searchSuggestions addObject:[NSString stringWithFormat:@"%s", entry->title]];
                ZIM_FreeDirectoryEntry(entry);
                entry = ZIM_GetDirectoryEntryForIndex(fptr, handle, entryIndex + ++count);
            }
            
            if (entry) ZIM_FreeDirectoryEntry(entry);
            [self.tableView reloadData];
        }
    }
}

- (void)setCurrentEntry:(ZIMDirectoryEntry *)entry {
    NSString *values[] = {
        [NSString stringWithFormat:@"%d", entry->mimeType],
        [NSString stringWithFormat:@"%d", entry->parameterLength],
        [NSString stringWithFormat:@"%c", entry->namespace],
        [NSString stringWithFormat:@"%d", entry->revision],
        @"", @"", @"",
        [NSString stringWithFormat:@"%s", entry->url],
        [NSString stringWithFormat:@"%s", entry->title],
    };
    
    switch (entry->type) {
        case ZIMDirectoryEntryType_Article: {
            values[4] = [NSString stringWithFormat:@"%d", ((ZIMDirectoryArticleEntry *)entry->typePtr)->clusterNumber];
            values[5] = [NSString stringWithFormat:@"%d", ((ZIMDirectoryArticleEntry *)entry->typePtr)->blobNumber];
        } break;
        case ZIMDirectoryEntryType_Redirect: {
            values[6] = [NSString stringWithFormat:@"%d", ((ZIMDirectoryRedirectEntry *)entry->typePtr)->redirectIndex];
        } break;
        case ZIMDirectoryEntryType_Other: {
        } break;
        default: break;
    }
    
    self.entryValues = [NSArray arrayWithObjects:values count:9];
    [self.entryTableView reloadData];
    self.entry = entry;
}

- (void)clearCurrentEntry {
    if (self.entry) ZIM_FreeDirectoryEntry(self.entry);
    self.entryValues = [NSArray arrayWithObjects:emptyKeys count:9];
}

- (NSView *)tableView:(NSTableView *)tableView viewForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row {
    static NSString *keys[] = {
        @"Mime Type", @"Parameter Length", @"Namespace", @"Revision", @"Cluster Number",
        @"Blob Number", @"Redirect Index", @"URL", @"Title"
    };
    
    NSTextField *result = [tableView makeViewWithIdentifier:@"MyView" owner:self];
    if (!result) {
        NSRect cellRect = NSMakeRect(0, 0, tableView.frame.size.width, 0);
        result = [[NSTextField alloc] initWithFrame:cellRect];
        [result setBordered:NO];
        [result setDrawsBackground:NO];
        result.identifier = @"MyView";
    }
    if ([tableView.identifier isEqualToString:@"TableView"]) {
        result.stringValue = self.searchSuggestions[row];
    } else if ([tableView.identifier isEqualToString:@"EntryTableView"]) {
        if ([tableColumn.identifier isEqualToString:@"EntryKeys"]) {
            result.stringValue = keys[row];
        } else if ([tableColumn.identifier isEqualToString:@"EntryValues"]) {
            result.stringValue = self.entryValues[row];
        }
    }
    
    return result;
}

- (NSInteger)numberOfRowsInTableView:(NSTableView *)tableView {
    if ([tableView.identifier isEqualToString:@"TableView"]) {
        return [self.searchSuggestions count];
    } else if ([tableView.identifier isEqualToString:@"EntryTableView"]) {
        return 9;
    }
    
    return 0;
}

- (BOOL)tableView:(NSTableView *)tableView shouldSelectRow:(NSInteger)row {
    if ([tableView.identifier isEqualToString:@"TableView"]) {
        ZIMHandle *handle = [self.handleDelegate getZimHandle];
        FILE *fptr = [self.handleDelegate getFilePointer];
        ZIMDirectoryEntry *entry = ZIM_GetDirectoryEntryForIndex(fptr, handle, (uint32_t)(row + self.suggestionOffset));
        if (entry) {
            [self clearCurrentEntry];
            [self setCurrentEntry:entry];
        } else {
            [self clearCurrentEntry];
        }
    } else if ([tableView.identifier isEqualToString:@"EntryTableView"]) {
        return NO;
    }
    
    return YES;
}

@end
