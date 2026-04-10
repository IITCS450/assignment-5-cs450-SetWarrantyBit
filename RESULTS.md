## Approach

1. Thread is constructed each with a stack

2. `uswtch.S` swaps by pushing the four regs, saving %esp into the old thread, loading %esp from the new thread, popping the regs back, and returning.

3. `thread_create` mallocs a stack, and sets the saved %eip to `thread_stub`.

4. First switch into new thread returns into `thread_stub`, marks the thread ZOMBIE, and yields out.

5. `thread_yield` picks the next runnable thread round robin and calls `uswtch`.

6. `thread_join` keeps yielding until the target is ZOMBIE, then frees its stack.


## Limitations

- Num of threads capped at 16, with max 4096B stacks each due to definition implemented in code

- Mutex does not guarantee fairnesss.