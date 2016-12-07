//
//  ZIMHandleDelegate.h
//  ZimReader
//
//  Created by Koissi Adjorlolo on 11/9/16.
//  Copyright © 2016 Koissi Adjorlolo. All rights reserved.
//

#import <Foundation/Foundation.h>
#import "zim.h"

@protocol ZIMHandleDelegate <NSObject>
- (ZIMHandle *)getZimHandle;
- (FILE *)getFilePointer;
@end
