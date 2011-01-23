/*
 * ipsw.h
 * Utilities for extracting and manipulating IPSWs
 *
 * Copyright (c) 2010 Joshua Hill. All Rights Reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <zip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <gtk/gtk.h>
#include <sys/stat.h>

#include "ipsw.h"
#include "file.h"
#include "decrypt_img3.h"
#include "ui.h"
#include "util.h"
#include "baseband.h"

typedef struct {
	struct zip* zip;
} ipsw_archive;

ipsw_archive* ipsw_open(const char* ipsw);
void ipsw_close(ipsw_archive* archive);

ipsw_archive* ipsw_open(const char* ipsw) {
	int err = 0;
	ipsw_archive* archive = (ipsw_archive*) malloc(sizeof(ipsw_archive));
	if (archive == NULL) {
		printf("ERROR: Out of memory\n");
		return NULL;
	}

	archive->zip = zip_open(ipsw, 0, &err);
	if (archive->zip == NULL) {
		printf("ERROR: zip_open: %s: %d\n", ipsw, err);
		free(archive);
		return NULL;
	}

	return archive;
}

int ipsw_extract_to_file(const char* ipsw, const char* infile, const char* outfile) {
	ipsw_archive* archive = ipsw_open(ipsw);
	if (archive == NULL || archive->zip == NULL) {
		printf("ERROR: Invalid archive\n");
		return -1;
	}

	int zindex = zip_name_locate(archive->zip, infile, 0);
	if (zindex < 0) {
		printf("ERROR: zip_name_locate: %s\n", infile);
		return -1;
	}

	struct zip_stat zstat;
	zip_stat_init(&zstat);
	if (zip_stat_index(archive->zip, zindex, 0, &zstat) != 0) {
		printf("ERROR: zip_stat_index: %s\n", infile);
		return -1;
	}

	char* buffer = (char*) malloc(BUFSIZE);
	if (buffer == NULL) {
		printf("ERROR: Unable to allocate memory\n");
		return -1;
	}

	struct zip_file* zfile = zip_fopen_index(archive->zip, zindex, 0);
	if (zfile == NULL) {
		printf("ERROR: zip_fopen_index: %s\n", infile);
		return -1;
	}

	FILE* fd = fopen(outfile, "wb");
	if (fd == NULL) {
		printf("ERROR: Unable to open output file: %s\n", outfile);
		zip_fclose(zfile);
		return -1;
	}

	int i = 0;
	int size = 0;
	int bytes = 0;
	int count = 0;
	double progress = 0;
	for(i = zstat.size; i > 0; i -= count) {
		if (i < BUFSIZE)
			size = i;
		else
			size = BUFSIZE;
		count = zip_fread(zfile, buffer, size);
		if (count < 0) {
			printf("ERROR: zip_fread: %s\n", infile);
			zip_fclose(zfile);
			free(buffer);
			return -1;
		}
		fwrite(buffer, 1, count, fd);

		bytes += size;
		progress = ((double) bytes/ (double) zstat.size) * 100.0;
		//print_progress_bar(progress);
	}

	fclose(fd);
	zip_fclose(zfile);
	ipsw_close(archive);
	free(buffer);
	return 0;
}

int ipsw_extract_to_memory(const char* ipsw, const char* infile, char** pbuffer, uint32_t* psize) {
	ipsw_archive* archive = ipsw_open(ipsw);
	if (archive == NULL || archive->zip == NULL) {
		printf("ERROR: Invalid archive\n");
		return -1;
	}

	int zindex = zip_name_locate(archive->zip, infile, 0);
	if (zindex < 0) {
		printf("ERROR: zip_name_locate: %s\n", infile);
		return -1;
	}

	struct zip_stat zstat;
	zip_stat_init(&zstat);
	if (zip_stat_index(archive->zip, zindex, 0, &zstat) != 0) {
		printf("ERROR: zip_stat_index: %s\n", infile);
		return -1;
	}

	struct zip_file* zfile = zip_fopen_index(archive->zip, zindex, 0);
	if (zfile == NULL) {
		printf("ERROR: zip_fopen_index: %s\n", infile);
		return -1;
	}

	int size = zstat.size;
	char* buffer = (unsigned char*) malloc(size);
	if (buffer == NULL) {
		printf("ERROR: Out of memory\n");
		zip_fclose(zfile);
		return -1;
	}

	if (zip_fread(zfile, buffer, size) != size) {
		printf("ERROR: zip_fread: %s\n", infile);
		zip_fclose(zfile);
		free(buffer);
		return -1;
	}

	zip_fclose(zfile);
	ipsw_close(archive);

	*pbuffer = buffer;
	*psize = size;
	return 0;
}

int ipsw_extract_build_manifest(const char* ipsw, plist_t* buildmanifest, int *tss_enabled) {
	int size = 0;
	char* data = NULL;

	*tss_enabled = 0;

	/* older devices don't require personalized firmwares and use a BuildManifesto.plist */
	if (ipsw_extract_to_memory(ipsw, "BuildManifesto.plist", &data, &size) == 0) {
		plist_from_xml(data, size, buildmanifest);
		return 0;
	}

	data = NULL;
	size = 0;

	/* whereas newer devices do not require personalized firmwares and use a BuildManifest.plist */
	if (ipsw_extract_to_memory(ipsw, "BuildManifest.plist", &data, &size) == 0) {
		*tss_enabled = 1;
		plist_from_xml(data, size, buildmanifest);
		return 0;
	}

	return -1;
}

