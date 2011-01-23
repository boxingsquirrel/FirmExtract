//
//  FirmExtract_OSXAppDelegate.h
//  FirmExtract-OSX
//
//  Created by boxingsquirrel on 11/20/10.
//  Copyright 2010 N/A. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface FirmExtract_OSXAppDelegate : NSObject <NSApplicationDelegate> {
    NSWindow *window;
}

@property (assign) IBOutlet NSWindow *window;

@end
