//
//  AppDelegate.m
//  ZimReader
//
//  Created by Koissi Adjorlolo on 11/7/16.
//  Copyright © 2016 Koissi Adjorlolo. All rights reserved.
//

#import "AppDelegate.h"
#import "HeaderViewController.h"
#import "MimeListViewController.h"
#import "EntriesViewController.h"
#import "zim.h"

@interface AppDelegate () <NSTableViewDelegate, NSTableViewDataSource, ZIMHandleDelegate>
@property (weak) IBOutlet NSWindow *window;
@property (weak) IBOutlet HeaderViewController *headerViewController;
@property (weak) IBOutlet MimeListViewController *mimeListViewController;
@property (weak) IBOutlet EntriesViewController *entriesViewController;
@property FILE *filePointer;
@property ZIMHandle *fileHandle;
@end

@implementation AppDelegate

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {
    // Insert code here to initialize your application
    [self.entriesViewController setHandleDelegate:self];
}

- (void)applicationWillTerminate:(NSNotification *)aNotification {
    // Insert code here to tear down your application
    [self freeResources];
}

- (IBAction)pressedCloseFile:(id)sender {
    [self freeResources];
}

- (IBAction)pressedOpenFile:(id)sender {
    NSOpenPanel *openPanel = [NSOpenPanel openPanel];
    [openPanel setCanChooseFiles:YES];
    [openPanel setCanChooseDirectories:NO];
    [openPanel setAllowsMultipleSelection:NO];
    [openPanel setResolvesAliases:YES];
    
    if ([openPanel runModal] == NSFileHandlingPanelOKButton)
    {
        NSArray *files = [openPanel URLs];
        for (int fileIndex = 0; fileIndex < [files count]; fileIndex++) {
            [self freeResources];
            // There should only be one, but just in case there are more.
            NSURL *url = [files objectAtIndex:fileIndex];
            const char *urlString = [[url relativePath] cStringUsingEncoding:[NSString defaultCStringEncoding]];
            self.filePointer = fopen(urlString, "rb");
            if (self.filePointer);  // Successfully opened file.
            else {  // Failed to open file.
                NSLog(@"Unable to open selected file: %s", urlString);
                return;
            }
            
            uint32_t error = 0;
            self.fileHandle = (ZIMHandle *)malloc(sizeof(ZIMHandle));
            
            ZIM_InitHandle(self.filePointer, self.fileHandle, &error);
            
            if (error != ZIM_ERROR_NONE) {
                ZIM_PrintErrorCode(error);
                NSLog(@"Something terrible happened.");
                return;
            }
            
            [self.headerViewController fillHeaderLabels:self.fileHandle->header];
            [self.mimeListViewController fillMimeList:self.fileHandle->mimeList];
        }
    }
}

- (ZIMHandle *)getZimHandle {
    return self.fileHandle;
}

- (FILE *)getFilePointer {
    return self.filePointer;
}

- (void)freeResources {
    [self.headerViewController emptyHeader];
    [self.mimeListViewController emptyMimeList];
    if (self.filePointer) {
        fclose(self.filePointer);
        self.filePointer = NULL;
    }
    if (self.fileHandle) {
        ZIM_DeinitHandle(self.fileHandle);
        free(self.fileHandle);
        self.fileHandle = NULL;
    }
}

@end