int build_identity_get_component_path(plist_t build_identity, const char* component, char** path) {
	char* filename = NULL;

	plist_t manifest_node = plist_dict_get_item(build_identity, "Manifest");
	if (!manifest_node || plist_get_node_type(manifest_node) != PLIST_DICT) {
		printf("ERROR: Unable to find manifest node\n");
		if (filename)
			free(filename);
		return -1;
	}

	plist_t component_node = plist_dict_get_item(manifest_node, component);
	if (!component_node || plist_get_node_type(component_node) != PLIST_DICT) {
		printf("ERROR: Unable to find component node for %s\n", component);
		if (filename)
			free(filename);
		return -1;
	}

	plist_t component_info_node = plist_dict_get_item(component_node, "Info");
	if (!component_info_node || plist_get_node_type(component_info_node) != PLIST_DICT) {
		printf("ERROR: Unable to find component info node for %s\n", component);
		if (filename)
			free(filename);
		return -1;
	}

	plist_t component_info_path_node = plist_dict_get_item(component_info_node, "Path");
	if (!component_info_path_node || plist_get_node_type(component_info_path_node) != PLIST_STRING) {
		printf("ERROR: Unable to find component info path node for %s\n", component);
		if (filename)
			free(filename);
		return -1;
	}
	plist_get_string_val(component_info_path_node, &filename);

	*path = filename;
	return 0;
}

plist_t build_manifest_get_build_identity(plist_t build_manifest, uint32_t identity) {
	// fetch build identities array from BuildManifest
	plist_t build_identities_array = plist_dict_get_item(build_manifest, "BuildIdentities");
	if (!build_identities_array || plist_get_node_type(build_identities_array) != PLIST_ARRAY) {
		printf("ERROR: Unable to find build identities node\n");
		return NULL;
	}

	// check and make sure this identity exists in buildmanifest
	if (identity >= plist_array_get_size(build_identities_array)) {
		return NULL;
	}

	plist_t build_identity = plist_array_get_item(build_identities_array, identity);
	if (!build_identity || plist_get_node_type(build_identity) != PLIST_DICT) {
		printf("ERROR: Unable to find build identities node\n");
		return NULL;
	}

	return plist_copy(build_identity);
}

void ipsw_close(ipsw_archive* archive) {
	if (archive != NULL) {
		zip_unchange_all(archive->zip);
		zip_close(archive->zip);
		free(archive);
	}
}

/*
 * All right, all code following this is Copyright 2010 boxingsquirrel.
 */

char *ipsw;
char *out_dir;
char *app_bundle;
char *target[512];
char *fail_log[1024];

char *device_type=NULL;

int read_info_from_ipsw=1;

