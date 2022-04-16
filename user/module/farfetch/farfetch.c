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
#include <linux/slab.h>
#include <linux/highmem.h>
#include <linux/delay.h>

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
	struct page **targetpage;
	void *pageaddr;
	long failed_bytes;
	unsigned long nr_pages, offset;
	int is_root, i, j;
	unsigned long copied;
	unsigned int gup_flags = FOLL_FORCE;
	unsigned long ret;

	if (cmd == FAR_WRITE)
		gup_flags |= FOLL_WRITE;

	nr_pages = 1;
	targetpid = find_get_pid(target_pid);
	targettask = get_pid_task(targetpid, PIDTYPE_PID);
	put_pid(targetpid);
	if (!targettask)
		return -ESRCH;

	is_root = !from_kuid_munged(current_user_ns(), task_euid(current));
	if (!is_root) {
		put_task_struct(targettask);
		return -EPERM;
	}

	if (target_addr >= TASK_SIZE_OF(targettask)) {
		put_task_struct(targettask);
		return -EFAULT;
	}

	targetmm = targettask->mm;
	put_task_struct(targettask);
	offset = offset_in_page(target_addr);
	if (len > PAGE_SIZE - offset) { //ceil the page
		if ((len - (PAGE_SIZE - offset))%PAGE_SIZE != 0)
			nr_pages = 2 + (len - (PAGE_SIZE - offset))/PAGE_SIZE;
		else
			nr_pages = 1 + (len - (PAGE_SIZE - offset))/PAGE_SIZE;
	}

	targetpage = kmalloc_array(nr_pages, sizeof(struct page *), GFP_KERNEL);
	if (targetpage == NULL)
		return -ENOMEM;

	ret = get_user_pages_remote(targetmm, target_addr, nr_pages,
			gup_flags, targetpage, NULL, NULL); //number of page

	if (ret <= 0) {
		kfree(targetpage);
		return ret;
	} else if (ret < nr_pages) {
		len = ret * PAGE_SIZE - offset; //length we get
	}


	copied = 0;
	for (i = 0; i < ret; i++) {
		pageaddr = kmap(targetpage[i]);
		if (i >  0)
			offset = 0;
		if (cmd == FAR_READ) {
			failed_bytes = copy_to_user(addr + copied, pageaddr + offset,
					min(PAGE_SIZE - offset, len - copied));
			if (failed_bytes) {
				for (j = i; j < ret; j++)
					put_page(targetpage[j]);
				kunmap(targetpage[i]);
				kfree(targetpage);
				return -EFAULT;
			}
			copied += min(PAGE_SIZE - offset, len - copied);
		} else if (cmd == FAR_WRITE) {
			failed_bytes = copy_from_user(pageaddr + offset, addr + copied,
					min(PAGE_SIZE - offset, len - copied));
			if (failed_bytes) {
				for (j = i; j < ret; j++)
					put_page(targetpage[j]);
				kunmap(targetpage[i]);
				kfree(targetpage);
				return -EFAULT;
			}
			copied += min(PAGE_SIZE - offset, len - copied);
			set_page_dirty_lock(targetpage[i]);
		}
		kunmap(targetpage[i]);
		put_page(targetpage[i]);
	}
	kfree(targetpage);
	return copied;
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
