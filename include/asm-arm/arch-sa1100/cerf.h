#ifndef _INCLUDE_CERF_H_
#define _INCLUDE_CERF_H_

/* GPIOs for CF+ slot lines */
#define GPIO_CF_IRQ		GPIO_GPIO (22)  /* 1111 MBGNT _OR_ CF IRQ   */
#define GPIO_CF_CD		GPIO_GPIO (23)  /* 1111 MBREQ _OR_ CF CD    */
#define GPIO_CF_BVD2		GPIO_GPIO (19)  /* Graphics IRQ _OR_ CF BVD */
#define GPIO_CF_BVD1		GPIO_GPIO (20)  /* 1111 IRQ _OR_ CF BVD     */

#define IRQ_GPIO_CF_IRQ		IRQ_GPIO22
#define IRQ_GPIO_CF_CD		IRQ_GPIO23
#define IRQ_GPIO_CF_BVD2	IRQ_GPIO19
#define IRQ_GPIO_CF_BVD1	IRQ_GPIO20

#define GPIO_UCB1200_IRQ	GPIO_GPIO (18)
#define IRQ_GPIO_UCB1200_IRQ	IRQ_GPIO18

#endif

