/*
 * ipsw.c
 * Definitions for IPSW utilities
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

#ifndef IDEVICERESTORE_IPSW_H
#define IDEVICERESTORE_IPSW_H

#ifdef __cplusplus
extern "C" {
#endif

#include <zip.h>
#include <stdint.h>
#include <plist/plist.h>

typedef struct {
	int index;
	char* name;
	unsigned int size;
	unsigned char* data;
} ipsw_file;

int ipsw_extract_to_memory(const char* ipsw, const char* infile, char** pbuffer, uint32_t* psize);
void ipsw_free_file(ipsw_file* file);

extern int ipsw_extract_all(char *ipsw_loc, char *out_loc, char *bundle_loc, int fake, char **fake_device, char **fake_version);
extern char *get_key(char *target, char *comp);
extern char *get_iv(char *target, char *comp);
extern int check_key(char *target, char *comp);
extern int check_iv(char *target, char *comp);
extern int replace_file(const char *oldfile, const char *newfile);
extern void decrypt_img3_file(const char *file, const char *what);
extern void decrypt_fs(const char *file);
extern int ipsw_extract_file(const char *ipsw, const char *file, const char *dest_file_name);

// Defined in decrypt_root_fs.c (duh)
extern int decrypt_root_fs(const char *crypt, const char *decrypt, char *key);

	struct ipsw_s {
		char *device;
		char *fw_version;
		char *target; // The target, in the following format: [device]_[iOS Version]; ie 'iPad1,1_3.2'
		char *platform; // Platform, ie 'n81ap'...
		// .
		char *fs; // The main filesystem
		char *restore_ramdisk; // The restore mode ramdisk
		char *update_ramdisk; // The update mode ramdisk
		char *kernelcache; // The kernel and associated junk

		// Firmware/dfu
		char *ibec_dfu; // Stripped down version of iBoot, helps with restore from DFU
		char *ibss_dfu; // Bootstraps iBec during DFU restore

		// Firmware/all_flash/all_flash.*.production
		char *apple_logo; // The boot logo
		char *batterycharging0; // Battery charging image
		char *batterycharging1; // Battery charging image
		char *batteryfull; // Battery full image
		char *batterylow0; // Battery low image
		char *batterylow1; // Battery low image
		char *device_tree; // I'm guessing this is just the tree, but...?
		char *glyph_charging; // Battery charging image
		char *glyph_plugin; // Battery waay low image
		char *iBoot; // Stage 2 bootloader
		char *LLB; // Low level bootloader, sig checks and jumps to iBoot
		char *need_service; // What's this? This is NOT present on 4.0!
		char *recovery_mode; // Recovery mode image?

		// Baseband stuff (yawn)
		int has_bb; // Do we even /have/ a baseband?
		int bb_in_ramdisk; // Most devices have the baseband in the ramdisk, making it a royal pain in tha a**
		char *bb; // If bb_in_ramdisk==0, this should have the path to the baseband

	};

	typedef struct ipsw_s *ipsw_t;

	struct ipsw_info_s {
		char *target; // The target processor (sort of). For example, the S5L8930 in the iPad and iPhone 4 is reported as '0x8930'
		char *version; // The iOS version
	};

	typedef struct ipsw_info_s *ipsw_info_t;

extern char *ipsw;
extern char *out_dir;

extern ipsw_t load_ipsw(const char *ipsw);

#ifdef __cplusplus
}
#endif

#endif