int ipsw_extract_all(char *ipsw_loc, char *out_loc, char *bundle_loc, int fake, char **fake_device, char **fake_version)
{
	ipsw=ipsw_loc;
	out_dir=out_loc;
	app_bundle=bundle_loc;
	//strcpy(ipsw, ipsw_loc);
	//strcpy(out_dir, out_loc);
	printf("File is %s\n", ipsw);

	if (fake==1)
	{
		read_info_from_ipsw=0;
		char target_str[512];
		snprintf(target_str, 512, "%s_%s", fake_device, fake_version);
		strcpy((char *)target, (char *)target_str);
		strcpy(device_type, fake_device);
		printf("INFO: The target has been manually set to %s. Using keys for that firmware...\n", target);
	}

	progress("Verifying...");

	ipsw_t ipsw_file=NULL;
	ipsw_file=(ipsw_t)malloc(sizeof(struct ipsw_s));
	ipsw_file=load_ipsw((const char *)ipsw);
	char path[512];
	snprintf(path, 512, "%s/Contents/Resources/%s.plist", app_bundle, target);
	int f_ok=r_file((const char *)path);
	if (f_ok==-1)
	{
		//gtk_button_set_label((GtkButton *)button1, "Extract");
		info("Could not find keys for the IPSW!", "I ain't got no keys!");
		return -1;
	}

	printf("INFO: %s\n", ipsw_file->platform);

	progress("Setting up the directory tree...");
	char fw_dir[512];
	snprintf(fw_dir, 512, "%s/%s", out_dir, "Firmware");
	char dfu_dir[512];
	snprintf(dfu_dir, 512, "%s/%s", fw_dir, "dfu");
	char all_flash_dir[512];
	snprintf(all_flash_dir, 512, "%s/%s", fw_dir, "all_flash");
	char all_flash_production_dir[512];
	snprintf(all_flash_production_dir, 512, "%s/%s.%s.%s", all_flash_dir, "all_flash", ipsw_file->platform, "production");

	mkdir(fw_dir, 0777);
	mkdir(dfu_dir, 0777);
	mkdir(all_flash_dir, 0777);
	mkdir(all_flash_production_dir, 0777);

	progress("Extracting the filesystem...");
	ipsw_extract_file((const char *)ipsw, (const char *)ipsw_file->fs, ipsw_file->fs);

	progress("Extracting the restore ramdisk...");
	ipsw_extract_file((const char *)ipsw, (const char *)ipsw_file->restore_ramdisk, (const char *)ipsw_file->restore_ramdisk);

	progress("Extracting the kernel cache...");
	ipsw_extract_file((const char *)ipsw, (const char *)ipsw_file->kernelcache, (const char *)ipsw_file->kernelcache);

	progress("Extracting the DFU bootloaders (iBEC and iBSS)...");
	ipsw_extract_file((const char *)ipsw, (const char *)ipsw_file->ibec_dfu, (const char *)ipsw_file->ibec_dfu);
	ipsw_extract_file((const char *)ipsw, (const char *)ipsw_file->ibss_dfu, (const char *)ipsw_file->ibss_dfu);

	progress("Extracting the boot and recovery logos...");
	ipsw_extract_file((const char *)ipsw, (const char *)ipsw_file->apple_logo, (const char *)ipsw_file->apple_logo);
	ipsw_extract_file((const char *)ipsw, (const char *)ipsw_file->recovery_mode, (const char *)ipsw_file->recovery_mode);

	progress("Extracting the 'Need Service' stuff...");
	//ipsw_extract_file((const char *)ipsw, (const char *)ipsw_file->need_service, "Firmware/all_flash/all_flash.production/needservice.img3");

	progress("Extracting the battery state images...");
	ipsw_extract_file((const char *)ipsw, (const char *)ipsw_file->batterycharging0, (const char *)ipsw_file->batterycharging0);
	ipsw_extract_file((const char *)ipsw, (const char *)ipsw_file->batterycharging1, (const char *)ipsw_file->batterycharging1);
	ipsw_extract_file((const char *)ipsw, (const char *)ipsw_file->batteryfull, (const char *)ipsw_file->batteryfull);
	ipsw_extract_file((const char *)ipsw, (const char *)ipsw_file->batterylow0, (const char *)ipsw_file->batterylow0);
	ipsw_extract_file((const char *)ipsw, (const char *)ipsw_file->batterylow1, (const char *)ipsw_file->batterylow1);
	ipsw_extract_file((const char *)ipsw, (const char *)ipsw_file->glyph_charging, (const char *)ipsw_file->glyph_charging);
	ipsw_extract_file((const char *)ipsw, (const char *)ipsw_file->glyph_plugin, (const char *)ipsw_file->glyph_plugin);

	progress("Extracting the device tree...");
	ipsw_extract_file((const char *)ipsw, (const char *)ipsw_file->device_tree, (const char *)ipsw_file->device_tree);

	progress("Extracting the bootloaders (iBoot and LLB)...");
	ipsw_extract_file((const char *)ipsw, (const char *)ipsw_file->iBoot, (const char *)ipsw_file->iBoot);
	ipsw_extract_file((const char *)ipsw, (const char *)ipsw_file->LLB, (const char *)ipsw_file->LLB);

	if (ipsw_file->has_bb!=0 && ipsw_file->bb_in_ramdisk==0)
	{
		progress("Extracting the baseband...");
		iPhone4_do_bb_extract(ipsw, ipsw_file);
	}

	progress("Decrypting the filesystem..."); 
	decrypt_fs((const char *)ipsw_file->fs);

	progress("Decrypting the restore ramdisk...");
	decrypt_img3_file((const char *)ipsw_file->restore_ramdisk, "RestoreRamDisk");

	progress("Decryptng iBoot...");
	decrypt_img3_file((const char *)ipsw_file->iBoot, "iBoot");

	progress("Decypting the battery images...");
	decrypt_img3_file((const char *)ipsw_file->glyph_charging, "GlyphCharging");
	decrypt_img3_file((const char *)ipsw_file->batterycharging0, "BatteryCharging0");
	decrypt_img3_file((const char *)ipsw_file->batterycharging1, "BatteryCharging1");
	decrypt_img3_file((const char *)ipsw_file->batteryfull, "BatteryFull");
	decrypt_img3_file((const char *)ipsw_file->batterylow0, "BatteryLow0");
	decrypt_img3_file((const char *)ipsw_file->batterylow1, "BatteryLow1");
	decrypt_img3_file((const char *)ipsw_file->glyph_plugin, "GlyphPlugin");

	progress("Decrypting the boot and recovery logos...");
	decrypt_img3_file((const char *)ipsw_file->apple_logo, "AppleLogo");
	decrypt_img3_file((const char *)ipsw_file->recovery_mode, "RecoveryMode");

	progress("Decrypting the LLB...");
	decrypt_img3_file("Firmware/all_flash/all_flash.production/LLB.img3", "LLB");

	progress("Decrypting the device tree...");
	decrypt_img3_file((const char *)ipsw_file->device_tree, "DeviceTree");

	progress("Decrypting the KernelCache...");
	decrypt_img3_file((const char *)ipsw_file->kernelcache, "KernelCache");

	progress("Decrypting iBEC...");
	decrypt_img3_file((const char *)ipsw_file->ibec_dfu, "iBEC");

	progress("Decrypting iBSS...");
	decrypt_img3_file((const char *)ipsw_file->ibss_dfu, "iBSS");

	if (ipsw_file->has_bb!=0 && ipsw_file->bb_in_ramdisk!=0)
	{
		progress("Extracting the baseband...");
		iPhone_do_bb_extract(app_bundle, ipsw, ipsw_file);
	}

/*	progress("Extracting all files from Filesystem...");
	char fs_dir[512];
	snprintf(fs_dir, 512, "%s/%s", out_dir, "Extracted_Filesystem");
	mkdir(fs_dir, 0777);
	char cmd[512];
	snprintf(cmd, 512, "/usr/share/ipswTool/scripts/extract_fs.py %s/%s %s", out_dir, (const char *)ipsw_file->fs, fs_dir);
	system((const char *)cmd);
*/

	progress("Finished!");

	if (strcmp((const char *)fail_log, "")!=0)
	{
		info((const char *)fail_log, "The following errors occured:");
	}
	else {
		info("Everything in the IPSW was decrypted!", "All done!");
	}

	return 0;
}

