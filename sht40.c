// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Simple skeleton driver for Sensorion SHT4x sensors
 *
 * Copyright (c) 2025 Hans de Goede <hansg@kernel.org>
 */

#include <linux/delay.h>
#include <linux/device.h>
#include <linux/i2c.h>
#include <linux/mod_devicetable.h>
#include <linux/module.h>
#include <linux/sysfs.h>
#include <linux/version.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(6, 12, 0)
#include <asm/unaligned.h>
#else
#include <linux/unaligned.h>
#endif
#define PRECISION 0xFD 
#define SERIALNO 0x89

struct sht40_data {
	struct i2c_client *client;

};

// in Progress 

char resp[6];

static int perform_i2c_operations(struct i2c_client *client, int sn) {
	struct device *dev = &client->dev;

	int ret;

	// Write a byte to the I2C client
	ret = i2c_smbus_write_byte(client, sn );

	if (ret < 0) {
		dev_err(dev, "Write byte error: %d\n", ret);
		return ret;
	}

	// Wait for 1 ms
	usleep_range(1000, 1001);  // Equivalent to fsleep(1 * USEC_PER_MSEC)

	// Read 6 bytes from the I2C client
	ret = i2c_transfer_buffer_flags(client, resp, 6, I2C_M_RD);
	if (ret != 6) {
		dev_err(dev, "Read bytes error: %d\n", ret);
		return (ret < 0) ? ret : -EIO;
	}

	return 0;  // Success
}

// In progress

static ssize_t temp_show(struct device *dev, struct device_attribute *attr, char *buf)

{
	struct sht40_data *data = dev_get_drvdata(dev);
	int s = SERIALNO;

	if (perform_i2c_operations(data->client, s))
		return -EIO;
	int temp = get_unaligned_be16(&resp[0]);
	temp = (temp * (175)/(65536 - 1)) - 45;
	return sysfs_emit(buf, "%d.%d° Celsius\n", temp / 10, temp % 10);
}
static DEVICE_ATTR_RO(temp);
static ssize_t hello_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	struct sht40_data *data = dev_get_drvdata(dev);

	return sysfs_emit(buf, "%s says: 'Hello'\n", dev_name(&data->client->dev));
}
static DEVICE_ATTR_RO(hello);

static ssize_t world_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	return sysfs_emit(buf, "Hello World\n");
}
static DEVICE_ATTR_RO(world);
static ssize_t serialno_show(struct device *dev, struct device_attribute *attr, char *buf)
{
	//char resp[6];
	struct sht40_data *data = dev_get_drvdata(dev);
	int s = SERIALNO;
	
	if(perform_i2c_operations(data->client, s))
                return -EIO;


	return sysfs_emit(buf, "%02x%02x%02x%02x\n", resp[0], resp[1], resp[3], resp[4]);
}

static DEVICE_ATTR_RO(serialno);


static struct attribute *sht40_attrs[] = {
	&dev_attr_hello.attr,
	&dev_attr_world.attr,
	&dev_attr_serialno.attr,
	&dev_attr_temp.attr,
	NULL
};
ATTRIBUTE_GROUPS(sht40);

static int sht40_probe(struct i2c_client *client)
{
	struct device *dev = &client->dev;
	struct sht40_data *data;

	data = devm_kzalloc(dev, sizeof(*data), GFP_KERNEL);
	if (!data)
		return -ENOMEM;

	data->client = client;

//	if(perform_i2c_operations(dev))
//		return -EIO;

	dev_info(dev, "SHT4x sensor probe success\n");
	i2c_set_clientdata(client, data);
	return 0;
}

static const struct i2c_device_id sht40_i2c_id[] = {
	{ "sht40" },
	{ }
};
MODULE_DEVICE_TABLE(i2c, sht40_i2c_id);

static struct i2c_driver sht40_driver = {
	.driver = {
		.name = "sht40",
		.dev_groups = sht40_groups,
	},
	.probe = sht40_probe,
	.id_table = sht40_i2c_id,
};
module_i2c_driver(sht40_driver);

MODULE_DESCRIPTION("Simple skeleton driver for Sensorion SHT4x sensors");
MODULE_AUTHOR("Jules Irenge<jbi.octave@gmail.com>");
MODULE_AUTHOR("Hans de Goede <hansg@kernel.org>");

MODULE_LICENSE("GPL");
