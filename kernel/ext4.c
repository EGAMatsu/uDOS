#include <stdint.h>
#include <panic.h>
#include <fs.h>

#define EXT4_MAGIC 0xEF53

struct Ext4Superblock {
    uint32_t total_inodes;
    uint32_t total_blocks;
    uint32_t reserved_blocks;
    uint32_t total_unallocated_blocks;
    uint32_t total_unallocated_inodes;
    uint32_t superblock_num;
    uint32_t block_size;
    uint32_t fragment_size;
    uint32_t num_blocks_in_group;
    uint32_t num_fragments_in_group;
    uint32_t num_inodes_in_group;
    uint32_t last_mount_time;
    uint32_t last_write_time;
    uint16_t times_mounted;
    uint16_t num_mounts_fsck;
    uint16_t magic;
    uint16_t fs_state;
    uint16_t on_error;
    uint16_t minor_version;
    uint32_t last_fsck_time;
    uint32_t fsck_interval;
    uint32_t os_id;
    uint32_t major_version;
    uint16_t user_id;
    uint16_t group_id;
};

int ModGetExt4File(
    struct fs_handle *hdl,
    struct fs_fdscb *out_fdscb,
    const char *name)
{
    struct fs_fdscb fdscb = { 0, 0, 0 };
    struct Ext4Superblock sblock;
    
    KeReadWithFdscbFsNode(hdl, &fdscb, &sblock, sizeof(struct Ext4Superblock));

    if(sblock.magic != EXT4_MAGIC) {
        KePanic("Invalid EXT4 signature");
        return -1;
    }
    return 0;
}
