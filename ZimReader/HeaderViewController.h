//
//  HeaderViewController.h
//  ZimReader
//
//  Created by Koissi Adjorlolo on 11/8/16.
//  Copyright © 2016 Koissi Adjorlolo. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "zim.h"

@interface HeaderViewController : NSViewController
- (void)fillHeaderLabels:(ZIMHeader *)header;
- (void)emptyHeader;
@end