void progress(const char *what)
{
	printf("INFO: %s\n", what);

	//gtk_button_set_label((GtkButton *)button1, what);
	//gtk_main_iteration();
}

void add_to_fail(const char *what)
{
	//printf("LOG: %s->%s\n", what, fail_log);
	char e[512];
	snprintf(e, 512, "%s\n", what);
	strcat((char *)fail_log, (char *)e);
}

/*
void show_fail()
{
	GtkBuilder *builder;
	GError* error = NULL;

	builder = gtk_builder_new ();
	if (!gtk_builder_add_from_file (builder, "/usr/share/ipswTool/res/ui.xml", &error))
	{
		g_warning ("Couldn't load builder file: %s", error->message);
		g_error_free (error);
	}

	gtk_builder_connect_signals (builder, NULL);
	window = GTK_WIDGET (gtk_builder_get_object (builder, "window1"));
	file_choose=GTK_WIDGET(gtk_builder_get_object(builder, "file_choose"));
	dir_choose=GTK_WIDGET(gtk_builder_get_object(builder, "file_choose1"));
	button1=GTK_WIDGET(gtk_builder_get_object(builder, "button1"));

	g_signal_connect (G_OBJECT (file_choose), "file-set",
		      G_CALLBACK (set_file), NULL);
	g_signal_connect (G_OBJECT (dir_choose), "file-set",
		      G_CALLBACK (set_dir), NULL);
	g_signal_connect (G_OBJECT (button1), "released",
		      G_CALLBACK (ipsw_extract_thread), NULL);
        g_signal_connect (G_OBJECT (window), "destroy",
		      G_CALLBACK (destroy), NULL);

	g_object_unref (builder);

	gtk_widget_show (window);

}
*/

