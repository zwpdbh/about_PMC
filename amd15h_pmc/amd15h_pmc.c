/*
 * Copyright (C) 2010 Silas Boyd-Wickizer
 *
 * All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE, GOOD TITLE or
 * NON INFRINGEMENT.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * Send feedback to <sbw@mit.edu>
 */

#include <linux/module.h>
#include <linux/version.h>
#include <linux/slab.h>
#include <asm/nmi.h>

#ifndef MSR_F15H_NBPERF_CTL 
#define MSR_F15H_NBPERF_CTL       0xc0010240 
#endif

#ifndef MSR_F15H_NBPERF_CTR 
#define MSR_F15H_NBPERF_CTR       0xc0010241 
#endif


#ifndef MSR_F15H_PERF_CTL 
#define MSR_F15H_PERF_CTL	0xc0010200 
#endif

#ifndef MSR_F15H_PERF_CTR 
#define MSR_F15H_PERF_CTR	0xc0010201 
#endif


#define AMD10H_ATTR(_name)					\
	struct kobj_attribute attr_##_name =			\
	       __ATTR(_name, 0666, amd15h_show, amd15h_store)

static struct kobject *spmc_kernel_kobj;
static struct kobject **spmc_cpu_kobj;

static inline int get_cpu_i(struct kobject *kobj)
{
	int i;

	for (i = 0; i < num_present_cpus(); i++) {	
		if (spmc_cpu_kobj[i] == kobj)
			return i;
	}

	printk(KERN_DEBUG "oops, couldn't find kobj\n");
	return -EINVAL;
}

static inline int get_event_i(struct kobj_attribute *attr)
{
	return simple_strtol(attr->attr.name, (char **)NULL, 16);
}

static ssize_t amd15h_store(struct kobject *kobj, struct kobj_attribute *attr,
			    const char *buf, size_t count)
{
	int c, e, r;
	u64 v;

	if ((c = get_cpu_i(kobj)) < 0)
		return c;

	if ((e = get_event_i(attr)) < 0)
		return e;

	v = simple_strtoull(buf, (char **)NULL, 16);
	/* Interrupts must be disabled */
	v &= ~(1 << 20);


#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,34)
	switch (__this_cpu_read(cpu_info.x86)) 
#else
	switch(current_cpu_data.x86)
#endif
	{
	case 0x10: 
		if ((r = wrmsr_on_cpu(c, MSR_K7_EVNTSEL0 + e, (u32)v, (u32)(v >> 32))) < 0)
			return r;
		break;
	case 0x15:
	    if(e > 5){
		if ((r = wrmsr_on_cpu(c, MSR_F15H_NBPERF_CTL + (e-6) * 2, (u32)v, (u32)(v >> 32))) < 0)
			return r;
	    } else {
		if ((r = wrmsr_on_cpu(c, MSR_F15H_PERF_CTL + e * 2, (u32)v, (u32)(v >> 32))) < 0)
			return r;
	    }
	}
	return count;
}

static ssize_t amd15h_show(struct kobject *kobj, 
			   struct kobj_attribute *attr, char *buf)
{
	int c, e, r;
	u32 l, h;
	u64 v;

	if ((c = get_cpu_i(kobj)) < 0)
		return c;

	if ((e = get_event_i(attr)) < 0)
		return e;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,34)
	switch (__this_cpu_read(cpu_info.x86)) 
#else
	switch(current_cpu_data.x86)
#endif
	{
	case 0x10: 
		if ((r = rdmsr_on_cpu(c, MSR_K7_EVNTSEL0 + e, &l, &h)) < 0)
			return r;
		break;
	case 0x15:
	    if(e > 5){
		if ((r = rdmsr_on_cpu(c, MSR_F15H_NBPERF_CTL + (e-6) * 2, &l, &h)) < 0)
			return r;
	    } else {
		if ((r = rdmsr_on_cpu(c, MSR_F15H_PERF_CTL + e * 2, &l, &h)) < 0)
			return r;
	    }
	}		
	v = l | ((u64)h << 32);
	return snprintf(buf, PAGE_SIZE, "%llx\n", v);
}

static AMD10H_ATTR(0);
static AMD10H_ATTR(1);
static AMD10H_ATTR(2);
static AMD10H_ATTR(3);
static AMD10H_ATTR(4);
static AMD10H_ATTR(5);
static AMD10H_ATTR(6);
static AMD10H_ATTR(7);
static AMD10H_ATTR(8);
static AMD10H_ATTR(9);
#define MAX_NPERFCTRS 10

