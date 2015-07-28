/*
 * Copyright (C) 2009, Lars-Peter Clausen <lars@metafoo.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 *  You should have received a copy of the  GNU General Public License along
 *  with this program; if not, write  to the Free Software Foundation, Inc.,
 *  675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/of.h>
#include <linux/timer.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/soc.h>


static const struct snd_soc_dapm_widget rt5350_es9038_widgets[] = {
	SND_SOC_DAPM_SPK("Speaker", NULL),
};

static const struct snd_soc_dapm_route rt5350_es9038_routes[] = {
	{"Speaker", NULL, "HP_L"},
	{"Speaker", NULL, "HP_R"},
};

#define RT5350_DAIFMT (SND_SOC_DAIFMT_I2S | \
			SND_SOC_DAIFMT_NB_NF | \
			SND_SOC_DAIFMT_CBM_CFM) //rt5350 as slave

static int rt5350_es9038_codec_init(struct snd_soc_pcm_runtime *rtd)
{
	struct snd_soc_codec *codec = rtd->codec;
	struct snd_soc_dai *cpu_dai = rtd->cpu_dai;
	struct snd_soc_dapm_context *dapm = &codec->dapm;
	int ret;

	snd_soc_dapm_enable_pin(dapm, "HP_L");
	snd_soc_dapm_enable_pin(dapm, "HP_R");

	ret = snd_soc_dai_set_fmt(cpu_dai, RT5350_DAIFMT);
	if (ret < 0) {
		dev_err(codec->dev, "Failed to set cpu dai format: %d\n", ret);
		return ret;
	}

	return 0;
}

static struct snd_soc_dai_link rt5350_es9038_dai = {
	.name = "mt7620",
	.stream_name = "mt7620",
	.init = rt5350_es9038_codec_init,
	.codec_dai_name = "es9038-hifi",
};

static struct snd_soc_card rt5350_es9038 = {
	.name = "rt5350-es9038",
	.owner = THIS_MODULE,
	.dai_link = &rt5350_es9038_dai,
	.num_links = 1,

	.dapm_widgets = rt5350_es9038_widgets,
	.num_dapm_widgets = ARRAY_SIZE(rt5350_es9038_widgets),
	.dapm_routes = rt5350_es9038_routes,
	.num_dapm_routes = ARRAY_SIZE(rt5350_es9038_routes),
};

static int rt5350_es9038_probe(struct platform_device *pdev)
{
	struct device_node *np = pdev->dev.of_node;
	struct snd_soc_card *card = &rt5350_es9038;
	int ret;

	card->dev = &pdev->dev;

	rt5350_es9038_dai.cpu_of_node = of_parse_phandle(np, "cpu-dai", 0);
	rt5350_es9038_dai.codec_of_node = of_parse_phandle(np, "codec-dai", 0);
	rt5350_es9038_dai.platform_of_node = rt5350_es9038_dai.cpu_of_node;

	ret = snd_soc_register_card(card);
	if (ret) {
		dev_err(&pdev->dev, "snd_soc_register_card() failed: %d\n",
			ret);
	}
	return ret;
}

static int rt5350_es9038_remove(struct platform_device *pdev)
{
	struct snd_soc_card *card = platform_get_drvdata(pdev);

	snd_soc_unregister_card(card);
	return 0;
}

static const struct of_device_id rt5350_audio_match[] = {
	{ .compatible = "ralink,es9038-audio" },
	{},
};
MODULE_DEVICE_TABLE(of, rt5350_audio_match);

static struct platform_driver rt5350_es9038_driver = {
	.driver		= {
		.name	= "es9038-audio",
		.owner	= THIS_MODULE,
		.of_match_table = rt5350_audio_match,
	},
	.probe		= rt5350_es9038_probe,
	.remove		= rt5350_es9038_remove,
};

module_platform_driver(rt5350_es9038_driver);

MODULE_AUTHOR("Derek Quan <qdk0901@qq.com>");
MODULE_DESCRIPTION("ALSA SoC Sxx Audio support");
MODULE_LICENSE("GPL v2");