int replace_file(const char *oldfile, const char *newfile)
{
	remove(oldfile);

	rename(newfile, oldfile);

	return 0;
}

void decrypt_img3_file(const char *file, const char *what)
{
	//printf("INFO: Going to decrypt %s now...\n", file);

	char mess[512];
	//snprintf(mess, 512, "Decrypting %s", what);

	//progress((const char *)mess);

	char *key=NULL;
	char *iv=NULL;

	char out_file[512];
	char in_file[512];

	int hasKey=check_key((char *)target, (char *)what);
	int hasIV=check_iv((char *)target, (char *)what);

	if (hasKey==1 && hasIV==1)
	{
		key=get_key((char *)target, (char *)what);
		iv=get_iv((char *)target, (char *)what);
		printf("IMG3_KEY: %s\n", key);

		snprintf(in_file, 512, "%s/%s", out_dir, file);
		snprintf(out_file, 512, "%s/%s", out_dir, "scratch.img3");

		//decrypt_img3((char *)in_file, (char *)out_file, key, iv);
		char cmd[2048];
		snprintf(cmd, 2048, "%s/Contents/MacOS/xpwntool %s %s -k %s -iv %s", app_bundle, in_file, out_file, key, iv);
		system(cmd);
		replace_file((const char *)in_file, (const char *)out_file);

		printf("INFO: Decrypted %s\n", file);
	}

	else {
		snprintf(mess, 512, "Unable to decrypt %s!", what);
		//printf("%s\n", mess);
		add_to_fail((const char *)mess);
	}
}

void decrypt_fs(const char *file)
{
	printf("INFO: Going to decrypt %s now...\n", file);

	// Input and output files...
	char out_file[512];
	char in_file[512];

	// We only need the key, 'cause there's no IV
	char *key=NULL;
	key=get_key((char *)target, "OS");
	printf("KEY:%s\n", key);

	// Put together the paths...
	snprintf(in_file, 512, "%s/%s", out_dir, file);
	snprintf(out_file, 512, "%s/%s", out_dir, "scratch.dmg");

	// Do the decryption!
	//decrypt_root_fs((const char *)in_file, (const char *)out_file, key);
	char cmd[2048];
	snprintf(cmd, 2048, "%s/Contents/MacOS/dmg extract %s %s -k %s", app_bundle, in_file, out_file, key);
	system(cmd);
	// And move the scratch file back to the correct name...
	replace_file((const char *)in_file, (const char *)out_file);

	printf("INFO: Decrypted %s\n", file);
}

