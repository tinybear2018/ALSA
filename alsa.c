http://blog.csdn.net/qq405180763/article/details/16881047
(1)#define container_of(ptr, type, member) \
    (type *)((char*)(ptr) - offsetof(type, member))
(2)#define list_entry(ptr, type, member) \
	container_of(ptr, type, member)
(3)#define list_for_each_entry(pos, head, member)				\
	for (pos = list_entry((head)->next, typeof(*pos), member);	\
	     &pos->member != (head); 	\
	     pos = list_entry(pos->member.next, typeof(*pos), member))
		 
		 list_for_each_entry(dev, &card->devices, list)
					for (pos = list_entry((head)->next, typeof(*pos), member);	\
					 &pos->member != (head); 	\
					 pos = list_entry(pos->member.next, typeof(*pos), member))
snd_card=[
struct list_head devices;
struct list_head controls;
]

1 create a card
err = snd_card_create(index, id, THIS_MODULE, sizeof(struct snd_ad1816a), &card); 

	int snd_card_create(int idx, const char *xid,
		    struct module *module, int extra_size,
		    struct snd_card **card_ret)
	(1)	
	card = kzalloc(sizeof(*card) + extra_size, GFP_KERNEL);
	(2)some init
		card->number = idx;  
		card->module = module;  
		INIT_LIST_HEAD(&card->devices);  
		init_rwsem(&card->controls_rwsem);  
		rwlock_init(&card->ctl_files_rwlock);  
		INIT_LIST_HEAD(&card->controls);  
		INIT_LIST_HEAD(&card->ctl_files);  
		spin_lock_init(&card->files_lock);  
		INIT_LIST_HEAD(&card->files_list);  
		init_waitqueue_head(&card->shutdown_sleep);
	(2)Control
		err = snd_ctl_create(card);
	(4)create proc  infor: /proc/asound/card0
		snd_info_card_create(card)
	(5)point the extra_size that kzalloc
		card->private_data = (char *)card + sizeof(struct snd_card);
2 private data

struct snd_ad1816a *chip;
chip = card->private_data;

3create low level device
snd_device_new(card, SNDRV_DEV_LOWLEVEL, chip, &ops);


4 create logic devices
list_head devices;

	PCM  ----        snd_pcm_new()

    RAWMIDI --    snd_rawmidi_new()

    CONTROL --   snd_ctl_create()

    TIMER   --       snd_timer_new()

    INFO    --        snd_card_proc_new()

    JACK    --        snd_jack_new()

5 ret = snd_card_register(card);
		add all the devuces ,for example :  PCM an follow

	(1)card->card_dev = device_create(sound_class, card->dev,
					       MKDEV(0, 0), card,
					       "card%i", card->number);
	(2)snd_device_register_all(card)
			list_for_each_entry(dev, &card->devices, list) {
		if (dev->state == SNDRV_DEV_BUILD && dev->ops->dev_register) {
			if ((err = dev->ops->dev_register(dev)) < 0)
		
		
/*******PCM*******/
PCM shili de create

snd_pcm is device of sound card
	struct snd_pcm_str streams[2];
		struct snd_pcm_substream *substream;

soc_probe_link_dais ->
		snd_new_pcm ->	
			snd_pcm_new->
				1) static struct snd_device_ops ops = {
					.dev_free = snd_pcm_dev_free,
					.dev_register =	snd_pcm_dev_register,
					.dev_disconnect = snd_pcm_dev_disconnect,
				 };
				2) snd_pcm_new_stream SNDRV_PCM_STREAM_PLAYBACK
									  SNDRV_PCM_STREAM_CAPTURE
									
				3) snd_device_new  add the device in the card//
												card->device
													list the logic device ----snd_device
								
								(1)     struct snd_minor {  
									    int type;           /* SNDRV_DEVICE_TYPE_XXX */  
									    int card;           /* card number */  
									    int device;         /* device number */  
									    const struct file_operations *f_ops;    /* file operations */  
									    void *private_data;     /* private data for f_ops->open */  
									    struct device *dev;     /* device for sysfs */  
									};
								(2)
									
									playback  --  pcmCxDxp，通常系统中只有一各声卡和一个pcm，它就是pcmC0D0p
									capture  --  pcmCxDxc，通常系统中只有一各声卡和一个pcm，它就是pcmC0D0c 
								(3)snd_pcm_f_ops  
									snd_open的时候根据 minor信息调用的是这里的？那么下面的那个set_ops是干吗的！
				snd_pcm_set_ops register the ops 
					stream->substream->ops
		

