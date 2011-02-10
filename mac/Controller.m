//
//  Controller.m
//  NanoFW
//
//  Created by boxingsquirrel on 11/18/10.
//  Copyright 2010 N/A. All rights reserved.
//

#import "Controller.h"
#import "ZipArchive.h"
#import <Foundation/NSBundle.h>

// Filenames
NSString *ipsw_fname=@"/Users/boxingsquirrel/Downloads/iPod2,1_4.0_8A293_Restore.ipsw";
NSString *outdir_fname=@"/Users/boxingsquirrel/Desktop/dump";

@implementation Controller

// Browse for an IPSW. Nothing more to say...
-(IBAction)chooseIPSW:(id)sender {
	NSOpenPanel *choose=[NSOpenPanel openPanel];
	[choose setCanChooseFiles:YES];
	[choose setCanChooseDirectories:NO];
	[choose setTitle:@"Browse for IPSW..."];
	if ([choose runModalForTypes:[NSArray arrayWithObject:@"ipsw"]] == NSOKButton)
	{
		NSArray *f=[choose filenames];
		ipsw_fname=[f objectAtIndex:0];
		[ipsw_fname retain];
	}
}

// Browse for the output directory...
-(IBAction)chooseOutDir:(id)sender {
	NSOpenPanel *choose=[NSOpenPanel openPanel];
	[choose setCanChooseFiles:NO];
	[choose setCanChooseDirectories:YES];

	// <evil>Show a "New Folder" button if supported...</evil>
	if ([choose respondsToSelector:@selector(_setIncludeNewFolderButton:)]==YES) {
		[choose _setIncludeNewFolderButton:YES];
	}

	[choose setTitle:@"Browse for Output Directory..."];
	if ([choose runModalForDirectory:nil file:nil] == NSOKButton)
	{
		NSArray *f=[choose filenames];
		outdir_fname=[f objectAtIndex:0];
		[outdir_fname retain];
	}	
}

// Get the actual product given a raw BuildManifest in dictionary form...
-(NSString *)getProductForBuildManifest:(NSDictionary *)man {
	NSArray *prodTypes=[man objectForKey:@"SupportedProductTypes"];

	// Jedi Mind Trick: the AppleTV actually has iProd2,1 listed first, so...
	if ([[[prodTypes objectAtIndex:0] componentsSeparatedByString:@"Prod"] count]>1 && [prodTypes count]>1) {
		return [prodTypes objectAtIndex:1];
	} else {
		return [prodTypes objectAtIndex:0];
	}
}

// Get the version given a raw BuildManifest in NSDictionary form
-(NSString *)getVersionForBuildManifest:(NSDictionary *)man {
	return [man objectForKey:@"ProductVersion"];
}

// Get the actual product manifest (contents)
-(NSDictionary *)getActualManifest:(NSDictionary *)man {
	// Jedi mind trick: Everything but the AppleTV is the first build identity. AppleTV is the 4th.
	if ([[[self getProductForBuildManifest:man] componentsSeparatedByString:@"AppleTV"] count]>1) {
		return [[[man objectForKey:@"BuildIdentities"] objectAtIndex:2] objectForKey:@"Manifest"];		
	}

	return [[[man objectForKey:@"BuildIdentities"] objectAtIndex:0] objectForKey:@"Manifest"];
}

// Get the path of an IPSW component /in the output directory/
-(NSString *)getPathForItem:(NSString *)what withManifest:(NSDictionary *)man {
	// GlyphCharging==BattryCharging, just different terminology...
	if ([what isEqualToString:@"GlyphCharging"]==YES) {
		what=@"BatteryCharging";
	}
	// Same with GlyphPlugin and BatteryPlugin
	if ([what isEqualToString:@"GlyphPlugin"]==YES) {
		what=@"BatteryPlugin";
	}

	return [[[man objectForKey:what] objectForKey:@"Info"] objectForKey:@"Path"];
}

// Get the key from the keys manifest
-(NSString *)getKeyForItem:(NSString *)what withKeys:(NSDictionary *)keys {
	return [[keys objectForKey:what] objectForKey:@"Key"];
}

// Get the injection vector (IV) from the keys plist
-(NSString *)getIVForItem:(NSString *)what withKeys:(NSDictionary *)keys {
	return [[keys objectForKey:what] objectForKey:@"IV"];
}

// Decrypt an IMG3 file
-(BOOL)decryptIMG3:(NSString *)what withKeys:(NSDictionary *)keys withManifest:(NSDictionary *)man {
	// Some variables...
	NSString *img3path=[NSString stringWithFormat:@"%@/%@", outdir_fname, [self getPathForItem:what withManifest:man]];
	NSString *scratchPath=[NSString stringWithFormat:@"%@/scratch.img3", outdir_fname];
	NSString *key=[self getKeyForItem:what withKeys:keys];
	NSString *iv=[self getIVForItem:what withKeys:keys];

	// The command to execute (cheating & doing it with xpwntool ftw!)
	NSString *cmd=[NSString stringWithFormat:@"\"%@\" '%@' '%@' -k %@ -iv %@", [[NSBundle mainBundle] pathForResource:@"xpwntool" ofType:@""], img3path, scratchPath, key, iv];
	system((const char *)[cmd UTF8String]);
	system((const char *)[[NSString stringWithFormat:@"mv \"%@\" \"%@\"", scratchPath, img3path] UTF8String]);
	return YES;
}

