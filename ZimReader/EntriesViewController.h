//
//  EntriesViewController.h
//  ZimReader
//
//  Created by Koissi Adjorlolo on 11/9/16.
//  Copyright © 2016 Koissi Adjorlolo. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "ZIMHandleDelegate.h"
#import "zim.h"

@interface EntriesViewController : NSViewController
@property (weak) id<ZIMHandleDelegate> handleDelegate;
- (void)setCurrentEntry:(ZIMDirectoryEntry *)entry;
@end
