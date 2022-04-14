/*
 * farfetch.c
 */
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/farfetch.h>

#include <linux/kernel.h>
#include <asm/uaccess.h>
#include <linux/fs.h> 
#include <linux/cdev.h>
#include <linux/proc_fs.h>
#include <linux/pid.h>
#include <linux/pgtable.h>
#include <linux/mm.h>
#include <asm/page.h>
#include <asm/pgtable.h>

#define _PAGE_READ          (1<<5)
#define pte_read(pte)		(pte_val(pte) & _PAGE_READ)
#define _PAGE_WRITE         (1<<4)
#define pte_write(pte)		(pte_val(pte) & _PAGE_WRITE)
extern long (*farfetch_ptr)(unsigned int cmd, void __user *addr,
			    pid_t target_pid, unsigned long target_addr,
			    size_t len);
extern long farfetch_default(unsigned int cmd, void __user *addr,
			     pid_t target_pid, unsigned long target_addr,
			     size_t len);

long farfetch(unsigned int cmd, void __user *addr, pid_t target_pid,
	      unsigned long target_addr, size_t len)
{
	/* implement here */
	struct pid *targetpid;
	struct task_struct *targettask;
	struct mm_struct *targetmm;
	struct page *targetpage;
	void * pageaddr;
	long failed_bytes;
	unsigned long offset;
	int is_root, is_self;
	pgd_t *pgd;
	pmd_t *pmd;
	pud_t *pud;
	p4d_t *p4d;
	pte_t *ptep, pte;

	targetpid = find_get_pid(target_pid);
	targettask = get_pid_task(targetpid, PIDTYPE_PID);
	put_pid(targetpid);
	if (!targettask)
		return -ESRCH;

	is_root = !from_kuid_munged(current_user_ns(), task_euid(current));
	is_self = uid_eq(task_euid(current), task_uid(targettask));
	if (!is_root && !is_self)
		return -EPERM;

	if (target_addr >= TASK_SIZE_OF(targettask))
		return -EFAULT;

	targetmm = targettask->mm;
	pgd = pgd_offset(targetmm, target_addr);

	put_task_struct(targettask);

	if (cmd != FAR_READ && cmd != FAR_WRITE)
		return -EINVAL;

	if (pgd_none(*pgd) || pgd_bad(*pgd))
		return -1;
	p4d = p4d_offset(pgd, target_addr);
	if (p4d_none(*p4d) || p4d_bad(*p4d))
		return -1;
	pud = pud_offset(p4d, target_addr);
	if (pud_none(*pud) || pud_bad(*pud))
		return -1;
	pmd = pmd_offset(pud, target_addr);
	if (pmd_none(*pmd) || pmd_bad(*pmd))
		return -1;
	ptep = pte_offset_map(pmd, target_addr);
	if (!ptep)
		return -1;
	pte = *ptep;
	pte_unmap(ptep);
	targetpage = pte_page(pte);
	pageaddr = page_address(targetpage);
	offset = offset_in_page(target_addr);

	if (cmd == FAR_READ) {
		if (!pte_read(pte))
			return -EPERM;
		if ((failed_bytes = copy_to_user(addr, pageaddr + offset,len)))
			return -EFAULT;
	} else if (cmd == FAR_WRITE) {
		if (!pte_write(pte))
			return -EPERM;
		if ((failed_bytes = copy_from_user(pageaddr + offset, addr, len)))
			return -EFAULT;
	}

	return len - failed_bytes;
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
