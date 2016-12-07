//
//  MimeListViewController.h
//  ZimReader
//
//  Created by Koissi Adjorlolo on 11/8/16.
//  Copyright © 2016 Koissi Adjorlolo. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "zim.h"

@interface MimeListViewController : NSViewController
- (void)fillMimeList:(StringList *)list;
- (void)emptyMimeList;
@end
