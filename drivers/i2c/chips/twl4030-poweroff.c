/*
 * linux/drivers/i2c/chips/twl4030_poweroff.c
 *
 * Power off device
 *
 * Copyright (C) 2008 Nokia Corporation
 *
 * Written by Peter De Schrijver <peter.de-schrijver@nokia.com>
 *
 * This file is subject to the terms and conditions of the GNU General
 * Public License. See the file "COPYING" in the main directory of this
 * archive for more details.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <linux/module.h>
#include <linux/pm.h>
#include <linux/i2c/twl.h>

#include <mach/gpio.h>
#include <plat/mux.h>
#include <plat/microusbic.h>
#if defined(CONFIG_USB_ANDROID)
#include <linux/usb/android_composite.h>
#endif
#include <linux/reboot.h>

#if defined(CONFIG_USB_SWITCH_FSA9480) //me add
#include <linux/fsa9480.h>
#endif

#define PWR_P1_SW_EVENTS	0x10
#define PWR_DEVOFF	(1<<0)

extern  int sec_switch_get_cable_status(void); //me add

#define GPIO_PS_HOLD	25

#define GPIO_MSM_RST	178
#define GPIO_FONE_ACTIVE 140	

static void twl4030_poweroff(void)
{
	u8 val;
	int err;

	err = twl_i2c_read_u8(TWL4030_MODULE_PM_MASTER, &val,
				  PWR_P1_SW_EVENTS);
	if (err) {
		printk(KERN_WARNING "I2C error %d while reading TWL4030"
					"PM_MASTER P1_SW_EVENTS\n", err);
		return ;
	}

	val |= PWR_DEVOFF;

	err = twl_i2c_write_u8(TWL4030_MODULE_PM_MASTER, val,
				   PWR_P1_SW_EVENTS);

	if (err) {
		printk(KERN_WARNING "I2C error %d while writing TWL4030"
					"PM_MASTER P1_SW_EVENTS\n", err);
		return ;
	}

	return;
}

#if 0 //defined(CONFIG_MACH_SAMSUNG_ARCHER) && defined(CONFIG_PANEL_AMS353)
extern void ams353_lcd_poweroff(void);
#endif
extern void omap_watchdog_reset(void);
extern void tl2796_lcd_poweroff(void);

static void zeus_poweroff(void)
{
	/* int n_usbic_state; */

	printk("\nZEUS BOARD GOING TO SHUTDOWN!!!\n");

#if defined(CONFIG_USB_ANDROID)
	android_usb_set_connected(0);
#endif
        tl2796_lcd_poweroff();
	/* get_real_usbic_state(); */
	//gpio_direction_output(GPIO_MSM_RST,0);
	//gpio_direction_output(GPIO_FONE_ACTIVE, 0);
	// if (GPIO_TA_CONNECTED_N is LOW)
//#if (CONFIG_ARCHER_REV < ARCHER_REV13)
//	if ( get_real_usbic_state() )
         if ( sec_switch_get_cable_status() != CABLE_TYPE_NONE ) //me add
//#else
//	if ( !gpio_get_value( OMAP_GPIO_TA_NCONNECTED ) || gpio_get_value( OMAP_GPIO_IF_CON_SENSE ) )
//#endif
	{
		printk("Warmreset by TA or USB or Jtag\n\n");

		preempt_disable();
		local_irq_disable();
		local_fiq_disable();

#ifdef CONFIG_ARCHER_KOR_DEBUG
		omap_writel(0x54455352, OMAP343X_CTRL_BASE + 0x9C4); // set to normal reset
#endif
#if 1

		/* using watchdog reset */
		omap_watchdog_reset();
		/* machine_restart("ta_inserted"); */
#else
		/* using core_dpll_warmreset with global reset */
		//omap3_configure_core_dpll_warmreset();
		machine_restart("ta_inserted");
#endif

		while(1);
	}
	else
	{
		printk("Power Off !\n\n");
		while(1)
		{
			gpio_direction_output(GPIO_PS_HOLD, 0);

			if (0 /*is_powerbutton_pressed*/)
				printk("Power button is pressed\n\n");
			else
				twl4030_poweroff();
		}
	}

	return;
}



static int __init twl4030_poweroff_init(void)
{
	pm_power_off = zeus_poweroff;

	return 0;
}

static void __exit twl4030_poweroff_exit(void)
{
	pm_power_off = NULL;
}

module_init(twl4030_poweroff_init);
module_exit(twl4030_poweroff_exit);

MODULE_ALIAS("i2c:twl4030-poweroff");
MODULE_DESCRIPTION("Triton2 device power off");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Peter De Schrijver");
