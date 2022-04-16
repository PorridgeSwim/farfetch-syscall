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
	unsigned long offset, nr_pages;
	int is_root, is_self, i, j;
	unsigned long copied;

	struct vm_area_struct *vma;
	unsigned int gup_flags = FOLL_FORCE;
	unsigned long ret;

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
	offset = offset_in_page(target_addr);
	if (len > PAGE_SIZE - offset) { //ceil the page
		if ((len - (PAGE_SIZE - offset))%PAGE_SIZE != 0)
			nr_pages = 2 + (len - (PAGE_SIZE - offset))/PAGE_SIZE;
		else
			nr_pages = 1 + (len - (PAGE_SIZE - offset))/PAGE_SIZE;
	}

	targetpage = kmalloc(nr_pages * sizeof(struct page *), GFP_KERNEL);
	if (targetpage == NULL)
		return -ENOMEM;
	pr_info("here\n");
	ret = get_user_pages_remote(targetmm, target_addr, nr_pages, 
					gup_flags, targetpage, &vma, NULL); //number of page
	
	if (ret <= 0) {
		kfree(targetpage);
		return ret;
	} else if (ret < nr_pages) {
		len = ret * PAGE_SIZE - offset; //length we get
	}

	pr_info("%lu\n", sizeof(addr));

	pr_info("ret = %lu\n", ret);
	copied = 0;
	for (i = 0; i < ret; i++) {
		// pageaddr = page_address(targetpage[i]);
		pageaddr = kmap(targetpage[i]);
		if (i >  0)
			offset = 0;

		pr_info("len is %zu, copied is %lu, pageaddr is %lu, offset is %lu, pagesize = %lu\n", len, copied, target_addr, offset, PAGE_SIZE);
		if (cmd == FAR_READ) {
			if ((failed_bytes = copy_to_user(addr + copied, pageaddr + offset, min(PAGE_SIZE - offset, len - copied)))) {
				pr_info("copy_to_user fail %d", i);
				for (j = i; j < ret; j++)
					put_page(targetpage[j]);
				kunmap(targetpage[i]);
				kfree(targetpage);
				return -EFAULT;
			}
			else{
				copied += min(PAGE_SIZE - offset, len - copied);
				pr_info("copy_to_user succedd %d", i);
			}
		} else if (cmd == FAR_WRITE) {
			if ((failed_bytes = copy_from_user(pageaddr + offset, addr + copied, min(PAGE_SIZE - offset, len - copied)))) {
				pr_info("copy_from_user fail %d", i);
				for (j = i; j < ret; j++)
					put_page(targetpage[j]);
				kunmap(targetpage[i]);
				kfree(targetpage);
				return -EFAULT;
			}
			else{
				copied += min(PAGE_SIZE - offset, len - copied);
				pr_info("copy_from_user succeed %d", i);
			}
			set_page_dirty_lock(targetpage[i]);
		}
		kunmap(targetpage[i]);
		put_page(targetpage[i]);

	}

	//kfree(targetpage);
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