/*******CONTROL*******/
1) logic device
snd_card_new->
	snd_ctl_create->
			static struct snd_device_ops ops = {
				.dev_free = snd_ctl_dev_free,
				.dev_register =	snd_ctl_dev_register,
				.dev_disconnect = snd_ctl_dev_disconnect,
			};
			return snd_device_new(card, SNDRV_DEV_CONTROL, card, &ops); 
						card->device
						list the logic device ----snd_device

2)concrete kcontrol snd_kcontrol_new
			snd_soc_add_controls->
						err = snd_ctl_add(card, snd_soc_cnew(control, data,
						     control->name, prefix));

							 
							 
/****mixer control*****/							 
1) snd_kcontrol_new
struct snd_kcontrol_new {
	snd_ctl_elem_iface_t iface;	/* interface identifier */
	unsigned int device;		/* device/client number */
	unsigned int subdevice;		/* subdevice (substream) number */
	const unsigned char *name;	/* ASCII name of item */
	unsigned int index;		/* index of item */
	unsigned int access;		/* access rights */
	unsigned int count;		/* count of same elements */
	snd_kcontrol_info_t *info;
	snd_kcontrol_get_t *get;
	snd_kcontrol_put_t *put;
	union {
		snd_kcontrol_tlv_rw_t *c;
		const unsigned int *p;
	} tlv;
	unsigned long private_value;
};

	(1)->SOC_SINGLE_EXT
			#define SOC_DOUBLE_VALUE(xreg, shift_left, shift_right, xmax, xinvert, xautodisable) \
				((unsigned long)&(struct soc_mixer_control) \
				{.reg = xreg, .rreg = xreg, .shift = shift_left, \
				.rshift = shift_right, .max = xmax, .platform_max = xmax, \
				.invert = xinvert, .autodisable = xautodisable})
			#define SOC_SINGLE_VALUE(xreg, xshift, xmax, xinvert, xautodisable) \
				SOC_DOUBLE_VALUE(xreg, xshift, xshift, xmax, xinvert, xautodisable)
			#define SOC_DOUBLE_EXT(xname, reg, shift_left, shift_right, max, invert,\
			 xhandler_get, xhandler_put) \
		{	.iface = SNDRV_CTL_ELEM_IFACE_MIXER, .name = (xname),\
			.info = snd_soc_info_volsw, \
			.get = xhandler_get, .put = xhandler_put, \
			.private_value = \
				SOC_SINGLE_VALUE(reg, shift_left, shift_right, max, invert, 0) }
	(2)->SOC_SINGLE_EXT_TLV   //mei kand dong
		//min is -94.5db,what is max? SMTHDG_MAX_VAL is 0xffff,what is the max db?
		static const DECLARE_TLV_DB_SCALE(smthdg_step_tlv, -9450, 150, 1);
		SOC_SINGLE_EXT_TLV("VBC DAC0 SMTHDG Step Set",
			SND_SOC_NOPM,
			VBC_SMTHDG_DAC0, SMTHDG_MAX_VAL, 0,
			vbc_smthdg_step_get,
			vbc_smthdg_step_put, smthdg_step_tlv),
	    -> SOC_DOUBLE_R_EXT_TLV		//set two channel by one mixer	 

/*****Machine*****/

/*****Codec*****/

/*****Platform*****/

/*****DAPM*****/

1) 
  --> snd_soc_dapm_new_controls()
       (1) alloc memory
		if ((w = dapm_cnew_widget(widget)) == NULL)
       (2)set power_check
		w->power_check = dapm_generic_check_power;
       (3)add the widget to card
   		list_add(&w->list, &dapm->card->widgets);
	(4)set connected
		w->connected = 1;
  --> snd_soc_dapm_new_widgets(card);



