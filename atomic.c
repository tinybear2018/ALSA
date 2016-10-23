atomic_t open_cnt = ATOMIC_INIT(0);
static int vbc_power(int enable)
{
	int i;
	atomic_t *open_status = &open_cnt;

	if (enable) {
		atomic_inc(open_status);
		if (atomic_read(open_status) == 1) {
			do_xxx();
		}
	} else {
		if (atomic_dec_and_test(open_status)) {
			do_xxx();
		}
		if (atomic_read(open_status) < 0)
			atomic_set(open_status, 0);
	}
	sp_asoc_pr_dbg("s: %d", atomic_read(open_status));
	return 0;
}
