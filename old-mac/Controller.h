//
//  Controller.h
//  FirmExtract-OSX
//
//  Created by boxingsquirrel on 11/18/10.
//  Copyright 2010 N/A. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "MAAttachedWindow.h"


@interface Controller : NSObject {
	IBOutlet id ipswButton;
	IBOutlet id ipswButtonIcon;
	IBOutlet id outDirButton;
	IBOutlet id outDirButtonIcon;
	IBOutlet id decButton;
	IBOutlet id decButtonIcon;
	IBOutlet id ipswButton1;
	IBOutlet id ipswButtonIcon1;
	IBOutlet id outDirButton1;
	IBOutlet id outDirButtonIcon1;
	IBOutlet id decButton1;
	IBOutlet id decButtonIcon1;
	IBOutlet id ipswButton2;
	IBOutlet id ipswButtonIcon2;
	IBOutlet id outDirButton2;
	IBOutlet id outDirButtonIcon2;
	IBOutlet id decButton2;
	IBOutlet id decButtonIcon2;
	IBOutlet id iPhone;
	IBOutlet id iPod;
	IBOutlet id iPad;
	IBOutlet id win;
	IBOutlet id view;
	IBOutlet id copyright;
	IBOutlet id copyright_close_b;
	IBOutlet id iPad_box;
	IBOutlet id iPhone_box;
	IBOutlet id iPod_box;
	IBOutlet id credits_box;
	MAAttachedWindow *awin;
}

-(IBAction)chooseIPSW:(id)sender;
-(IBAction)chooseOutDir:(id)sender;
-(IBAction)decrypt:(id)sender;
-(IBAction)setiPhone:(id)sender;
-(IBAction)setiPod:(id)sender;
-(IBAction)setiPad:(id)sender;
-(IBAction)showCredits:(id)sender;
-(IBAction)close_box:(id)sender;
@end
