/*
 * Directory operations for InterMezzo filesystem
 * Original version: (C) 1996 P. Braam and M. Callahan
 * Rewritten for Linux 2.1. (C) 1997 Carnegie Mellon University
 *
 * Stelias encourages users to contribute improvements to
 * the InterMezzo project. Contact Peter Braam (coda@stelias.com).
 */

#define __NO_VERSION__
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/fs.h>
#include <linux/stat.h>
#include <linux/errno.h>
#include <linux/locks.h>
#include <asm/segment.h>
#include <asm/uaccess.h>
#include <linux/string.h>

#include <linux/intermezzo_fs.h>

static int presto_dentry_revalidate(struct dentry *de, int );


/* called when a cache lookup succeeds */
static int presto_dentry_revalidate(struct dentry *de, int flag)
{
	struct inode *inode = de->d_inode;
	ENTRY;
	if (!inode) {
		EXIT;
		return 1;
	}
	if (is_bad_inode(inode)) {
		EXIT;
		return 0;
	}

	if ( S_ISDIR(inode->i_mode) ) {
		EXIT;
		return (presto_chk(de, PRESTO_DATA) &&
			(presto_chk(de, PRESTO_ATTR)));
	} else {
		EXIT;
		return presto_chk(de, PRESTO_ATTR);
	}
}

static void presto_dentry_iput(struct dentry *dentry, struct inode *inode)
{
	dentry->d_time = 0;
	iput(inode);
}

struct dentry_operations presto_dentry_ops = 
{
	d_revalidate: presto_dentry_revalidate,
        d_iput: presto_dentry_iput
};

