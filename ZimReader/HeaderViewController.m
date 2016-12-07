//
//  HeaderViewController.m
//  ZimReader
//
//  Created by Koissi Adjorlolo on 11/8/16.
//  Copyright © 2016 Koissi Adjorlolo. All rights reserved.
//

#import "HeaderViewController.h"

@interface HeaderViewController ()
@property (weak) IBOutlet NSTextField *magicNumber;
@property (weak) IBOutlet NSTextField *version;
@property (weak) IBOutlet NSTextField *uuidLower;
@property (weak) IBOutlet NSTextField *uuidUpper;
@property (weak) IBOutlet NSTextField *articleCount;
@property (weak) IBOutlet NSTextField *clusterCount;
@property (weak) IBOutlet NSTextField *urlPtrPosition;
@property (weak) IBOutlet NSTextField *titlePtrPosition;
@property (weak) IBOutlet NSTextField *clusterPtrPosition;
@property (weak) IBOutlet NSTextField *mimeListPosition;
@property (weak) IBOutlet NSTextField *mainPage;
@property (weak) IBOutlet NSTextField *layoutPage;
@property (weak) IBOutlet NSTextField *checksumPosition;
@property (weak) IBOutlet NSTextField *geoIndexPosition;
@end

@implementation HeaderViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    // Do view setup here.
}

- (void)fillHeaderLabels:(ZIMHeader *)header {
    self.magicNumber.stringValue = [NSString stringWithFormat:@"%d", header->magicNumber];
    self.version.stringValue = [NSString stringWithFormat:@"%d", header->version];
    self.uuidLower.stringValue = [NSString stringWithFormat:@"%llu", header->uuidLower];
    self.uuidUpper.stringValue = [NSString stringWithFormat:@"%llu", header->uuidUpper];
    self.articleCount.stringValue = [NSString stringWithFormat:@"%d", header->articleCount];
    self.clusterCount.stringValue = [NSString stringWithFormat:@"%d", header->clusterCount];
    self.urlPtrPosition.stringValue = [NSString stringWithFormat:@"%llu", header->urlPtrPos];
    self.titlePtrPosition.stringValue = [NSString stringWithFormat:@"%llu", header->titlePtrPos];
    self.clusterPtrPosition.stringValue = [NSString stringWithFormat:@"%llu", header->clusterPtrPos];
    self.mimeListPosition.stringValue = [NSString stringWithFormat:@"%llu", header->mimeListPos];
    self.mainPage.stringValue = [NSString stringWithFormat:@"%d", header->mainPage];
    self.layoutPage.stringValue = [NSString stringWithFormat:@"%d", header->layoutPage];
    self.checksumPosition.stringValue = [NSString stringWithFormat:@"%llu", header->checksumPos];
    self.geoIndexPosition.stringValue = [NSString stringWithFormat:@"%llu", header->geoIndexPos];
}

- (void)emptyHeader {
    self.magicNumber.stringValue = @"";
    self.version.stringValue = @"";
    self.uuidLower.stringValue = @"";
    self.uuidUpper.stringValue = @"";
    self.articleCount.stringValue = @"";
    self.clusterCount.stringValue = @"";
    self.urlPtrPosition.stringValue = @"";
    self.titlePtrPosition.stringValue = @"";
    self.clusterPtrPosition.stringValue = @"";
    self.mimeListPosition.stringValue = @"";
    self.mainPage.stringValue = @"";
    self.layoutPage.stringValue = @"";
    self.checksumPosition.stringValue = @"";
    self.geoIndexPosition.stringValue = @"";
}

@end