// Really the same as IMG3 decryption, just using scratch.dmg, not scratch.img3
-(BOOL)decryptRamDisk:(NSString *)what withKeys:(NSDictionary *)keys withManifest:(NSDictionary *)man {
	// Some variables...
	NSString *img3path=[NSString stringWithFormat:@"%@/%@", outdir_fname, [self getPathForItem:what withManifest:man]];
	NSString *scratchPath=[NSString stringWithFormat:@"%@/scratch.dmg", outdir_fname];
	NSString *key=[self getKeyForItem:what withKeys:keys];
	NSString *iv=[self getIVForItem:what withKeys:keys];
	
	// The command to execute (cheating & doing it with xpwntool ftw!)
	NSString *cmd=[NSString stringWithFormat:@"\"%@\" '%@' '%@' -k %@ -iv %@", [[NSBundle mainBundle] pathForResource:@"xpwntool" ofType:@""], img3path, scratchPath, key, iv];
	system((const char *)[cmd UTF8String]);
	system((const char *)[[NSString stringWithFormat:@"mv \"%@\" \"%@\"", scratchPath, img3path] UTF8String]);
	return YES;
}

// Uses dmg (from xpwn) to extract the iOS root filesystem
-(BOOL)decryptFilesystem:(NSString *)what withKeys:(NSDictionary *)keys withManifest:(NSDictionary *)man {
	// Some variables...
	NSString *dmgpath=[NSString stringWithFormat:@"%@/%@", outdir_fname, [self getPathForItem:what withManifest:man]];
	NSString *scratchPath=[NSString stringWithFormat:@"%@/scratch.dmg", outdir_fname];
	NSString *key=[self getKeyForItem:what withKeys:keys];

	// The command to execute (cheating & doing it with dmg ftw!)
	NSString *cmd=[NSString stringWithFormat:@"\"%@\" extract '%@' '%@' -k %@", [[NSBundle mainBundle] pathForResource:@"dmg" ofType:@""], dmgpath, scratchPath, key];
	NSLog(@"%@", cmd);
	system((const char *)[cmd UTF8String]);
	system((const char *)[[NSString stringWithFormat:@"mv '%@' '%@'", scratchPath, dmgpath] UTF8String]);
	return YES;
}

// Decrypt the IPSW contents in the given output directory
-(void)decryptContents:(NSString *)outdir withKeys:(NSDictionary *)keys withManifest:(NSDictionary *)man {
	NSDictionary *manifest=[self getActualManifest:man];
	[self decryptFilesystem:@"OS" withKeys:keys withManifest:manifest];
	[pBar setDoubleValue:80.0];
	[self decryptRamDisk:@"RestoreRamDisk" withKeys:keys withManifest:manifest];
	[self decryptIMG3:@"AppleLogo" withKeys:keys withManifest:manifest];
	[self decryptIMG3:@"BatteryCharging0" withKeys:keys withManifest:manifest];
	[self decryptIMG3:@"BatteryCharging1" withKeys:keys withManifest:manifest];
	[self decryptIMG3:@"BatteryFull" withKeys:keys withManifest:manifest];
	[pBar setDoubleValue:85.0];
	[self decryptIMG3:@"BatteryLow0" withKeys:keys withManifest:manifest];
	[self decryptIMG3:@"BatteryLow1" withKeys:keys withManifest:manifest];
	[self decryptIMG3:@"DeviceTree" withKeys:keys withManifest:manifest];
	[self decryptIMG3:@"GlyphCharging" withKeys:keys withManifest:manifest];
	[pBar setDoubleValue:90.0];
	[self decryptIMG3:@"GlyphPlugin" withKeys:keys withManifest:manifest];
	[self decryptIMG3:@"BatteryCharging0" withKeys:keys withManifest:manifest];
	[self decryptIMG3:@"iBEC" withKeys:keys withManifest:manifest];
	[self decryptIMG3:@"iBoot" withKeys:keys withManifest:manifest];
	[pBar setDoubleValue:95.0];
	[self decryptIMG3:@"iBSS" withKeys:keys withManifest:manifest];
	[self decryptIMG3:@"KernelCache" withKeys:keys withManifest:manifest];
	[self decryptIMG3:@"LLB" withKeys:keys withManifest:manifest];
	[self decryptIMG3:@"RecoveryMode" withKeys:keys withManifest:manifest];
}

// Extract button callback
-(IBAction)decrypt:(id)sender {
	// Set up the progress bar
	[pBar setUsesThreadedAnimation:YES];
	[pBar displayIfNeeded];
	[pBar setDoubleValue:10.0];

	// Extract the IPSW to the output directory
	ZipArchive *za=[[ZipArchive alloc] init];
	[za UnzipOpenFile:ipsw_fname];
	[za UnzipFileTo:outdir_fname overWrite:YES];
	[za release];

	// Decryption now...
	[pBar setDoubleValue:50.0];
	NSDictionary *buildManifest=[NSDictionary dictionaryWithContentsOfFile:[NSString stringWithFormat:@"%@/BuildManifest.plist", outdir_fname]];
	NSString *prod=[self getProductForBuildManifest:buildManifest];
	NSString *ver=[self getVersionForBuildManifest:buildManifest];
	NSString *target=[NSString stringWithFormat:@"%@_%@", prod, ver];
	NSString *plpath=[[NSBundle mainBundle] pathForResource:target ofType:@"plist"];
	NSDictionary *keys=[NSDictionary dictionaryWithContentsOfFile:plpath];
	keys=[keys objectForKey:target];
	[self decryptContents:outdir_fname withKeys:keys withManifest:buildManifest];
	[pBar setDoubleValue:100.0];

	// Done, show an alert to that effect...
	NSAlert *a=[[NSAlert alloc] init];
	[a addButtonWithTitle:@"OK"];
	[a setMessageText:@"Done!"];
	[a setInformativeText:@"The IPSW was extracted!"];
	[a setAlertStyle:NSWarningAlertStyle];
	[a beginSheetModalForWindow:win modalDelegate:nil didEndSelector:nil contextInfo:nil];
}
@end