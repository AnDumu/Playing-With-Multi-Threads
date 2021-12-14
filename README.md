# Playing-With-Threads

    * 7 Threads:
               With Lock + sleep 23 + tick 45:     137015 ms
               With Lock + tick 45:                68828 ms
               With Lock:                          24938 ms
               With Lock + getProcessing_FALSE:    14422 ms
     
               WithOut Lock + sleep 23 + tick 45:  27969 ms
               WithOut Lock + tick 45:             68844 ms
               WithOut Lock:                       24907 ms
               WithOut Lock + getProcessing_FALSE: 13656 ms
     
      5 Threads:
               With Lock + sleep 23 + tick 45:     136875 ms
               With Lock + tick 45:                68938 ms
               With Lock:                          25281 ms
               With Lock + getProcessing_FALSE:    14094 ms
     
               WithOut Lock + sleep 23 + tick 45:  37079 ms
               WithOut Lock + tick 45:             68828 ms
               WithOut Lock:                       25375 ms
               WithOut Lock + getProcessing_FALSE: 14859 ms
     
     1 Thread:
               With Lock + sleep 23 + tick 45:     138297 ms
               With Lock + tick 45:                69219 ms
               With Lock:                          25266 ms
               With Lock + getProcessing_FALSE:    11969 ms
     
               WithOut Lock + sleep 23 + tick 45:  138312 ms
               WithOut Lock + tick 45:             68860 ms
               WithOut Lock:                       25187 ms
    *           WithOut Lock + getProcessing_FALSE: 12547 ms

Threads were created to perform various actions in different code sections/regions, ofc you can try to use many threads on the same region code to process it faster.
If you need the lock to prevent crash, it means that you are doing it wrong.