static struct attribute *spmc_kernel_attrs[MAX_NPERFCTRS + 1];

static struct attribute_group spmc_kernel_attr_group = {
	.attrs = spmc_kernel_attrs,
};

static void amd15h_init_one(void *ignore)
{
	write_cr4(read_cr4() | X86_CR4_PCE);
}

static void release_msrs(void)
{
	int i;

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,34)
	switch (__this_cpu_read(cpu_info.x86)) 
#else
	switch(current_cpu_data.x86)
#endif
	{
	case 0x10: 
		for (i = 0; i < 3; i++)
			release_evntsel_nmi(MSR_K7_EVNTSEL0 + i);
		for (i = 0; i < 3; i++)
			release_perfctr_nmi(MSR_K7_PERFCTR0 + i);
		break;
	case 0x15:
		for (i = 0; i < 6; i++)
			release_evntsel_nmi(MSR_F15H_PERF_CTL + i * 2);
		for (i = 0; i < 6; i++)
			release_perfctr_nmi(MSR_F15H_PERF_CTR + i * 2);
                for (i = 0; i < 4; i++)
                        release_evntsel_nmi(MSR_F15H_NBPERF_CTL + i * 2);
                for (i = 0; i < 4; i++)
                        release_perfctr_nmi(MSR_F15H_NBPERF_CTR + i * 2);

		break;
	default:
		printk(KERN_WARNING "release_msrs(): not 10h or 15h\n");
	}
		
}

static int __init reserve_msrs(void)
{
	int i;

	if (!boot_cpu_has(X86_FEATURE_MSR) || 
#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,34)
	    __this_cpu_read(cpu_info.x86_vendor) != X86_VENDOR_AMD
#else
	    current_cpu_data.x86_vendor != X86_VENDOR_AMD
#endif
	    )
	{
		printk(KERN_ERR "reserve_msrs: not an AMD CPU\n");
		return -EINVAL;
	}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,34)
	switch (__this_cpu_read(cpu_info.x86)) 
#else
	switch(current_cpu_data.x86)
