1 Timer 
	(1) struct timer_list stimer;
		stimer.data = 0;
		stimer.expires = jiffies + 100;
		stimer.function = time_handler;
	(2) in time_handler()
		1)	mod_timer(&stimer, jiffies + 100); //change value and add it 
		2) stimer.expires = jiffies + 100;
		    add_timer(&stimer);
	(3)del_timer

2 hrtimer

3 delayed_work
	#define to_device(x) container_of((x), struct xx_priv, xx_work)
	#define to_delay_worker(x) container_of((x), struct delayed_work, work)
    (1) struct xx_priv{
		 struct delayed_work xx_work;
		};


	(2) INIT_DELAYED_WORK(&xx_dev->xx_work, work_handler);
	(3) static void xx_start(struct xx_priv *xx_dev, int ms)
		{
			schedule_delayed_work(&xx_dev->xx_work,
						  msecs_to_jiffies(ms));
		}
	(4)void work_handler(struct work_struct *work){
			struct xx_priv *xx_dev =
	    to_device(to_delay_worker(work));
	}
4 delay in kernel
	(1) msleep()
	(2)long delay 
		time_before(),time_after()
	(3)sleep for delay
		schedule_timeout
		

