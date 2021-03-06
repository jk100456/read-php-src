
	/* $Id: fpm_worker_pool.c,v 1.15.2.1 2008/12/13 03:21:18 anight Exp $ */
	/* (c) 2007,2008 Andrei Nigmatulin */

#include "fpm_config.h"

#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "fpm.h"
#include "fpm_worker_pool.h"
#include "fpm_cleanup.h"
#include "fpm_children.h"
#include "fpm_shm.h"
#include "fpm_scoreboard.h"
#include "fpm_conf.h"

struct fpm_worker_pool_s *fpm_worker_all_pools;

static void fpm_worker_pool_cleanup(int which, void *arg) /* {{{ */
{
	struct fpm_worker_pool_s *wp, *wp_next;

	for (wp = fpm_worker_all_pools; wp; wp = wp_next) {
		wp_next = wp->next;
		fpm_worker_pool_config_free(wp->config);
		fpm_children_free(wp->children);
		if ((which & FPM_CLEANUP_CHILD) == 0 && fpm_globals.parent_pid == getpid()) {
			fpm_scoreboard_free(wp->scoreboard);
		}
		free(wp->config);
		free(wp->user);
		free(wp->home);
		free(wp);
	}
	fpm_worker_all_pools = NULL;
}
/* }}} */

struct fpm_worker_pool_s *fpm_worker_pool_alloc() /* {{{ */
{
	struct fpm_worker_pool_s *ret;

	ret = malloc(sizeof(struct fpm_worker_pool_s));
	if (!ret) {
		return 0;
	}

	memset(ret, 0, sizeof(struct fpm_worker_pool_s));

	ret->idle_spawn_rate = 1;
	ret->log_fd = -1;
	return ret;
}
/* }}} */

int fpm_worker_pool_init_main() /* {{{ */
{
	if (0 > fpm_cleanup_add(FPM_CLEANUP_ALL, fpm_worker_pool_cleanup, 0)) {
		return -1;
	}
	return 0;
}
/* }}} */
