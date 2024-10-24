# 1 local-local
 <!-- [Thread level parallel] here -->
for fd in range(FILENUMBER):
    inner_loop(FILESIZE, fd, 0, [large_offset, small_offset, half_ctrl_offset, half_targ_offset])
return

# 2 global-global
fd_pair = make_work_pair(NGQB, ctrl, targ)  
 <!-- [Thread level parallel] here -->
for [fd1, fd2] in fd_pair:
    inner_loop2(FILESIZE, fd1, 0, fd2, 0, CHUNKSIZE)
return

# 3 global-local
fd_pair = make_work_pair(NGQB, ctrl, -1)
for [fd, _] in fd_pair:
    inner_loop(FILESIZE, fd, 0, half_small_offset)
return

# 4 local-global
fd_pair = make_work_pair(NGQB, -1, targ)
for [fd1, fd2] in fd_pair:
    inner_loop2(FILESIZE, fd1, 0, fd2, 0, [2*CHUNKSIZE, small_offset, half_small_offset, CHUNKSIZE])
return

# 5 global-thread
fd_pair = make_work_pair(NGQB, ctrl, -1)
thread_pair = make_work_pair(NSQB-NGQB, -1, targ-NGQB)
threads_per_file = (1<<(NSQB-NGQB))
thread_size = 1 << (N-NSQB)
 <!-- [Thread level parallel] here -->
for [fd1, fd2] in fd_pair:
    for [t1, t2] in thread_pair:
        t1_off = t1 * thread_size # equal to t1 << (N-NSQB)
        t2_off = t2 * thread_size # equal to t2 << (N-NSQB)
        inner_loop2(thread_size, fd1, t1_off, fd2, t2_off, CHUNKSIZE)
return


# 6 thread-global
fd_pair = make_work_pair(NGQB, -1, targ)
thread_pair = make_work_pair(NSQB-NGQB, ctrl-NGQB, -1)
threads_per_file = (1<<(NSQB-NGQB))
thread_size = 1 << (N-NSQB)
 <!-- [Thread level parallel] here -->
for [fd1, fd2] in fd_pair:
    for [t1, t2] in thread_pair:
        t1_off = t1 * thread_size # equal to t1 << (N-NSQB)
        t2_off = t2 * thread_size # equal to t2 << (N-NSQB)
        inner_loop2(thread_size, fd1, t1_off, fd2, t2_off, CHUNKSIZE)
return

# 7 global-middle
fd_pair = make_work_pair(NGQB, ctrl, -1)
for [fd1, fd2] in fd_pair: # fd1 must equal to fd2
    for t in range(threads_per_file):
        t1_off = t * thread_size # equal to t << (N-NSQB)
        t2_off = t1_off + half_small_offset
        #這邊可能在其他gate要注意|0> |1>的順序
        #x gate 沒差
        for i in range(0, thread_size, small_offset):
            inner_loop2(half_small_offset, fd1, t1_off, fd2, t2_off, CHUNKSIZE)
            t1_off += small_offset
            t2_off += small_offset
return

# 8 middle-global
fd_pair = make_work_pair(NGQB, -1, targ)
for [fd1, fd2] in fd_pair: # fd1 must equal to fd2
    for t in range(threads_per_file):
        t_off = t * thread_size # equal to t << (N-NSQB)
        #這邊可能在其他gate要注意|0> |1>的順序
        #x gate 沒差
        t_off += half_small_offset
        for i in range(0, thread_size, small_offset):
            inner_loop2(half_small_offset, fd1, t_off, fd2, t_off, CHUNKSIZE)
            t_off += small_offset
return

# 9 thread-thread

thread_pair = make_work_pair(NSQB-NGQB, ctrl-NGQB, targ-NGQB)       
for fd in range(FILENUMBER):
    for [t1, t2] in thread_pair:
        t1_off = t1 * thread_size
        t2_off = t2 * thread_size
        inner_loop2(thread_size, fd, t1_off, fd, t2_off, CHUNKSIZE)
return

# 10 thread-local
thread_pair = make_work_pair(NSQB-NGQB, ctrl-NGQB, -1)
for fd in range(FILENUMBER):
    for [t1, t2] in thread_pair: # t1==t2
        t_off = t1 * thread_size
        inner_loop(thread_size, fd, t_off, half_small_offset)
return

# 11 local-thread
thread_pair = make_work_pair(NSQB-NGQB, -1, targ-NGQB)
for fd in range(FILENUMBER):
    for [t1, t2] in thread_pair:
        t1_off = t1 * thread_size
        t2_off = t2 * thread_size
        inner_loop2(thread_size, fd, t1_off, fd, t2_off, [2*CHUNKSIZE, small_offset, half_ctrl_offset, CHUNKSIZE])
return

# 12 thread-middle
thread_pair = make_work_pair(NSQB-NGQB, ctrl-NGQB, -1)
for fd in range(FILENUMBER):
    for [t1, t2] in thread_pair: # t1 == t2
        t1_off = t1 * thread_size
        t2_off = t1 * thread_size + half_targ_offset
        for i in range(0, thread_size, small_offset):
            inner_loop2(half_small_offset, fd, t1_off, fd, t2_off, CHUNKSIZE)
            t1_off += small_offset
            t2_off += small_offset
return

# 13 middle-thread