#endif
        {
	case 0x10:	/* AMD 10H */
		printk(KERN_WARNING "reserve_msrs: amd10h cpu\n");
		spmc_kernel_attrs[0] = &attr_0.attr;
		spmc_kernel_attrs[1] = &attr_1.attr;
		spmc_kernel_attrs[2] = &attr_2.attr;
		spmc_kernel_attrs[3] = &attr_3.attr;
		spmc_kernel_attrs[4] = NULL;

		/* release all evntsel and perfctr registers first */

		for (i = 0; i < 3; i++) {
			if (!reserve_perfctr_nmi(MSR_K7_PERFCTR0 + i)) {
				printk(KERN_ERR 
				       "failed to reserve perfctr MSR %#x\n",
			               MSR_K7_PERFCTR3);
				while (--i >= 0)
					release_perfctr_nmi(MSR_K7_PERFCTR0 + i);
				return -ENOMEM;
			}
		}
	
		for (i = 0; i < 3; i++) {
			if (!reserve_evntsel_nmi(MSR_K7_EVNTSEL0 + i)) {
				printk(KERN_ERR 
				       "failed to reserve evntsel MSR %#x\n",
				       MSR_K7_EVNTSEL3);
				while (--i >= 0)
					release_evntsel_nmi(MSR_K7_EVNTSEL0 + i);
				for (i = 0; i < 3; i++)
					release_perfctr_nmi(MSR_K7_PERFCTR0 + i);
				return -ENOMEM;
			}
		}
		break;
	case 0x15:	/* AMD 15H */
		printk(KERN_WARNING "reserve_msrs: amd15h cpu\n");
		spmc_kernel_attrs[0] = &attr_0.attr;
		spmc_kernel_attrs[1] = &attr_1.attr;
		spmc_kernel_attrs[2] = &attr_2.attr;
		spmc_kernel_attrs[3] = &attr_3.attr;
		spmc_kernel_attrs[4] = &attr_4.attr;
		spmc_kernel_attrs[5] = &attr_5.attr;
                spmc_kernel_attrs[6] = &attr_6.attr;
                spmc_kernel_attrs[7] = &attr_7.attr;
                spmc_kernel_attrs[8] = &attr_8.attr;
                spmc_kernel_attrs[9] = &attr_9.attr;
		spmc_kernel_attrs[10] = NULL;

		for (i = 0; i < 6; i++) {
			if (!reserve_perfctr_nmi(MSR_F15H_PERF_CTR + i * 2)) {
				printk(KERN_ERR 
				       "failed to reserve perfctr MSR %#x\n",
			               MSR_F15H_PERF_CTR + i * 2);
				while (--i >= 0)
					release_perfctr_nmi(MSR_F15H_PERF_CTR + i * 2);
				return -ENOMEM;
			}
		}
	
		for (i = 0; i < 6; i++) {
			if (!reserve_evntsel_nmi(MSR_F15H_PERF_CTL + i * 2)) {
				printk(KERN_ERR 
				       "failed to reserve evntsel MSR %#x\n",
				       MSR_F15H_PERF_CTL + i * 2);
				while (--i >= 0)
					release_evntsel_nmi(MSR_F15H_PERF_CTL + i * 2);
				for (i = 0; i < 6; i++)
					release_perfctr_nmi(MSR_F15H_PERF_CTR + i * 2);
				return -ENOMEM;
			}
		}

                for (i = 0; i < 4; i++) {
                        if (!reserve_perfctr_nmi(MSR_F15H_NBPERF_CTR + i * 2)) {
                                printk(KERN_ERR
                                       "failed to reserve perfctr MSR %#x\n",
                                       MSR_F15H_NBPERF_CTR + i * 2);
                                while (--i >= 0)
                                        release_perfctr_nmi(MSR_F15H_NBPERF_CTR + i * 2);
                                return -ENOMEM;
                        }
                }

                for (i = 0; i < 4; i++) {
                        if (!reserve_evntsel_nmi(MSR_F15H_NBPERF_CTL + i * 2)) {
                                printk(KERN_ERR
                                       "failed to reserve evntsel MSR %#x\n",
                                       MSR_F15H_NBPERF_CTL + i * 2);
                                while (--i >= 0)
                                        release_evntsel_nmi(MSR_F15H_NBPERF_CTL + i * 2);
                                for (i = 0; i < 4; i++)
                                        release_perfctr_nmi(MSR_F15H_NBPERF_CTR + i * 2);
                                return -ENOMEM;
                        }
                }

		break;
	default:
		printk(KERN_WARNING "not 10h or 15h\n");
		return -EINVAL;
	}
	return 0;
}

static int __init amd15h_pmc_init(void) 
{
	char b[8];
	int ret, i;

	if ((ret = reserve_msrs()))
		return ret;

	ret = -ENOMEM;
	spmc_kernel_kobj = kobject_create_and_add("amd15h-pmc", kernel_kobj);
	if (!spmc_kernel_kobj) {
		printk(KERN_DEBUG "cannot add kernel object: /sys/kernel/amd15h-pmc\n");
		return ret;
	}

	spmc_cpu_kobj = kzalloc(sizeof(*spmc_cpu_kobj) * num_present_cpus(), 
				GFP_KERNEL);
	if (!spmc_cpu_kobj) {
		kobject_put(spmc_kernel_kobj);
		return ret;
	}

	for (i = 0; i < num_present_cpus(); i++) {
		sprintf(b, "cpu%d", i);
		spmc_cpu_kobj[i] = kobject_create_and_add(b, spmc_kernel_kobj);
		if (!spmc_cpu_kobj[i]) {
			ret = -ENOMEM;
			goto error_cpu;
		}

		ret = sysfs_create_group(spmc_cpu_kobj[i], &spmc_kernel_attr_group);
		if (ret)
			goto error_cpu;
	}

	on_each_cpu(amd15h_init_one, NULL, 1);

	printk(KERN_WARNING "amd15h_pmc successfully loaded\n");
	return 0;

 error_cpu:
	kfree(spmc_cpu_kobj);
	kobject_del(spmc_kernel_kobj);
	return ret;
}

static void __exit amd15h_pmc_exit(void) 
{
	release_msrs();
	kfree(spmc_cpu_kobj);
	kobject_del(spmc_kernel_kobj);

	/*
	 * Leave X86_CR4_PCE enabled.
	 */
	printk(KERN_WARNING "amd15h_pmc successfully unloaded\n");
}

module_init(amd15h_pmc_init);
module_exit(amd15h_pmc_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Silas Boyd-Wickizer <sbw@mit.edu>");
MODULE_DESCRIPTION("Simple PMC for AMD10h");
