/*************************************************************************
> FileName: demo-list-device.c
> Author  : DingJing
> Mail    : dingjing@live.cn
> Created Time: 2022年02月14日 星期一 11时55分11秒
 ************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <parted/parted.h>
#include <parted/filesys.h>

/**
 * @brief 执行需要 root 权限，否则没有任何输出
 */
int main (int argc, char* argv[])
{
    PedDevice*      cur = NULL;
    PedDevice*      next = NULL;

    // 检测所有设备
    ped_device_probe_all();

    while (NULL != (next = ped_device_get_next (cur))) {
        cur = next;
        next = NULL;
        PedDeviceType type = cur->type;                     // @note: 废弃的 api
        PedDisk* disk = ped_disk_new (cur);                 // 从设备读取分区表

        printf ("path: %s\n", cur->path);
        printf ("model: %s\n", cur->model);
        printf ("read only: %s\n", cur->read_only ? "yes" : "no");
        printf ("is busy: %s\n", ped_device_is_busy(cur) ? "yes" : "no");
        printf ("sector size: %lld\n", cur->sector_size);
        printf ("phys sector size: %lld\n", cur->phys_sector_size);

        if (!disk) continue;
        // ped_disk_is_flag_available
        printf ("type: %s\n", disk->type->name);
        printf ("partition table: %s\n", ped_disk_check (disk) ? "yes" : "no");
        printf ("highest partition number: %d\n", ped_disk_get_last_partition_num (disk));
        printf ("the number of primary partitions: %d\n", ped_disk_get_primary_partition_count (disk));
        printf ("maximum number of primary partitions: %d\n", ped_disk_get_max_primary_partition_count (disk));
//        ped_disk_print (disk);

        // partition
        PedPartition* cur1 = NULL;
        PedPartition* next1 = NULL;
        while (NULL != (next1 = ped_disk_next_partition (disk, cur1))) {
            cur1 = next1;
            next1 = NULL;

            const PedFileSystemType* fsType = cur1->fs_type;
            if (!fsType) continue;
            const char* name= ped_partition_type_get_name (cur1->type);

            printf ("    %s:\n", ped_partition_get_path (cur1));
            printf ("      is busy: %s\n", ped_partition_is_busy (cur1) ? "yes" : "no");
            printf ("      is active: %s\n", ped_partition_is_active (cur1) ? "yes" : "no");
            printf ("      filesystem type: %s\n", fsType->name);
            printf ("      partition type: \"%s\"\n", name? name : "<unknown>");
            printf ("\n");
        }

        printf ("\n");
    }

    return 0;
}
