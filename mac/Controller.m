//
//  Controller.m
//  FirmExtract-OSX
//
//  Created by boxingsquirrel on 11/18/10.
//  Copyright 2010 N/A. All rights reserved.
//

#import "Controller.h"
#import "MAAttachedWindow.h"
#include "c-src/ipsw.h"

NSString *ipsw_fname=@"/Users/boxingsquirrel/Downloads/iPod2,1_4.0_8A293_Restore.ipsw";
NSString *outdir_fname=@"/Users/boxingsquirrel/Desktop/dump";

#define PLATFORM_CODE_IPHONE 0
#define PLATFORM_CODE_IPOD 1;
#define PLATFORM_CODE_IPAD 2;
#define PLATFORM_CODE_ATV 3;

int plat=PLATFORM_CODE_IPOD;

@implementation Controller
-(IBAction)chooseIPSW:(id)sender {
	NSOpenPanel *choose=[NSOpenPanel openPanel];
	[choose setCanChooseFiles:YES];
	[choose setCanChooseDirectories:NO];
	//NSArray *types=[NSArray arrayWithObject:@"ipsw"];
	//[choose setAllowedFileTypes:types];
	[choose setTitle:@"Browse for IPSW..."];
	if ([choose runModalForTypes:[NSArray arrayWithObject:@"ipsw"]] == NSOKButton)
	{
		NSArray *f=[choose filenames];
		ipsw_fname=[f objectAtIndex:0];
		[ipsw_fname retain];
		FILE *file=fopen("/tmp/ipsw_loc.txt", "w");
		fprintf(file, "%s", [ipsw_fname UTF8String]);
		fclose(file);
		NSLog(ipsw_fname);
	}
}

-(IBAction)chooseOutDir:(id)sender {
	NSOpenPanel *choose=[NSOpenPanel openPanel];
	[choose setCanChooseFiles:NO];
	[choose setCanChooseDirectories:YES];
	[choose setTitle:@"Browse for Output Directory..."];
	if ([choose runModalForDirectory:nil file:nil] == NSOKButton)
	{
		NSArray *f=[choose filenames];
		outdir_fname=[f objectAtIndex:0];
		[outdir_fname retain];
		FILE *file=fopen("/tmp/out_loc.txt", "w");
		fprintf(file, "%s", [outdir_fname UTF8String]);
		fclose(file);
		NSLog(outdir_fname);
	}	
}

-(IBAction)decrypt:(id)sender {
	NSBeep();
	NSString *filePath = [[NSBundle mainBundle] bundlePath];
	FILE *file=fopen("/tmp/bundle_loc.txt", "w");
	fprintf(file, "%s", [filePath UTF8String]);
	fclose(file);
	[filePath retain];
	//[ipsw_fname writeToFile:@"/tmp/ipsw.txt"];
	//[outdir_fname writeToFile:@"/tmp/out.txt"];
	//ipsw=(const char *)[ipsw_fname UTF8String];
	//out_dir=[outdir_fname UTF8String];
	ipsw_extract_all([ipsw_fname UTF8String], [outdir_fname UTF8String], [filePath UTF8String], 0, NULL, NULL);
}

-(IBAction)setiPhone:(id)sender {
	plat=PLATFORM_CODE_IPHONE;
	[awin retain];
	awin=[[MAAttachedWindow alloc] initWithView:iPhone_box attachedToPoint:NSMakePoint(NSMidX([[view window] frame])-(212/2), NSMidY([[view window] frame])-(48/2)) inWindow:[view window]];
	[awin setHasArrow:0];
	NSColor *c=[NSColor colorWithCalibratedRed:0 green:0 blue:1.0 alpha:1.0];
	[awin setBackgroundColor:c];
	[[outDirButton1 cell] setBackgroundColor:c];
	[[outDirButtonIcon1 cell] setBackgroundColor:c];
	[[decButton1 cell] setBackgroundColor:c];
	[[decButtonIcon1 cell] setBackgroundColor:c];
	[[ipswButton1 cell] setBackgroundColor:c];
	[[ipswButtonIcon1 cell] setBackgroundColor:c];
	[[view window] addChildWindow:awin ordered:NSWindowAbove];
}

-(IBAction)setiPod:(id)sender {
	plat=PLATFORM_CODE_IPOD;
	[awin retain];
	awin=[[MAAttachedWindow alloc] initWithView:iPod_box attachedToPoint:NSMakePoint(NSMidX([[view window] frame])-(212/2), NSMidY([[view window] frame])-(48/2)) inWindow:[view window]];
	[awin setHasArrow:0];
	NSColor *c=[NSColor colorWithCalibratedRed:1.0 green:0.25 blue:0.1 alpha:1.0];
	[awin setBackgroundColor:c];
	[[outDirButton2 cell] setBackgroundColor:c];
	[[outDirButtonIcon2 cell] setBackgroundColor:c];
	[[decButton2 cell] setBackgroundColor:c];
	[[decButtonIcon2 cell] setBackgroundColor:c];
	[[ipswButton2 cell] setBackgroundColor:c];
	[[ipswButtonIcon2 cell] setBackgroundColor:c];
	[[view window] addChildWindow:awin ordered:NSWindowAbove];
}

-(IBAction)setiPad:(id)sender {
	plat=PLATFORM_CODE_IPAD;
	[awin retain];
	awin=[[MAAttachedWindow alloc] initWithView:iPad_box attachedToPoint:NSMakePoint(NSMidX([[view window] frame])-(212/2), NSMidY([[view window] frame])-(48/2)) inWindow:[view window]];
	[awin setHasArrow:0];
	NSColor *c=[NSColor colorWithCalibratedRed:0 green:0.5 blue:0 alpha:1.0];
	[awin setBackgroundColor:c];
	[[outDirButton cell] setBackgroundColor:c];
	[[outDirButtonIcon cell] setBackgroundColor:c];
	[[decButton cell] setBackgroundColor:c];
	[[decButtonIcon cell] setBackgroundColor:c];
	[[ipswButton cell] setBackgroundColor:c];
	[[ipswButtonIcon cell] setBackgroundColor:c];
	[[view window] addChildWindow:awin ordered:NSWindowAbove];
	//[view addSubview:attached];
}

-(IBAction)showCredits:(id)sender {
	[awin retain];
	awin=[[MAAttachedWindow alloc] initWithView:credits_box attachedToPoint:NSMakePoint(NSMidX([[view window] frame])-(212/2), NSMidY([[view window] frame])-(48/2)) inWindow:[view window]];
	[awin setHasArrow:0];
	NSColor *c=[NSColor colorWithCalibratedRed:0 green:0.5 blue:0 alpha:1.0];
	[awin setBackgroundColor:c];
	[[view window] addChildWindow:awin ordered:NSWindowAbove];	
}

-(IBAction)close_box:(id)sender {
	[[view window] removeChildWindow:awin];
	[awin orderOut:self];
	[awin release];
	awin=nil;
}
@end
