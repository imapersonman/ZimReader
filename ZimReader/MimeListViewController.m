//
//  MimeListViewController.m
//  ZimReader
//
//  Created by Koissi Adjorlolo on 11/8/16.
//  Copyright © 2016 Koissi Adjorlolo. All rights reserved.
//

#import "MimeListViewController.h"

@interface MimeListViewController () <NSTableViewDelegate, NSTableViewDataSource>
@property (weak) IBOutlet NSTableView *tableView;
@property (strong, nonatomic) NSMutableArray<NSString *> *mimeStrings;
@end

@implementation MimeListViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do view setup here.
    [self.tableView setDelegate:self];
    [self.tableView setDataSource:self];
}

- (void)fillMimeList:(StringList *)list {
    self.mimeStrings = [[NSMutableArray alloc] init];
    for (size_t stringIndex = 0; stringIndex < list->size; stringIndex++) {
        [self.mimeStrings addObject:[NSString stringWithFormat:@"%s", list->list[stringIndex]]];
    }
    [self.tableView reloadData];
}

- (NSView *)tableView:(NSTableView *)tableView viewForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row {
    NSTextField *result = [tableView makeViewWithIdentifier:@"MyView" owner:self];
    
    if (!result) {
        NSRect cellRect = NSMakeRect(0, 0, tableView.frame.size.width, 0);
        result = [[NSTextField alloc] initWithFrame:cellRect];
        [result setBordered:NO];
        [result setDrawsBackground:NO];
        result.identifier = @"MyView";
    }
    
    result.stringValue = self.mimeStrings[row];
    
    return result;
}

- (NSInteger)numberOfRowsInTableView:(NSTableView *)tableView {
    return [self.mimeStrings count];
}

- (void)emptyMimeList {
    self.mimeStrings = nil;
    [self.tableView reloadData];
}

@end
