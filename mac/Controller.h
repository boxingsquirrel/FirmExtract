//
//  Controller.h
//  FirmExtract-OSX
//
//  Created by boxingsquirrel on 11/18/10.
//  Copyright 2010 N/A. All rights reserved.
//

#import <Cocoa/Cocoa.h>


@interface Controller : NSObject {
	IBOutlet id win;

	IBOutlet id ipswButton;
	IBOutlet id outDirButton;
	IBOutlet id decButton;

	IBOutlet id pBar;
}

-(IBAction)chooseIPSW:(id)sender;
-(IBAction)chooseOutDir:(id)sender;

-(NSString *)getProductForBuildManifest:(NSDictionary *)man;
-(NSString *)getVersionForBuildManifest:(NSDictionary *)man;
-(NSDictionary *)getActualManifest:(NSDictionary *)man;
-(NSString *)getPathForItem:(NSString *)what withManifest:(NSDictionary *)man;
-(NSString *)getKeyForItem:(NSString *)what withKeys:(NSDictionary *)keys;
-(NSString *)getIVForItem:(NSString *)what withKeys:(NSDictionary *)keys;
-(BOOL)decryptIMG3:(NSString *)what withKeys:(NSDictionary *)keys withManifest:(NSDictionary *)man;
-(BOOL)decryptRamDisk:(NSString *)what withKeys:(NSDictionary *)keys withManifest:(NSDictionary *)man;
-(BOOL)decryptFilesystem:(NSString *)what withKeys:(NSDictionary *)keys withManifest:(NSDictionary *)man;
-(void)decryptContents:(NSString *)outdir withKeys:(NSDictionary *)keys withManifest:(NSDictionary *)man;

-(IBAction)decrypt:(id)sender;
@end

// Kill a compiler warning...
@interface NSOpenPanel (__dieCompilerWarning)
-(void)_setIncludeNewFolderButton:(BOOL)show;
@end