ipsw_t load_ipsw(const char *ipsw)
{
	plist_t man=NULL;
	char *ios_v=NULL;
	char *device_target=NULL;
	int tss=0;

	ipsw_t ipsw_cont=(ipsw_t)malloc(sizeof(struct ipsw_s));

	int e=ipsw_extract_build_manifest((const char *)ipsw, &man, &tss);

	if (e!=0)
	{
		return ipsw_cont;
	}

	else {
		printf("Starting extraction now...\n");		
		plist_t build_ident=build_manifest_get_build_identity(man, 0);

		plist_t targets_array=plist_dict_get_item(man, "SupportedProductTypes");
		if (targets_array!=NULL)
		{
			plist_t target_node=plist_array_get_item(targets_array, 0);

			if (target_node!=NULL)
			{
				plist_get_string_val(target_node, &device_target);
				plist_free(target_node);
				target_node=NULL;
			}

			else {
				printf("ERROR: Could not get the target :(\n");
			}
		}

		else {
			printf("ERROR: Could not get the array of targets :(\n");
		}

		plist_t info=plist_dict_get_item(build_ident, "Info");
		if (info!=NULL)
		{
			plist_t plat_node=plist_dict_get_item(info, "DeviceClass");

			if (plat_node!=NULL)
			{
				plist_get_string_val(plat_node, &ipsw_cont->platform);
				plist_free(plat_node);
				plat_node=NULL;
			}

			else {
				printf("ERROR: Could not get the platform :(\n");
			}
		}

		else {
			printf("ERROR: Could not get the 'Info' dict :(\n");
		}

		plist_t node=plist_dict_get_item(man, "ProductVersion");
		plist_get_string_val(node, &ios_v);
		plist_free(node);
		node=NULL;

		if (read_info_from_ipsw==1)
		{
			char target_str[512];
			snprintf(target_str, 512, "%s_%s", device_target, ios_v);
			strcpy((char *)target, (char *)target_str);
		}

		ipsw_cont->device=device_target;
		printf("INFO: %s\n", ipsw_cont->device);
		//strcpy(ipsw_cont->target, dev_ident);

		printf("INFO: %s\n", ipsw_cont->platform);

		//printf("TARGET: %s\n", ipsw_cont->target);

		printf("INFO: Getting OS path...\n");
		build_identity_get_component_path(build_ident, "OS", &ipsw_cont->fs);

		printf("INFO: Getting restore ramdisk path...\n");
		build_identity_get_component_path(build_ident, "RestoreRamDisk", &ipsw_cont->restore_ramdisk);

		printf("INFO: Getting KernelCache path...\n");
		build_identity_get_component_path(build_ident, "KernelCache", &ipsw_cont->kernelcache);

		printf("INFO: Getting AppleLogo path...\n");
		build_identity_get_component_path(build_ident, "AppleLogo", &ipsw_cont->apple_logo);

		printf("INFO: Getting Battery stuff path...\n");
		build_identity_get_component_path(build_ident, "BatteryCharging", &ipsw_cont->glyph_charging);
		build_identity_get_component_path(build_ident, "BatteryCharging0", &ipsw_cont->batterycharging0);
		build_identity_get_component_path(build_ident, "BatteryCharging1", &ipsw_cont->batterycharging1);
		build_identity_get_component_path(build_ident, "BatteryFull", &ipsw_cont->batteryfull);
		build_identity_get_component_path(build_ident, "BatteryLow0", &ipsw_cont->batterylow0);
		build_identity_get_component_path(build_ident, "BatteryLow1", &ipsw_cont->batterylow1);
		build_identity_get_component_path(build_ident, "BatteryPlugin", &ipsw_cont->glyph_plugin);

		printf("INFO: Getting device tree...\n");
		build_identity_get_component_path(build_ident, "DeviceTree", &ipsw_cont->device_tree);

		printf("INFO: Getting recovery mode image...\n");
		build_identity_get_component_path(build_ident, "RecoveryMode", &ipsw_cont->recovery_mode);

		printf("INFO: Getting 'NeedService'...\n");
		//build_identity_get_component_path(build_ident, "NeedService", &ipsw_cont->need_service);

		printf("INFO: Getting the various bootloaders...\n");
		build_identity_get_component_path(build_ident, "LLB", &ipsw_cont->LLB);
		build_identity_get_component_path(build_ident, "iBoot", &ipsw_cont->iBoot);
		build_identity_get_component_path(build_ident, "iBEC", &ipsw_cont->ibec_dfu);
		build_identity_get_component_path(build_ident, "iBSS", &ipsw_cont->ibss_dfu);

		printf("INFO: Getting the baseband...\n");
		if (read_info_from_ipsw==0)
		{
			if (strcmp(device_type, "iPhone3,1")==0)
			{
				build_identity_get_component_path(build_ident, "BasebandFirmware", &ipsw_cont->bb);
				ipsw_cont->bb_in_ramdisk=0;
				ipsw_cont->has_bb=1;
			}
			else {
				if (strcmp(device_type, "iPod1,1")==0 || strcmp(device_type, "iPod2,1")==0 || strcmp(device_type, "iPod3,1")==0 || strcmp(device_type, "iPod4,1")==0 || strcmp(device_type, "iPad1,1")==0 || strcmp(device_type, "iProd2,1")==0 || strcmp(device_type, "AppleTV2,1")==0)
				{
					ipsw_cont->has_bb=0;
				}
				else {
					ipsw_cont->bb_in_ramdisk=1;
				}
			}
		}
		else {
			if (strcmp(device_target, "iPhone3,1")==0)
			{
				build_identity_get_component_path(build_ident, "BasebandFirmware", &ipsw_cont->bb);
				ipsw_cont->bb_in_ramdisk=0;
				ipsw_cont->has_bb=1;
			}
			else {
				if (strcmp(device_target, "iPod1,1")==0 || strcmp(device_target, "iPod2,1")==0 || strcmp(device_target, "iPod3,1")==0 || strcmp(device_target, "iPod4,1")==0 || strcmp(device_target, "iPad1,1")==0 || strcmp(device_target, "iProd2,1")==0 || strcmp(device_target, "AppleTV2,1")==0)
				{
					ipsw_cont->has_bb=0;
				}
				else {
					//ipsw_cont->bb=NULL;
					//strcpy(ipsw_cont->bb, "/usr/local/standalone");
					ipsw_cont->bb_in_ramdisk=1;
				}
			}
		}

		plist_free(man);

		return ipsw_cont;
	}
	return NULL;
}