/****DMA*****/
1)CPU_DAI
static struct snd_soc_dai_ops sprd_i2s_dai_ops = {
	.startup = i2s_startup,
	.shutdown = i2s_shutdown,
	.hw_params = i2s_hw_params,
	.trigger = i2s_trigger,
};
--->i2s_hw_params
	struct sprd_pcm_dma_params *dma_data;//config dma destnaton parameter

	snd_soc_dai_set_dma_data(dai, substream, dma_data);

2) platform ---dma
	(1) for destnation 
	struct snd_soc_pcm_runtime *srtd = substream->private_data;

	dma_data = snd_soc_dai_get_dma_data(srtd->cpu_dai, substream);//get dma+data from cpu_dai

    (2)for srource
		1 sprd_pcm_new->
			card->dev->coherent_dma_mask = DMA_BIT_MASK(64);
		2 sprd_pcm_open->
			struct snd_pcm_runtime *runtime = substream->runtime;
			struct snd_soc_pcm_runtime *srtd = substream->private_data;
			struct sprd_runtime_data *rtd;
			runtime->private_data = rtd;
			        (0) 	set  dma burst_len
						burst_len = I2S_FIFO_DEPTH - config->tx_watermark;
			       	(1)rtd->dma_cfg_phy[0] =  audio_mem_alloc(DDR32, &(runtime->hw.periods_max *sizeof(struct sprd_dma_cfg)));  audio_mem_vmap
                                  rtd->dma_cfg_phy[1] =  audio_mem_alloc(DDR32, &(runtime->hw.periods_max *sizeof(struct sprd_dma_cfg)));  audio_mem_vmap
			   	(2)rtd->dma_cfg_array = devm_kzalloc(dev, hw_chan * runtime->hw.periods_max* sizeof(struct sprd_dma_cfg), GFP_KERNEL);
		                   rtd->dma_pdata = devm_kzalloc(dev, hw_chan* sizeof(struct sprd_dma_callback_data), GFP_KERNEL);
				(3)malloc dma buffer: substream->dma_buffer
					   sprd_pcm_preallocate_dma_ddr32_buffer
			        
		3 sprd_pcm_hw_params
			(1)	size_t totsize = params_buffer_bytes(params);// the wrap buffet size 
				size_t period = params_period_bytes(params); //period bytes 
  			(2) cpu-dai   dma  config
				struct sprd_pcm_dma_params *dma_data;
				dma_data = snd_soc_dai_get_dma_data(srtd->cpu_dai, substream);


			(3) snd_pcm_set_runtime_buffer(substream, &substream->dma_buffer);
					struct snd_pcm_runtime *runtime = substream->runtime;
					if (bufp) {
						runtime->dma_buffer_p = bufp;
						runtime->dma_area = bufp->area;
						runtime->dma_addr = bufp->addr;
						runtime->dma_bytes = bufp->bytes;
					}
			(4) 	ret = sprd_pcm_config_dma(substream, params, dma_config_ptr,
				  dma_buff_phys);
	         			cfg->datawidth = dma_data->desc.datawidth;
					cfg->fragmens_len = fragments_len;		fragments_len = water_mask * dma_data->desc.datawidth;
					cfg->block_len = period / ch_cnt;
					cfg->transcation_len = 0;
					cfg->req_mode = FRAG_REQ_MODE;
					cfg->irq_mode = p_wakeup ? BLK_DONE : NO_INT;

					cfg->link_cfg_v = (unsigned long)(rtd->dma_cfg_virt[i]);// do what???
					cfg->link_cfg_p = (unsigned long)(rtd->dma_cfg_phy[i]);
			(5)//config DMA
				ret = dmaengine_device_control(rtd->dma_chn[i],
				       DMA_SLAVE_CONFIG,
				       (unsigned long)
				       (dma_config_ptr[i]));
			->/**request and configure dma**/
					rtd->dma_tx_des[0]->callback = sprd_pcm_dma_buf_done;
					rtd->dma_tx_des[0]->callback_param = (void *)(dma_pdata_ptr[0]);
				/*****DMA CallBack*****/	
				sprd_pcm_dma_buf_done->
							snd_pcm_period_elapsed->
								sprd_pcm_pointer //return the offset of wrap buffet(frames)











