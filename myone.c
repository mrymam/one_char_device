#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

MODULE_LICENSE("GPL v2");

#define MINOR_BASE	0
#define MINOR_NUMBER	1

static struct class * myone_class;
static struct cdev myone_cdev;
static dev_t dev;


static ssize_t read_one(struct file *file, char __user *buf,
			 size_t count, loff_t *ppos)
{
	size_t cleared = 0;

	while (count) {
		size_t chunk = min_t(size_t, count, PAGE_SIZE);
		size_t left;
		left = clear_user(buf + cleared, chunk);
		if (unlikely(left)) {
			cleared += (chunk - left);
			if (!cleared)
				return -EFAULT;
			break;
		}
		cleared += chunk;
		count -= chunk;

		if (signal_pending(current))
			break;
		cond_resched();
	}

	int i = 0;
	while (i < cleared) {
		buf[i] = ~buf[i];
		i++;
	}
	return cleared;
}

static struct file_operations myone_drv_fops = {
	.owner		= THIS_MODULE,
	.open		= NULL,
	.release	= NULL,
	.read		= read_one,
	.write		= NULL,
};


static int __init myone_init(void)
{
	int result;
	struct device *myone_dev = NULL;

	pr_info("myone: init function start\n");

	/* Major/Minor番号を動的に確保し/proc/devicesへ登録 */
	result = alloc_chrdev_region(&dev, MINOR_BASE, MINOR_NUMBER, "myone");

	if(result){
		pr_err("myone: error, alloc_chrdev_region = %d\n", result);
		goto MMNUMBER_ERR;
	}

	/* モジュールのクラスを/sys/class/へ登録 */
	myone_class = class_create(THIS_MODULE, "myone");
	if(IS_ERR(myone_class)){
		result = PTR_ERR(myone_class);
		pr_err("myone: error, class_create = %d\n", result);
		goto CLASS_ERR;
	}
	
	/* キャラクタデバイス構造体（cdev構造体）の初期化およびfopsの設定 */
	cdev_init(&myone_cdev, &myone_drv_fops);

	/* キャラクタデバイス構造体の登録 */
	result = cdev_add(&myone_cdev, dev, MINOR_NUMBER);
	if(result){
		pr_err("myone: error, cdev_add = %d\n", result);
		goto CDEV_ERR;
	}

	/* デバイスノードの作成．作成したノードは/dev以下からアクセス可能 */
	myone_dev = device_create(myone_class, NULL, dev, NULL, "myone");
	if(IS_ERR(myone_dev)){
		result = PTR_ERR(myone_dev);
		pr_err("myone: error, device_create = %d\n", result);
		goto DEV_ERR;
	}
	pr_info("myone: init function end\n");
	return result;

DEV_ERR:
	cdev_del(&myone_cdev);
CDEV_ERR:
	class_destroy(myone_class);
CLASS_ERR:
	unregister_chrdev_region(dev, MINOR_NUMBER);
MMNUMBER_ERR:
	return result;
}

static void __exit myone_exit(void)
{
	pr_info("myone: exit function start\n");

	/* デバイスノードの削除 */
	device_destroy(myone_class, dev);

	/* キャラクタデバイスをKernelから削除 */
	cdev_del(&myone_cdev);

	/* デバイスのクラス登録を解除 */
	class_destroy(myone_class);

	/* デバイスが使用していたメジャー番号の登録を解除 */
	unregister_chrdev_region(dev, MINOR_NUMBER);

	pr_info("myone: exit function end\n");
}

module_init(myone_init);
module_exit(myone_exit);