char *get_key(char *target, char *comp)
{
	char keys_plist[4096];
	char *key=NULL;

	char path[512];
	snprintf(path, 512, "%s/Contents/Resources/%s.plist", app_bundle, target);
	//printf("PATH_TO_KEY_FILE:%s\n", path);
	int f_ok=r_file((const char *)path);

	if (f_ok!=-1)
	{
		strcpy(keys_plist, data);

		//printf("INFO: Loaded keys.plist...\n");

		plist_t keys=NULL;
		plist_from_xml(keys_plist, strlen(keys_plist), &keys);

		//printf("INFO: Converted keys.plist from XML\n");

		plist_t target_item=plist_dict_get_item(keys, target);

		if (target_item!=NULL)
		{
			//printf("INFO: Got the target's node\n");
			plist_t comp_item=plist_dict_get_item(target_item, comp);

			if (comp_item!=NULL)
			{
				//printf("INFO: Got the component's node\n");
				plist_t key_item=plist_dict_get_item(comp_item, "Key");
				//printf("INFO: Got the key's node\n");

				if (key_item!=NULL)
				{
					plist_get_string_val(key_item, &key);
					//printf("INFO: Got the key! %s\n", key);
					return key;
				}

				else {
					printf("ERROR: Couldn't get the key :(\n");
				}
			}
		}

		return key;
	}

	else {
		return (char *)"?";
	}
}

int check_key(char *target, char *comp)
{
	char *q=NULL;
	q=get_key(target, comp);

	if (strcmp(q, "?")!=0)
	{
		return 1;
	}

	return 0;
}

char *get_iv(char *target, char *comp)
{
	char keys_plist[4096];
	char *key=NULL;

	char path[512];
	snprintf(path, 512, "%s/Contents/Resources/%s.plist", app_bundle, target);
	int f_ok=r_file((const char *)path);

	if (f_ok!=-1)
	{
		strcpy(keys_plist, data);

		//printf("INFO: Loaded keys.plist...\n");

		plist_t keys=NULL;
		plist_from_xml(keys_plist, strlen(keys_plist), &keys);

		//printf("INFO: Converted keys.plist from XML\n");

		plist_t target_item=plist_dict_get_item(keys, target);

		if (target_item!=NULL)
		{
			//printf("INFO: Got the target's node\n");
			plist_t comp_item=plist_dict_get_item(target_item, comp);

			if (comp_item!=NULL)
			{
				//printf("INFO: Got the component's node\n");
				plist_t key_item=plist_dict_get_item(comp_item, "IV");
				//printf("INFO: Got the key's node\n");

				if (key_item!=NULL)
				{
					plist_get_string_val(key_item, &key);
					//printf("INFO: Got the key! %s\n", key);
				}

				else {
					printf("ERROR: Couldn't get the IV :(\n");
				}
			}
		}

		return key;
	}

	else {
		return (char *)"?";
	}
}

int check_iv(char *target, char *comp)
{
	char *q=get_iv(target, comp);

	if (strcmp(q, "?")!=0)
	{
		return 1;
	}

	return 0;
}


int ipsw_extract_file(const char *ipsw, const char *file, const char *dest_file_name)
{
	char out_file[512];
	snprintf(out_file, 512, "%s/%s", out_dir, dest_file_name);
	return ipsw_extract_to_file(ipsw, file, (const char *)out_file);
}
