/*
 * farfetch.c
 */
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/farfetch.h>

#include <linux/kernel.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/proc_fs.h>
#include <linux/pid.h>
#include <linux/pgtable.h>
#include <linux/mm.h>
#include <asm/page.h>
#include <linux/pgtable.h>

extern long (*farfetch_ptr)(unsigned int cmd, void __user *addr,
			    pid_t target_pid, unsigned long target_addr,
			    size_t len);
extern long farfetch_default(unsigned int cmd, void __user *addr,
			     pid_t target_pid, unsigned long target_addr,
			     size_t len);

long farfetch(unsigned int cmd, void __user *addr, pid_t target_pid,
	      unsigned long target_addr, size_t len)
{
	struct pid *targetpid;
	struct task_struct *targettask;
	struct mm_struct *targetmm;
	struct page *targetpage;
	void *pageaddr;
	long failed_bytes;
	unsigned long offset, nr_pages;
	int is_root, is_self, ret, i;

	struct vm_area_struct *vma;
	unsigned int gup_flags = FOLL_FORCE;

	if (cmd == FAR_WRITE) 
		gup_flags |= FOLL_COW;

	nr_pages = 1;
	targetpid = find_get_pid(target_pid);
	targettask = get_pid_task(targetpid, PIDTYPE_PID);
	put_pid(targetpid);
	if (!targettask)
		return -ESRCH;

	is_root = !from_kuid_munged(current_user_ns(), task_euid(current));
	is_self = uid_eq(task_euid(current), task_uid(targettask));
	if (!is_root && !is_self) {
		put_task_struct(targettask);
		return -EPERM;
	}

	if (target_addr >= TASK_SIZE_OF(targettask)) {
		put_task_struct(targettask);
		return -EFAULT;
	}

	targetmm = targettask->mm;
	put_task_struct(targettask);

	if (len > PAGE_SIZE - offset) {
		if ((len - (PAGE_SIZE - offset))%PAGE_SIZE != 0)
			nr_pages = 2 + (len - (PAGE_SIZE - offset))/PAGE_SIZE;
		else
			nr_pages = 1 + (len - (PAGE_SIZE - offset))/PAGE_SIZE;
	}

	ret = get_user_pages_remote(targetmm, target_addr, nr_pages, 
					gup_flags, &targetpage, &vma, NULL);
	
	if (ret <= 0) {
		return -EFAULT;
	} else if (ret < nr_pages) {
		len = ret * PAGE_SIZE - offset;
	}

	for (i = 0; i < ret; i++) {

		pageaddr = page_address(targetpage);
		offset = offset_in_page(target_addr);

		pr_info("len is %zu and pageaddr is %lu, offset is %lu, pagesize = %lu\n", len, target_addr, offset, PAGE_SIZE);
		if (cmd == FAR_READ) {
			if ((failed_bytes = copy_to_user(addr, pageaddr + offset, min(PAGE_SIZE - offset, len)))) {
				put_page(targetpage);
				return -EFAULT;
			}
		} else if (cmd == FAR_WRITE) {
			if (!pte_write(pte)) {
				put_page(targetpage);
				return -EFAULT;
			}
			if ((failed_bytes = copy_from_user(pageaddr + offset, addr, min(PAGE_SIZE - offset, len)))) {
				put_page(targetpage);
				return -EFAULT;
			}
			set_page_dirty_lock(targetpage);
		}
		put_page(targetpage);

	}

	return min(PAGE_SIZE - offset, len);
}

int farfetch_init(void)
{
	pr_info("Installing farfetch\n");
	farfetch_ptr = farfetch;
	return 0;
}

void farfetch_exit(void)
{
	pr_info("Removing farfetch\n");
	farfetch_ptr = farfetch_default;
}

module_init(farfetch_init);
module_exit(farfetch_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("farfetch: for fetching pages from afar");
MODULE_AUTHOR("Kent Hall");